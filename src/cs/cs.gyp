{
    "includes": [
        '../../common.gypi',
    ],
    "targets":
    [
        {
            "target_name": "cs",
            "type": "static_library",
            "dependencies": [
                "../../vendor/sqlite3pp/sqlite3pp.gyp:sqlite3pp",
            ],
            "sources": [
                "int_types.h",
                "clearskiesprotocol.cpp",
                "clearskiesprotocol.hpp",
                "message.cpp",
                "message.hpp",
                "messagecoder.cpp",
                "messagecoder.hpp",
                "protocolstate.cpp",
                "protocolstate.hpp",
                "protocolstatecore.hpp",
                "utils.hpp",
            ],
            "include_dirs": [
                "../",
            ],
        },
    ],
}
