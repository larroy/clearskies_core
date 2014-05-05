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
                "../../vendor/libuv/uv.gyp:libuv",
            ],
            "sources": [
                "int_types.h",
                "daemon/daemon.hpp",
                "daemon/daemon.cpp",
                "core/message.cpp",
                "core/message.hpp",
                "core/coder.cpp",
                "core/coder.hpp",
                "core/protocol.hpp",
                "core/protocol.cpp",
                "core/serverinfo.hpp",
                "core/share.hpp",
                "core/share.cpp",
                "protocolstate.cpp",
                "protocolstate.hpp",
                "utils.hpp",
                "utils.cpp",
                "file.hpp",
                "file.cpp",
                "vclock.hpp",
                "vclock.cpp",
                "conf.hpp",
                "conf.cpp",
                "config.hpp",
                "server.hpp",
                "server.cpp",
            ],
            "include_dirs": [
                "../",
                "../../vendor",
            ],
        },
    ],
}
