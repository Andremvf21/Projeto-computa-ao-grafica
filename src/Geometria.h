// =============================================================================
//  Geometria — normais, coordenadas de textura e malhas procedurais
//  Responsável: Artur Uchôa          Requisitos: 3a, 3d e a cena (mesa e bola)
// =============================================================================
#pragma once
#include "Common.h"

// Centraliza no (0,0,0) e escala para caber numa esfera de raio 1.
void normalizarMalha(Malha& m);

// Calcula Triangulo::normalFace (Flat) e Malha::normaisVertice (Smooth, média).
void calcularNormais(Malha& m);

// Gera Malha::uvs quando o .obj não tem 'vt' (esférica, cilíndrica ou planar).
void gerarUV(Malha& m, TipoUV tipo);

// Esfera por latitude/longitude, já com normais e UV (a bola da cena).
void criarEsfera(Malha& m, float raio, int fatias, int pilhas);

// Mesa: tampo + 4 pernas (caixas), com UV planar para a textura de madeira.
void criarMesa(Malha& m, float largura, float profundidade, float altura);
