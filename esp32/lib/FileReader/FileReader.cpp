#include "FileReader.h"
#include "SPIFFS.h"

FileReader::FileReader() {}

bool FileReader::begin() {
    if(!SPIFFS.begin(true)){
        return false;
    }
    return true;
}

char* FileReader::readFile(const char* filename) {
    File file = SPIFFS.open(filename, "r");
    if(!file){
        Serial.printf("Failed to open file: %s\n", filename);
        return nullptr;
    }

    String content = file.readString();
    file.close();

    if (content.length() == 0) {
        Serial.printf("File is empty: %s\n", filename);
        return nullptr;
    }

    char* result = new char[content.length() + 1];
    strcpy(result, content.c_str());
    
    return result;
}