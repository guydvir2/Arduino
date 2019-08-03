// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2019
// MIT License

#pragma once

#include <stdint.h>
#include <stdlib.h>  // for size_t
#include "../Configuration.hpp"
#include "../Polyfills/math.hpp"

<<<<<<< HEAD
namespace ArduinoJson {
namespace Internals {
=======
namespace ARDUINOJSON_NAMESPACE {
>>>>>>> 71a09487d18c8b01d889cc54dde62335db4d16d6

template <typename T, typename F>
struct alias_cast_t {
  union {
    F raw;
    T data;
  };
};

template <typename T, typename F>
T alias_cast(F raw_data) {
  alias_cast_t<T, F> ac;
  ac.raw = raw_data;
  return ac.data;
}
<<<<<<< HEAD

}  // namespace Internals
}  // namespace ArduinoJson
=======
}  // namespace ARDUINOJSON_NAMESPACE
>>>>>>> 71a09487d18c8b01d889cc54dde62335db4d16d6
