{
    'target_defaults': {
        'cflags': [
            '-std=c++11',
            '-Wnon-virtual-dtor',
            '-Wno-deprecated'
        ],
    },
    "targets":
    [
        {
            "target_name": "unit_test",
            "type": "executable",
            "sources": [
                "main.cpp",
                "message.cpp",
            ],
            'link_settings': {
              'libraries': [
                '-lboost_unit_test_framework',
              ],
#              'library_dirs': [
#                '/usr/lib',
#              ],
            },
        },
    ],
}

