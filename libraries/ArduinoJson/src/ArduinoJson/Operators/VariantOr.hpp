// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2019
// MIT License

#pragma once

<<<<<<< HEAD:libraries/ArduinoJson/src/ArduinoJson/JsonVariantOr.hpp
#include "Data/JsonVariantAs.hpp"
#include "Polyfills/attributes.hpp"
#include "TypeTraits/EnableIf.hpp"
#include "TypeTraits/IsIntegral.hpp"
=======
#include "../Polyfills/attributes.hpp"
#include "../Polyfills/type_traits.hpp"
#include "../Variant/VariantAs.hpp"
>>>>>>> 71a09487d18c8b01d889cc54dde62335db4d16d6:libraries/ArduinoJson/src/ArduinoJson/Operators/VariantOr.hpp

namespace ARDUINOJSON_NAMESPACE {

template <typename TImpl>
class VariantOr {
 public:
  // Returns the default value if the VariantRef is undefined of incompatible
  template <typename T>
  typename EnableIf<!IsIntegral<T>::value, T>::type operator|(
      const T &defaultValue) const {
    if (impl()->template is<T>())
      return impl()->template as<T>();
    else
      return defaultValue;
  }

  // Returns the default value if the VariantRef is undefined of incompatible
  // Special case for string: null is treated as undefined
  const char *operator|(const char *defaultValue) const {
    const char *value = impl()->template as<const char *>();
    return value ? value : defaultValue;
  }

  // Returns the default value if the JsonVariant is undefined of incompatible
  // Special case for integers: we also accept double
  template <typename Integer>
  typename EnableIf<IsIntegral<Integer>::value, Integer>::type operator|(
      const Integer &defaultValue) const {
    if (impl()->template is<double>())
      return impl()->template as<Integer>();
    else
      return defaultValue;
  }

 private:
  const TImpl *impl() const {
    return static_cast<const TImpl *>(this);
  }
};
<<<<<<< HEAD:libraries/ArduinoJson/src/ArduinoJson/JsonVariantOr.hpp
}  // namespace Internals
}  // namespace ArduinoJson
=======
}  // namespace ARDUINOJSON_NAMESPACE
>>>>>>> 71a09487d18c8b01d889cc54dde62335db4d16d6:libraries/ArduinoJson/src/ArduinoJson/Operators/VariantOr.hpp
