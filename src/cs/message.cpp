#include "message.hpp"
namespace cs
{
namespace message
{


#define SC(X) static_cast<size_t>(X)
typedef std::array<const char*, SC(MType::MAX)> mtype_str_table_t;

namespace
{
mtype_str_table_t mtype_str_table_init()
{
    mtype_str_table_t res;
    res[SC(MType::UNKNOWN)] = "unknown";
    res[SC(MType::EMPTY)] = "empty";
    res[SC(MType::PING)] = "ping";
    res[SC(MType::GREETING)] = "greeting";
    res[SC(MType::START)] = "start";
    res[SC(MType::CANNOT_START)] = "cannot_start";
    res[SC(MType::STARTTLS)] = "starttls";
    res[SC(MType::IDENTITY)] = "identity";
    res[SC(MType::KEYS)] = "keys";
    res[SC(MType::KEYS_ACKNOWLEDGMENT)] = "keys_acknowledgment";
    res[SC(MType::MANIFEST)] = "manifest";
    res[SC(MType::GET_MANIFEST)] = "get_manifest";
    res[SC(MType::GET)] = "get";
    res[SC(MType::FILE_DATA)] = "file_data";
    res[SC(MType::UPDATE)] = "update";
    res[SC(MType::MOVE)] = "move";
    return res;
}
} // end anon ns

std::string mtype_to_string(MType type)
{
    static const mtype_str_table_t mtype_str_table = mtype_str_table_init();
    return mtype_str_table[SC(type)];
}
#undef SC

MType mtype_from_string(const std::string& type)
{
    if (type == "ping")
        return MType::PING;

    if (type == "greeting")
        return MType::GREETING;

    if (type == "start")
        return MType::START;

    if (type == "cannot_start")
        return MType::CANNOT_START;

    if (type == "starttls")
        return MType::STARTTLS;

    if (type == "identity")
        return MType::IDENTITY;

    if (type == "keys")
        return MType::KEYS;

    if (type == "keys_acknowledgment")
        return MType::KEYS;

    if (type == "manifest")
        return MType::MANIFEST;

    if (type == "get_manifest")
        return MType::GET_MANIFEST;

    if (type == "manifest_current")
        return MType::MANIFEST_CURRENT;

    if (type == "get")
        return MType::GET;

    if (type == "file_data")
        return MType::FILE_DATA;

    if (type == "update")
        return MType::UPDATE;

    if (type == "move")
        return MType::MOVE;

    return MType::UNKNOWN;
}



} // end ns
} // end ns
