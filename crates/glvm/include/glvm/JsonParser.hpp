#pragma once

#include "glvm/HashMap.hpp"
#include "glvm/Vector.hpp"
#include "glvm/stack.hpp"
#include "glvm/typenames.hpp"

#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string.h>
#include <string>
#include <thread>

namespace glvm::Core {
enum JsonType {
    JSON_INVALID_VALUE,
    JSON_OBJECT,
    JSON_FLOAT_NUMBER,
    JSON_INTEGER_NUMBER,
    JSON_STRING,
    JSON_BOOLEAN,
    JSON_NULL,
    JSON_ARRAY
};

struct JsonValue;

union JsonVariant {
    std::string* string;
    double fNumber;
    int iNumber;
    bool boolean;
    void* null;
    glvm::core::vector<JsonValue>* array;
    HashMap<JsonValue>* object;

    JsonVariant() {}

    JsonVariant(const JsonVariant& object) {
        memcpy((void*)this, &object, sizeof(JsonVariant));
    }

    ~JsonVariant() {}
};

struct JsonValue {
    JsonVariant value;
    JsonType type;

    JsonValue() {
        type = JSON_INVALID_VALUE;
    }

    JsonValue(std::string _string) {
        type = JSON_STRING;
        value.string = new std::string(_string);
    }

    JsonValue(double _float) {
        type = JSON_FLOAT_NUMBER;
        value.fNumber = _float;
    }

    JsonValue(int _int) {
        type = JSON_INTEGER_NUMBER;
        value.iNumber = _int;
    }

    JsonValue(bool _bool) {
        type = JSON_BOOLEAN;
        value.boolean = _bool;
    }

    JsonValue(const JsonValue& _value) {
        type = JSON_INVALID_VALUE;

        switch (_value.type) {
            case JSON_OBJECT:
                value.object = new HashMap<JsonValue>(*_value.value.object);
                break;
            case JSON_INTEGER_NUMBER:
                value.iNumber = _value.value.iNumber;
                break;
            case JSON_FLOAT_NUMBER:
                value.fNumber = _value.value.fNumber;
                break;
            case JSON_STRING:
                value.string = new std::string(*_value.value.string);
                break;
            case JSON_BOOLEAN:
                value.boolean = _value.value.boolean;
                break;
            case JSON_NULL:
                value.null = _value.value.null;
                break;
            case JSON_ARRAY:
                value.array = new core::vector<JsonValue>(*_value.value.array);
                break;
            default:
                break;
        }
        type = _value.type;
    }

    ~JsonValue() {
        switch (type) {
            case JSON_INVALID_VALUE:
                break;
            case JSON_OBJECT:
                delete value.object;
                break;
            case JSON_INTEGER_NUMBER:
                break;
            case JSON_FLOAT_NUMBER:
                break;
            case JSON_STRING:
                delete value.string;
                break;
            case JSON_BOOLEAN:
                break;
            case JSON_NULL:
                break;
            case JSON_ARRAY:
                delete value.array;
                break;
        }
    }

    void operator=(const JsonValue& _value) {
        switch (type) {
            case JSON_INVALID_VALUE:
                break;
            case JSON_OBJECT:
                delete value.object;
                break;
            case JSON_INTEGER_NUMBER:
                break;
            case JSON_FLOAT_NUMBER:
                break;
            case JSON_STRING:
                delete value.string;
                break;
            case JSON_BOOLEAN:
                break;
            case JSON_NULL:
                break;
            case JSON_ARRAY:
                delete value.array;
                break;
        }

        switch (_value.type) {
            case JSON_OBJECT:
                value.object = new HashMap<JsonValue>(*_value.value.object);
                break;
            case JSON_INTEGER_NUMBER:
                value.iNumber = _value.value.iNumber;
                break;
            case JSON_FLOAT_NUMBER:
                value.fNumber = _value.value.fNumber;
                break;
            case JSON_STRING:
                value.string = new std::string(*_value.value.string);
                break;
            case JSON_BOOLEAN:
                value.boolean = _value.value.boolean;
                break;
            case JSON_NULL:
                value.null = _value.value.null;
                break;
            case JSON_ARRAY:
                value.array = new core::vector<JsonValue>(*_value.value.array);
                break;
            default:
                break;
        }
        type = _value.type;
    }

    JsonValue& operator[](std::string key_) {
        const char* key = key_.c_str();
        switch (type) {
            case JSON_OBJECT:
                return (*value.object)[key];
                break;
            default:
                throw std::out_of_range("Type is not a json object");
                break;
        }
    }

    JsonValue& operator[](const unsigned int index_) {
        switch (type) {
            case JSON_ARRAY:
                return (*value.array)[index_];
                break;
            default:
                throw std::out_of_range("Type is not a json array");
                break;
        }
    }

    bool isInvalid() {
        return type == JSON_INVALID_VALUE;
    }

    bool isObject() {
        return type == JSON_OBJECT;
    }

    bool isFloat() {
        return type == JSON_FLOAT_NUMBER;
    }

    bool isInterger() {
        return type == JSON_INTEGER_NUMBER;
    }

    bool isString() {
        return type == JSON_STRING;
    }

    bool isBoolean() {
        return type == JSON_BOOLEAN;
    }

    bool isNull() {
        return type == JSON_NULL;
    }

    bool isArray() {
        return type == JSON_ARRAY;
    }
};

class CJsonParser {
    std::string sJsonFileData_;
    const char* pJsonFileData_;
    char currentChar_;
    unsigned int globalFileCounter_ = 0;

    core::vector<JsonValue*> stackOfJsonValues_;
    JsonValue* root_;
    bool keyFlag = true;
    std::string lastKey_ = "";
    std::string bufferString_ = "";

    void SearchInJsonArray(
        core::vector<JsonValue>* arrayValue,
        const char* key_,
        core::vector<JsonValue>& resultVector
    ) const;

public:
    void SearchInJsonObject(
        HashMap<JsonValue>* mapValue,
        const char* key_,
        core::vector<JsonValue>& resultVector
    ) const;

    ~CJsonParser();

    JsonValue* GetRoot() {
        return root_;
    }

    void ReadFile(const char* _filePath);
    void Parse();
    JsonValue CreateJsonHashMap();
    JsonValue CreateJsonArray();
    std::string BoolOrNullParse();
    bool IsContainChar(std::string _string, char _char);
    std::string NumberAsStringParse();
    std::string StringParse();
    core::vector<char> StringToVectorOfChars(std::string _string);
    int ParseInteger(core::vector<char> _word);
    double ParseFloating(core::vector<char> _word);
    core::vector<JsonValue> Search(const char* key_) const;
    void LoadGLTF(
        const char* pathsGLTF_,
        std::vector<float>& aVertexes_,
        std::vector<uint32_t>& aIndices_,
        core::vector<core::vector<mat4>>& jointMatricesPerMesh,
        core::vector<float>& frames,
        bool& noAnimations,
        float& topY
    );
    void traversalBones(
        core::vector<core::vector<int>> children,
        Core::JsonValue joints,
        core::stack<u32> node_stack,
        core::stack<u32> deepness_stack,
        core::vector<core::vector<u32>>& result
    );
    core::vector<core::vector<unsigned int>> makeRenderJointsIndices(
        core::vector<core::vector<unsigned int>>& input
    );
    bool containsElemnt(
        core::vector<core::vector<unsigned int>> container,
        unsigned int element
    );
    unsigned int getJointIndex(Core::JsonValue joints, int searchingIndex);
};
} // namespace glvm::Core
