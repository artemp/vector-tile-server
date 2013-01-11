/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#include "vector_renderer.hpp"
#include "opensciencemap_backend_pbf.hpp"
// not currently used
//#include "opensciencemap_backend.hpp"

// node
#include <node.h>                       // for FatalException, NODE_MODULE
#include <node_buffer.h>                // for Buffer
#include <node_object_wrap.h>           // for ObjectWrap

// node-mapnik
#include "mapnik_map.hpp"

// std
#include <cassert>
#include <exception>

// boost
#include <boost/shared_ptr.hpp>

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

    if (!args[0]->IsObject())
    {
        return ThrowException(Exception::TypeError(
                                  String::New("First argument must be a Map object")));
    }

    Local<Object> obj = args[0]->ToObject();
    if (obj->IsNull() || obj->IsUndefined() || !Map::constructor->HasInstance(obj->ToObject()))
    {
        return ThrowException(Exception::TypeError(
                                  String::New("First argument must be a Map object")));
    }

    if (!args[1]->IsFunction())
    {
        return ThrowException(Exception::TypeError(
                                  String::New("Second argument must be a callback function")));
    }

    Map* m = node::ObjectWrap::Unwrap<Map>(args[0]->ToObject());
    if (m->active() != 0)
    {
        return ThrowException(Exception::TypeError(
                                  String::New("Use a map pool to avoid sharing map objects between concurrent rendering")));
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
    try
    {
        std::string output;
        mapnik::opensciencemap_backend_pbf backend(output);
        mapnik::vector_renderer<mapnik::opensciencemap_backend_pbf> ren(*map_ptr->get(),backend);
        ren.apply();
        uint32_t bytes = backend.output_vector_tile();
        state->output = output.substr(0,bytes);
    }
    catch (std::exception const& ex)
    {
        state->error = true;
        state->error_message = std::string("error occured during rendering: ") + ex.what();
    }
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

        node::Buffer *buf = node::Buffer::New((char*)state->output.data(), state->output.size());

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

NODE_MODULE(node_vector_server, RegisterModule)
