// Interface.cpp — Responsável: André Mota (Sprints 1 e 2)
#include "Interface.h"
#include <GL/glui.h>
#include <commdlg.h>
#include <cstdlib>

static GLUI* g_glui = nullptr;

static void cbSair(int) { exit(0); }

void criarInterface(int janelaPrincipal) {
    // Painel mínimo para validar que a GLUI está vinculada (Sprint 0).
    // Obs.: a GLUI não exibe acentos — textos do painel ficam sem acento.
    g_glui = GLUI_Master.create_glui_subwindow(janelaPrincipal, GLUI_SUBWINDOW_RIGHT);
    g_glui->add_statictext("Trabalho 2 - CG");
    g_glui->add_statictext("Visualizador 3D");
    g_glui->add_separator();
    g_glui->add_statictext("Sprint 0: ambiente OK");
    g_glui->add_separator();
    g_glui->add_button("Sair", 0, cbSair);
    g_glui->set_main_gfx_window(janelaPrincipal);
}

bool abrirDialogo(const char* filtro, std::string& caminho) {
    char buffer[MAX_PATH] = "";
    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = filtro;
    ofn.lpstrFile   = buffer;
    ofn.nMaxFile    = MAX_PATH;
    ofn.Flags       = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (!GetOpenFileNameA(&ofn)) return false;
    caminho = buffer;
    return true;
}

void sincronizarPainel() { /* TODO Sprint 2 */ }
