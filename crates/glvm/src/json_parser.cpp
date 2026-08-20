#include "glvm/json_parser.hpp"

#include <cassert>
#include <chrono>
#include <cstdint>
#include <limits>
#include <ostream>
#include <pthread.h>
#include <vector>

namespace glvm::Core {

void CJsonParser::ReadFile(const char* _filePath) {
    const char* _pJsonFilePath = _filePath;
    std::ifstream jsonFileInputStream;
    std::stringstream jsonFileOutputStream;

    jsonFileInputStream.open(_pJsonFilePath);
    if (jsonFileInputStream.good()) {
        jsonFileOutputStream << jsonFileInputStream.rdbuf();
        jsonFileInputStream.close();
        sJsonFileData_ = jsonFileOutputStream.str();
    } else {
        std::cout << "Error of reading json file" << std::endl;
        return;
    }

    pJsonFileData_ = sJsonFileData_.c_str();
}

void CJsonParser::Parse() {
    currentChar_ = pJsonFileData_[globalFileCounter_];

    while (currentChar_ != '\0') {
        currentChar_ = pJsonFileData_[globalFileCounter_];

        if (currentChar_ == '"' && keyFlag) {
            lastKey_ = StringParse();
            while (currentChar_ == ' ' || currentChar_ == ':') {
                ++globalFileCounter_;
                currentChar_ = pJsonFileData_[globalFileCounter_];
            }
        }

        if (currentChar_ == '"') {
            bufferString_ = StringParse();

            if (keyFlag) {
                JsonValue jsonString(bufferString_);
                (*stackOfJsonValues_.back()->value.object)[lastKey_.c_str()] =
                    jsonString;
            } else {
                JsonValue jsonString(bufferString_);
                stackOfJsonValues_.back()->value.array->push_back(jsonString);
            }
        } else if (
            (currentChar_ >= '0' && currentChar_ <= '9') || currentChar_ == '+'
            || currentChar_ == '-'
        ) {
            bufferString_ = NumberAsStringParse();
            std::vector<char> vector = StringToVectorOfChars(bufferString_);
            double fNumber = 0.0f;
            int iNumber = 0;
            if (IsContainChar(bufferString_, '.')) {
                fNumber = ParseFloating(vector);

                if (keyFlag) {
                    JsonValue jsonFloat(fNumber);
                    (*stackOfJsonValues_.back()
                          ->value.object)[lastKey_.c_str()] = jsonFloat;
                } else {
                    JsonValue jsonFloat(fNumber);
                    stackOfJsonValues_.back()->value.array->push_back(jsonFloat);
                }
            } else {
                iNumber = ParseInteger(vector);

                if (keyFlag) {
                    JsonValue jsonInt(iNumber);
                    (*stackOfJsonValues_.back()
                          ->value.object)[lastKey_.c_str()] = jsonInt;
                } else {
                    JsonValue jsonInt(iNumber);
                    stackOfJsonValues_.back()->value.array->push_back(jsonInt);
                }
            }

        } else if (
            currentChar_ == 't' || currentChar_ == 'f' || currentChar_ == 'n'
        ) {
            std::string boolOrNullString = BoolOrNullParse();

            if (boolOrNullString == "true") {
                if (keyFlag) {
                    JsonValue jsonTrue(true);
                    (*stackOfJsonValues_.back()
                          ->value.object)[lastKey_.c_str()] = jsonTrue;
                } else {
                    JsonValue jsonTrue(true);
                    stackOfJsonValues_.back()->value.array->push_back(jsonTrue);
                }
            } else if (boolOrNullString == "false") {
                if (keyFlag) {
                    JsonValue jsonFalse(false);
                    (*stackOfJsonValues_.back()
                          ->value.object)[lastKey_.c_str()] = jsonFalse;
                } else {
                    JsonValue jsonFalse(false);
                    stackOfJsonValues_.back()->value.array->push_back(jsonFalse);
                }
            } else if (boolOrNullString == "null") {
                if (keyFlag) {
                    JsonValue jsonNull;
                    jsonNull.type = JSON_NULL;
                    jsonNull.value.null = NULL;
                    (*stackOfJsonValues_.back()
                          ->value.object)[lastKey_.c_str()] = jsonNull;
                } else {
                    JsonValue jsonNull;
                    jsonNull.type = JSON_NULL;
                    jsonNull.value.null = NULL;
                    stackOfJsonValues_.back()->value.array->push_back(jsonNull);
                }
            }
        } else if (currentChar_ == '{') {
            if (stackOfJsonValues_.size() == 0) {
                root_ = new JsonValue;
                *root_ = CreateJsonHashMap();
                stackOfJsonValues_.push_back(root_);
            } else if (keyFlag) {
                JsonValue jsonObject = CreateJsonHashMap();
                (*stackOfJsonValues_.back()->value.object)[lastKey_.c_str()] =
                    jsonObject;
                stackOfJsonValues_.push_back(&(
                    *stackOfJsonValues_.back()->value.object
                )[lastKey_.c_str()]);
            } else if (!keyFlag) {
                JsonValue jsonObject = CreateJsonHashMap();
                stackOfJsonValues_.back()->value.array->push_back(jsonObject);
                stackOfJsonValues_.push_back(
                    &stackOfJsonValues_.back()->value.array->back()
                );
            }

            keyFlag = true;
        } else if (currentChar_ == '[') {
            if (stackOfJsonValues_.size() == 0) {
                root_ = new JsonValue;
                *root_ = CreateJsonArray();
                stackOfJsonValues_.push_back(root_);
            } else if (keyFlag) {
                JsonValue jsonArray = CreateJsonArray();
                (*stackOfJsonValues_.back()->value.object)[lastKey_.c_str()] =
                    jsonArray;
                stackOfJsonValues_.push_back(&(
                    *stackOfJsonValues_.back()->value.object
                )[lastKey_.c_str()]);
            } else if (!keyFlag) {
                JsonValue jsonArray = CreateJsonArray();
                stackOfJsonValues_.back()->value.array->push_back(jsonArray);
                stackOfJsonValues_.push_back(
                    &stackOfJsonValues_.back()->value.array->back()
                );
            }

            keyFlag = false;
        } else if (currentChar_ == '}') {
            stackOfJsonValues_.pop_back();
            if (stackOfJsonValues_.size()
                && stackOfJsonValues_.back()->type == JSON_OBJECT) {
                keyFlag = true;
            } else {
                keyFlag = false;
            }

        } else if (currentChar_ == ']') {
            stackOfJsonValues_.pop_back();
            if (stackOfJsonValues_.size()
                && stackOfJsonValues_.back()->type == JSON_OBJECT) {
                keyFlag = true;
            } else {
                keyFlag = false;
            }
        }

        ++globalFileCounter_;
    }
}

JsonValue CJsonParser::CreateJsonHashMap() {
    JsonValue jsonObject;
    jsonObject.type = JSON_OBJECT;
    jsonObject.value.object = new HashMap<JsonValue>;
    return jsonObject;
}

JsonValue CJsonParser::CreateJsonArray() {
    JsonValue jsonArray;
    jsonArray.type = JSON_ARRAY;
    jsonArray.value.array = new std::vector<JsonValue>;
    return jsonArray;
}

std::string CJsonParser::BoolOrNullParse() {
    std::string boolOrNullString = "";
    while (1) {
        currentChar_ = pJsonFileData_[globalFileCounter_];
        if (currentChar_ >= 'a' && currentChar_ <= 'z') {
            boolOrNullString.push_back(currentChar_);
            ++globalFileCounter_;
        } else {
            return boolOrNullString;
        }
    }
}

bool CJsonParser::IsContainChar(std::string _string, char _char) {
    for (unsigned int i = 0; i < _string.size(); ++i) {
        if (_string[i] == _char) {
            return true;
        }
    }

    return false;
}

std::string CJsonParser::NumberAsStringParse() {
    std::string numberAsString = "";
    while (1) {
        currentChar_ = pJsonFileData_[globalFileCounter_];
        if ((currentChar_ >= '0' && currentChar_ <= '9') || currentChar_ == '+'
            || currentChar_ == '-' || currentChar_ == 'e') {
            numberAsString.push_back(currentChar_);
            ++globalFileCounter_;
        } else if (currentChar_ == '.') {
            numberAsString.push_back(currentChar_);
            ++globalFileCounter_;
        } else {
            return numberAsString;
        }
    }
}

std::string CJsonParser::StringParse() {
    ++globalFileCounter_;
    std::string localBuffer = "";
    while (1) {
        currentChar_ = pJsonFileData_[globalFileCounter_];
        if (currentChar_ == '"') {
            ++globalFileCounter_;
            currentChar_ = pJsonFileData_[globalFileCounter_];
            return localBuffer;
        } else {
            localBuffer.push_back(currentChar_);
            ++globalFileCounter_;
        }
    }
}

std::vector<char> CJsonParser::StringToVectorOfChars(std::string _string) {
    std::vector<char> vectorWithChars;
    for (unsigned int i = 0; i < _string.size(); ++i) {
        vectorWithChars.push_back(_string[i]);
    }

    return vectorWithChars;
}

int CJsonParser::ParseInteger(std::vector<char> _word) {
    std::vector<int> baseContainer;

    for (unsigned int i = 0; i < _word.size(); ++i) {
        baseContainer.push_back(_word[i] - 48);
    }

    int iResult = 0;
    bool negateFlag = false;

    unsigned int baseContainerSize = baseContainer.size();
    for (unsigned int i = 0; i < baseContainerSize; ++i) {
        if (baseContainer[i] == -3 && i == 0) {
            negateFlag = true;
            continue;
        } else if (baseContainer[i] == -5 && i == 0) {
            continue;
        }

        iResult += baseContainer[i] * std::pow(10, (baseContainerSize - 1) - i);
    }

    if (negateFlag) {
        iResult *= -1;
    }

    return iResult;
}

double CJsonParser::ParseFloating(std::vector<char> _word) {
    std::vector<int> baseContainer;

    for (unsigned int i = 0; i < _word.size(); ++i) {
        baseContainer.push_back(_word[i] - 48);
    }

    int integerPart = 0;
    double floatingPart = 0;
    int eNumber = 0;
    std::vector<int> integerPartContainer;
    std::vector<int> floatingPartContainer;
    std::vector<int> ePartContainer;
    bool dotFlag = false;
    bool negateFlag = false;
    bool eFlag = false;
    // False value equal "+" sign.
    bool eSign = false;
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
        } else if (baseContainer[i] == 53) {
            eFlag = true;
            continue;
        }

