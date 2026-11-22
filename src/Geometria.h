// Geometria.h — normais e coordenadas de textura
#pragma once
#include "Common.h"

// Centraliza na origem e escala para raio 1.
void normalizarMalha(Malha& m);

// Normal por face (Flat) e média por vértice (Smooth).
void calcularNormais(Malha& m);

// UV por projeção esférica, para .obj sem 'vt'.
void gerarUV(Malha& m);
