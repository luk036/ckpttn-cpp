#pragma once

#include <string>

namespace ckpttn {

    /**  Language codes to be used with the CkPttn class */
    enum class LanguageCode { EN, DE, ES, FR };

    /**
     * @brief A class for saying hello in multiple languages
     */
    class CkPttn {
        std::string name;

      public:
        /**
         * @brief Creates a new ckpttn
         * @param[in] name the name to greet
         */
        CkPttn(std::string name);

        /**
         * @brief Creates a localized string containing the greeting
         * @param[in] lang the language to greet in
         * @return a string containing the greeting
         */
        std::string greet(LanguageCode lang = LanguageCode::EN) const;
    };

}  // namespace ckpttn