        if (eFlag) {
            if (baseContainer[i] == -5) {
                continue;
            } else if (baseContainer[i] == -3) {
                eSign = true;
                continue;
            }

            ePartContainer.push_back(baseContainer[i]);
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

    unsigned int ePartContainerSize = ePartContainer.size();
    for (unsigned int i = 0; i < ePartContainerSize; ++i) {
        eNumber +=
            ePartContainer[i] * std::pow(10, (ePartContainerSize - 1) - i);
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

    double result = 0;
    result = (double)(integerPart + floatingPart);

    if (eFlag) {
        if (eSign) {
            result /= std::pow(10, eNumber);
        } else {
            result *= std::pow(10, eNumber);
        }
    }

    if (negateFlag) {
        result *= -1.0f;
    }

    return result;
}

void CJsonParser::SearchInJsonArray(
    std::vector<JsonValue>* arrayValue,
    const char* key_,
    std::vector<JsonValue>& resultVector
) const {
    for (unsigned int i = 0; i < arrayValue->size(); ++i) {
        if ((*arrayValue)[i].type == JSON_OBJECT) {
            SearchInJsonObject(
                (*arrayValue)[i].value.object,
                key_,
                resultVector
            );
        }

        if ((*arrayValue)[i].type == JSON_ARRAY) {
            SearchInJsonArray((*arrayValue)[i].value.array, key_, resultVector);
        }
    }
}

void CJsonParser::SearchInJsonObject(
    HashMap<JsonValue>* mapValue,
    const char* key_,
    std::vector<JsonValue>& resultVector
) const {
    for (unsigned int i = 0; i < mapValue->GetCapacity(); ++i) {
        if (mapValue->hashMap_[i] != nullptr) {
            Node<JsonValue>* current = mapValue->hashMap_[i];
            while (current != nullptr) {
                std::string searchKey = key_;
                std::string currentKey = current->key_;
                if (currentKey == searchKey) {
                    resultVector.push_back(current->value_);
                }

                if (current->value_.type == JSON_OBJECT) {
                    SearchInJsonObject(
                        current->value_.value.object,
                        key_,
                        resultVector
                    );
                }

                if (current->value_.type == JSON_ARRAY) {
                    SearchInJsonArray(
                        current->value_.value.array,
                        key_,
                        resultVector
                    );
                }

                current = current->next_;
            }
        }
    }
}

std::vector<JsonValue> CJsonParser::Search(const char* key_) const {
    std::vector<JsonValue> resultVector;
    SearchInJsonObject(root_->value.object, key_, resultVector);
    return resultVector;
}

template<typename T>
bool isElementExist(const T element, const std::vector<T>& array) {
    for (uint32_t n = 0; n < array.size(); ++n) {
        if (element == array[n]) {
            return true;
        }
    }

    return false;
}

template<typename T>
T getElementIndex(const T element, const std::vector<T>& array) {
    for (uint32_t n = 0; n < array.size(); ++n) {
        if (element == array[n]) {
            return n;
        }
    }

    return std::numeric_limits<T>::max();
}

void calculateElementsMemorySize(
    const uint32_t indices_elements_count,
    std::string* indices_element_type,
    const uint32_t indices_componet_type,
    uint32_t* indices_buffer_view_byte_length
) {
    if (*indices_element_type == "VEC2") {
        if (indices_componet_type == 5120 || indices_componet_type == 5121) {
            *indices_buffer_view_byte_length = indices_elements_count * 2;
        } else if (indices_componet_type == 5122 || indices_componet_type == 5123) {
            *indices_buffer_view_byte_length = indices_elements_count * 4;
        } else if (indices_componet_type == 5125 || indices_componet_type == 5126) {
            *indices_buffer_view_byte_length = indices_elements_count * 8;
        }
    } else if (*indices_element_type == "VEC3") {
        if (indices_componet_type == 5120 || indices_componet_type == 5121) {
            *indices_buffer_view_byte_length = indices_elements_count * 3;
        } else if (indices_componet_type == 5122 || indices_componet_type == 5123) {
            *indices_buffer_view_byte_length = indices_elements_count * 6;
        } else if (indices_componet_type == 5125 || indices_componet_type == 5126) {
            *indices_buffer_view_byte_length = indices_elements_count * 12;
        }
    } else if (*indices_element_type == "VEC4") {
        if (indices_componet_type == 5120 || indices_componet_type == 5121) {
            *indices_buffer_view_byte_length = indices_elements_count * 4;
        } else if (indices_componet_type == 5122 || indices_componet_type == 5123) {
            *indices_buffer_view_byte_length = indices_elements_count * 8;
        } else if (indices_componet_type == 5125 || indices_componet_type == 5126) {
            *indices_buffer_view_byte_length = indices_elements_count * 16;
        }
    } else if (*indices_element_type == "SCALAR") {
        if (indices_componet_type == 5120 || indices_componet_type == 5121) {
            *indices_buffer_view_byte_length = indices_elements_count;
        } else if (indices_componet_type == 5122 || indices_componet_type == 5123) {
            *indices_buffer_view_byte_length = indices_elements_count * 2;
        } else if (indices_componet_type == 5125 || indices_componet_type == 5126) {
            *indices_buffer_view_byte_length = indices_elements_count * 4;
        }
    } else if (*indices_element_type == "MAT4") {
        *indices_buffer_view_byte_length = indices_elements_count * 64;
    }
}

struct ComponentType {
    enum Type {
        I8 = 5120,
        U8 = 5121,
        I16 = 5122,
        U16 = 5123,
        U32 = 5125,
        F32 = 5126
    };
};

void calculateByteStep(uint32_t componetType, unsigned int* byteStep) {
    if (componetType == ComponentType::I8
        || componetType == ComponentType::U8) {
        *byteStep = 1;
    } else if (
        componetType == ComponentType::I16 || componetType == ComponentType::U16
    ) {
        *byteStep = 2;
    } else if (
        componetType == ComponentType::U32 || componetType == ComponentType::F32
    ) {
        *byteStep = 4;
    }
}

// Metadata structs to binary buffer with actual data.
struct AccessorMetaData {
    uint32_t bufferView;
    uint32_t byteOffset;
    uint32_t componentType;
    uint32_t count;
    std::string type;
};

struct BufferViewMetaData {
    uint32_t byteLength;
    uint32_t byteOffset;
};

[[nodiscard]] AccessorMetaData readAccessorMetaData(
    Core::JsonValue* gltf,
    const uint32_t accessorIndex
) {
    AccessorMetaData bufferMetaData;
    bufferMetaData.bufferView =
        (*gltf)["accessors"][accessorIndex]["bufferView"].value.iNumber;
    bufferMetaData.count =
        (*gltf)["accessors"][accessorIndex]["count"].value.iNumber;
    bufferMetaData.type =
        *(*gltf)["accessors"][accessorIndex]["type"].value.string;
    bufferMetaData.componentType =
        (*gltf)["accessors"][accessorIndex]["componentType"].value.iNumber;

    bufferMetaData.byteOffset = 0;
    if ((*gltf)["accessors"][accessorIndex].isObject() == JSON_OBJECT) {
        HashMap<JsonValue>* ptr =
            (*gltf)["accessors"][accessorIndex].value.object;
        if (ptr->Contain("byteOffset")) {
            bufferMetaData.byteOffset =
                (*gltf)["accessors"][accessorIndex]["byteOffset"].value.iNumber;
        }
    }

    return bufferMetaData;
}

[[nodiscard]] BufferViewMetaData readBufferViewMetaData(
    Core::JsonValue* gltf,
    const uint32_t bufferViewIndex
) {
    BufferViewMetaData bufferViewMetaData;
    bufferViewMetaData.byteLength =
        (*gltf)["bufferViews"][bufferViewIndex]["byteLength"].value.iNumber;
    bufferViewMetaData.byteOffset = 0;
    if ((*gltf)["bufferViews"][bufferViewIndex].isObject() == JSON_OBJECT) {
        HashMap<JsonValue>* ptr =
            (*gltf)["bufferViews"][bufferViewIndex].value.object;
        if (ptr->Contain("byteOffset")) {
            bufferViewMetaData.byteOffset =
                (*gltf)["bufferViews"][bufferViewIndex]["byteOffset"]
                    .value.iNumber;
        }
    }

    return bufferViewMetaData;
}

template<typename T>
void readBinaryBufferData(
    char* buffer,
    AccessorMetaData accessorMetaData,
    BufferViewMetaData bufferViewMetaData,
    std::vector<T>& outputData
) {
    uint32_t indicesByteStep = 0;
    calculateByteStep(accessorMetaData.componentType, &indicesByteStep);

    unsigned int byteLength = 0;
    calculateElementsMemorySize(
        accessorMetaData.count,
        &accessorMetaData.type,
        accessorMetaData.componentType,
        &byteLength
    );
    for (unsigned int i =
             bufferViewMetaData.byteOffset + accessorMetaData.byteOffset;
         i < bufferViewMetaData.byteOffset + accessorMetaData.byteOffset
             + byteLength;
         i += indicesByteStep) {
        switch (accessorMetaData.componentType) {
            case ComponentType::U8:
                outputData.push_back(
                    reinterpret_cast<unsigned char&>(buffer[i])
                );
                break;
            case ComponentType::U16:
                outputData.push_back(
                    reinterpret_cast<unsigned short&>(buffer[i])
                );
                break;
            case ComponentType::U32:
                outputData.push_back(reinterpret_cast<unsigned int&>(buffer[i]));
                break;
            case ComponentType::F32:
                outputData.push_back(reinterpret_cast<float&>(buffer[i]));
                break;
        }
    }
}

void CJsonParser::LoadGLTF(
    const char* pathsGLTF_,
    std::vector<float>& aVertexes_,
    std::vector<uint32_t>& aIndices_,
    std::vector<std::vector<Matrix<float, 4>>>& jointMatricesPerMesh,
    std::vector<float>& frames,
    bool& noAnimations,
    float& topY
) {
    std::cout << "path: " << pathsGLTF_ << std::endl;
    ReadFile(pathsGLTF_);
    Parse();

    Core::JsonValue* gltf = GetRoot();
    std::string binary_path = *(*gltf)["buffers"][0]["uri"].value.string;
    int full_byte_size = (*gltf)["buffers"][0]["byteLength"].value.iNumber;
    ;
    size_t lastSeparator = std::string(pathsGLTF_).find_last_of("/\\");
    std::string binary_full_path = lastSeparator == std::string::npos
        ? binary_path
        : std::string(pathsGLTF_).substr(0, lastSeparator + 1) + binary_path;
    std::ifstream in_stream;
    in_stream.open(binary_full_path, std::ios::binary);
    if (!in_stream.is_open()) {
        throw std::runtime_error(
            "failed to open gltf binary buffer: " + binary_full_path
        );
    }
    char* buffer = new char[full_byte_size];
    in_stream.read(buffer, full_byte_size);
    in_stream.close();

    const uint32_t indicesAccessorIndex =
        (*gltf)["meshes"][0]["primitives"][0]["indices"].value.iNumber;
    AccessorMetaData indicesAccessorMetaData =
        readAccessorMetaData(gltf, indicesAccessorIndex);
    BufferViewMetaData indicesBufferViewMetaData =
        readBufferViewMetaData(gltf, indicesAccessorMetaData.bufferView);
    std::vector<uint32_t> indices;
    readBinaryBufferData(
        buffer,
        indicesAccessorMetaData,
        indicesBufferViewMetaData,
        indices
    );

    const uint32_t verticesPositionAccessorIndex =
        (*gltf)["meshes"][0]["primitives"][0]["attributes"]["POSITION"]
            .value.iNumber;
    AccessorMetaData verticesPositionAccessorMetaData =
        readAccessorMetaData(gltf, verticesPositionAccessorIndex);
    BufferViewMetaData verticesPositionBufferViewMetaData =
        readBufferViewMetaData(
            gltf,
            verticesPositionAccessorMetaData.bufferView
        );
    std::vector<float> verticesPosition;
    readBinaryBufferData(
        buffer,
        verticesPositionAccessorMetaData,
        verticesPositionBufferViewMetaData,
        verticesPosition
    );

    const uint32_t textureCoordinatesAccessorIndex =
        (*gltf)["meshes"][0]["primitives"][0]["attributes"]["TEXCOORD_0"]
            .value.iNumber;
    AccessorMetaData textureCoordinatesAccessorMetaData =
        readAccessorMetaData(gltf, textureCoordinatesAccessorIndex);
    BufferViewMetaData textureCoordinatesBufferViewMetaData =
        readBufferViewMetaData(
            gltf,
            textureCoordinatesAccessorMetaData.bufferView
        );
    std::vector<float> textureCoordinates;
    readBinaryBufferData(
        buffer,
        textureCoordinatesAccessorMetaData,
        textureCoordinatesBufferViewMetaData,
        textureCoordinates
    );

    const uint32_t normalsAccessorIndex =
        (*gltf)["meshes"][0]["primitives"][0]["attributes"]["NORMAL"]
            .value.iNumber;
    AccessorMetaData normalsAccessorMetaData =
        readAccessorMetaData(gltf, normalsAccessorIndex);
    BufferViewMetaData normalsBufferViewMetaData =
        readBufferViewMetaData(gltf, normalsAccessorMetaData.bufferView);
    std::vector<float> normals;
    readBinaryBufferData(
        buffer,
        normalsAccessorMetaData,
        normalsBufferViewMetaData,
        normals
    );

    std::vector<Core::JsonValue> skins = Search("skins");
    Core::JsonValue joints;
    std::vector<Matrix<float, 4>> globalTransformJointNode;
    std::vector<Matrix<float, 4>> inverseBindMatrixSet;
    std::vector<std::vector<Matrix<float, 4>>> jointMatrices;
    std::vector<float> weightsContainer;
    std::vector<int> jointsIndices;
    std::vector<std::vector<int>> children;

    if (skins.size() > 0) {
        noAnimations = false;
        joints = (*gltf)["skins"][0]["joints"];

        Core::JsonValue nodes = (*gltf)["nodes"];
        // Loop on joints.
        for (unsigned int i = 0; i < joints.value.array->size(); ++i) {
            unsigned int jointIndexMapToNode =
                (*joints.value.array)[i].value.iNumber;
            Core::JsonValue node = nodes[jointIndexMapToNode];
            Quaternion rotationQuaternion;
            Matrix<float, 4> rotation(1.0f);
            Matrix<float, 4> scale(1.0f);
            Matrix<float, 4> translation(1.0f);

            if (node.value.object->Contain("rotation")) {
                Core::JsonValue array = (*node.value.object)["rotation"];
                for (unsigned int i = 0; i < array.value.array->size(); ++i) {
                    switch (i) {
                        case 0:
                            if (array[i].isInterger()) {
                                rotationQuaternion.x = array[i].value.iNumber;
                            } else if (array[i].isFloat()) {
                                rotationQuaternion.x = array[i].value.fNumber;
                            }
                            break;
                        case 1:
                            if (array[i].isInterger()) {
                                rotationQuaternion.y = array[i].value.iNumber;
                            } else if (array[i].isFloat()) {
                                rotationQuaternion.y = array[i].value.fNumber;
                            }
                            break;
                        case 2:
                            if (array[i].isInterger()) {
                                rotationQuaternion.z = array[i].value.iNumber;
                            } else if (array[i].isFloat()) {
                                rotationQuaternion.z = array[i].value.fNumber;
                            }
                            break;
                        case 3:
                            if (array[i].isInterger()) {
                                rotationQuaternion.w = array[i].value.iNumber;
                            } else if (array[i].isFloat()) {
                                rotationQuaternion.w = array[i].value.fNumber;
                            }
                            break;
                    }
                }

                rotation = rotateQuaternion<float, 4>(rotationQuaternion);
                rotation.SelfTensorTranspose();
            }

            std::vector<int> local_children;
            // Collect children indices.
            if (node.value.object->Contain("children")) {
                Core::JsonValue array = (*node.value.object)["children"];
                for (unsigned int i = 0; i < array.value.array->size(); ++i) {
                    local_children.push_back(array[i].value.iNumber);
                }
                // Linearly put all children to every root joint.
                children.push_back(local_children);
            } else {
                std::vector<int> emptyChildren;
                // Put empty pack of children if can find a one.
                children.push_back(emptyChildren);
            }

            if (node.value.object->Contain("scale")) {
                Core::JsonValue array = (*node.value.object)["scale"];
                for (unsigned int i = 0; i < array.value.array->size(); ++i) {
                    if (array[i].isInterger()) {
                        scale[i][i] = array[i].value.iNumber;
                    } else if (array[i].isFloat()) {
                        scale[i][i] = array[i].value.fNumber;
                    }
                }
            }

            if (node.value.object->Contain("translation")) {
                Core::JsonValue array = (*node.value.object)["translation"];
                for (unsigned int i = 0; i < array.value.array->size(); ++i) {
                    if (array[i].isInterger()) {
                        translation[3][i] = array[i].value.iNumber;
                    } else if (array[i].isFloat()) {
                        translation[3][i] = array[i].value.fNumber;
                    }
                }
            }

            // Compute model matrix.
            Matrix<float, 4> model = scale * rotation * translation;
            globalTransformJointNode.push_back(model);
        }

        // Get the inverse bind matrices accessor index.
        const uint32_t inverseBindMatricesAccessorIndex =
            (*gltf)["skins"][0]["inverseBindMatrices"].value.iNumber;
        AccessorMetaData inverseBindMatricesAccessorMetaData =
            readAccessorMetaData(gltf, inverseBindMatricesAccessorIndex);
        BufferViewMetaData inveresBindMatricesBufferViewMetaData =
            readBufferViewMetaData(
                gltf,
                inverseBindMatricesAccessorMetaData.bufferView
            );
        std::vector<float> inverseBindMatricesData;
        readBinaryBufferData(
            buffer,
            inverseBindMatricesAccessorMetaData,
            inveresBindMatricesBufferViewMetaData,
            inverseBindMatricesData
        );

        Matrix<float, 4> inverseBindMatrix(0.0f);
        for (unsigned int n = 0; n < joints.value.array->size(); ++n) {
            for (unsigned int g = 0; g < 4; ++g) {
                for (unsigned int j = 0; j < 4; ++j) {
                    // Put row float data into mat4.
                    inverseBindMatrix[g][j] =
                        inverseBindMatricesData[n * 16 + g * 4 + j];
                }
            }
            inverseBindMatrixSet.push_back(inverseBindMatrix);
        }

        const uint32_t jointsAccessorIndex =
            (*gltf)["meshes"][0]["primitives"][0]["attributes"]["JOINTS_0"]
                .value.iNumber;
        AccessorMetaData jointsAccessorMetaData =
            readAccessorMetaData(gltf, jointsAccessorIndex);
        BufferViewMetaData jointsBufferViewMetaData =
            readBufferViewMetaData(gltf, jointsAccessorMetaData.bufferView);
        readBinaryBufferData(
            buffer,
            jointsAccessorMetaData,
            jointsBufferViewMetaData,
            jointsIndices
        );

        unsigned int weightsAccessorIndex =
            (*gltf)["meshes"][0]["primitives"][0]["attributes"]["WEIGHTS_0"]
                .value.iNumber;
        AccessorMetaData weightsAccessorMetaData =
            readAccessorMetaData(gltf, weightsAccessorIndex);
        BufferViewMetaData weightsBufferViewMetaData =
            readBufferViewMetaData(gltf, weightsAccessorMetaData.bufferView);
        readBinaryBufferData(
            buffer,
            weightsAccessorMetaData,
            weightsBufferViewMetaData,
            weightsContainer
        );
    } else {
        noAnimations = true;
    }

    std::vector<Core::JsonValue> animations = Search("animations");

    if (animations.size() > 0) {
        std::vector<Core::JsonValue> samplerIndices;
        std::vector<Core::JsonValue> targetNodes;
        std::vector<Core::JsonValue> targetPaths;
        Core::JsonValue channels = (*gltf)["animations"][0]["channels"];
        for (unsigned int i = 0; i < channels.value.array->size(); ++i) {
            samplerIndices.push_back(channels[i]["sampler"]);
        }

        for (unsigned int i = 0; i < channels.value.array->size(); ++i) {
            targetNodes.push_back(channels[i]["target"]["node"]);
        }

        for (unsigned int i = 0; i < channels.value.array->size(); ++i) {
            targetPaths.push_back(channels[i]["target"]["path"]);
        }

        std::vector<unsigned int> translationSamplerIndices;
        std::vector<unsigned int> rotationSamplerIndices;
        std::vector<unsigned int> scaleSamplerIndices;
        std::vector<uint32_t> nodesMapTranslations;
        std::vector<uint32_t> nodesMapRotations;
        std::vector<uint32_t> nodesMapScales;
        for (unsigned int i = 0; i < samplerIndices.size(); ++i) {
            if (*targetPaths[i].value.string == "translation") {
                translationSamplerIndices.push_back(
                    samplerIndices[i].value.iNumber
                );
                nodesMapTranslations.push_back(targetNodes[i].value.iNumber);
            } else if (*targetPaths[i].value.string == "rotation") {
                rotationSamplerIndices.push_back(
                    samplerIndices[i].value.iNumber
                );
                nodesMapRotations.push_back(targetNodes[i].value.iNumber);
            } else if (*targetPaths[i].value.string == "scale") {
                scaleSamplerIndices.push_back(samplerIndices[i].value.iNumber);
                nodesMapScales.push_back(targetNodes[i].value.iNumber);
            }
        }

        Core::JsonValue samplers = (*gltf)["animations"][0]["samplers"];

        std::vector<unsigned int> translationInputs;
        std::vector<unsigned int> translationOutputs;

        for (unsigned int i = 0; i < translationSamplerIndices.size(); ++i) {
            translationInputs.push_back(
                samplers[translationSamplerIndices[i]]["input"].value.iNumber
            );
        }

        for (unsigned int i = 0; i < translationSamplerIndices.size(); ++i) {
            translationOutputs.push_back(
                samplers[translationSamplerIndices[i]]["output"].value.iNumber
            );
        }

        std::vector<std::vector<float>> frameInputsTranslation;
        for (unsigned int i = 0; i < translationInputs.size(); ++i) {
            AccessorMetaData frameInputsTranslationAccessorMetaData =
                readAccessorMetaData(gltf, translationInputs[i]);
            BufferViewMetaData frameInputsTranslationBufferViewMetaData =
                readBufferViewMetaData(
                    gltf,
                    frameInputsTranslationAccessorMetaData.bufferView
                );
            std::vector<float> temp;
            readBinaryBufferData(
                buffer,
                frameInputsTranslationAccessorMetaData,
                frameInputsTranslationBufferViewMetaData,
                temp
            );
            frameInputsTranslation.push_back(temp);
        }

        std::vector<std::vector<float>> translations;
        for (unsigned int i = 0; i < translationOutputs.size(); ++i) {
            AccessorMetaData frameOutputsTranslationAccessorMetaData =
                readAccessorMetaData(gltf, translationOutputs[i]);
            BufferViewMetaData frameOutputsTranslationBufferViewMetaData =
                readBufferViewMetaData(
                    gltf,
                    frameOutputsTranslationAccessorMetaData.bufferView
                );
            std::vector<float> temp;
            readBinaryBufferData(
                buffer,
                frameOutputsTranslationAccessorMetaData,
                frameOutputsTranslationBufferViewMetaData,
                temp
            );
            translations.push_back(temp);
        }

        std::vector<unsigned int> rotationInputs;
        std::vector<unsigned int> rotationOutputs;

        for (unsigned int i = 0; i < rotationSamplerIndices.size(); ++i) {
            rotationInputs.push_back(
                samplers[rotationSamplerIndices[i]]["input"].value.iNumber
            );
        }

        for (unsigned int i = 0; i < rotationSamplerIndices.size(); ++i) {
            rotationOutputs.push_back(
                samplers[rotationSamplerIndices[i]]["output"].value.iNumber
            );
        }

        std::vector<std::vector<float>> frameInputsRotation;
        for (unsigned int i = 0; i < rotationInputs.size(); ++i) {
            AccessorMetaData frameInputsRotationAccessorMetaData =
                readAccessorMetaData(gltf, rotationInputs[i]);
            BufferViewMetaData frameInputsRotationBufferViewMetaData =
                readBufferViewMetaData(
                    gltf,
                    frameInputsRotationAccessorMetaData.bufferView
                );
            std::vector<float> temp;
            readBinaryBufferData(
                buffer,
                frameInputsRotationAccessorMetaData,
                frameInputsRotationBufferViewMetaData,
                temp
            );
            frameInputsRotation.push_back(temp);
        }

        std::vector<std::vector<float>> rotations;
        for (unsigned int i = 0; i < rotationOutputs.size(); ++i) {
            AccessorMetaData frameOutputsRotationAccessorMetaData =
                readAccessorMetaData(gltf, rotationOutputs[i]);
            BufferViewMetaData frameOutputsRotationBufferViewMetaData =
                readBufferViewMetaData(
                    gltf,
                    frameOutputsRotationAccessorMetaData.bufferView
                );
            std::vector<float> temp;
            readBinaryBufferData(
                buffer,
                frameOutputsRotationAccessorMetaData,
                frameOutputsRotationBufferViewMetaData,
                temp
            );
            rotations.push_back(temp);
        }

        std::vector<unsigned int> scaleInputs;
        std::vector<unsigned int> scaleOutputs;

        for (unsigned int i = 0; i < scaleSamplerIndices.size(); ++i) {
            scaleInputs.push_back(
                samplers[scaleSamplerIndices[i]]["input"].value.iNumber
            );
        }

        for (unsigned int i = 0; i < scaleSamplerIndices.size(); ++i) {
            scaleOutputs.push_back(
                samplers[scaleSamplerIndices[i]]["output"].value.iNumber
            );
        }

        std::vector<std::vector<float>> frameInputsScale;
        for (unsigned int i = 0; i < scaleInputs.size(); ++i) {
            AccessorMetaData frameInputsScaleAccessorMetaData =
                readAccessorMetaData(gltf, scaleInputs[i]);
            BufferViewMetaData frameInputsScaleBufferViewMetaData =
                readBufferViewMetaData(
                    gltf,
                    frameInputsScaleAccessorMetaData.bufferView
                );
            std::vector<float> temp;
            readBinaryBufferData(
                buffer,
                frameInputsScaleAccessorMetaData,
                frameInputsScaleBufferViewMetaData,
                temp
            );
            frameInputsScale.push_back(temp);
        }

        std::vector<std::vector<float>> scales;
        for (unsigned int i = 0; i < scaleOutputs.size(); ++i) {
            AccessorMetaData frameOutputsScaleAccessorMetaData =
                readAccessorMetaData(gltf, scaleOutputs[i]);
            BufferViewMetaData frameOutputsScaleBufferViewMetaData =
                readBufferViewMetaData(
                    gltf,
                    frameOutputsScaleAccessorMetaData.bufferView
                );
            std::vector<float> temp;
            readBinaryBufferData(
                buffer,
                frameOutputsScaleAccessorMetaData,
                frameOutputsScaleBufferViewMetaData,
                temp
            );
            scales.push_back(temp);
        }

        // Searching for root joins.
        std::vector<int> rootNodes;
        for (unsigned int s = 0; s < joints.value.array->size(); ++s) {
            int current_joint = (*joints.value.array)[s].value.iNumber;

            for (unsigned w = 0; w < children.size(); ++w) {
                for (unsigned q = 0; q < children[w].size(); ++q) {
                    if (children[w][q] == current_joint) {
                        goto most_scary_operator_of_all_time;
                    }
                }
            }
            // If we execute this line then this joint index ectualy the root.
            rootNodes.push_back(current_joint);

        most_scary_operator_of_all_time: // Not so scary at all. Am i right?
            continue;
        }

        for (unsigned int i = 0; i < rootNodes.size(); ++i) {
            std::cout << "root joint: " << rootNodes[i] << std::endl;
        }

        std::vector<std::vector<unsigned int>> nodesHierarchy;
        // Loop on parent joints.
        for (unsigned int w = 0; w < rootNodes.size(); ++w) {
            std::vector<std::vector<unsigned int>> nodes_bones;
            unsigned int currentRoot = rootNodes[w];
            std::vector<uint32_t> node_stack;
            // Start from root joint.
            node_stack.push_back(currentRoot);

            std::vector<uint32_t> deepness_stack;
            traversalBones(
                children,
                joints,
                node_stack,
                deepness_stack,
                nodes_bones
            );

            for (unsigned int e = 0; e < nodes_bones.size(); ++e) {
                nodesHierarchy.push_back(nodes_bones[e]);
            }
        }

        // This logic related to joints that has inverseBindMatrices.
        [[maybe_unused]] uint32_t transformationsMax =
            translations.size() > scales.size()
            ? (translations.size() > rotations.size() ? translations.size()
                                                      : rotations.size())
            : (scales.size() > rotations.size() ? scales.size()
                                                : rotations.size());

        const uint32_t numJoints = joints.value.array->size();
        uint32_t translationFramesNumber = 0;
        for (uint32_t k = 0; k < frameInputsTranslation.size(); ++k) {
            if (frameInputsTranslation[k].size() > translationFramesNumber) {
                translationFramesNumber = frameInputsTranslation[k].size();
            }
        }
        uint32_t rotationFramesNumber = 0;
        for (uint32_t k = 0; k < frameInputsRotation.size(); ++k) {
            if (frameInputsRotation[k].size() > rotationFramesNumber) {
                rotationFramesNumber = frameInputsRotation[k].size();
            }
        }
        uint32_t scaleFramesNumber = 0;
        for (uint32_t k = 0; k < frameInputsScale.size(); ++k) {
            if (frameInputsScale[k].size() > scaleFramesNumber) {
                scaleFramesNumber = frameInputsScale[k].size();
            }
        }

        const uint32_t framesMax = translationFramesNumber > scaleFramesNumber
            ? (translationFramesNumber > rotationFramesNumber
                   ? translationFramesNumber
                   : rotationFramesNumber)
            : (scaleFramesNumber > rotationFramesNumber ? scaleFramesNumber
                                                        : rotationFramesNumber);

        for (uint32_t k = 0; k < frameInputsTranslation.size(); ++k) {
            if (frameInputsTranslation[k].size() > frames.size()) {
                frames = frameInputsTranslation[k];
            }
        }
        for (uint32_t k = 0; k < frameInputsRotation.size(); ++k) {
            if (frameInputsRotation[k].size() > frames.size()) {
                frames = frameInputsRotation[k];
            }
        }
        for (uint32_t k = 0; k < frameInputsScale.size(); ++k) {
            if (frameInputsScale[k].size() > frames.size()) {
                frames = frameInputsScale[k];
            }
        }

        std::vector<int> jointToTranslationCh;
        std::vector<int> jointToRotationCh;
        std::vector<int> jointToScaleCh;
        for (uint32_t k = 0; k < numJoints; ++k) {
            jointToTranslationCh.push_back(-1);
            jointToRotationCh.push_back(-1);
            jointToScaleCh.push_back(-1);
        }
        for (uint32_t k = 0; k < nodesMapTranslations.size(); ++k) {
            uint32_t jIdx = getJointIndex(joints, (int32_t)nodesMapTranslations[k]);
            if (jIdx != UINT32_MAX) {
                jointToTranslationCh[jIdx] = (int)k;
            }
        }
        for (uint32_t k = 0; k < nodesMapRotations.size(); ++k) {
            uint32_t jIdx = getJointIndex(joints, (int32_t)nodesMapRotations[k]);
            if (jIdx != UINT32_MAX) {
                jointToRotationCh[jIdx] = (int)k;
            }
        }
        for (uint32_t k = 0; k < nodesMapScales.size(); ++k) {
            uint32_t jIdx = getJointIndex(joints, (int32_t)nodesMapScales[k]);
            if (jIdx != UINT32_MAX) {
                jointToScaleCh[jIdx] = (int)k;
            }
        }

        // Build animatedNodesMatricesAccumulator indexed by joint-index
        // (0..numJoints - 1).
        std::vector<std::vector<Matrix<float, 4>>> animatedNodesMatricesAccumulator;
        for (unsigned int j = 0; j < numJoints; ++j) {
            int tIdx = jointToTranslationCh[j];
            int rIdx = jointToRotationCh[j];
            int sIdx = jointToScaleCh[j];

            // Local defaults fresh on every joint, for not make possible to
            // collect data from previous iterations.
            std::vector<float> defaultTranslations;
            std::vector<float> defaultRotations;
            std::vector<float> defaultScales;

            // Static TRS from node. Using if channel not exists.
            int32_t nodeIdx = (int32_t)(*joints.value.array)[j].value.iNumber;
            float sTx = 0.f, sTy = 0.f, sTz = 0.f;
            float sRx = 0.f, sRy = 0.f, sRz = 0.f, sRw = 1.f;
            float sSx = 1.f, sSy = 1.f, sSz = 1.f;
            if ((*gltf)["nodes"][nodeIdx].isObject() == JSON_OBJECT) {
                auto* nd = (*gltf)["nodes"][nodeIdx].value.object;
                if (nd->Contain("translation")) {
                    sTx = (*gltf)["nodes"][nodeIdx]["translation"][0]
                              .value.fNumber;
                    sTy = (*gltf)["nodes"][nodeIdx]["translation"][1]
                              .value.fNumber;
                    sTz = (*gltf)["nodes"][nodeIdx]["translation"][2]
                              .value.fNumber;
                }
                if (nd->Contain("rotation")) {
                    sRx =
                        (*gltf)["nodes"][nodeIdx]["rotation"][0].value.fNumber;
                    sRy =
                        (*gltf)["nodes"][nodeIdx]["rotation"][1].value.fNumber;
                    sRz =
                        (*gltf)["nodes"][nodeIdx]["rotation"][2].value.fNumber;
                    sRw =
                        (*gltf)["nodes"][nodeIdx]["rotation"][3].value.fNumber;
                }
                if (nd->Contain("scale")) {
                    sSx = (*gltf)["nodes"][nodeIdx]["scale"][0].value.fNumber;
                    sSy = (*gltf)["nodes"][nodeIdx]["scale"][1].value.fNumber;
                    sSz = (*gltf)["nodes"][nodeIdx]["scale"][2].value.fNumber;
                }
            }

            if (tIdx < 0) {
                for (uint32_t f = 0; f < framesMax; ++f) {
                    defaultTranslations.push_back(sTx);
                    defaultTranslations.push_back(sTy);
                    defaultTranslations.push_back(sTz);
                }
            }
            if (rIdx < 0) {
                for (uint32_t f = 0; f < framesMax; ++f) {
                    defaultRotations.push_back(sRx);
                    defaultRotations.push_back(sRy);
                    defaultRotations.push_back(sRz);
                    defaultRotations.push_back(sRw);
                }
            }
            if (sIdx < 0) {
                for (uint32_t f = 0; f < framesMax; ++f) {
                    defaultScales.push_back(sSx);
                    defaultScales.push_back(sSy);
                    defaultScales.push_back(sSz);
                }
            }

            std::vector<float>& boneT =
                (tIdx >= 0) ? translations[tIdx] : defaultTranslations;
            std::vector<float>& boneR =
                (rIdx >= 0) ? rotations[rIdx] : defaultRotations;
            std::vector<float>& boneS =
                (sIdx >= 0) ? scales[sIdx] : defaultScales;

            // Chennels can has verious number of frames; framesMax - gloabal
            // maximum. Clamp index to last valid chennel frame, for not run out
            // after vectors bounds.
            const uint32_t tFrames = boneT.size() / 3;
            const uint32_t rFrames = boneR.size() / 4;
            const uint32_t sFrames = boneS.size() / 3;
            std::vector<Matrix<float, 4>> perFrameMatrices;
            for (unsigned int i = 0; i < framesMax; ++i) {
                if (tFrames == 0 || rFrames == 0 || sFrames == 0) {
                    // Malformed data; skip joint.
                    std::vector<Matrix<float, 4>> empty;
                    animatedNodesMatricesAccumulator.push_back(empty);
                    continue;
                }

                const uint32_t ti = (i < tFrames) ? i : tFrames - 1;
                const uint32_t ri = (i < rFrames) ? i : rFrames - 1;
                const uint32_t si = (i < sFrames) ? i : sFrames - 1;
                Matrix<float, 4> frameTranslation(1.0f);
                Matrix<float, 4> frameScale(1.0f);
                for (unsigned int q = 0; q < 3; ++q) {
                    frameTranslation[3][q] = boneT[ti * 3 + q];
                    frameScale[q][q] = boneS[si * 3 + q];
                }
                Quaternion frameRotationQuaternion;
                Matrix<float, 4> frameRotation(1.0f);
                frameRotationQuaternion.x = boneR[ri * 4];
                frameRotationQuaternion.y = boneR[ri * 4 + 1];
                frameRotationQuaternion.z = boneR[ri * 4 + 2];
                frameRotationQuaternion.w = boneR[ri * 4 + 3];
                frameRotation =
                    rotateQuaternion<float, 4>(frameRotationQuaternion);
                frameRotation.SelfTensorTranspose();
                Matrix<float, 4> localTransform =
                    frameScale * frameRotation * frameTranslation;
                perFrameMatrices.push_back(localTransform);
            }
            animatedNodesMatricesAccumulator.push_back(perFrameMatrices);
        }

        // Final comstruction of joint-matrices. Both arrays indexed by
        // joint-index now, that's why nodesHierarchy[j][b] address accumulator
        // correctly.
        for (unsigned int j = 0; j < numJoints; ++j) {
            std::vector<Matrix<float, 4>> globalAllFrameNodeMatrix;
            for (unsigned int i = 0; i < framesMax; ++i) {
                Matrix<float, 4> rootTransform(1.0f);
                for (unsigned int b = 0; b < nodesHierarchy[j].size() - 1;
                     ++b) {
                    rootTransform =
                        animatedNodesMatricesAccumulator[nodesHierarchy[j][b]][i]
                        * rootTransform;
                }
                globalAllFrameNodeMatrix.push_back(
                    inverseBindMatrixSet[j]
                    * animatedNodesMatricesAccumulator[j][i] * rootTransform
                );
            }
            jointMatrices.push_back(globalAllFrameNodeMatrix);
        }
    }

    jointMatricesPerMesh = jointMatrices;
    topY = -999.999f;
    for (uint32_t i = 0; i < indices.size(); ++i) {
        aIndices_.push_back(i);

        unsigned int index = indices[i] * 3;
        if (index + 2 < verticesPosition.size()) {
            Vector<float, 3> position = {
                verticesPosition[index],
                verticesPosition[index + 1],
                verticesPosition[index + 2]
            };

            if (position[1] > topY) {
                topY = position[1];
            }

            aVertexes_.push_back(position[0]);
            aVertexes_.push_back(position[1]);
            aVertexes_.push_back(position[2]);
        }

        if (index + 2 < normals.size()) {
            Vector<float, 3> normal =
                {normals[index], normals[index + 1], normals[index + 2]};

            aVertexes_.push_back(normal[0]);
            aVertexes_.push_back(normal[1]);
            aVertexes_.push_back(normal[2]);
        }

        index = indices[i] * 2;
        if (index + 1 < textureCoordinates.size()) {
            aVertexes_.push_back(textureCoordinates[index]);
            aVertexes_.push_back(textureCoordinates[index + 1]);
        }

        index = indices[i] * 4;
        if (index + 3 < jointsIndices.size()) {
            aVertexes_.push_back(jointsIndices[index]);
            aVertexes_.push_back(jointsIndices[index + 1]);
            aVertexes_.push_back(jointsIndices[index + 2]);
            aVertexes_.push_back(jointsIndices[index + 3]);
        }

        if (index + 3 < weightsContainer.size()) {
            aVertexes_.push_back(weightsContainer[index]);
            aVertexes_.push_back(weightsContainer[index + 1]);
            aVertexes_.push_back(weightsContainer[index + 2]);
            aVertexes_.push_back(weightsContainer[index + 3]);
        }
    }

    delete[] buffer;
    buffer = nullptr;
}

void CJsonParser::traversalBones(
    std::vector<std::vector<int>> children,
    Core::JsonValue joints,
    std::vector<uint32_t> node_stack,
    std::vector<uint32_t> deepness_stack,
    std::vector<std::vector<uint32_t>>& result
) {
    uint32_t topJointIndex = 0;
    if (!node_stack.empty()) {
        // Pass array of all joints and root joint and return index of root
        // joint in array.
        topJointIndex = getJointIndex(joints, node_stack.back());
    }

    if (node_stack.size() > deepness_stack.size()) {
        // First 0 level start from.
        uint32_t firstChild = 0;
        deepness_stack.push_back(firstChild);
    }

    // Main exit check.
    if (deepness_stack.empty()) {
        return;
    }

    uint32_t nextNodeIndex = 0;
    // Check current root joint has any children. Children maps linearly with
    // root joint array index.
    if (topJointIndex != UINT32_MAX && !children[topJointIndex].empty()) {
        // Check if on last child level.
        if (deepness_stack.back() > 0
            && deepness_stack.back() == children[topJointIndex].size()) {
            deepness_stack.pop_back();
            node_stack.pop_back();
            traversalBones(children, joints, node_stack, deepness_stack, result);
            return;
        }

        // Check if not on last child level.
        if (deepness_stack.back() > 0
            && deepness_stack.back() < children[topJointIndex].size()) {
            nextNodeIndex = children[topJointIndex][deepness_stack.back()];
            node_stack.push_back(nextNodeIndex);

            std::vector<uint32_t> current_node_indices;
            for (uint32_t i = 0; i < node_stack.size(); ++i) {
                uint32_t currentJoinIndex = getJointIndex(joints, node_stack[i]);
                current_node_indices.push_back(currentJoinIndex);
            }
            ++deepness_stack.back();
            traversalBones(children, joints, node_stack, deepness_stack, result);
            return;
        } else {
            std::vector<uint32_t> current_node_indices;
            for (uint32_t i = 0; i < node_stack.size(); ++i) {
                uint32_t currentJoinIndex = getJointIndex(joints, node_stack[i]);
                current_node_indices.push_back(currentJoinIndex);
            }

            result.push_back(current_node_indices);

            nextNodeIndex = children[topJointIndex][deepness_stack.back()];
            node_stack.push_back(nextNodeIndex);
            ++deepness_stack.back();
            traversalBones(children, joints, node_stack, deepness_stack, result);
            return;
        }
    } else {
        std::vector<uint32_t> current_node_indices;
        if (topJointIndex == UINT32_MAX) {
            current_node_indices.push_back(node_stack.back());
            result.push_back(current_node_indices);
            return;
        }

        for (uint32_t i = 0; i < node_stack.size(); ++i) {
            uint32_t currentJoinIndex = getJointIndex(joints, node_stack[i]);
            current_node_indices.push_back(currentJoinIndex);
        }

        result.push_back(current_node_indices);

        deepness_stack.pop_back();
        node_stack.pop_back();
        traversalBones(children, joints, node_stack, deepness_stack, result);
        return;
    }
}

std::vector<std::vector<unsigned int>> CJsonParser::makeRenderJointsIndices(
    std::vector<std::vector<unsigned int>>& input
) {
    std::vector<std::vector<unsigned int>> result;

    bool accumulatorFlag = false;
    bool innerFlag = false;
    unsigned int accumulator = input[0][0];
    for (unsigned int i = 0; i < input.size(); ++i) {
        for (unsigned int j = 0; j < input[i].size(); ++j) {
            std::vector<unsigned int> inner;
            for (unsigned int v = 0; v < j + 1; ++v) {
                if (input[i][j] == accumulator && accumulatorFlag) {
                    innerFlag = false;
                    continue;
                } else {
                    innerFlag = true;
                    inner.push_back(input[i][v]);

                    if (accumulatorFlag == false) {
                        accumulatorFlag = true;
                    }
                }
            }

            if (innerFlag) {
                result.push_back(inner);
            }
        }
    }

    return result;
}

bool CJsonParser::containsElemnt(
    std::vector<std::vector<unsigned int>> container,
    unsigned int element
) {
    bool flag = false;

    for (unsigned int i = 0; i < container.size(); ++i) {
        for (unsigned int j = 0; j < container[i].size(); ++j) {
            if (container[i][j] == element) {
                return true;
            }
        }
    }

    return flag;
}

uint32_t CJsonParser::getJointIndex(Core::JsonValue joints, int32_t searchingIndex) {
    for (unsigned int i = 0; i < joints.value.array->size(); ++i) {
        int currentJointIndex = (*joints.value.array)[i].value.iNumber;

        if (currentJointIndex == searchingIndex) {
            return i;
        }
    }

    return -1;
}

CJsonParser::~CJsonParser() {
    delete root_;
}
} // namespace glvm::Core
