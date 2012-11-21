{
  'targets': [
    {
      'target_name': 'node_vector_server',
      'sources': [
        'src/vector_server.cpp',
        'src/tags.cpp',
        'src/vector_renderer.cpp'
      ],
      'dependencies': [
      ],
      'include_dirs': [
          'node_modules/mapnik/src',
          '<!@(mapnik-config --cflags | sed s/-I//g)'
      ],
      'conditions': [
      ['OS=="mac"', {
        'libraries':[
          '-lmapnik',
          '-lprotobuf-lite',
          '-undefined dynamic_lookup'
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
