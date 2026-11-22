// Textura.h — imagens, sphere map e cor do material
#pragma once
#include "Common.h"

// Carrega .png/.jpg/.bmp e devolve o id OpenGL (0 se falhar).
GLuint carregarTextura(const std::string& caminho, std::string& erro);

// Ativa a textura ou o sphere map conforme o modo. Retorna true se ativou.
bool ligarTextura(const Objeto3D& obj);
void desligarTextura();

void enviarUV(const Malha& m, const Triangulo& t, int i);

struct CorNomeada { const char* nome; float r, g, b; };
extern const CorNomeada CORES[];
extern const int NUM_CORES;
void aplicarCor(Objeto3D& obj, int indiceCor);
