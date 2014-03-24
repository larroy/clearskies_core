#pragma once
#include <sstream>

/// Formatted string, allows to use stream operators and returns a std::string with the resulting format
#define fs(x) ((std::ostringstream{} << x).str())

/// Format error
#define fe(x) \
  ((std::ostringstream() << "ERROR: "<< __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ <<  x).str ())
