// ObjLoader.h — leitura de arquivos .obj
#pragma once
#include "Common.h"

// Lê v, vt e f (faces v, v/vt, v//vn, v/vt/vn; polígonos viram triângulos).
// Retorna false e preenche 'erro' se falhar.
bool carregarOBJ(const std::string& caminho, Malha& saida, std::string& erro);
