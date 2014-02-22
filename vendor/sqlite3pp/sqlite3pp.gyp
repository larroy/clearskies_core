{
    "includes": [
        '../../common.gypi',
    ],
    "targets":
    [
        {
            "target_name": "sqlite3pp",
            "type": "static_library",
            "sources": [
                "sqlite3pp.cpp",
                "sqlite3pp.hpp",
                "sqlite3ppext.cpp",
                "sqlite3ppext.hpp",
            ]
        },
    ],
}

