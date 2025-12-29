#include "console.h"

Console console;

bool Console::isJson(const String& str) {
  // A very simple check: starts and ends with {} or [].
  return (str.startsWith("{") && str.endsWith("}")) || 
         (str.startsWith("[") && str.endsWith("]"));
}

Console::Console() {
  disabled = false;
}

// Improved JSON prettifier with proper handling of escaped quotes.
void Console::prettifyJson(const String& jsonString) {
  int indentLevel = 0;
  bool inQuote = false;

  for (size_t i = 0; i < jsonString.length(); i++) {
    char c = jsonString[i];

    // Check if the current quote is escaped by counting preceding backslashes.
    if (c == '"') {
      int backslashCount = 0;
      int j = i - 1;
      while (j >= 0 && jsonString[j] == '\\') {
        backslashCount++;
        j--;
      }
      // Only toggle inQuote if the quote is not escaped (i.e. even number of backslashes)
      if (backslashCount % 2 == 0) {
        inQuote = !inQuote;
      }
    }

    if (!inQuote) {
      if (c == '{' || c == '[') {
        Serial.print(c);
        Serial.println();
        indentLevel += 2;
        printIndent(indentLevel);
      } else if (c == '}' || c == ']') {
        indentLevel -= 2;
        Serial.println();
        printIndent(indentLevel);
        Serial.print(c);
      } else if (c == ',') {
        Serial.print(c);
        Serial.println();
        printIndent(indentLevel);
      } else if (c == ':') {
        Serial.print(c);
        Serial.print(' ');
      } else if (c == ' ') {
        // Skip printing extra spaces outside quotes.
      } else {
        Serial.print(c);
      }
    } else {
      Serial.print(c);
    }
  }
  Serial.println(); // Ensure we end with a newline.
}

void Console::printIndent(int indentLevel) {
  for (int i = 0; i < indentLevel; i++) {
    Serial.print(' ');
  }
}
