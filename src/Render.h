// =============================================================================
//  Render — iluminação, shading, transparência e desenho da cena
//  Responsável: Isabel Lugon         Requisitos: 2c, 3a, 3b, 3c
// =============================================================================
#pragma once
#include "Common.h"

// Liga/desliga GL_LIGHT0 e posiciona a luz (chamar depois da câmera).
void configurarLuz(const Luz& luz);

// Esferinha amarela na posição da luz.
void desenharIndicadorLuz(const Luz& luz);

// Desenha um objeto com seu material, shading e modo de visualização.
void desenharObjeto(const Objeto3D& obj);

// Opacos primeiro; depois transparentes ordenados do mais longe ao mais perto,
// com GL_BLEND e glDepthMask(GL_FALSE).
void desenharCena();
