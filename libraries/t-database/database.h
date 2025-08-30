#ifndef DATABASE_H__
#define DATABASE_H__
#include <Arduino.h>

#ifndef ESP32
#define FILE_WRITE "w"
#define FILE_READ "r"
#define FILE_APPEND "a"
#else
#include <SPIFFS.h>
#define UsedDB  SPIFFS
#endif
#define FORMAT_UsedDB_IF_FAILED    true
class Database {
    String _payload;
public:
    void begin();
    void listDir(const char* dirname = "/", uint8_t levels = 0);
    bool format();
    bool createFile(String name);
    bool writeFile(String name, String data, char* mode=FILE_WRITE);
    bool appendFile(String name, String data);
    bool readFile(String name);
    bool hasFile(String name);
    bool renameFile(String original, String newer);
    bool removeFile(String filename);
    String& payload();
};

extern Database database;
#endif