#pragma once
#include <string>
#include <unordered_map>

enum class Language {
    English = 0,
    Slovak = 1
};

class Localization {
public:
    static void SetLanguage(Language lang);
    static Language GetLanguage();

    static const char* Get(const std::string& key);
    static const std::string& GetString(const std::string& key);

    static const char* GetLanguageDisplayName(Language lang);

private:
    static void Initialize();

    static Language currentLanguage;
    static bool initialized;

    static std::unordered_map<std::string, std::string> en;
    static std::unordered_map<std::string, std::string> sk;
};