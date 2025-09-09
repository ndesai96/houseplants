#include "FileReader.h"
#include "SPIFFS.h"

FileReader::FileReader() {}

bool FileReader::begin() {
    if(!SPIFFS.begin(true)){
        return false;
    }
    return true;
}

String FileReader::readFile(const char* filename) {
    File file = SPIFFS.open(filename, "r");
    if(!file){
        Serial.printf("Failed to open file: %s\n", filename);
        return "";
    }

    String content = file.readString();
    file.close();

    return content;
}