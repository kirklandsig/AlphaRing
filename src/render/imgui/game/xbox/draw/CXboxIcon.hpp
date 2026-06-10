#pragma once

#include <imgui.h>
#include <unordered_map>
#include "glad/glad.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint LoadTextureFromFile(const char* filename)
{
    int w, h, channels;
    unsigned char* data = stbi_load(filename, &w, &h, &channels, 4);
    if (!data) return 0;

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return tex;
}

void DrawIcon(GLuint tex, int x, int y, int w, int h)
{
    ImGui::SetCursorScreenPos(ImVec2((float)x, (float)y));
    ImGui::Image((ImTextureID)(intptr_t)tex, ImVec2((float)w, (float)h));
}

void drawIcon(
    ImDrawList* draw,
    ImVec2 pos,
    float w,
    float h,
    const void* data,
    size_t len
) {
    static std::unordered_map<const void*, GLuint> s_cache;
    GLuint& tex = s_cache[data];
    if (tex == 0) {
        int iw, ih, ch;
        unsigned char* pixels = stbi_load_from_memory(
            (const stbi_uc*)data, (int)len, &iw, &ih, &ch, 4);
        if (!pixels) return;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iw, ih, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(pixels);
    }
    draw->AddImage(
        (ImTextureID)(intptr_t)tex,
        pos,
        ImVec2(pos.x + w, pos.y + h)
    );
}