// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2019
// MIT License

#pragma once

namespace ARDUINOJSON_NAMESPACE {

// A meta-function that return the type T without the const modifier
template <typename T>
struct remove_const {
  typedef T type;
};
template <typename T>
struct remove_const<const T> {
  typedef T type;
};
<<<<<<< HEAD:libraries/ArduinoJson/src/ArduinoJson/TypeTraits/RemoveConst.hpp
}  // namespace Internals
}  // namespace ArduinoJson
=======
}  // namespace ARDUINOJSON_NAMESPACE
>>>>>>> 71a09487d18c8b01d889cc54dde62335db4d16d6:libraries/ArduinoJson/src/ArduinoJson/Polyfills/type_traits/remove_const.hpp
