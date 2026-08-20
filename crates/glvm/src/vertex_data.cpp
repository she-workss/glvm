const int kVertex_Size = 9;
constexpr float kWidth_Offset = 1.0f / 3;

float aVertices[kVertex_Size] = {
    -0.5f,
    -0.5f,
    0.5f, // Left vertex.
    -0.5f,
    0.5f,
    0.0f, // Right vertex.
    0.0f,
    0.0f,
    0.0f // Upper vertex.
};

float aVertices2[kVertex_Size] = {
    0.5f,
    -0.5f,
    // Left vertex.
    -0.5f,
    0.5f,
    0.5f,
    // Right vertex.
    0.5f,
    0.0f,
    0.0f,
    // Upper vertex.
    -1.0f
};

float aVertices_Static_Object[] = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, 1.0f, 1.0f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, 0.0f, 1.0f, // Up left vertex.
    0.5f,  0.5f,  0.0f, 1.0f, 1.0f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f
};

float vertices[] = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, kWidth_Offset, 1.0f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, kWidth_Offset, 0.75f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, 0.0f,          0.75f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, 0.0f,          1.0f, // Up left vertex.
    0.5f,  0.5f,  0.0f, kWidth_Offset, 1.0f,  -0.5f, -0.5f, 0.0f, 0.0f, 0.75f
};

float vertices2[] = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, kWidth_Offset * 2, 1.0f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, kWidth_Offset * 2, 0.75f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, kWidth_Offset,     0.75f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, kWidth_Offset,     1.0f, // Up left vertex.
    0.5f,  0.5f,  0.0f, kWidth_Offset * 2, 1.0f,
    -0.5f, -0.5f, 0.0f, kWidth_Offset,     0.75f
};

float vertices3[] = {
    // Coordinates.
    0.5f,
    0.5f,
    0.0f,
    1.0f,
    1.0f, // Up right vertex.
    0.5f,
    -0.5f,
    0.0f,
    1.0f,
    0.75f, // Bottom right vertex.
    -0.5f,
    -0.5f,
    0.0f,
    kWidth_Offset * 2,
    0.75f, // Bottom left vertex.
    -0.5f,
    0.5f,
    0.0f,
    kWidth_Offset * 2,
    1.0f, // Up left vertex.
    0.5f,
    0.5f,
    0.0f,
    1.0f,
    1.0f,
    -0.5f,
    -0.5f,
    0.0f,
    kWidth_Offset * 2,
    0.75f
};

float vertices4[] = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, kWidth_Offset, 0.75f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, kWidth_Offset, 0.5f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, 0.0f,          0.5f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, 0.0f,          0.75f, // Up left vertex.
    0.5f,  0.5f,  0.0f, kWidth_Offset, 0.75f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f
};

float vertices5[] = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, kWidth_Offset * 2, 0.75f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, kWidth_Offset * 2, 0.5f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, kWidth_Offset,     0.5f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, kWidth_Offset,     0.75f, // Up left vertex.
    0.5f,  0.5f,  0.0f, kWidth_Offset * 2, 0.75f,
    -0.5f, -0.5f, 0.0f, kWidth_Offset,     0.5f
};

float vertices6[] = {
    // Coordinates.
    0.5f,
    0.5f,
    0.0f,
    1.0f,
    0.75f, // Up right vertex.
    0.5f,
    -0.5f,
    0.0f,
    1.0f,
    0.5f, // Bottom right vertex.
    -0.5f,
    -0.5f,
    0.0f,
    kWidth_Offset * 2,
    0.5f, // Bottom left vertex.
    -0.5f,
    0.5f,
    0.0f,
    kWidth_Offset * 2,
    0.75f, // Up left vertex.
    0.5f,
    0.5f,
    0.0f,
    1.0f,
    0.75f,
    -0.5f,
    -0.5f,
    0.0f,
    kWidth_Offset * 2,
    0.5f
};

float vertices7[] = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, kWidth_Offset, 0.5f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, kWidth_Offset, 0.25f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, 0.0f,          0.25f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, 0.0f,          0.5f, // Up left vertex.
    0.5f,  0.5f,  0.0f, kWidth_Offset, 0.5f,  -0.5f, -0.5f, 0.0f, 0.0f, 0.25f
};

float vertices8[] = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, kWidth_Offset * 2, 0.5f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, kWidth_Offset * 2, 0.25f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, kWidth_Offset,     0.25f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, kWidth_Offset,     0.5f, // Up left vertex.
    0.5f,  0.5f,  0.0f, kWidth_Offset * 2, 0.5f,
    -0.5f, -0.5f, 0.0f, kWidth_Offset,     0.25f
};

float vertices9[] = {
    // Coordinates.
    0.5f,
    0.5f,
    0.0f,
    1.0f,
    0.5f, // Up right vertex.
    0.5f,
    -0.5f,
    0.0f,
    1.0f,
    0.25f, // Bottom right vertex.
    -0.5f,
    -0.5f,
    0.0f,
    kWidth_Offset * 2,
    0.25f, // Bottom left vertex.
    -0.5f,
    0.5f,
    0.0f,
    kWidth_Offset * 2,
    0.5f, // Up left vertex.
    0.5f,
    0.5f,
    0.0f,
    1.0f,
    0.5f,
    -0.5f,
    -0.5f,
    0.0f,
    kWidth_Offset * 2,
    0.25f
};

float vertices10[] = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, kWidth_Offset, 0.25f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, kWidth_Offset, 0.0f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, 0.0f,          0.0f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, 0.0f,          0.25f, // Up left vertex.
    0.5f,  0.5f,  0.0f, kWidth_Offset, 0.25f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f
};

float vertices11[] = {
    // Coordinates.
    0.5f,  0.5f,  0.0f, kWidth_Offset * 2, 0.25f, // Up right vertex.
    0.5f,  -0.5f, 0.0f, kWidth_Offset * 2, 0.0f, // Bottom right vertex.
    -0.5f, -0.5f, 0.0f, kWidth_Offset,     0.0f, // Bottom left vertex.
    -0.5f, 0.5f,  0.0f, kWidth_Offset,     0.25f, // Up left vertex.
    0.5f,  0.5f,  0.0f, kWidth_Offset * 2, 0.25f,
    -0.5f, -0.5f, 0.0f, kWidth_Offset,     0.0f
};

float vertices12[] = {
    // Coordinates.
    0.5f,
    0.5f,
    0.0f,
    1.0f,
    0.25f, // Up right vertex.
    0.5f,
    -0.5f,
    0.0f,
    1.0f,
    0.0f, // Bottom right vertex.
    -0.5f,
    -0.5f,
    0.0f,
    kWidth_Offset * 2,
    0.0f, // Bottom left vertex.
    -0.5f,
    0.5f,
    0.0f,
    kWidth_Offset * 2,
    0.25f, // Up left vertex.
    0.5f,
    0.5f,
    0.0f,
    1.0f,
    0.25f,
    -0.5f,
    -0.5f,
    0.0f,
    kWidth_Offset * 2,
    0.0f
};

int Vertices_Size = sizeof(vertices);
