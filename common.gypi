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
                    "-std=c++11",
                    "-Wnon-virtual-dtor",
                    "-Wno-deprecated",
                    "-D_GLIBCXX_DEBUG",
                    "-DDEBUG",
                    "-Wall",
                    "-Werror",
                    "-fmessage-length=0",
                ],
                "xcode_settings": {
                    "CLANG_CXX_LANGUAGE_STANDARD": "c++11",
                    "CLANG_CXX_LIBRARY": "libc++",
                    "OTHER_CFLAGS": [
                        "-g",
                        "-O0",
                    ],
                    "OTHER_CPLUSPLUSFLAGS": [
                        "-fcolor-diagnostics",
                        "-g",
                        "-O0",
                        "-Wall",
                        "-Werror",
                    ],
                },
            },
            "Release": {
                "cflags": [
                    "-O3",
                    "-std=c++11",
                    "-Wnon-virtual-dtor",
                    "-Wno-deprecated",
                    "-Wall",
                    "-Werror",
                    "-fmessage-length=0",
                ],
                "defines": [
                    "NDEBUG",
                ],
                "xcode_settings": {
                    "CLANG_CXX_LANGUAGE_STANDARD": "c++11",
                    "CLANG_CXX_LIBRARY": "libc++",
                    "OTHER_CFLAGS": [
                        "-O3",
                    ],
                    "OTHER_CPLUSPLUSFLAGS": [
                        "-fcolor-diagnostics",
                        "-O3",
                        "-Wall",
                        "-Werror",
                    ],
                },
            },
        },
    },
}
