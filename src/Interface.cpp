// Interface.cpp — painel GLUI e diálogos de arquivo
// Obs.: a GLUI não exibe acentos, e o MessageBoxA também não com o código
// compilado em UTF-8. Por isso os textos que aparecem na tela ficam sem acento.
#include "Interface.h"
#include "ObjLoader.h"
#include "Geometria.h"
#include "Textura.h"
#include <GL/glui.h>
#include <commdlg.h>
#include <cstdio>
#include <cstdlib>

namespace {

GLUI*            g_glui       = nullptr;
GLUI_StaticText* g_txtNome    = nullptr;
GLUI_StaticText* g_txtTri     = nullptr;
GLUI_StaticText* g_txtUV      = nullptr;
GLUI_RadioGroup* g_grupoUV    = nullptr;

// Variáveis "vivas": a GLUI escreve nelas quando o usuário mexe no controle.
int s_shading    = SHADING_SMOOTH;
int s_tipoUV     = UV_ESFERICA;
int s_luzLigada  = 1;

void redesenhar() {
    // Os callbacks rodam com o painel como janela atual. glutPostWindowRedisplay
    // pede o redesenho da janela principal sem precisar trocar de janela.
    glutPostWindowRedisplay(g_janela);
}

void mostrarErro(const std::string& msg) {
    MessageBoxA(nullptr, msg.c_str(), "Trabalho 2 - CG", MB_OK | MB_ICONERROR);
}

// "C:\...\assets\modelos\gato.obj" -> "gato"
std::string nomeDoArquivo(const std::string& caminho) {
    size_t barra = caminho.find_last_of("\\/");
    std::string nome = (barra == std::string::npos) ? caminho : caminho.substr(barra + 1);
    size_t ponto = nome.find_last_of('.');
    if (ponto != std::string::npos) nome = nome.substr(0, ponto);
    if (nome.size() > 22) nome = nome.substr(0, 22);   // não alarga o painel
    return nome;
}

void atualizarInfo() {
    const Objeto3D* obj = objetoSelecionado();
    if (!obj) {
        g_txtNome->set_text("Nenhum modelo aberto");
        g_txtTri->set_text("");
        g_txtUV->set_text("");
        return;
    }
    g_txtNome->set_text(("Modelo: " + obj->nome).c_str());
    g_txtTri->set_text(("Triangulos: " + std::to_string(obj->malha.numTriangulos())).c_str());
    g_txtUV->set_text(obj->malha.temUVdoArquivo ? "UV: do arquivo (vt)" : "UV: gerada");
}

// --- Callbacks -------------------------------------------------------------

void cbAbrirOBJ(int) {
    std::string caminho;
    if (!abrirDialogo("Modelos OBJ (*.obj)\0*.obj\0Todos os arquivos\0*.*\0",
                      caminho, "assets\\modelos"))
        return;

    Malha malha;
    std::string erro;
    if (!carregarOBJ(caminho, malha, erro)) { mostrarErro(erro); return; }

    // Sprint 1: visualizador de um objeto. Se já existe um selecionado, o
    // modelo novo entra no lugar dele (textura e opções são mantidas);
    // senão, cria o primeiro objeto da cena.
    Objeto3D* obj = objetoSelecionado();
    if (!obj) {
        g_cena.push_back(Objeto3D());
        g_idSelecionado = (int)g_cena.size() - 1;
        obj = &g_cena.back();
    }
    obj->malha = std::move(malha);
    obj->nome  = nomeDoArquivo(caminho);
    if (!obj->malha.temUVdoArquivo && obj->tipoUV != UV_ESFERICA)
        gerarUV(obj->malha, obj->tipoUV);   // o parser gera esférica por padrão

    std::printf("%s: %d triangulos, %d vertices\n", obj->nome.c_str(),
                obj->malha.numTriangulos(), (int)obj->malha.vertices.size());
    sincronizarPainel();
    redesenhar();
}

void cbAbrirTextura(int) {
    Objeto3D* obj = objetoSelecionado();
    if (!obj) { mostrarErro("Abra um modelo .obj antes de carregar a textura."); return; }

    std::string caminho;
    if (!abrirDialogo("Imagens (*.png, *.jpg, *.bmp)\0*.png;*.jpg;*.jpeg;*.bmp\0Todos os arquivos\0*.*\0",
                      caminho, "assets\\texturas"))
        return;

    std::string erro;
    GLuint nova = carregarTextura(caminho, erro);
    if (nova == 0) { mostrarErro(erro); return; }

    liberarTextura(obj->textura);   // a anterior não fica ocupando memória de vídeo
    obj->textura = nova;
    obj->modo = MODO_TEXTURA;
    redesenhar();
}

void cbShading(int) {
    if (Objeto3D* obj = objetoSelecionado()) obj->shading = (Shading)s_shading;
    redesenhar();
}

void cbTipoUV(int) {
    Objeto3D* obj = objetoSelecionado();
    if (!obj) return;
    obj->tipoUV = (TipoUV)s_tipoUV;
    if (!obj->malha.temUVdoArquivo) gerarUV(obj->malha, obj->tipoUV);
    redesenhar();
}

void cbLuz(int) {
    g_luz.ligada = (s_luzLigada != 0);
    redesenhar();
}

void cbSair(int) { std::exit(0); }

}  // namespace

