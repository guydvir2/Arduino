// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2019
// MIT License
//
// clang-format off

#ifdef __GNUC__

#define ARDUINOJSON_PRAGMA(x) _Pragma(#x)

#define ARDUINOJSON_COMPILE_ERROR(msg) ARDUINOJSON_PRAGMA(GCC error msg)

#define ARDUINOJSON_STRINGIFY(S) #S

#define ARDUINOJSON_DEPRECATION_ERROR(X, Y) \
<<<<<<< HEAD
  ARDUINOJSON_COMPILE_ERROR(ARDUINOJSON_STRINGIFY(X is a Y from ArduinoJson 6 but version 5 is installed. Visit arduinojson.org to get more information.))

#define StaticJsonDocument ARDUINOJSON_DEPRECATION_ERROR(StaticJsonDocument, class)
#define DynamicJsonDocument ARDUINOJSON_DEPRECATION_ERROR(DynamicJsonDocument, class)
#define JsonDocument ARDUINOJSON_DEPRECATION_ERROR(JsonDocument, class)
#define DeserializationError ARDUINOJSON_DEPRECATION_ERROR(DeserializationError, class)
#define deserializeJson ARDUINOJSON_DEPRECATION_ERROR(deserializeJson, function)
#define deserializeMsgPack ARDUINOJSON_DEPRECATION_ERROR(deserializeMsgPack, function)
#define serializeJson ARDUINOJSON_DEPRECATION_ERROR(serializeJson, function)
#define serializeMsgPack ARDUINOJSON_DEPRECATION_ERROR(serializeMsgPack, function)
#define serializeJsonPretty ARDUINOJSON_DEPRECATION_ERROR(serializeJsonPretty, function)
#define measureMsgPack ARDUINOJSON_DEPRECATION_ERROR(measureMsgPack, function)
#define measureJson ARDUINOJSON_DEPRECATION_ERROR(measureJson, function)
#define measureJsonPretty ARDUINOJSON_DEPRECATION_ERROR(measureJsonPretty, function)
=======
  ARDUINOJSON_COMPILE_ERROR(ARDUINOJSON_STRINGIFY(X is a Y from ArduinoJson 5. Please see arduinojson.org/upgrade to learn how to upgrade your program to ArduinoJson version 6))

#define StaticJsonBuffer ARDUINOJSON_DEPRECATION_ERROR(StaticJsonBuffer, class)
#define DynamicJsonBuffer ARDUINOJSON_DEPRECATION_ERROR(DynamicJsonBuffer, class)
#define JsonBuffer ARDUINOJSON_DEPRECATION_ERROR(JsonBuffer, class)
#define RawJson ARDUINOJSON_DEPRECATION_ERROR(RawJson, function)
>>>>>>> 71a09487d18c8b01d889cc54dde62335db4d16d6

#endif
