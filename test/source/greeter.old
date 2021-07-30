#include <ckpttn/greeter.h>
#include <ckpttn/version.h>
#include <doctest/doctest.h>

#include <string>

TEST_CASE("CkPttn") {
    using namespace ckpttn;

    CkPttn ckpttn("Tests");

    CHECK(ckpttn.greet(LanguageCode::EN) == "Hello, Tests!");
    CHECK(ckpttn.greet(LanguageCode::DE) == "Hallo Tests!");
    CHECK(ckpttn.greet(LanguageCode::ES) == "¡Hola Tests!");
    CHECK(ckpttn.greet(LanguageCode::FR) == "Bonjour Tests!");
}

TEST_CASE("CkPttn version") {
    static_assert(std::string_view(CKPTTN_VERSION) == std::string_view("1.0"));
    CHECK(std::string(CKPTTN_VERSION) == std::string("1.0"));
}
