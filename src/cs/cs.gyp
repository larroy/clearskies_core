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
                "protocol.hpp",
                "utils.hpp",
            ],                                                                        
            "include_dirs": [
                "../",
            ],
        },                                                                            
    ],                                                                                
}                                                                                     
