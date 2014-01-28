{
    "target_defaults": {
        "default_configuration": "Release",
        "defines": [
        ],

        "configurations": {
            "Debug": {
                "cflags": [
                    "-g",
                    "-O0",
                    '-std=c++11',
                    '-Wnon-virtual-dtor',
                    '-Wno-deprecated',
                    '-D_GLIBCXX_DEBUG',
                ],
                "xcode_settings": {
                    "OTHER_CFLAGS": [
                        "-g",
                        "-O0"
                    ],
                    "OTHER_CPLUSPLUSFLAGS": [
                        "-g",
                        "-O0"
                    ]
                }
            },
            "Release": {
                "cflags": [
                    "-O3",
                    '-std=c++11',
                    '-Wnon-virtual-dtor',
                    '-Wno-deprecated',
                ],
                "defines": [
                    "NDEBUG"
                ],
                "xcode_settings": {
                    "OTHER_CFLAGS": [
                        "-O3"
                    ],
                    "OTHER_CPLUSPLUSFLAGS": [
                        "-O3",
                        "-DNDEBUG"
                    ]
                }
            }
        }
    }
}
