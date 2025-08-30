#ifndef JSON_H__
#define JSON_H__

#include <map>
#include <vector>
#include <memory>
#include <type_traits>
#include <Arduino.h>
#include <functional>
#include <definitions.h>
// Provide make_unique if not available (for C++11)


enum JSONType {
  EMPTY,
  STRING,
  BOOLEAN,
  NUMERIC,
  OBJECT,
  LIST
};

class JSON {
private:
  // Use unique_ptr for sub-JSON objects.
  std::map<String, std::unique_ptr<JSON>> mapping;
  std::vector<std::unique_ptr<JSON>> list;
  String raw_content;
  byte type;
  static int count;
  int id;
  
public:
  JSON();
  JSON(const JSON& other);
  JSON(JSON&& other) noexcept;
  
  // Generic constructor and load method.
  template<typename T>
  JSON(const T& input) { load(input); }
  
  template<typename T>
  JSON& load(const T& input) {
    id = count++;
    // Instead of just clear(), reinitialize the containers so that allocated capacity is released.
    mapping = std::map<String, std::unique_ptr<JSON>>();
    list = std::vector<std::unique_ptr<JSON>>();
    raw_content = String(input);
    raw_content.trim();
    if (isObject(raw_content)) {
      type = JSONType::OBJECT;
      int index = 1;
      while (index < raw_content.length()) {
        auto [i, overhead] = skipOverhead(index);
        auto [j, key] = skip(index);
        key.replace("\"", "");
        skipOverhead(index);
        auto [l, value] = skip(index);
        if (key.length()) {
          mapping[key] = std::make_unique<JSON>(value);
        }
        skipOverhead(index);
        skipEnd(index);
      }
    } else if (isArray(raw_content)) {
      type = JSONType::LIST;
      int index = 1;
      while (index < raw_content.length()) {
        auto [i, overhead] = skipOverhead(index);
        auto [j, value] = skip(index);
        if (value.length()) {
          list.push_back(std::make_unique<JSON>(value));
        }
        skipOverhead(index);
        auto [n, ends] = skipEnd(index);
      }
      raw_content = "";
    } else {
      // Use type traits to decide type.
      if constexpr (std::is_same<T, bool>::value) {
        raw_content = input ? "true" : "false";
        type = JSONType::BOOLEAN;
      }
      else if constexpr (std::is_same<T, char>::value) { // Treat char as string.
        raw_content = String(input);
        type = JSONType::STRING;
      }
      else if constexpr (std::is_same<T, const char*>::value) { // C-string.
        raw_content = String(input);
        type = JSONType::STRING;
      }
      else if constexpr (std::is_arithmetic<T>::value) {
        type = JSONType::NUMERIC;
      }
      else if constexpr (std::is_same<T, String>::value) {
        type = JSONType::STRING;
      }
      else {
        type = JSONType::STRING;
      }
    }
    return *this;
  }
  
  JSON& operator=(const JSON& other);
  JSON& operator=(JSON&& other) noexcept;
  ~JSON();
  
  // Optimized clear() which reinitializes containers.
  void clear();
  
  template<typename T>
  JSON& operator[](const T& index) {
    if (type == JSONType::OBJECT) {
      String key = String(index);
      auto it = mapping.find(key);
      if (it == mapping.end()) {
        mapping[key] = std::make_unique<JSON>();
        return *mapping[key];
      }
      return *it->second;
    } else if (type == JSONType::LIST) {
      int idx = String(index).toInt();
      if (idx >= 0 && idx < (int)list.size()) {
        return *list[idx];
      }
    }
    return *this;
  }
  
  String listToStr() const;
  String mapToStr() const;
  String toString(const bool partOfSomeObject = false) const;
  int32_t toInt();
  float toFloat();
  double toDouble();
  std::pair<int, String> skipOverhead(int& index);
  std::pair<int, String> skipEnd(int& index);
  std::pair<int, String> skip(int& index, bool enclosed = false);
  std::pair<int, String> skipString(int& index, bool enclosed = false);
  std::pair<int, String> skipObject(int& index);
  bool endSymbol(char ch);
  char complement(char ch);
  bool isObject(const String& x);
  bool isArray(const String& x);
  void push_back(const JSON& element);
  std::vector<std::unique_ptr<JSON>>::iterator begin();
  std::vector<std::unique_ptr<JSON>>::iterator end();
  JSON& front();
  JSON& back();
  bool operator!=(const JSON& other) const;
  bool operator!() const;
  operator bool() const;
  bool operator==(const JSON& other) const;
  int findIndex(std::function<bool(JSON&)> predicate);
  JSON& find(std::function<bool(JSON&)> predicate);
  void findAndReplace(std::function<bool(JSON&)> predicate, const JSON& newValue);
  JSON map(std::function<JSON(const JSON&)> predicate);
  void forEach(std::function<void(JSON&)> predicate);
  JSON& remove(std::function<bool(JSON&)> predicate);
  JSON& remove(int index);
  JSON& remove(std::function<bool(const String&, JSON&)> predicate);
  void update(std::function<bool(const String&, JSON&)> predicate, const JSON& newValue);
  void update(std::function<bool(JSON&)> predicate, const JSON& newValue);
  
  template<typename T>
  bool contains(const T& key) {
    if (type == JSONType::OBJECT) {
      for (const auto& [k, _] : mapping) {
        if (k == String(key)) {
          return true;
        }
      }
    } else if (type == JSONType::LIST) {
      for (const auto& item : list) {
        if (*item == JSON(key)) {
          return true;
        }
      }
    }
    return false;
  }
  
  void sort(std::function<bool(JSON*, JSON*)> predicate);
  static bool isJSON(const String& str);
  int size();
  static JSON nullJSON;
};

class JSONArray : public JSON {
public:
  JSONArray() : JSON("[]") {}
  template<typename T>
  JSONArray(const T& input) : JSON(input) {}
};

#endif
