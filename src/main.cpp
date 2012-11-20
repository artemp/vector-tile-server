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

// mapnik
//#include <mapnik/vector_renderer_impl.hpp>
//#include <mapnik/feature_style_processor_impl.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include "vector_renderer.hpp"
#include "dummy_backend.hpp"

#include <iostream>
#include <fstream>
#include <mapnik/config.hpp>

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <mapfile>" << std::endl;
        return EXIT_FAILURE;
    }

    mapnik::datasource_cache::instance().register_datasources("/opt/mapnik/lib/mapnik/input/");
    mapnik::freetype_engine::register_fonts("/opt/mapnik/lib/mapnik/fonts/");

    std::cerr << "vector tile server" << std::endl;
    std::string mapfile(argv[1]);

    mapnik::Map m(256,256); // tile map
    try
    {
        std::cerr << "loading " << mapfile << "..."  << std::endl;
        mapnik::load_map(m, mapfile);
        //m.zoom_to_box(mapnik::box2d<double>(-8024477.28459,5445190.38849,-7381388.20071,5662941.44855));
        //m.zoom_to_box(mapnik::box2d<double>(-141233.598318,6756305.73374,-140782.086463,6756728.00478));

        //mapnik::box2d<double> bbox(-141233.598318,6755505.73374,-140782.086463,6756728.00478);
        //mapnik::box2d<double> bbox(-141233.598318 + 1000,6755505.73374 + 1000,-140782.086463+1000,6756728.00478+1000);
        mapnik::box2d<double> bbox(-141867.12449728712,6760702.277767269,-141255.62827100573,6761313.773993552);
        m.zoom_to_box(mapnik::box2d<double>(bbox));

        //m.zoom_to_box(mapnik::box2d<double>(-144715.338031,6746062.31756,-133583.388952,6764043.36075));
    }
    catch (...)
    {
        std::cerr << "Opps.." << std::endl;
        return EXIT_FAILURE;
    }

    std::string output;
    mapnik::dummy_backend backend(output);
    mapnik::vector_renderer<mapnik::dummy_backend> ren(m, backend);
    ren.apply();

    std::cerr << "TILE TAGS SIZE=" << backend.tags().size() << std::endl;
    std::cerr << "TILE ELEM SIZE=" << backend.tile_elements().size() << std::endl;

    uint32_t bytes = backend.output_vector_tile();
    std::string trimmed = output.substr(0,bytes);
    char head[4];
    head[0] = (bytes >> 24) & 0xff;
    head[1] = (bytes >> 16) & 0xff;
    head[2] = (bytes >> 8) & 0xff;
    head[3] = bytes & 0xff;

    std::ofstream file("test.osmtile");
    if (file)
    {
        file.write(head,4);
        file.write(trimmed.c_str(), trimmed.size());
    }
    file.flush();

    return EXIT_SUCCESS;
}
