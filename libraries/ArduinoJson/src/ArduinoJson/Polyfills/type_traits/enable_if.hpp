// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2019
// MIT License

#pragma once

namespace ARDUINOJSON_NAMESPACE {

// A meta-function that return the type T if Condition is true.
template <bool Condition, typename T = void>
struct enable_if {};

template <typename T>
struct enable_if<true, T> {
  typedef T type;
};
<<<<<<< HEAD:libraries/ArduinoJson/src/ArduinoJson/TypeTraits/EnableIf.hpp
}  // namespace Internals
}  // namespace ArduinoJson
=======
}  // namespace ARDUINOJSON_NAMESPACE
>>>>>>> 71a09487d18c8b01d889cc54dde62335db4d16d6:libraries/ArduinoJson/src/ArduinoJson/Polyfills/type_traits/enable_if.hpp
