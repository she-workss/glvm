#pragma once

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace glvm::core {
class SVertex {
    float x;
    float y;
    float z;

public:
    float& operator[](const unsigned int _iIndex) {
        assert(_iIndex < 3 && _iIndex >= 0 && "Wrong index");
        switch (_iIndex) {
            default:
            case 0:
                return x;
            case 1:
                return y;
            case 2:
                return z;
        }
    }
};

class SFace {
    std::vector<int> vertexIndex;
    std::vector<int> textureIndex;
    std::vector<int> normalIndex;

public:
    std::vector<int>& operator[](const unsigned int _iIndex) {
        assert(_iIndex < 3 && _iIndex >= 0 && "Wrong index");
        switch (_iIndex) {
            default:
            case 0:
                return vertexIndex;
            case 1:
                return textureIndex;
            case 2:
                return normalIndex;
        }
    }

    const std::vector<int>& operator[](const unsigned int _iIndex) const {
        assert(_iIndex < 3 && _iIndex >= 0 && "Wrong index");
        switch (_iIndex) {
            default:
            case 0:
                return vertexIndex;
            case 1:
                return textureIndex;
            case 2:
                return normalIndex;
        }
    }
};

class CWaveFrontObjParser {
    std::vector<SVertex> coordinateVertices_;
    std::vector<SVertex> textureVertices_;
    std::vector<SVertex> normals_;
    std::vector<SFace> faces_;

    std::string sWavefrontObjFileData;
    const char* pWavefrontObjFileData;
    unsigned int uiCounter = 0;

public:
    CWaveFrontObjParser();

    [[nodiscard]] const std::vector<SVertex>& getCoordinateVertices() const;
    [[nodiscard]] const std::vector<SVertex>& getTextureVertices() const;
    [[nodiscard]] const std::vector<SVertex>& getNormals() const;
    [[nodiscard]] const std::vector<SFace>& getFaces() const;

    void ReadFile(const char* _filePath);
    void ParseFile();
    std::vector<std::vector<char>> Split(
        const char* _pWaveFrontObjFileData,
        const char _separator,
        const char _exitSymbol,
        unsigned int& _uiCounter
    );
    SVertex ParseVertices(std::vector<std::vector<char>> _wordsContainer);
    SFace ParseFaces(std::vector<std::vector<char>> _wordsContainer);
    int ParseInteger(std::vector<char> _word);
    float ParseFloating(std::vector<char> _word);
};
} // namespace glvm::core
