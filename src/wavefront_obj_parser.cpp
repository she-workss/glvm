// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#include "wavefront_obj_parser.hpp"

#include "vector.hpp"

#include <chrono>
#include <iterator>

namespace GLVM::core {
CWaveFrontObjParser::CWaveFrontObjParser() = default;

auto CWaveFrontObjParser::getCoordinateVertices() const
    -> const GLVM::core::vector<SVertex>& {
    return coordinateVertices_;
}

auto CWaveFrontObjParser::getTextureVertices() const
    -> const GLVM::core::vector<SVertex>& {
    return textureVertices_;
}

auto CWaveFrontObjParser::getNormals() const
    -> const GLVM::core::vector<SVertex>& {
    return normals_;
}

auto CWaveFrontObjParser::getFaces() const -> const GLVM::core::vector<SFace>& {
    return faces_;
}

void CWaveFrontObjParser::ReadFile(const char* file_path) {
    const char* p_wavefront_obj_file = file_path;
    std::ifstream wavefront_obj_file_input_stream;
    std::stringstream wavefront_obj_file_output_stream;
    wavefront_obj_file_input_stream.open(p_wavefront_obj_file);
    if (wavefront_obj_file_input_stream.good()) {
        wavefront_obj_file_output_stream
            << wavefront_obj_file_input_stream.rdbuf();
        wavefront_obj_file_input_stream.close();
        sWavefrontObjFileData = wavefront_obj_file_output_stream.str();
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

auto CWaveFrontObjParser::Split(
    const char* p_wave_front_obj_file_data,
    const char separator,
    const char exit_symbol,
    unsigned int& ui_counter
) -> GLVM::core::vector<vector<char>> {
    GLVM::core::vector<vector<char>> words_container;
    unsigned int outerIndex = 0;
    words_container.Push({});

    for (;; ++ui_counter) {
        if (p_wave_front_obj_file_data[ui_counter] == '#') {
            while (p_wave_front_obj_file_data[ui_counter] != '\n') {
                ++ui_counter;
            }
            continue;
        }
        if (p_wave_front_obj_file_data[ui_counter] == separator) {
            words_container[outerIndex].Push('\0');
            words_container.Push({});
            ++outerIndex;
            continue;
        }
        if (p_wave_front_obj_file_data[ui_counter] == exit_symbol) {
            ++ui_counter;
            words_container[outerIndex].Push('\0');
            return words_container;
        }
        words_container[outerIndex].Push(p_wave_front_obj_file_data[ui_counter]);
    }
}

auto CWaveFrontObjParser::ParseVertices(
    GLVM::core::vector<vector<char>> words_container
) -> SVertex {
    SVertex vertex {};
    unsigned int ui_vertex_index = 0;

    unsigned int ui_words_container_size = words_container.GetSize();
    for (unsigned int i = 1; i < ui_words_container_size; ++i) {
        float float_number = ParseFloating(words_container[i]);
        vertex[ui_vertex_index++] = float_number;
    }

    return vertex;
}

auto CWaveFrontObjParser::ParseFaces(
    GLVM::core::vector<vector<char>> words_container
) -> SFace {
    SFace face;
    GLVM::core::vector<vector<char>> words_inner_container;
    GLVM::core::vector<char> word;

    unsigned int ui_words_container_size = words_container.GetSize();

    for (unsigned int i = 1; i < ui_words_container_size; ++i) {
        unsigned int counter = 0;
        words_inner_container =
            Split(words_container[i].GetVectorContainer(), '/', '\0', counter);

        for (unsigned int j = 0; j < words_inner_container.GetSize(); ++j) {
            word = words_inner_container[j];
            int i_value = ParseInteger(word);

            face[j].Push(i_value);
        }
    }
    return face;
}

auto CWaveFrontObjParser::ParseInteger(GLVM::core::vector<char> word) -> int {
    GLVM::core::vector<int> base_container;

    for (unsigned int i = 0; i < word.GetSize() - 1; ++i) {
        base_container.Push(word[i] - 48);
    }

    int i_result = 0;
    bool negate_flag = false;

    unsigned int base_container_size = base_container.GetSize();
    for (unsigned int i = 0; i < base_container_size; ++i) {
        if ((negate_flag && i == 0) || (base_container[i] == -5 && i == 0)) {
            continue;
        }

        i_result +=
            base_container[i] * std::pow(10, (base_container_size - 1) - i);
    }

    return i_result;
}

auto CWaveFrontObjParser::ParseFloating(GLVM::core::vector<char> word)
    -> float {
    GLVM::core::vector<int> base_container;

    for (unsigned int i = 0; i < word.GetSize() - 1; ++i) {
        base_container.Push(word[i] - 48);
    }

    int integer_part = 0;
    float floating_part = 0;
    GLVM::core::vector<int> integer_part_container;
    GLVM::core::vector<int> floating_part_container;
    bool dot_flag = false;
    bool negate_flag = false;
    unsigned int base_container_size = base_container.GetSize();

    if (base_container[0] == -3) {
        negate_flag = true;
    }

    for (unsigned int i = 0; i < base_container_size; ++i) {
        if (base_container[i] == -2) {
            dot_flag = true;
        }
        if ((negate_flag && i == 0) || base_container[i] == -2
            || (base_container[i] == -5 && i == 0)) {
            continue;
        }
        if (base_container[i] >= 0 && base_container[i] <= 9) {
            if (dot_flag) {
                floating_part_container.Push(base_container[i]);
            } else {
                integer_part_container.Push(base_container[i]);
            }
        } else {
            return NAN;
        }
    }

    unsigned int integer_part_container_size = integer_part_container.GetSize();
    for (unsigned int i = 0; i < integer_part_container_size; ++i) {
        integer_part += integer_part_container[i]
            * std::pow(10, (integer_part_container_size - 1) - i);
    }

    unsigned int floating_part_container_size =
        floating_part_container.GetSize();
    for (unsigned int i = 0; i < floating_part_container_size; ++i) {
        floating_part += floating_part_container[i] / std::pow(10, i + 1);
    }

    float result = 0;
    result = (float)(integer_part + floating_part);

    if (negate_flag) {
        result *= -1.0f;
    }

    return result;
}
} // namespace GLVM::core
