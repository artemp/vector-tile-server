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
 
+        // FIXME: make user config
+        layer_descriptor lay_desc = ds->get_descriptor();
+        BOOST_FOREACH(attribute_descriptor const& desc, lay_desc.get_descriptors())
+        {
+            q.add_property_name(desc.get_name());
+        }
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
   node-gyp -v configure build


## Running server

```bash
node ./server.js osm_vectors.xml 8000
```
