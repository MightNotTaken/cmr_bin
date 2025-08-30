#include "JSON.h"
#include <algorithm>
#include <utility>

int JSON::count = 0;
JSON JSON::nullJSON;

int JSON::size() {
  switch (type) {
    case JSONType::OBJECT: return mapping.size();
    case JSONType::LIST:   return list.size();
    default:               return raw_content.length();
  }
}

void JSON::sort(std::function<bool(JSON*, JSON*)> predicate) {
  if (type == JSONType::LIST) {
    std::sort(list.begin(), list.end(),
      [&](const std::unique_ptr<JSON>& a, const std::unique_ptr<JSON>& b) {
        return predicate(a.get(), b.get());
      }
    );
  }
}

JSON::JSON()
  : type(JSONType::OBJECT), id(count++) {}

JSON::JSON(const JSON& other)
  : type(other.type), raw_content(other.raw_content), id(count++) {
  if (other.type == JSONType::OBJECT) {
    for (const auto& entry : other.mapping) {
      mapping[entry.first] = std::make_unique<JSON>(*entry.second);
    }
  } else if (other.type == JSONType::LIST) {
    for (const auto& item : other.list) {
      list.push_back(std::make_unique<JSON>(*item));
    }
  }
}

JSON::JSON(JSON&& other) noexcept
  : mapping(std::move(other.mapping)), list(std::move(other.list)),
    raw_content(std::move(other.raw_content)), type(other.type), id(count++) {
  other.type = JSONType::STRING;
}

JSON& JSON::operator=(const JSON& other) {
  if (this != &other) {
    clear();
    type = other.type;
    raw_content = other.raw_content;
    if (type == JSONType::OBJECT) {
      for (const auto& entry : other.mapping) {
        mapping[entry.first] = std::make_unique<JSON>(*entry.second);
      }
    } else if (type == JSONType::LIST) {
      for (const auto& item : other.list) {
        list.push_back(std::make_unique<JSON>(*item));
      }
    }
  }
  id = count++;
  return *this;
}

JSON& JSON::operator=(JSON&& other) noexcept {
  if (this != &other) {
    clear();
    type = other.type;
    raw_content = std::move(other.raw_content);
    mapping = std::move(other.mapping);
    list = std::move(other.list);
  }
  id = count++;
  return *this;
}

JSON::~JSON() {
  clear();
}

void JSON::clear() {
  // Reinitialize the containers so that memory is freed.
  mapping = std::map<String, std::unique_ptr<JSON>>();
  list = std::vector<std::unique_ptr<JSON>>();
  // For vector, try to shrink capacity if supported.
  list.shrink_to_fit();
  raw_content = "";
}

String JSON::listToStr() const {
  String data = "[";
  for (size_t i = 0; i < list.size(); i++) {
    if (i > 0)
      data += ",";
    data += list[i]->toString(true);
  }
  data += "]";
  return data;
}

String JSON::mapToStr() const {
  String data = "{";
  bool first = true;
  for (const auto& entry : mapping) {
    if (!first)
      data += ",";
    first = false;
    data += "\"" + entry.first + "\":";
    data += entry.second->toString(true);
  }
  data += "}";
  return data;
}

String JSON::toString(const bool partOfSomeObject) const {
  switch (type) {
    case JSONType::OBJECT: return mapToStr();
    case JSONType::LIST:   return listToStr();
    case JSONType::STRING:
      if (partOfSomeObject)
        return "\"" + raw_content + "\"";
      return raw_content;
    default:
      return raw_content;
  }
}

int32_t JSON::toInt() {
  return raw_content.toInt();
}

float JSON::toFloat() {
  return raw_content.toFloat();
}

double JSON::toDouble() {
  return raw_content.toDouble();
}

std::pair<int, String> JSON::skipOverhead(int& index) {
  String response;
  while (index < raw_content.length()) {
    char ch = raw_content[index];
    if (ch == ':' || ch == ' ' || ch == '\t' || ch == ',' || ch == '\n') {
      response += ch;
      index++;
    } else {
      break;
    }
  }
  return { index, response };
}

std::pair<int, String> JSON::skipEnd(int& index) {
  String response;
  while (index < raw_content.length()) {
    char ch = raw_content[index];
    if (ch == ']' || ch == '}' || ch == ' ' || ch == '\t' || ch == '\n') {
      response += ch;
      index++;
    } else {
      break;
    }
  }
  return { index, response };
}

std::pair<int, String> JSON::skip(int& index, bool enclosed) {
  char ch = raw_content[index];
  if (ch == '[' || ch == '{')
    return skipObject(index);
  else
    return skipString(index, enclosed);
}

std::pair<int, String> JSON::skipString(int& index, bool enclosed) {
  String response = "";
  char quote = raw_content[index];
  bool escaped = false;
  if (quote != '"') {
    while (index < raw_content.length() && !endSymbol(raw_content[index])) {
      response += raw_content[index++];
    }
    return { index, response };
  }
  response += quote;
  index++;
  while (index < raw_content.length()) {
    char ch = raw_content[index];
    response += ch;
    index++;
    if (ch == '\\') {
      escaped = !escaped;
    } else if (ch == quote && !escaped) {
      break;
    } else {
      escaped = false;
    }
  }
  if (!enclosed) {
    if (response.startsWith("\""))
      response = response.substring(1);
    if (response.endsWith("\""))
      response = response.substring(0, response.length() - 1);
  }
  return { index, response };
}

