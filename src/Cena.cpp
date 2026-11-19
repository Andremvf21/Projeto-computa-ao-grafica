// Cena.cpp — estado global compartilhado (declarado em Common.h)
#include "Common.h"

std::vector<Objeto3D> g_cena;
int    g_idSelecionado = -1;
Luz    g_luz;
Camera g_camera;
GLuint g_texSphereMap  = 0;
int    g_janela        = 0;
