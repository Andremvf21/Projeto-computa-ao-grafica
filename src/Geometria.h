// =============================================================================
//  Geometria — normais e coordenadas de textura
//  Responsável: Artur Uchôa          Requisitos: 3a e 3d
// =============================================================================
#pragma once
#include "Common.h"

// Centraliza no (0,0,0) e escala para caber numa esfera de raio 1.
void normalizarMalha(Malha& m);

// Calcula Triangulo::normalFace (Flat) e Malha::normaisVertice (Smooth, média).
void calcularNormais(Malha& m);

// Gera Malha::uvs (projeção esférica) quando o .obj não tem 'vt' (requisito 3d).
void gerarUV(Malha& m);
