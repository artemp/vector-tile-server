{
  'target_defaults': {
      'default_configuration': 'Release',
      'configurations': {
          'Debug': {
              'cflags_cc!': ['-O3', '-DNDEBUG']
          },
          'Release': {
             # nothing needed, use defaults
          }
      },
      'include_dirs': [
          'node_modules/mapnik/src',
      ],
      'conditions': [
        ['OS=="linux" or OS=="freebsd" or OS=="openbsd" or OS=="netbsd" or OS=="mac"', {
          'cflags_cc!': ['-fno-rtti', '-fno-exceptions'],
          'cflags_cc' : ['<!@(mapnik-config --cflags)'],
          'libraries':[
            '-lmapnik',
            '-lprotobuf-lite'
          ]
        }],
        ['OS=="linux"', {
          'libraries':[
            '-licuuc',
            '-lboost_regex',
            # if the above are not enough, link all libs
            # mapnik uses by uncommenting the next line
            #'<!@(mapnik-config --ldflags --dep-libs)'
          ]
        }],
      ],
      'sources': [
        './src/tags.cpp',
        './src/vector_renderer.cpp',
        './src/TileData.pb.cc'
      ],
  },
  'targets': [
    {
      'target_name': 'node_vector_server',
      'sources': [
        './src/vector_server.cpp',
      ],
      # this has to be per target to correctly
      # override node-gyp defaults
      'xcode_settings': {
        'OTHER_CPLUSPLUSFLAGS':[
           '<!@(mapnik-config --cflags)'
        ],
        'GCC_ENABLE_CPP_RTTI': 'YES',
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
      }
    },
    {
      'target_name': 'vector-tile-server',
      'type': 'executable',
      'sources': [
        './src/main.cpp',
      ],
      # this has to be per target to correctly
      # override node-gyp defaults
      'xcode_settings': {
        'OTHER_CPLUSPLUSFLAGS':[
           '<!@(mapnik-config --cflags)'
        ],
        'GCC_ENABLE_CPP_RTTI': 'YES',
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
      }
    },
  ]
}