#include <iostream>

#include <mapnik/config.hpp>
#include <mapnik/vector_renderer.hpp>
#include <mapnik/vector_renderer_impl.hpp>
#include <mapnik/feature_style_processor_impl.hpp>
#include <mapnik/load_map.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/util/geometry_to_wkb.hpp>
//boost
#include <boost/algorithm/string/trim.hpp>
// proto
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

namespace mapnik
{
struct dummy_backend
{
    template <typename T>
    void add_path(T & path)
    {
        mapnik::vertex2d vtx(mapnik::vertex2d::no_init);
        path.rewind(0);
        std::cout << "<Path>" << std::endl;
        std::string output;
        google::protobuf::io::ZeroCopyOutputStream* raw_output = new google::protobuf::io::StringOutputStream(&output);
        google::protobuf::io::CodedOutputStream* coded_output = new google::protobuf::io::CodedOutputStream(raw_output);
        unsigned count = 0;
        uint32_t x=0,y=0;
        while ((vtx.cmd = path.vertex(&vtx.x, &vtx.y)) != mapnik::SEG_END)
        {
            std::cout << vtx.x << "," << vtx.y << " cmd=" << vtx.cmd << std::endl;

            if (count > 0 && vtx.cmd == mapnik::SEG_LINETO &&
                std::fabs(static_cast<uint32_t>(vtx.x) - x) < 1.0 &&
                std::fabs(static_cast<uint32_t>(vtx.y) - y) < 1.0)
            {
                continue;
            }
            x = static_cast<uint32_t>(vtx.x);
            y = static_cast<uint32_t>(vtx.y);
            coded_output->WriteVarint32(x);
            coded_output->WriteVarint32(y);
            coded_output->WriteTag(vtx.cmd);
            ++count;
        }

        std::cout << "count = " << count << std::endl;
        std::cout << "size = " << (count*(8 + 8 + 4)) << " new_size=" << output.size() << std::endl;
        std::string hex = mapnik::util::to_hex(output.data(),output.size());
        boost::algorithm::trim_right_if(hex,is_any_of("0"));
        std::cout << hex << std::endl;
        std::cout << "</Path>" << std::endl;

        google::protobuf::io::ZeroCopyInputStream* raw_input = new google::protobuf::io::ArrayInputStream(output.c_str(),output.size());
        google::protobuf::io::CodedInputStream * coded_input = new google::protobuf::io::CodedInputStream(raw_input);
        for (int i = 0; i < count ; ++i)
        {
            coded_input->ReadVarint32(&x);
            coded_input->ReadVarint32(&y);
            uint32_t cmd = coded_input->ReadTag();
            std::cout << x << "," << y << " cmd=" << cmd << std::endl;
        }
        delete coded_output;
        delete raw_output;
    }

};


template class feature_style_processor<vector_renderer<dummy_backend> >;
template class vector_renderer<dummy_backend>;

}

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
        m.zoom_to_box(mapnik::box2d<double>(-8024477.28459,5445190.38849,-7381388.20071,5662941.44855));
    }
    catch (...)
    {
        std::cerr << "Opps.." << std::endl;
        return EXIT_FAILURE;
    }

    mapnik::dummy_backend backend;
    mapnik::vector_renderer<mapnik::dummy_backend> ren(m, backend);
    ren.apply();
    return EXIT_SUCCESS;
}
