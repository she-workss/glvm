// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts:
// <fellfrostqtw@gmail.com> Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef CONSTANTS
#define CONSTANTS

#define MATRIX_RANGE 16
#define VERTEX_ARRAY_RANGE 180
#define SIZE_OF_VERTEX_DATA 5
#define LAYOUT_0 0
#define LAYOUT_1 1
#define VERTEX_SIZE 3
#define TEXTURE_SIZE 2
#define VERTEX_OFFSET 0
#define TEXTURE_OFFSET 3
#define NUMBER_OF_CREATING_VBO_OBJECT_1 1
#define NUMBER_OF_CREATING_VAO_OBJECT_1 1
#define BASE_ARRAY_COUNTER_VALUE 0
#define BASE_INDEX_VERTEX_ARRAY 0
#define NUMBER_OF_DROWING_VERTEXES 36
#define HOMOGENEOUS_COORDINATE 1
#define NUMBER_OF_MATRICES 1
#define LIMITER 1
#define PI 3.14159265

#define NUMBER_OF_CREATING_TEXTURE_OBJECT_1 1
#define SOME_STRANGE_STUFF 0
#define MIPMAP_LEVEL 0
#define SOME_OLD_STUFF 0

constexpr unsigned int k_iUint_Max = 4000000000;
constexpr int k_iNull = 0;
constexpr int boxIndicesForIndexBuffer[36] = {0, 1, 2, 3, 0, 2, 4, 0, 3,
                                              7, 4, 3, 4, 5, 1, 0, 4, 1,
                                              1, 5, 6, 2, 1, 6, 5, 4, 7,
                                              6, 5, 7, 3, 2, 6, 7, 3, 6};

#endif
