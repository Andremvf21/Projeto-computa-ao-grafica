// Textura.cpp — Responsável: Ana Beatriz (Sprints 1 e 2)
#define STB_IMAGE_IMPLEMENTATION   // só aqui, em nenhum outro .cpp
#include "stb_image.h"
#include "Textura.h"

GLuint carregarTextura(const std::string& caminho, std::string& erro) {
    // TODO (Sprint 1): stbi_set_flip_vertically_on_load(1); stbi_load(...);
    //                  glGenTextures / glTexImage2D (ou gluBuild2DMipmaps)
    (void)caminho;
    erro = "Carregamento de textura ainda nao implementado";
    return 0;
}

void aplicarPreset(Objeto3D& obj, Preset p) {
    // TODO (Sprint 2): valores finais de cada preset
    (void)obj; (void)p;
}
