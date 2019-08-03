// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2019
// MIT License

#include <ArduinoJson/version.hpp>
#include <catch.hpp>
#include <sstream>

<<<<<<< HEAD
=======
using Catch::Matchers::StartsWith;

>>>>>>> 71a09487d18c8b01d889cc54dde62335db4d16d6
TEST_CASE("ARDUINOJSON_VERSION") {
  std::stringstream version;

  version << ARDUINOJSON_VERSION_MAJOR << "." << ARDUINOJSON_VERSION_MINOR
          << "." << ARDUINOJSON_VERSION_REVISION;

<<<<<<< HEAD
  REQUIRE(version.str() == ARDUINOJSON_VERSION);
=======
  REQUIRE_THAT(ARDUINOJSON_VERSION, StartsWith(version.str()));
>>>>>>> 71a09487d18c8b01d889cc54dde62335db4d16d6
}
