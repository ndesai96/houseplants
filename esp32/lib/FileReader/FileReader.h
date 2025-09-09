#ifndef FILE_READER_H
#define FILE_READER_H

#include <FS.h>

class FileReader {
public:
    FileReader();
    bool begin();
    String readFile(const char* filename);
};

#endif // FILE_READER_H