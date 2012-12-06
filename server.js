// vector tile server

var fs = require('fs'); // todo : remove
var mapnik = require('mapnik');
var mercator = require('./utils/sphericalmercator');
var mappool = require('./utils//pool.js');
var http = require('http');
var parseXYZ = require('./utils/tile.js').parseXYZ;
var vector_ren = require('./lib/vector_server');


var TMS_SCHEME = false;

// create a pool of 10 maps to manage concurrency under load
var maps = mappool.create_pool(10);

var usage = 'usage: server.js <stylesheet> <port>';

var stylesheet = process.argv[2];

if (!stylesheet) {
    console.log(usage);
    process.exit(1);
}

var port = process.argv[3];

if (!port) {
    console.log(usage);
    process.exit(1);
}

var aquire = function(id,options,callback) {
    methods = {
        create: function(cb) {
            var obj = new mapnik.Map(options.width || 256, options.height || 256);
            obj.load(id, {strict: true},function(err,obj) {
                if (options.bufferSize) {
                    obj.bufferSize = options.bufferSize;
                }
                cb(err,obj);
            });
        },
        destroy: function(obj) {
            delete obj;
        }
    };
    maps.acquire(id,methods,function(err,obj) {
        callback(err, obj);
    });
};


http.createServer(function(req, res) {
    parseXYZ(req, TMS_SCHEME, function(err,params) {
        if (err) {
            res.writeHead(500, {
                'Content-Type': 'text/plain'
            });
            res.end(err.message);
        }
        else
        {
            var bbox = mercator.xyz_to_envelope(params.x, params.y, params.z, TMS_SCHEME);
            console.log('BBOX %s', bbox);
            aquire(stylesheet, {}, function(err, map) {
                if (err)
                {
                    process.nextTick(function() {
                        maps.release(stylesheet, map);
                    });
                    res.writeHead(500, {
                        'Content-Type': 'text/plain'
                    });
                    res.end(err.message);
                }
                else
                {
                    // bbox for x,y,z
                    map.extent = bbox;
                    vector_ren.render(map, function(err, output) {
                        process.nextTick(function() {
                            maps.release(stylesheet, map);
                        });
                        if (err)
                        {
                            res.writeHead(500, {
                                'Content-Type': 'text/plain'
                            });
                            res.end(err.message);
                        }
                        else
                        {
                            var content_length = output.length + 4;
                            var head = new Buffer(4);
                            head[0] = (output.length >> 24) & 0xff;
                            head[1] = (output.length >> 16) & 0xff;
                            head[2] = (output.length >>  8) & 0xff;
                            head[3] = output.length & 0xff;
                            console.log("TILE(%d/%d/%d) OUTPUT len=%d", params.z, params.x, params.y, output.length);
                            res.useChunkedEncodingByDefault=false;
                            res.chunkedEncoding=false;
                            res.writeHead(200,{'Content-length': content_length,
                                               'Content-type': 'application/osmtile'
                                              });
                            res.write(head);
                            res.end(output);

                        }
                    });
                }
            });
        }
    });
}).listen(port);


console.log('Vector tile server listening on port %d', port);
