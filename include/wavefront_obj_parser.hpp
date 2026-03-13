// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT
#pragma once

#include "vector.hpp"

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

namespace GLVM::core {
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
    GLVM::core::vector<int> vertexIndex;
    GLVM::core::vector<int> textureIndex;
    GLVM::core::vector<int> normalIndex;

public:
    GLVM::core::vector<int>& operator[](const unsigned int _iIndex) {
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

    const GLVM::core::vector<int>& operator[](const unsigned int _iIndex) const {
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
    GLVM::core::vector<SVertex> coordinateVertices_;
    GLVM::core::vector<SVertex> textureVertices_;
    GLVM::core::vector<SVertex> normals_;
    GLVM::core::vector<SFace> faces_;

    std::string sWavefrontObjFileData;
    const char* pWavefrontObjFileData;
    unsigned int uiCounter = 0;

public:
    CWaveFrontObjParser();

    [[nodiscard]] const GLVM::core::vector<SVertex>& getCoordinateVertices() const;
    [[nodiscard]] const GLVM::core::vector<SVertex>& getTextureVertices() const;
    [[nodiscard]] const GLVM::core::vector<SVertex>& getNormals() const;
    [[nodiscard]] const GLVM::core::vector<SFace>& getFaces() const;

    void ReadFile(const char* _filePath);
    void ParseFile();
    GLVM::core::vector<vector<char>> Split(
        const char* _pWaveFrontObjFileData,
        const char _separator,
        const char _exitSymbol,
        unsigned int& _uiCounter
    );
    SVertex ParseVertices(GLVM::core::vector<vector<char>> _wordsContainer);
    SFace ParseFaces(GLVM::core::vector<vector<char>> _wordsContainer);
    int ParseInteger(GLVM::core::vector<char> _word);
    float ParseFloating(GLVM::core::vector<char> _word);
};
} // namespace GLVM::core
