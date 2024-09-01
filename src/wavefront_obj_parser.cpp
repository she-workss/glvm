// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "wavefront_obj_parser.hpp"
#include "vector.hpp"

#include <chrono>
#include <iterator>

namespace GLVM::core {
CWaveFrontObjParser::CWaveFrontObjParser() {
}

const GLVM::core::vector<SVertex> &
CWaveFrontObjParser::getCoordinateVertices() const {
    return coordinateVertices_;
}
const GLVM::core::vector<SVertex> &
CWaveFrontObjParser::getTextureVertices() const {
    return textureVertices_;
}
const GLVM::core::vector<SVertex> &CWaveFrontObjParser::getNormals() const {
    return normals_;
}
const GLVM::core::vector<SFace> &CWaveFrontObjParser::getFaces() const {
    return faces_;
}

void CWaveFrontObjParser::ReadFile(const char *_filePath) {
    const char *_pWavefrontObjFile = _filePath;
    std::ifstream WavefrontObjFileInputStream;
    std::stringstream WavefrontObjFileOutputStream;

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

void CWaveFrontObjParser::ParseFile() {
    if (pWavefrontObjFileData == nullptr) {
        return;
    }
    while (pWavefrontObjFileData[uiCounter] != '\0') {
        GLVM::core::vector<vector<char>> line =
                Split(pWavefrontObjFileData, ' ', '\n', uiCounter);

        if (line[0] == "v") {
            SVertex vertex = ParseVertices(line);
            coordinateVertices_.Push(vertex);
        }
        if (line[0] == "vt") {
            SVertex vertex = ParseVertices(line);
            textureVertices_.Push(vertex);
        }
        if (line[0] == "vn") {
            SVertex vertex = ParseVertices(line);
            normals_.Push(vertex);
        }
        if (line[0] == "f") {
            SFace face = ParseFaces(line);
            faces_.Push(face);
        }
    }
}

GLVM::core::vector<vector<char>>
CWaveFrontObjParser::Split(const char *_pWaveFrontObjFileData,
                           const char _separator, const char _exitSymbol,
                           unsigned int &_uiCounter) {
    GLVM::core::vector<vector<char>> wordsContainer;
    unsigned int outerIndex = 0;
    wordsContainer.Push({});

    for (;; ++_uiCounter) {
        if (_pWaveFrontObjFileData[_uiCounter] == '#') {
            while (_pWaveFrontObjFileData[_uiCounter] != '\n') {
                ++_uiCounter;
            }
            continue;
        }
        if (_pWaveFrontObjFileData[_uiCounter] == _separator) {
            wordsContainer[outerIndex].Push('\0');
            wordsContainer.Push({});
            ++outerIndex;
            continue;
        }
        if (_pWaveFrontObjFileData[_uiCounter] == _exitSymbol) {
            ++_uiCounter;
            wordsContainer[outerIndex].Push('\0');
            return wordsContainer;
        }
        wordsContainer[outerIndex].Push(_pWaveFrontObjFileData[_uiCounter]);
    }
}

SVertex CWaveFrontObjParser::ParseVertices(
        GLVM::core::vector<vector<char>> _wordsContainer) {
    SVertex vertex;
    unsigned int uiVertexIndex = 0;

    unsigned int uiWordsContainerSize = _wordsContainer.GetSize();
    for (unsigned int i = 1; i < uiWordsContainerSize; ++i) {
        float floatNumber = ParseFloating(_wordsContainer[i]);
        vertex[uiVertexIndex++] = floatNumber;
    }

    return vertex;
}

SFace CWaveFrontObjParser::ParseFaces(
        GLVM::core::vector<vector<char>> _wordsContainer) {
    SFace face;
    GLVM::core::vector<vector<char>> wordsInnerContainer;
    GLVM::core::vector<char> word;

    unsigned int uiWordsContainerSize = _wordsContainer.GetSize();

    for (unsigned int i = 1; i < uiWordsContainerSize; ++i) {
        unsigned int counter = 0;
        wordsInnerContainer = Split(_wordsContainer[i].GetVectorContainer(),
                                    '/', '\0', counter);

        for (unsigned int j = 0; j < wordsInnerContainer.GetSize(); ++j) {

            word = wordsInnerContainer[j];
            int iValue = ParseInteger(word);

            face[j].Push(iValue);
        }
    }
    return face;
}

int CWaveFrontObjParser::ParseInteger(GLVM::core::vector<char> _word) {
    GLVM::core::vector<int> baseContainer;

    for (unsigned int i = 0; i < _word.GetSize() - 1; ++i) {
        baseContainer.Push(_word[i] - 48);
    }

    int iResult = 0;
    bool negateFlag = false;

    unsigned int baseContainerSize = baseContainer.GetSize();
    for (unsigned int i = 0; i < baseContainerSize; ++i) {
        if (negateFlag && i == 0) {
            continue;
        } else if (baseContainer[i] == -5 && i == 0) {
            continue;
        }

        iResult += baseContainer[i] * std::pow(10, (baseContainerSize - 1) - i);
    }

    return iResult;
}

float CWaveFrontObjParser::ParseFloating(GLVM::core::vector<char> _word) {
    GLVM::core::vector<int> baseContainer;

    for (unsigned int i = 0; i < _word.GetSize() - 1; ++i) {
        baseContainer.Push(_word[i] - 48);
    }

    int integerPart = 0;
    float floatingPart = 0;
    GLVM::core::vector<int> integerPartContainer;
    GLVM::core::vector<int> floatingPartContainer;
    bool dotFlag = false;
    bool negateFlag = false;
    unsigned int baseContainerSize = baseContainer.GetSize();

    if (baseContainer[0] == -3) {
        negateFlag = true;
    }

    for (unsigned int i = 0; i < baseContainerSize; ++i) {
        if (negateFlag && i == 0) {
            continue;
        } else if (baseContainer[i] == -5 && i == 0) {
            continue;
        } else if (baseContainer[i] == -2) {
            dotFlag = true;
            continue;
        }

        if (baseContainer[i] >= 0 && baseContainer[i] <= 9) {
            if (dotFlag) {
                floatingPartContainer.Push(baseContainer[i]);
            } else {
                integerPartContainer.Push(baseContainer[i]);
            }
        } else {
            return NAN;
        }
    }

    unsigned int integerPartContainerSize = integerPartContainer.GetSize();
    for (unsigned int i = 0; i < integerPartContainerSize; ++i) {
        integerPart += integerPartContainer[i] *
                       std::pow(10, (integerPartContainerSize - 1) - i);
    }

    unsigned int floatingPartContainerSize = floatingPartContainer.GetSize();
    for (unsigned int i = 0; i < floatingPartContainerSize; ++i) {
        floatingPart += floatingPartContainer[i] / std::pow(10, i + 1);
    }

    float result = 0;
    result = (float)(integerPart + floatingPart);

    if (negateFlag) {
        result *= -1.0f;
    }

    return result;
}
} // namespace GLVM::core
