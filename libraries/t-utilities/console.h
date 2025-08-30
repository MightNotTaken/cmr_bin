#ifndef CONSOLE_H__
#define CONSOLE_H__
#include <Arduino.h>
#include <iterator> // for std::begin and std::end
extern SemaphoreHandle_t serialMutex;

template<typename T>
struct has_show_method {
  private:
    typedef char YesType[1];
    typedef char NoType[2];

    template<typename C>
    static YesType& test(decltype(&C::toString));

    template<typename>
    static NoType& test(...);

  public:
    static const bool value = sizeof(test<T>(nullptr)) == sizeof(YesType);
};

template<typename T>
struct has_begin_end {
  private:
    template<typename U>
    static auto test(U* p) -> decltype(std::begin(*p), std::end(*p), std::true_type {});

    template<typename>
    static std::false_type test(...);

  public:
    static const bool value = decltype(test<T>(nullptr))::value;
};

class Console {
  bool disabled;
  public:
    Console();

    void disable() {
      disabled = true;
    }

    void enable() {
      disabled = false;
    }
    
    template <typename... Args>
    void log(Args... args) {
      if (disabled) {
        return;
      }
      if (xSemaphoreTake(serialMutex, portMAX_DELAY)) {
        logHelper(args...);
        xSemaphoreGive(serialMutex);
      }
    }

  private:
    char separator = ' ';

    template <typename T, typename... Args>
    void logHelper(T first, Args... rest) {
      if constexpr (has_show_method<T>::value) {
        logHelper(first.toString());
      } else if constexpr (std::is_same<String, T>::value) {
        // If the argument is a String, check if it appears to be JSON.
        if (isJson(first)) {
          prettifyJson(first);
        } else {
          Serial.print(first);
        }
      } else if constexpr (has_begin_end<T>::value) {
        for (auto it = first.begin(); it != first.end(); ++it) {
          logHelper(*it);
        }
      } else {
        Serial.print(first);
      }
      Serial.print(separator);
      logHelper(rest...);
    }

    void logHelper() {
      Serial.println();
    }

    bool isJson(const String& str);
    void prettifyJson(const String& jsonString);
    void printIndent(int indentLevel);
};

extern Console console;

#endif // CONSOLE_H__
