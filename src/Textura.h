// =============================================================================
//  Textura — imagens, sphere map e presets de material
//  Responsável: Ana Beatriz          Requisitos: 3c (reflexão), 3d (textura)
// =============================================================================
#pragma once
#include "Common.h"

// Carrega .png/.jpg/.bmp com stb_image e devolve o id OpenGL (0 se falhar).
// Pode ser chamada de dentro de um callback da GLUI: a textura é criada no
// contexto da janela principal mesmo assim.
GLuint carregarTextura(const std::string& caminho, std::string& erro);

// Apaga a textura da memória de vídeo e zera o id
// (usar ao trocar a textura de um objeto ou ao remover o objeto).
void liberarTextura(GLuint& id);

// --- Uso no desenho (Render.cpp) ---------------------------------------------
// ligarTextura: ativa a textura do objeto se ele estiver no modo texturizado,
// tiver textura carregada e tiver UV. Retorna true se ativou; nesse caso,
// chame enviarUV() antes de cada glVertex e desligarTextura() no fim.
bool ligarTextura(const Objeto3D& obj);
void desligarTextura();

// Envia glTexCoord2f do vértice i (0, 1 ou 2) do triângulo.
// Não faz nada se esse vértice não tiver coordenada de textura.
void enviarUV(const Malha& m, const Triangulo& t, int i);

// Aplica um preset ao objeto (material + modo de visualização).
void aplicarPreset(Objeto3D& obj, Preset p);
