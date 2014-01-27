{
    "includes": [
        '../common.gypi',
    ],
    "targets":
    [
        {
            "target_name": "unit_test",
            "type": "executable",
            "dependencies": [
                "../src/cs/cs.gyp:cs",
            ],
            "sources": [
                "main.cpp",
                "message.cpp",
                "protocolstate.cpp",
            ],
            "include_dirs": [
                "../src",
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

