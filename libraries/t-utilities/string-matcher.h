#ifndef STRING_MATCHER_H__
#define STRING_MATCHER_H__
#include <vector>
#include <string>
#include <Arduino.h>

class StringMatcher {
    String smaller;
    String bigger;
public:
    StringMatcher(String first, String second, bool trimFirst = true);
    float getPercentage();
    void setStrings(String first, String second, bool trimFirst = true);
private:
    void ignoreBrackets();
    void ignoreSpecialCharacters();
    void ignoreSeparaters();
    void ignoreCase();
    float getCharInclusion();
    float getStrInclusion();
    float matchLength();
    float d2Match();
    void trim();
    std::vector<char> getUniques(const String&);
    String longestCommonSubstring(const String& str1, const String& str2);
};
#endif