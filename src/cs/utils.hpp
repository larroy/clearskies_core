#pragma once
#include <sstream>


namespace std
{

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

} // end ns


/// Formatted string, allows to use stream operators and returns a std::string with the resulting format
#define fs(x) \
   (static_cast<const std::ostringstream&>(((*std::make_unique<std::ostringstream>().get()) << x)).str ())


/// Format error
#define fe(x) \
   (static_cast<const std::ostringstream&>(((*std::make_unique<std::ostringstream>().get()) << "ERROR: "<< __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ <<  x)).str ())


