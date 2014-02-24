{
    "target_defaults": {
        "default_configuration": "Debug",
        "defines": [
        ],

        "configurations": {
            "Debug": {
                "cflags": [
                    "-g",
                    "-O0",
                    "-Wno-deprecated",
                    "-DDEBUG",
                    "-Wall",
                    "-Werror",
                    "-fmessage-length=0",
                ],
                "cflags_cc": [
                    "-std=c++11",
                    "-Wnon-virtual-dtor",
                    "-D_GLIBCXX_DEBUG",
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
                    "-Wno-deprecated",
                    "-Wall",
                    "-Werror",
                    "-fmessage-length=0",
                ],
                "cflags_cc": [
                    "-std=c++11",
                    "-Wnon-virtual-dtor",
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
