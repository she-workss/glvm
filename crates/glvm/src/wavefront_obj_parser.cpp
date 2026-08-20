#include "glvm/wavefront_obj_parser.hpp"

#include <chrono>
#include <cstring>
#include <iterator>
#include <vector>

namespace glvm::core {
static bool equalsCStr(const std::vector<char>& v, const char* s) {
    return strcmp(v.data(), s) == 0;
}

CWaveFrontObjParser::CWaveFrontObjParser() {}

const std::vector<SVertex>& CWaveFrontObjParser::getCoordinateVertices() const {
    return coordinateVertices_;
}

const std::vector<SVertex>& CWaveFrontObjParser::getTextureVertices() const {
    return textureVertices_;
}

const std::vector<SVertex>& CWaveFrontObjParser::getNormals() const {
    return normals_;
}

const std::vector<SFace>& CWaveFrontObjParser::getFaces() const {
    return faces_;
}

void CWaveFrontObjParser::ReadFile(const char* _filePath) {
    const char* _pWavefrontObjFile = _filePath;
    std::ifstream WavefrontObjFileInputStream;
    std::stringstream WavefrontObjFileOutputStream;

    WavefrontObjFileInputStream.open(_pWavefrontObjFile);
    if (WavefrontObjFileInputStream.good()) {
        WavefrontObjFileOutputStream << WavefrontObjFileInputStream.rdbuf();
        WavefrontObjFileInputStream.close();
        sWavefrontObjFileData = WavefrontObjFileOutputStream.str();
    } else {
        std::cout << "Error of reading " << _filePath << " file" << std::endl;
        return;
    }

    pWavefrontObjFileData = sWavefrontObjFileData.c_str();
}

void CWaveFrontObjParser::ParseFile() {
    while (pWavefrontObjFileData[uiCounter] != '\0') {
        std::vector<std::vector<char>> line =
            Split(pWavefrontObjFileData, ' ', '\n', uiCounter);
        if (equalsCStr(line[0], "v")) {
            SVertex vertex = ParseVertices(line);
            coordinateVertices_.push_back(vertex);
        }
        if (equalsCStr(line[0], "vt")) {
            SVertex vertex = ParseVertices(line);
            textureVertices_.push_back(vertex);
        }
        if (equalsCStr(line[0], "vn")) {
            SVertex vertex = ParseVertices(line);
            normals_.push_back(vertex);
        }
        if (equalsCStr(line[0], "f")) {
            SFace face = ParseFaces(line);
            faces_.push_back(face);
        }
    }
}

std::vector<std::vector<char>> CWaveFrontObjParser::Split(
    const char* _pWaveFrontObjFileData,
    const char _separator,
    const char _exitSymbol,
    unsigned int& _uiCounter
) {
    std::vector<std::vector<char>> wordsContainer;
    unsigned int outerIndex = 0;
    wordsContainer.push_back({});

    for (;; ++_uiCounter) {
        if (_pWaveFrontObjFileData[_uiCounter] == '#') {
            while (_pWaveFrontObjFileData[_uiCounter] != '\n') {
                ++_uiCounter;
            }
            continue;
        }
        if (_pWaveFrontObjFileData[_uiCounter] == _separator) {
            wordsContainer[outerIndex].push_back('\0');
            wordsContainer.push_back({});
            ++outerIndex;
            continue;
        }
        if (_pWaveFrontObjFileData[_uiCounter] == _exitSymbol) {
            ++_uiCounter;
            wordsContainer[outerIndex].push_back('\0');
            return wordsContainer;
        }
        wordsContainer[outerIndex].push_back(_pWaveFrontObjFileData[_uiCounter]);
    }
}

SVertex CWaveFrontObjParser::ParseVertices(
    std::vector<std::vector<char>> _wordsContainer
) {
    SVertex vertex;
    unsigned int uiVertexIndex = 0;

    unsigned int uiWordsContainerSize = _wordsContainer.size();
    for (unsigned int i = 1; i < uiWordsContainerSize; ++i) {
        float floatNumber = ParseFloating(_wordsContainer[i]);
        vertex[uiVertexIndex++] = floatNumber;
    }

    return vertex;
}

SFace CWaveFrontObjParser::ParseFaces(
    std::vector<std::vector<char>> _wordsContainer
) {
    SFace face;
    std::vector<std::vector<char>> wordsInnerContainer;
    std::vector<char> word;

    unsigned int uiWordsContainerSize = _wordsContainer.size();

    for (unsigned int i = 1; i < uiWordsContainerSize; ++i) {
        unsigned int counter = 0;
        wordsInnerContainer =
            Split(_wordsContainer[i].data(), '/', '\0', counter);

        for (unsigned int j = 0; j < wordsInnerContainer.size(); ++j) {
            word = wordsInnerContainer[j];
            int iValue = ParseInteger(word);

            face[j].push_back(iValue);
        }
    }
    return face;
}

int CWaveFrontObjParser::ParseInteger(std::vector<char> _word) {
    std::vector<int> baseContainer;

    for (unsigned int i = 0; i < _word.size() - 1; ++i) {
        baseContainer.push_back(_word[i] - 48);
    }

    int iResult = 0;
    bool negateFlag = false;

    unsigned int baseContainerSize = baseContainer.size();
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

float CWaveFrontObjParser::ParseFloating(std::vector<char> _word) {
    std::vector<int> baseContainer;

    for (unsigned int i = 0; i < _word.size() - 1; ++i) {
        baseContainer.push_back(_word[i] - 48);
    }

    int integerPart = 0;
    float floatingPart = 0;
    std::vector<int> integerPartContainer;
    std::vector<int> floatingPartContainer;
    bool dotFlag = false;
    bool negateFlag = false;
    unsigned int baseContainerSize = baseContainer.size();

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
                floatingPartContainer.push_back(baseContainer[i]);
            } else {
                integerPartContainer.push_back(baseContainer[i]);
            }
        } else {
            std::cout << "Element is not a number" << std::endl;
            return NAN;
        }
    }

    unsigned int integerPartContainerSize = integerPartContainer.size();
    for (unsigned int i = 0; i < integerPartContainerSize; ++i) {
        integerPart += integerPartContainer[i]
            * std::pow(10, (integerPartContainerSize - 1) - i);
    }

    unsigned int floatingPartContainerSize = floatingPartContainer.size();
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
} // namespace glvm::core
