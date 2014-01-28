{                                                                                     
    "includes": [
        '../../common.gypi',
    ],
    "targets":                                                                        
    [                                                                                 
        {                                                                             
            "target_name": "cs",                                                     
            "type": "static_library",
            "sources": [                                                              
                "int_types.h",
                "utils.hpp",
                "message.cpp", 
                "message.hpp", 
                "protocolstate.hpp",
                "protocolstate.cpp",
                "clearskiesprotocol.hpp",
                "clearskiesprotocol.cpp",
            ],                                                                        
            "include_dirs": [
                "../",
            ],
        },                                                                            
    ],                                                                                
}                                                                                     
