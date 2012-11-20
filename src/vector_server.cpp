#include <node.h>
#include <node_buffer.h>
//#include <node_object_wrap.h>
#include "mapnik_map.hpp"

#include <string>
#include "vector_renderer.hpp"
#include "dummy_backend.hpp"

v8::Handle<v8::Value> render(v8::Arguments const& args)
{
    v8::HandleScope scope;
    v8::Local<v8::Value> val = args[0];
    v8::Local<v8::Object> obj = val->ToObject();
    if (Map::constructor->HasInstance(obj))
    {
	Map * map_ptr = node::ObjectWrap::Unwrap<Map>(obj);
	std::string output;
	mapnik::dummy_backend backend(output);
	mapnik::vector_renderer<mapnik::dummy_backend> ren(*map_ptr->get(),backend);
	ren.apply();
	uint32_t bytes = backend.output_vector_tile();
	std::string trimmed = output.substr(0,bytes);
	node::Buffer *buf = Buffer::New((char*)trimmed.data(),trimmed.size());
	return scope.Close(buf->handle_);
    }
    else
    {
	return v8::Undefined();
    }
}

void RegisterModule(v8::Handle<v8::Object> target)
{
    target->Set(v8::String::NewSymbol("render"),
		v8::FunctionTemplate::New(render)->GetFunction());
}

NODE_MODULE(node_vector_server, RegisterModule);
