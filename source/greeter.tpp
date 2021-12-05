#include <ckpttn/greeter.h>
#include <fmt/format.h>

using namespace ckpttn;
using namespace std;

CkPttn::CkPttn(string _name) : name(move(_name)) {}

string CkPttn::greet(LanguageCode lang) const {
    switch (lang) {
        default:
        case LanguageCode::EN:
            return fmt::format("Hello, {}!", name);
        case LanguageCode::DE:
            return fmt::format("Hallo {}!", name);
        case LanguageCode::ES:
            return fmt::format("¡Hola {}!", name);
        case LanguageCode::FR:
            return fmt::format("Bonjour {}!", name);
    }
}