std::pair<int, String> JSON::skipObject(int& index) {
  int stack = 0;
  char start = raw_content[index];
  char end = complement(start);
  String response = "";
  while (index < raw_content.length()) {
    char ch = raw_content[index];
    if (ch == '"') {
      auto [i, value] = skipString(index, true);
      index = i;
      response += value;
      continue;
    }
    response += ch;
    if (ch == start)
      stack++;
    else if (ch == end)
      stack--;
    if (stack == 0)
      break;
    index++;
  }
  return { index, response };
}

bool JSON::endSymbol(char ch) {
  return ch == ',' || ch == '}' || ch == ']';
}

char JSON::complement(char ch) {
  switch (ch) {
    case '{': return '}';
    case '[': return ']';
    case '(': return ')';
    case '"': return '"';
    case '<': return '>';
    default:  return '\0';
  }
}

bool JSON::isObject(const String& x) {
  return (x.length() > 0) && (x.charAt(0) == '{');
}

bool JSON::isArray(const String& x) {
  return (x.length() > 0) && (x.charAt(0) == '[');
}

void JSON::push_back(const JSON& element) {
  if (type == JSONType::LIST)
    list.push_back(std::make_unique<JSON>(element));
}

std::vector<std::unique_ptr<JSON>>::iterator JSON::begin() {
  if (type == JSONType::LIST && !list.empty())
    return list.begin();
  return std::vector<std::unique_ptr<JSON>>::iterator();
}

std::vector<std::unique_ptr<JSON>>::iterator JSON::end() {
  if (type == JSONType::LIST && !list.empty())
    return list.end();
  return std::vector<std::unique_ptr<JSON>>::iterator();
}

JSON& JSON::front() {
  if (type == JSONType::LIST && !list.empty())
    return *list.front();
  return *this;
}

JSON& JSON::back() {
  if (type == JSONType::LIST && !list.empty())
    return *list.back();
  return *this;
}

bool JSON::operator!=(const JSON& other) const {
  return !(*this == other);
}

bool JSON::operator==(const JSON& other) const {
  if (raw_content != other.raw_content)
    return false;
  if (list.size() != other.list.size())
    return false;
  for (size_t i = 0; i < list.size(); ++i) {
    if (*list[i] != *other.list[i])
      return false;
  }
  if (mapping.size() != other.mapping.size())
    return false;
  for (const auto& entry : mapping) {
    auto it = other.mapping.find(entry.first);
    if (it == other.mapping.end() || *(entry.second) != *(it->second))
      return false;
  }
  return true;
}

int JSON::findIndex(std::function<bool(JSON&)> predicate) {
  for (size_t i = 0; i < list.size(); ++i) {
    if (predicate(*list[i]))
      return i;
  }
  return -1;
}

JSON& JSON::find(std::function<bool(JSON&)> predicate) {
  for (auto& item : list) {
    if (predicate(*item))
      return *item;
  }
  return nullJSON;
}

void JSON::findAndReplace(std::function<bool(JSON&)> predicate, const JSON& newValue) {
  if (type == JSONType::LIST) {
    for (auto& item : list) {
      if (predicate(*item)) {
        *item = newValue;
        return;
      }
    }
  }
}

JSON JSON::map(std::function<JSON(const JSON&)> predicate) {
  JSON result("[]");
  for (auto& item : list) {
    result.push_back(predicate(*item));
  }
  return result;
}

void JSON::forEach(std::function<void(JSON&)> predicate) {
  for (auto& item : list) {
    predicate(*item);
  }
}

JSON& JSON::remove(std::function<bool(JSON&)> predicate) {
  if (type == JSONType::LIST) {
    auto it = list.begin();
    while (it != list.end()) {
      if (predicate(*(*it))) {
        it = list.erase(it);
      } else {
        ++it;
      }
    }
  } else if (type == JSONType::OBJECT) {
    for (auto it = mapping.begin(); it != mapping.end(); ) {
      if (predicate(*it->second))
        it = mapping.erase(it);
      else
        ++it;
    }
  }
  return *this;
}

JSON& JSON::remove(int index) {
  if (type == JSONType::LIST) {
    if (index >= 0 && index < (int)list.size())
      list.erase(list.begin() + index);
  }
  return *this;
}

JSON& JSON::remove(std::function<bool(const String&, JSON&)> predicate) {
  if (type == JSONType::OBJECT) {
    for (auto it = mapping.begin(); it != mapping.end(); ) {
      if (predicate(it->first, *it->second))
        it = mapping.erase(it);
      else
        ++it;
    }
  }
  return *this;
}

void JSON::update(std::function<bool(const String&, JSON&)> predicate, const JSON& newValue) {
  if (type == JSONType::OBJECT) {
    for (auto& entry : mapping) {
      if (predicate(entry.first, *entry.second))
        *(entry.second) = newValue;
    }
  }
}

void JSON::update(std::function<bool(JSON&)> predicate, const JSON& newValue) {
  if (type == JSONType::LIST) {
    for (auto& item : list) {
      if (predicate(*item))
        *item = newValue;
    }
  } else if (type == JSONType::OBJECT) {
    for (auto& entry : mapping) {
      if (predicate(*entry.second))
        *(entry.second) = newValue;
    }
  }
}

bool JSON::isJSON(const String& str) {
  if (str.length() == 0)
    return false;
  char start = str.charAt(0);
  char end = str.charAt(str.length() - 1);
  return (start == '[' && end == ']') || (start == '{' && end == '}');
}
