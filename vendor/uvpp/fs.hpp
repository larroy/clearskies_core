#pragma once
#include <sstream>

/// Formatted string, allows to use stream operators and returns a std::string with the resulting format
#define fs(x) (static_cast<std::ostringstream&>(std::ostringstream{} << x).str())

/// Format error
#define fe(x) \
   (static_cast<const std::ostringstream&>(((*std::make_unique<std::ostringstream>().get()) << "ERROR: "<< __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ <<  x)).str ())


