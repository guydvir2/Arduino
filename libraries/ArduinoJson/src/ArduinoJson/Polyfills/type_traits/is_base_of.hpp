// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2019
// MIT License

#pragma once

namespace ARDUINOJSON_NAMESPACE {

// A meta-function that returns true if Derived inherits from TBase is an
// integral type.
template <typename TBase, typename TDerived>
class is_base_of {
 protected:  // <- to avoid GCC's "all member functions in class are private"
  typedef char Yes[1];
  typedef char No[2];

  static Yes &probe(const TBase *);
  static No &probe(...);

 public:
  static const bool value =
      sizeof(probe(reinterpret_cast<TDerived *>(0))) == sizeof(Yes);
};
<<<<<<< HEAD:libraries/ArduinoJson/src/ArduinoJson/TypeTraits/IsBaseOf.hpp
}  // namespace Internals
}  // namespace ArduinoJson
=======
}  // namespace ARDUINOJSON_NAMESPACE
>>>>>>> 71a09487d18c8b01d889cc54dde62335db4d16d6:libraries/ArduinoJson/src/ArduinoJson/Polyfills/type_traits/is_base_of.hpp
