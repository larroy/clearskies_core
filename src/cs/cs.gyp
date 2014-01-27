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
                "message.cpp", 
                "message.hpp", 
                "protocolstate.hpp",
                "protocolstate.cpp",
                "utils.hpp",
                "protocolstatecore.hpp",
            ],                                                                        
            "include_dirs": [
                "../",
            ],
        },                                                                            
    ],                                                                                
}                                                                                     
