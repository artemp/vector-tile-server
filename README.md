vector-tile-server
==================

Vector tile server and rendering backend for Mapnik


## Dependencies

* mapnik
* node-mapnik
* google-protobuf


## Building

1) Get mapnik master from github, at least https://github.com/mapnik/mapnik/commit/7ded35ef94a792ff2313e2087a5684c938fc1497

3) Build mapnik and install protobuf

Install Mapnik like:

    git clone git://github.com/mapnik/mapnik.git
    cd mapnik && ./configure && make && make install

On ubuntu get protobuf:

    sudo apt-get install libprotobuf7 libprotobuf-dev protobuf-compiler
    
2) Then within this directory do:

    protoc -Iproto/ --cpp_out=src/ proto/TileData.proto
    npm install


If you get an error like `cannot run in wd` then try:

   npm install node-gyp -g
   node-gyp configure build

### Debug version

To build a debug version do:

    node-gyp configure build -v -d

Then edit the file `lib/vector_server.js` so that the Debug modules is used.


## Running server

```bash
export LD_PRELOAD=./node_modules/mapnik/lib/_mapnik.node
node ./server.js osm_vectors.xml 8000
```


## Troubleshooting

### symbol conflicts

If you hit an error like:

```
libprotobuf ERROR google/protobuf/descriptor_database.cc:109] Symbol name "google.protobuf.span" conflicts with the existing symbol "google.protobuf.span".
libprotobuf FATAL google/protobuf/descriptor.cc:862] CHECK failed: generated_database_->Add(encoded_file_descriptor, size):
```

This indicates that libprotobuf has been linked twice. See:

 - http://code.google.com/p/protobuf/issues/detail?id=128
 - http://code.google.com/p/protobuf/issues/detail?id=370

A crash like below may also indicate this problem:

```
Thread 2 Crashed:
0   libsystem_kernel.dylib        	0x00007fff8926ece2 __pthread_kill + 10
1   libsystem_c.dylib             	0x00007fff86f9a7d2 pthread_kill + 95
2   libsystem_c.dylib             	0x00007fff86f8ba7a abort + 143
3   libsystem_c.dylib             	0x00007fff86fea84c free + 389
4   libprotobuf.7.dylib           	0x0000000105ea3d19 google::protobuf::RepeatedField<unsigned int>::Add(unsigned int const&) + 33
5   node_vector_server.node       	0x000000010d1ac90b mapnik::opensciencemap_backend_pbf::output_vector_tile() + 857 (opensciencemap_backend_pbf.hpp:238)
6   node_vector_server.node       	0x000000010d1ac240 async_render(uv_work_s*) + 273 (vector_server.cpp:103)
```

### symbols not found

You will notice if you try to require just the vector_server
node module you will get an odd symbol error:

```sh
$ node
> require('./lib/vector_server');
Error: dlopen(/Users/dane/projects/vector-tile-server/build/Release/node_vector_server.node, 1): Symbol not found: __ZN3Map11constructorE
  Referenced from: /Users/dane/projects/vector-tile-server/build/Release/node_vector_server.node
  Expected in: dynamic lookup

    at Object.Module._extensions..node (module.js:485:11)
    at Module.load (module.js:356:32)
    at Function.Module._load (module.js:312:12)
    at Module.require (module.js:362:17)
    at require (module.js:378:17)
    at Object.<anonymous> (/Users/dane/projects/vector-tile-server/lib/vector_server.js:1:103)
    at Module._compile (module.js:449:26)
    at Object.Module._extensions..js (module.js:467:10)
    at Module.load (module.js:356:32)
    at Function.Module._load (module.js:312:12)
```

The reason for this is that the node-mapnik module symbols need to be available for
the `node_vector_server.node` code to initialize properly. So, to solve this make sure
to first require node-mapnik. This works:

```
$ node
> var mapnik = require('mapnik')
undefined
> require('./lib/vector_server');
{ render: [Function] }
```
