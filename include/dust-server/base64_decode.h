#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/remove_whitespace.hpp>

#include <iostream>
#include <string>

namespace dust_server {
  std::string decode_base64(std::string base64) {
    using namespace boost::archive::iterators;

    typedef transform_width< binary_from_base64<remove_whitespace<std::string::const_iterator> >, 8, 6 > it_binary_t;

    unsigned int paddChars = count(base64.begin(), base64.end(), '=');
    std::replace(base64.begin(), base64.end(),'=','A'); // replace '=' by base64 encoding of '\0'
    std::string result(it_binary_t(base64.begin()), it_binary_t(base64.end())); // decode
    result.erase(result.end()-paddChars, result.end());  // erase padding '\0' characters
    return result;
  }
}
