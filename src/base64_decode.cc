// Copyright (c) 2014, makielski.net
// Licensed under the MIT license
// https://raw.github.com/makielski/botscript/master/COPYING

#include "dust-server/base64_decode.h"

#include <iostream>
#include <algorithm>

#include "boost/archive/iterators/base64_from_binary.hpp"
#include "boost/archive/iterators/binary_from_base64.hpp"
#include "boost/archive/iterators/transform_width.hpp"
#include "boost/archive/iterators/remove_whitespace.hpp"

using namespace boost::archive::iterators;

typedef transform_width<
    binary_from_base64<remove_whitespace<std::string::const_iterator>>, 8, 6>
it_binary_t;

namespace dust_server {

// From http://stackoverflow.com/a/10973348
std::string decode_base64(std::string base64) {
  unsigned int padding = count(base64.begin(), base64.end(), '=');

  // replace '=' by base64 encoding of '\0'
  std::replace(base64.begin(), base64.end(), '=', 'A');

  // decode
  std::string result(it_binary_t(base64.begin()), it_binary_t(base64.end()));

  // erase padding '\0' characters
  result.erase(result.end() - padding, result.end());

  return result;
}

}  // namespace dust_server
