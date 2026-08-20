#pragma once

#include "glvm/Vector.hpp"

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
    glvm::core::vector<int> vertexIndex;
    glvm::core::vector<int> textureIndex;
    glvm::core::vector<int> normalIndex;

public:
    glvm::core::vector<int>& operator[](const unsigned int _iIndex) {
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

    const glvm::core::vector<int>& operator[](const unsigned int _iIndex) const {
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
    glvm::core::vector<SVertex> coordinateVertices_;
    glvm::core::vector<SVertex> textureVertices_;
    glvm::core::vector<SVertex> normals_;
    glvm::core::vector<SFace> faces_;

    std::string sWavefrontObjFileData;
    const char* pWavefrontObjFileData;
    unsigned int uiCounter = 0;

public:
    CWaveFrontObjParser();

    [[nodiscard]] const glvm::core::vector<SVertex>& getCoordinateVertices() const;
    [[nodiscard]] const glvm::core::vector<SVertex>& getTextureVertices() const;
    [[nodiscard]] const glvm::core::vector<SVertex>& getNormals() const;
    [[nodiscard]] const glvm::core::vector<SFace>& getFaces() const;

    void ReadFile(const char* _filePath);
    void ParseFile();
    glvm::core::vector<vector<char>> Split(
        const char* _pWaveFrontObjFileData,
        const char _separator,
        const char _exitSymbol,
        unsigned int& _uiCounter
    );
    SVertex ParseVertices(glvm::core::vector<vector<char>> _wordsContainer);
    SFace ParseFaces(glvm::core::vector<vector<char>> _wordsContainer);
    int ParseInteger(glvm::core::vector<char> _word);
    float ParseFloating(glvm::core::vector<char> _word);
};
} // namespace glvm::core
