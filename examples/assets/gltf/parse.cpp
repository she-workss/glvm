#include <bits/types/FILE.h>
#include <fstream>
#include <iostream>
#include <sstream>

void ReadFile(const char* _filePath) {
    const char* _pWavefrontObjFile = _filePath;
    std::ifstream WavefrontObjFileInputStream;
    std::stringstream WavefrontObjFileOutputStream;
    String sWavefrontObjFileData;
    const char* pWavefrontObjFileData;

    WavefrontObjFileInputStream.open(_pWavefrontObjFile);
    if (WavefrontObjFileInputStream.good()) {
        WavefrontObjFileOutputStream << WavefrontObjFileInputStream.rdbuf();
        WavefrontObjFileInputStream.close();
        sWavefrontObjFileData = WavefrontObjFileOutputStream.str();
    } else {
        return;
    }
    pWavefrontObjFileData = sWavefrontObjFileData.c_str();
}

void readFile(const char* _filePath) {
    char buffer[840];

    FILE* ptr;

    ptr = fopen("cube.bin", "rb");
    fread(buffer, 840, 1, ptr);

    for (u32 i = 0; i < 840; i += 2) {
        printf("%hu ", buffer[i]);
    }
}

auto main(i32 argc, char* argv[]) -> i32 {
    readFile("cube.bin");

    return 0;
}
