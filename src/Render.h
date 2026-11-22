// Render.h — iluminação, shading e transparência
#pragma once
#include "Common.h"

// Chamar depois de posicionar a câmera.
void configurarLuz(const Luz& luz);

void desenharIndicadorLuz(const Luz& luz);

void desenharCena();
