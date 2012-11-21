#include <node.h>
#include <node_buffer.h>
//#include <node_object_wrap.h>
#include "mapnik_map.hpp"

#include <string>
#include "vector_renderer.hpp"
#include "dummy_backend.hpp"

using namespace v8;

struct render_state
{
    Map * m;
    bool error;
    std::string error_message;
    std::string output;
    Persistent<Function> callback;
};

// fwd decl
void async_render(uv_work_t* req);
void after_render(uv_work_t* req);

Handle<Value> render(Arguments const& args)
{
    HandleScope scope;

    if (args.Length() != 2) {
        return ThrowException(Exception::TypeError(
                                  String::New("requires at least two arguments, a renderable mapnik object, and a callback")));
    }

    if (!Map::constructor->HasInstance(args[0]->ToObject()))
    {
        return ThrowException(Exception::TypeError(
                                  String::New("First argument must be a Map object")));
    }

    if (!args[1]->IsFunction())
    {
        return ThrowException(Exception::TypeError(
                                  String::New("Second argument must be a callback function")));
    }

    Map* m = ObjectWrap::Unwrap<Map>(args[0]->ToObject());
    if (m->active() != 0)
    {
        // FIXME
    }

    Local<Function> callback = Local<Function>::Cast(args[1]);
    render_state * state = new render_state();
    state->m = m;
    state->error = false;
    state->callback = Persistent<Function>::New(callback);
    uv_work_t * req = new uv_work_t();
    req->data = state;
    int status = uv_queue_work(uv_default_loop(),req, async_render, after_render);
    assert(status == 0);
    return Undefined();
}

void async_render(uv_work_t* req)
{
    render_state * state = static_cast<render_state*>(req->data);
    Map * map_ptr = state->m;
    std::string output;
    mapnik::dummy_backend backend(output);
    mapnik::vector_renderer<mapnik::dummy_backend> ren(*map_ptr->get(),backend);
    ren.apply();
    uint32_t bytes = backend.output_vector_tile();
    state->output = output.substr(0,bytes);
}

void after_render(uv_work_t* req)
{
    HandleScope scope;
    render_state * state = static_cast<render_state*>(req->data);

    if (state->error)
    {
        Local<Value> err = Exception::Error(String::New(state->error_message.c_str()));
        const unsigned argc = 1;
        Local<Value> argv[argc] = { err };
        TryCatch try_catch;
        state->callback->Call(Context::GetCurrent()->Global(), argc, argv);
        if (try_catch.HasCaught())
        {
            node::FatalException(try_catch);
        }
    }
    else
    {
        const unsigned argc = 2;

        node::Buffer *buf = Buffer::New((char*)state->output.data(), state->output.size());

        Local<Value> argv[argc] = {
            Local<Value>::New(Null()),
            Local<Value>::New(buf->handle_)
        };

        TryCatch try_catch;
        state->callback->Call(Context::GetCurrent()->Global(), argc, argv);
        if (try_catch.HasCaught())
        {
            node::FatalException(try_catch);
        }
    }

    state->callback.Dispose();
    delete state;
    delete req;
}

void RegisterModule(Handle<Object> target)
{
    target->Set(String::NewSymbol("render"),
                FunctionTemplate::New(render)->GetFunction());
}

NODE_MODULE(node_vector_server, RegisterModule);
