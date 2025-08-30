#include <string-matcher.h>

std::vector<char> StringMatcher::getUniques(const String& str) {
    std::vector<char> uniques;
    for (char c : str) {
        bool found = false;
        for (char uc : uniques) {
            if (c == uc) {
                found = true;
                break;
            }
        }
        if (!found) {
            uniques.push_back(c);
        }
    }
    return uniques;
}

String StringMatcher::longestCommonSubstring(const String& str1, const String& str2) {
    int m = str1.length();
    int n = str2.length();
    int dp[m + 1][n + 1];
    int maxLength = 0;
    int endIndex = 0;

    // Initialize the DP table
    for (int i = 0; i <= m; i++) {
        for (int j = 0; j <= n; j++) {
            if (i == 0 || j == 0)
                dp[i][j] = 0;
            else if (str1[i - 1] == str2[j - 1]) { // Access characters using array indexing
                dp[i][j] = dp[i - 1][j - 1] + 1;
                if (dp[i][j] > maxLength) {
                    maxLength = dp[i][j];
                    endIndex = i - 1; // Store the ending index of common substring
                }
            }
            else
                dp[i][j] = 0;
        }
    }

    String longestSubstring = "";
    // Construct the longest common substring using endIndex and maxLength
    for (int i = endIndex - maxLength + 1; i <= endIndex; i++)
        longestSubstring += str1[i];

    return longestSubstring;
}

StringMatcher::StringMatcher(String first, String second, bool trimFirst) {
    this->setStrings(first, second, trimFirst);
}

void StringMatcher::setStrings(String first, String second, bool trimFirst) {

    if (first.length() > second.length()) {
        smaller = second;
        bigger = first;
    } else {
        smaller = first;
        bigger = second;
    }
    if (trimFirst) {
        this->trim();
    }
}
    
float StringMatcher::getPercentage() {
    if (!smaller.length()) {
        return 0;
    }
    float factors = this->matchLength();
    float d2match = this->d2Match();
    int divisor = 5;
    this->trim();
    this->ignoreCase();
    this->ignoreSeparaters();
    this->ignoreSpecialCharacters();
    this->ignoreBrackets();
    factors += this->matchLength();
    factors += this->getCharInclusion();
    factors += this->getStrInclusion();
    factors += this->d2Match();
    if (factors == 100 * divisor) {
    divisor ++;
    factors += d2match;
    }
    return factors / divisor;
}

void StringMatcher::trim() {
    this->smaller.trim();
    this->bigger.trim();
}

float StringMatcher::d2Match() {
    int match = 0;
    for (int i=0; i<smaller.length(); i++) {
    match += smaller[i] == bigger[i];
    }
    return 100.0 * match / bigger.length();
}

float StringMatcher::matchLength() {
    return (100.0 * smaller.length()) / bigger.length();
}

float StringMatcher::getStrInclusion() {
    String longestSequence = longestCommonSubstring(this->bigger, this->smaller);
    return 100.0 * ((longestSequence.length() * 1.0 / this->smaller.length())) / 1.0;
}

float StringMatcher::getCharInclusion() {
    return getUniques(this->smaller).size() * 100.0 / getUniques(this->bigger).size();
}

void StringMatcher::ignoreCase() {
    this->smaller.toLowerCase();
    this->bigger.toLowerCase();
}

void StringMatcher::ignoreSeparaters() {
    this->smaller.replace("-", "");
    this->bigger.replace("-", "");

    this->smaller.replace("_", "");
    this->bigger.replace("_", "");

    this->smaller.replace(".", "");
    this->bigger.replace(".", "");

    this->smaller.replace(":", "");
    this->bigger.replace(":", "");

    this->smaller.replace(",", "");
    this->bigger.replace(",", "");

    this->smaller.replace(" ", "");
    this->bigger.replace(" ", "");
}

void StringMatcher::ignoreSpecialCharacters() {      
    this->smaller.replace("#", "");
    this->bigger.replace("#", "");

    this->smaller.replace("%", "");
    this->bigger.replace("%", "");

    this->smaller.replace("!", "");
    this->bigger.replace("!", "");

    this->smaller.replace("@", "");
    this->bigger.replace("@", "");

    this->smaller.replace("$", "");
    this->bigger.replace("$", "");

    this->smaller.replace("&", "");
    this->bigger.replace("&", "");

    this->smaller.replace("*", "");
    this->bigger.replace("*", "");
}

void StringMatcher::ignoreBrackets() {
    this->smaller.replace("[", "");
    this->bigger.replace("[", "");

    this->smaller.replace("]", "");
    this->bigger.replace("]", "");
    
    this->smaller.replace("(", "");
    this->bigger.replace("(", "");

    this->smaller.replace(")", "");
    this->bigger.replace(")", "");
    
    this->smaller.replace("{", "");
    this->bigger.replace("{", "");

    this->smaller.replace("}", "");
    this->bigger.replace("}", "");
    
    this->smaller.replace("<", "");
    this->bigger.replace("<", "");
    
    this->smaller.replace(">", "");
    this->bigger.replace(">", "");      
}
