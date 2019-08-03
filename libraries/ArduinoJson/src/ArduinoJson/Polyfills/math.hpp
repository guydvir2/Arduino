// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2019
// MIT License

#pragma once

namespace ARDUINOJSON_NAMESPACE {

// Some libraries #define isnan() and isinf() so we need to check before
// using this name

#ifndef isnan
template <typename T>
bool isnan(T x) {
  return x != x;
}
#endif

#ifndef isinf
template <typename T>
bool isinf(T x) {
  return x != 0.0 && x * 2 == x;
}
<<<<<<< HEAD
}  // namespace Internals
}  // namespace ArduinoJson
=======
#endif
}  // namespace ARDUINOJSON_NAMESPACE
>>>>>>> 71a09487d18c8b01d889cc54dde62335db4d16d6