void criarInterface(int janelaPrincipal) {
    g_glui = GLUI_Master.create_glui_subwindow(janelaPrincipal, GLUI_SUBWINDOW_RIGHT);
    g_glui->add_statictext("Trabalho 2 - CG");
    g_glui->add_statictext("Visualizador 3D");
    g_glui->add_separator();

    // Arquivos (requisitos 1, 2a e 3d: nada depende de linha de comando)
    GLUI_Panel* pArq = g_glui->add_panel("Arquivos");
    g_glui->add_button_to_panel(pArq, "Abrir modelo (.obj)", 0, cbAbrirOBJ);
    g_glui->add_button_to_panel(pArq, "Abrir textura", 0, cbAbrirTextura);
    g_txtNome = g_glui->add_statictext_to_panel(pArq, "");
    g_txtTri  = g_glui->add_statictext_to_panel(pArq, "");
    g_txtUV   = g_glui->add_statictext_to_panel(pArq, "");

    // Shading (requisito 3a)
    GLUI_Panel* pSh = g_glui->add_panel("Shading");
    GLUI_RadioGroup* gSh = g_glui->add_radiogroup_to_panel(pSh, &s_shading, 0, cbShading);
    g_glui->add_radiobutton_to_group(gSh, "Flat");
    g_glui->add_radiobutton_to_group(gSh, "Smooth (Gouraud)");

    // UV automática para modelos sem 'vt' (requisito 3d)
    GLUI_Panel* pUV = g_glui->add_panel("UV automatica");
    g_grupoUV = g_glui->add_radiogroup_to_panel(pUV, &s_tipoUV, 0, cbTipoUV);
    g_glui->add_radiobutton_to_group(g_grupoUV, "Esferica");
    g_glui->add_radiobutton_to_group(g_grupoUV, "Cilindrica");
    g_glui->add_radiobutton_to_group(g_grupoUV, "Planar");

    // Luz (requisito 2c)
    GLUI_Panel* pLuz = g_glui->add_panel("Luz");
    g_glui->add_checkbox_to_panel(pLuz, "Ligada", &s_luzLigada, 0, cbLuz);
    const char* eixos[3] = { "X", "Y", "Z" };
    for (int i = 0; i < 3; ++i) {
        GLUI_Spinner* sp = g_glui->add_spinner_to_panel(pLuz, eixos[i], GLUI_SPINNER_FLOAT,
                                                        &g_luz.pos[i], 0, cbLuz);
        sp->set_float_limits(-10.0f, 10.0f);
        sp->set_speed(0.5f);
    }

    g_glui->add_separator();
    g_glui->add_button("Sair", 0, cbSair);
    g_glui->set_main_gfx_window(janelaPrincipal);

    s_luzLigada = g_luz.ligada ? 1 : 0;
    sincronizarPainel();
}

bool abrirDialogo(const char* filtro, std::string& caminho, const char* pastaInicial) {
    char buffer[MAX_PATH] = "";
    char pasta[MAX_PATH]  = "";
    if (pastaInicial) GetFullPathNameA(pastaInicial, MAX_PATH, pasta, nullptr);

    // O diálogo pode mudar a pasta atual do programa para a pasta escolhida
    // (a documentação avisa que OFN_NOCHANGEDIR não vale para o "Abrir").
    // Se isso acontecer, os caminhos relativos ("assets/...") param de
    // funcionar. Por isso a pasta atual é guardada e restaurada.
    char pastaAtual[MAX_PATH] = "";
    GetCurrentDirectoryA(MAX_PATH, pastaAtual);

    OPENFILENAMEA ofn = {};
    ofn.lStructSize     = sizeof(ofn);
    ofn.hwndOwner       = GetActiveWindow();   // a caixa fica na frente da janela
    ofn.lpstrFilter     = filtro;
    ofn.lpstrFile       = buffer;
    ofn.nMaxFile        = MAX_PATH;
    ofn.lpstrInitialDir = pastaInicial ? pasta : nullptr;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    BOOL ok = GetOpenFileNameA(&ofn);

    SetCurrentDirectoryA(pastaAtual);
    if (!ok) return false;
    caminho = buffer;
    return true;
}

void sincronizarPainel() {
    if (!g_glui) return;
    if (const Objeto3D* obj = objetoSelecionado()) {
        s_shading = obj->shading;
        s_tipoUV  = obj->tipoUV;
        // Com 'vt' no arquivo a UV automática não é usada: desabilita a escolha.
        if (obj->malha.temUVdoArquivo) g_grupoUV->disable(); else g_grupoUV->enable();
    }
    s_luzLigada = g_luz.ligada ? 1 : 0;
    g_glui->sync_live();   // copia as variáveis vivas para os controles
    atualizarInfo();
}
