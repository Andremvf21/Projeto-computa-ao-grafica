// =============================================================================
//  Textura — imagens, sphere map e presets de material
//  Responsável: Ana Beatriz          Requisitos: 3c (reflexão), 3d (textura)
// =============================================================================
#pragma once
#include "Common.h"

// Carrega .png/.jpg/.bmp com stb_image e devolve o id OpenGL (0 se falhar).
GLuint carregarTextura(const std::string& caminho, std::string& erro);

// Aplica um preset ao objeto (material + modo de visualização).
void aplicarPreset(Objeto3D& obj, Preset p);
