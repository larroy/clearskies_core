{
    "includes": [
        "../common.gypi",
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
                "share.cpp",
                "utils.cpp",
            ],
            "include_dirs": [
                "../src",
                "../vendor",
            ],
            "link_settings": {
                "libraries": [
                    "-lboost_unit_test_framework",
                    "-lsqlite3",
                    "-lboost_system",
                    "-lboost_filesystem",
                ],
            },
        },
    ],
}
