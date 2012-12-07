// vector tile server

var fs = require('fs'); // todo : remove
var mapnik = require('mapnik');
var mercator = new(require('sphericalmercator'));
var pool = require('generic-pool').Pool;
var http = require('http');
var url = require("url");
var vector_ren = require('./lib/vector_server');
var eio = require('eio');
var path = require('path');
var exists = require('fs').exists || require('path').exists;
var mime = require('mime');

// Increase number of threads to 1.5x the number of logical CPUs.
var threads = Math.ceil(Math.max(4, require('os').cpus().length * 1.5));
eio.setMinParallel(threads);

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

var map_pool = pool({
    create: function(callback) {
        var obj = new mapnik.Map(256, 256);
        var opts = { strict: false };
        obj.load(stylesheet, opts, function(err,obj) {
            if (err) return callback(err);
            return callback(null,obj);
        });
    },
    destroy: function(obj) {
        delete obj;
    },
    max: require('os').cpus().length
});

var parse_url = function(req, callback) {
    var matches = req.url.match(/(\d+)/g);
    if (matches && matches.length == 3) {
        var format = path.extname(req.url);
        if (!format || !(format.indexOf('png') > -1 || format == '.osmtile')) {
            var msg = "Invalid format, only 'png' and 'osmtile' are supported";
            msg += ' (expected a url like /0/0/0.png or /0/0/0.osmtile';
            msg += ' but got: ' + req.url + ')';
            return callback(new Error(msg));
        }
        try {
            var x = parseInt(matches[1], 10);
            var y = parseInt(matches[2], 10);
            var z = parseInt(matches[0], 10);
            return callback(null,
               { z: z,
                 x: x,
                 y: y,
                 format: format.slice(1)
               });
        } catch (err) {
            return callback(err, null);
        }
    }
    return callback(new Error('not a tile'),null);
}

var error = function(res,msg) {
    res.writeHead(500, {
        'Content-Type': 'text/plain'
    });
    return res.end(msg);
}

var renderer = function(map, params, res) {
    var bbox = mercator.bbox(params.x, params.y, params.z, false, '900913');
    map.extent = bbox;
    console.log(bbox);
    if (params.format == 'osmtile') {
        vector_ren.render(map, function(err, output) {
            process.nextTick(function() {
                map_pool.release(map);
            });
            if (err) {
                return error(res,err.message)
            }
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
            return res.end(output);
        });
    } else {
        var im = new mapnik.Image(map.width,map.height);
        map.render(im,function(err,im) {
            process.nextTick(function() {
                map_pool.release(map);
            });
            if (err) {
                return error(res,err.message)
            }
            im.encode(params.format,function(err,buffer) {
                if (err) {
                    return error(res,err.message)
                }
                res.writeHead(200, {'Content-Type': 'image/png'});
                return res.end(buffer);
            });
        });
    }
};

var root = "./www";

http.createServer(function(req, res) {
    var uri = url.parse(req.url).pathname;
    if (!uri || uri == '/') {
        res.writeHead(200, {'Content-Type': 'text/html'});
        return res.end(fs.readFileSync(path.join(root,'index.html')));
    }
    var filepath = path.join(root, uri);
    exists(filepath, function(exist) {
        if (exist) {
            res.writeHead(200, {'Content-Type': mime.lookup(filepath)});
            return res.end(fs.readFileSync(filepath));
        }
        parse_url(req, function(err,params) {
            if (err) {
                return error(res,err.message);
            }
            map_pool.acquire(function(err, map) {
                if (err) {
                    return error(res,err.message)
                }
                return renderer(map,params,res);
            });
        });
    });
}).listen(port);

console.log('Vector tile server listening on port %d', port);
