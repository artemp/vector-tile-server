vector-tile-server
==================

Vector tile server and rendering backend for Mapnik


## Dependencies

* mapnik
* node-mapnik
* google-protobuf


## Building

1) Get mapnik master from github

2) Apply this patch to Mapnik:

```diff
--- a/include/mapnik/feature_style_processor_impl.hpp
+++ b/include/mapnik/feature_style_processor_impl.hpp
@@ -388,6 +388,13 @@ void feature_style_processor<Processor>::apply_to_layer(layer const& lay, Proces
             q.add_property_name(name);
         }
 
+        // FIXME: make user config
+        layer_descriptor lay_desc = ds->get_descriptor();
+        BOOST_FOREACH(attribute_descriptor const& desc, lay_desc.get_descriptors())
+        {
+            q.add_property_name(desc.get_name());
+        }
+
         // Update filter_factor for all enabled raster layers.
         BOOST_FOREACH (feature_type_style * style, active_styles)
         {
```

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
