// =============================================================================
//  ObjLoader — leitura de arquivos .obj            Responsável: Artur Uchôa
//  Requisitos: 2a (carregar .obj), 3d (aproveitar 'vt'), 5 (contar triângulos)
// =============================================================================
#pragma once
#include "Common.h"

// Lê o arquivo e preenche 'saida'. Deve aceitar:
//   - linhas v, vt, vn, f (demais linhas são ignoradas: o, g, s, usemtl, mtllib...)
//   - faces nos formatos  v   v/vt   v//vn   v/vt/vn
//   - índices negativos (relativos ao fim da lista)
//   - polígonos com 4+ vértices (triangular em leque)
// Ao final: chamar normalizarMalha() e calcularNormais() (Geometria.h) e,
// se o arquivo não tiver 'vt', gerarUV().
// Retorna false (e preenche 'erro') se o arquivo não abrir ou não tiver faces.
bool carregarOBJ(const std::string& caminho, Malha& saida, std::string& erro);
