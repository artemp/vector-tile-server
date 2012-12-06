{
  'targets': [
    {
      'target_name': 'node_vector_server',
      'sources': [
        './src/vector_server.cpp',
        './src/tags.cpp',
        './src/vector_renderer.cpp',
        './src/TileData.pb.cc'
      ],
      'dependencies': [
      ],
      'include_dirs': [
          'node_modules/mapnik/src',
          '<!@(mapnik-config --cflags | sed s/-I//g)'
      ],
      'conditions': [
        ['OS=="linux" or OS=="freebsd" or OS=="openbsd" or OS=="netbsd" or OS=="mac"', {
          'cflags_cc!': ['-fno-rtti', '-fno-exceptions'],
          'libraries':[
            '-lmapnik',
            '-lprotobuf'
          ],
           'xcode_settings': {
           'GCC_ENABLE_CPP_RTTI': 'YES',
           'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
           }
           }]
      ]
    },
    {
      'target_name': 'vector-tile-server',
      'type': 'executable',
      'sources': [
        './src/main.cpp',
        './src/vector_renderer.cpp',
        './src/tags.cpp',
        './src/TileData.pb.cc'
      ],
      'include_dirs': [
          'node_modules/mapnik/src',
          '<!@(mapnik-config --cflags | sed s/-I//g)'
      ],
      'conditions': [
        ['OS=="linux" or OS=="freebsd" or OS=="openbsd" or OS=="netbsd" or OS=="mac"', {
          'cflags_cc!': ['-fno-rtti', '-fno-exceptions'],
          'libraries':[
            '-lmapnik',
            '-lprotobuf'
          ],
           'xcode_settings': {
           'GCC_ENABLE_CPP_RTTI': 'YES',
           'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
           }
           }]
      ]
    }

  ]
}