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
                "../../vendor/sha2/sha2.gyp:sha2",
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
                "utils.cpp",
                "share.hpp",
                "share.cpp",
                "file.hpp",
                "file.cpp",
            ],
            "include_dirs": [
                "../",
                "../../vendor",
            ],
        },
    ],
}
