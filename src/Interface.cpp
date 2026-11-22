// Interface.cpp — painel GLUI e diálogos de arquivo
// Textos da tela sem acento: a GLUI e o MessageBoxA não os exibem
#include "Interface.h"
#include "ObjLoader.h"
#include "Geometria.h"
#include "Textura.h"
#include <GL/glui.h>
#include <commdlg.h>
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>

namespace {

// Aparência do painel
const int LARGURA_BOTAO = 190;
const unsigned char COR_FUNDO[3] = { 226, 229, 234 };

GLUI*            g_glui       = nullptr;
GLUI_StaticText* g_txtNome    = nullptr;
GLUI_StaticText* g_txtTri     = nullptr;
GLUI_StaticText* g_txtUV      = nullptr;
GLUI_StaticText* g_txtTextura = nullptr;
GLUI_StaticText* g_txtAlfa    = nullptr;
GLUI_StaticText* g_txtBrilho  = nullptr;

// Texturas da pasta (um botão cada) e cache das já carregadas
std::vector<std::string>      g_arquivosTextura;
std::map<std::string, GLuint> g_cacheTexturas;
std::string                   g_nomeTexturaAtual;

// Variáveis "vivas": a GLUI escreve nelas quando o controle muda
int   s_shading   = SHADING_SMOOTH;
int   s_modo      = MODO_SOLIDO;
int   s_cor       = 0;
float s_alfa      = 1.0f;
float s_brilho    = 32.0f;
int   s_luzLigada = 1;

void redesenhar() {
    // Redesenha a janela principal a partir do callback do painel
    glutPostWindowRedisplay(g_janela);
}

void mostrarErro(const std::string& msg) {
    MessageBoxA(nullptr, msg.c_str(), "Trabalho 2 - CG", MB_OK | MB_ICONERROR);
}

// "C:\...\assets\modelos\gato.obj" -> "gato"
std::string nomeDoArquivo(const std::string& caminho, size_t maximo = 22) {
    size_t barra = caminho.find_last_of("\\/");
    std::string nome = (barra == std::string::npos) ? caminho : caminho.substr(barra + 1);
    size_t ponto = nome.find_last_of('.');
    if (ponto != std::string::npos) nome = nome.substr(0, ponto);
    if (nome.size() > maximo) nome = nome.substr(0, maximo);
    return nome;
}

std::string formatar(const char* rotulo, float valor, int casas) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%s: %.*f", rotulo, casas, valor);
    return buf;
}

// Lista as imagens de assets/texturas, exceto o sphere map
void listarTexturas() {
    g_arquivosTextura.clear();
    const char* extensoes[] = { "*.png", "*.jpg", "*.jpeg", "*.bmp" };
    for (const char* ext : extensoes) {
        WIN32_FIND_DATAA dados;
        HANDLE h = FindFirstFileA((std::string("assets\\texturas\\") + ext).c_str(), &dados);
        if (h == INVALID_HANDLE_VALUE) continue;
        do {
            std::string nome = dados.cFileName;
            std::string minusculo = nome;
            for (char& c : minusculo) c = (char)tolower((unsigned char)c);
            if (minusculo.find("spheremap") != std::string::npos) continue;
            g_arquivosTextura.push_back("assets\\texturas\\" + nome);
        } while (FindNextFileA(h, &dados));
        FindClose(h);
    }
    std::sort(g_arquivosTextura.begin(), g_arquivosTextura.end());
    if (g_arquivosTextura.size() > 8) g_arquivosTextura.resize(8);
}

void atualizarInfo() {
    const Objeto3D* obj = objetoSelecionado();
    if (!obj) {
        g_txtNome->set_text("Nenhum modelo aberto");
        g_txtTri->set_text("");
        g_txtUV->set_text("");
    } else {
        g_txtNome->set_text(("Modelo: " + obj->nome).c_str());
        g_txtTri->set_text(("Triangulos: " + std::to_string(obj->malha.numTriangulos())).c_str());
        g_txtUV->set_text(obj->malha.temUVdoArquivo ? "UV: do arquivo (vt)" : "UV: gerada (esferica)");
    }
    g_txtTextura->set_text(("Atual: " + (g_nomeTexturaAtual.empty() ? std::string("nenhuma") : g_nomeTexturaAtual)).c_str());
    g_txtAlfa->set_text(formatar("Opacidade", s_alfa, 2).c_str());
    g_txtBrilho->set_text(formatar("Shininess", s_brilho, 0).c_str());
}

// Aplica a textura (carrega só na primeira vez) e liga o modo Textura
void aplicarTextura(const std::string& caminho) {
    Objeto3D* obj = objetoSelecionado();
    if (!obj) { mostrarErro("Abra um modelo .obj antes de aplicar a textura."); return; }

    GLuint id = 0;
    auto it = g_cacheTexturas.find(caminho);
    if (it != g_cacheTexturas.end()) {
        id = it->second;
    } else {
        std::string erro;
        id = carregarTextura(caminho, erro);
        if (id == 0) { mostrarErro(erro); return; }
        g_cacheTexturas[caminho] = id;
    }

    obj->textura = id;
    obj->modo = MODO_TEXTURA;
    g_nomeTexturaAtual = nomeDoArquivo(caminho, 18);
    sincronizarPainel();
    redesenhar();
}

// Callbacks

void cbAbrirOBJ(int) {
    std::string caminho;
    if (!abrirDialogo("Modelos OBJ (*.obj)\0*.obj\0Todos os arquivos\0*.*\0",
                      caminho, "assets\\modelos"))
        return;

    Malha malha;
    std::string erro;
    if (!carregarOBJ(caminho, malha, erro)) { mostrarErro(erro); return; }

    // O modelo novo substitui o anterior, mantendo material, modo e textura
    Objeto3D* obj = objetoSelecionado();
    if (!obj) {
        g_cena.push_back(Objeto3D());
        g_idSelecionado = (int)g_cena.size() - 1;
        obj = &g_cena.back();
        // O primeiro modelo usa o que estiver escolhido no painel
        aplicarCor(*obj, s_cor);
        obj->mat.alpha     = s_alfa;
        obj->mat.shininess = s_brilho;
        obj->shading       = (Shading)s_shading;
        obj->modo          = (ModoVisual)s_modo;
    }
    obj->malha = std::move(malha);
    obj->nome  = nomeDoArquivo(caminho);

    std::printf("%s: %d triangulos, %d vertices\n", obj->nome.c_str(),
                obj->malha.numTriangulos(), (int)obj->malha.vertices.size());
    sincronizarPainel();
    redesenhar();
}

void cbTexturaDaPasta(int id) {
    if (id >= 0 && id < (int)g_arquivosTextura.size()) aplicarTextura(g_arquivosTextura[id]);
}

// "Outra imagem...": qualquer .png/.jpg/.bmp do computador
void cbOutraTextura(int) {
    if (!objetoSelecionado()) { mostrarErro("Abra um modelo .obj antes de aplicar a textura."); return; }
    std::string caminho;
    if (!abrirDialogo("Imagens (*.png, *.jpg, *.bmp)\0*.png;*.jpg;*.jpeg;*.bmp\0Todos os arquivos\0*.*\0",
                      caminho, "assets\\texturas"))
        return;
    aplicarTextura(caminho);
}

void cbSemTextura(int) {
    Objeto3D* obj = objetoSelecionado();
    if (!obj) return;
    obj->modo = MODO_SOLIDO;
    sincronizarPainel();
    redesenhar();
}

void cbShading(int) {
    if (Objeto3D* obj = objetoSelecionado()) obj->shading = (Shading)s_shading;
    redesenhar();
}

void cbModo(int) {
    Objeto3D* obj = objetoSelecionado();
    if (!obj) return;
    obj->modo = (ModoVisual)s_modo;
    if (obj->modo == MODO_TEXTURA && obj->textura == 0)
        mostrarErro("Este modelo ainda nao tem textura. Escolha uma na secao Textura.");
    if (obj->modo == MODO_REFLEXAO && g_texSphereMap == 0)
        mostrarErro("A imagem do sphere map (assets\\texturas\\spheremap.png) nao foi carregada.");
    redesenhar();
}

void cbCor(int) {
    if (Objeto3D* obj = objetoSelecionado()) aplicarCor(*obj, s_cor);
    redesenhar();
}

void cbAlfa(int) {
    s_alfa = std::max(0.0f, std::min(1.0f, s_alfa));
    if (Objeto3D* obj = objetoSelecionado()) obj->mat.alpha = s_alfa;
    g_txtAlfa->set_text(formatar("Opacidade", s_alfa, 2).c_str());
    redesenhar();
}

void cbBrilho(int) {
    s_brilho = std::max(0.0f, std::min(128.0f, s_brilho));
    if (Objeto3D* obj = objetoSelecionado()) obj->mat.shininess = s_brilho;
    g_txtBrilho->set_text(formatar("Shininess", s_brilho, 0).c_str());
    redesenhar();
}

void cbLuz(int) {
    g_luz.ligada = (s_luzLigada != 0);
    redesenhar();
}

void cbSair(int) { std::exit(0); }

// Atalhos de layout

GLUI_Button* botao(GLUI_Panel* painel, const char* texto, int id, void (*cb)(int)) {
    GLUI_Button* b = g_glui->add_button_to_panel(painel, texto, id, cb);
    b->set_w(LARGURA_BOTAO);
    return b;
}

GLUI_StaticText* texto(GLUI_Panel* painel, const char* t = "") {
    return g_glui->add_statictext_to_panel(painel, t);
}

GLUI_Scrollbar* slider(GLUI_Panel* painel, float* var, float mn, float mx, void (*cb)(int)) {
    GLUI_Scrollbar* s = new GLUI_Scrollbar(painel, "", GLUI_SCROLL_HORIZONTAL, var, 0, cb);
    s->set_float_limits(mn, mx);
    s->set_w(LARGURA_BOTAO);
    return s;
}

}  // namespace

void criarInterface(int janelaPrincipal) {
    g_glui = GLUI_Master.create_glui_subwindow(janelaPrincipal, GLUI_SUBWINDOW_RIGHT);
    for (int i = 0; i < 3; ++i) g_glui->bkgd_color[i] = COR_FUNDO[i];

    GLUI_StaticText* titulo = g_glui->add_statictext("VISUALIZADOR 3D");
    titulo->set_alignment(GLUI_ALIGN_CENTER);
    GLUI_StaticText* sub = g_glui->add_statictext("Trabalho 2 - Computacao Grafica");
    sub->set_alignment(GLUI_ALIGN_CENTER);
    g_glui->add_separator();

    // Modelo
    GLUI_Rollout* rModelo = g_glui->add_rollout("Modelo", true);
    botao(rModelo, "Abrir modelo (.obj)...", 0, cbAbrirOBJ);
    g_txtNome = texto(rModelo);
    g_txtTri  = texto(rModelo);
    g_txtUV   = texto(rModelo);

    // Textura: um botão por imagem da pasta
    GLUI_Rollout* rTex = g_glui->add_rollout("Textura", true);
    listarTexturas();
    for (int i = 0; i < (int)g_arquivosTextura.size(); ++i)
        botao(rTex, nomeDoArquivo(g_arquivosTextura[i], 24).c_str(), i, cbTexturaDaPasta);
    if (g_arquivosTextura.empty()) texto(rTex, "(nenhuma imagem em assets)");
    g_glui->add_separator_to_panel(rTex);
    botao(rTex, "Outra imagem...", 0, cbOutraTextura);
    botao(rTex, "Sem textura (solido)", 0, cbSemTextura);
    g_txtTextura = texto(rTex);

    // Visualização
    GLUI_Rollout* rVis = g_glui->add_rollout("Visualizacao", true);
    GLUI_Panel* pSh = g_glui->add_panel_to_panel(rVis, "Shading");
    GLUI_RadioGroup* gSh = g_glui->add_radiogroup_to_panel(pSh, &s_shading, 0, cbShading);
    g_glui->add_radiobutton_to_group(gSh, "Flat");
    g_glui->add_radiobutton_to_group(gSh, "Smooth");
    g_glui->add_column_to_panel(rVis, false);
    GLUI_Panel* pModo = g_glui->add_panel_to_panel(rVis, "Modo");
    GLUI_RadioGroup* gModo = g_glui->add_radiogroup_to_panel(pModo, &s_modo, 0, cbModo);
    g_glui->add_radiobutton_to_group(gModo, "Solido");
    g_glui->add_radiobutton_to_group(gModo, "Textura");
    g_glui->add_radiobutton_to_group(gModo, "Reflexao");

    // Material: cor, opacidade e shininess
    GLUI_Rollout* rMat = g_glui->add_rollout("Material", true);
    GLUI_Listbox* lbCor = g_glui->add_listbox_to_panel(rMat, "Cor  ", &s_cor, 0, cbCor);
    for (int i = 0; i < NUM_CORES; ++i) lbCor->add_item(i, CORES[i].nome);
    lbCor->set_w(LARGURA_BOTAO);
    g_txtAlfa = texto(rMat, "Opacidade");
    slider(rMat, &s_alfa, 0.0f, 1.0f, cbAlfa);
    g_txtBrilho = texto(rMat, "Shininess");
    slider(rMat, &s_brilho, 0.0f, 128.0f, cbBrilho);

    // Luz
    GLUI_Rollout* rLuz = g_glui->add_rollout("Luz", true);
    g_glui->add_checkbox_to_panel(rLuz, "Ligada", &s_luzLigada, 0, cbLuz);
    const char* eixos[3] = { "Posicao X", "Posicao Y", "Posicao Z" };
    for (int i = 0; i < 3; ++i) {
        GLUI_Spinner* sp = g_glui->add_spinner_to_panel(rLuz, eixos[i], GLUI_SPINNER_FLOAT,
                                                        &g_luz.pos[i], 0, cbLuz);
        sp->set_float_limits(-10.0f, 10.0f);
        sp->set_speed(0.5f);
        sp->set_w(LARGURA_BOTAO);
    }

    g_glui->add_separator();
    GLUI_Button* sair = g_glui->add_button("Sair", 0, cbSair);
    sair->set_w(LARGURA_BOTAO);
    g_glui->set_main_gfx_window(janelaPrincipal);

    s_luzLigada = g_luz.ligada ? 1 : 0;
    sincronizarPainel();
}

bool abrirDialogo(const char* filtro, std::string& caminho, const char* pastaInicial) {
    char buffer[MAX_PATH] = "";
    char pasta[MAX_PATH]  = "";
    if (pastaInicial) GetFullPathNameA(pastaInicial, MAX_PATH, pasta, nullptr);

    // Restaura a pasta atual, que o diálogo pode mudar
    char pastaAtual[MAX_PATH] = "";
    GetCurrentDirectoryA(MAX_PATH, pastaAtual);

    OPENFILENAMEA ofn = {};
    ofn.lStructSize     = sizeof(ofn);
    ofn.hwndOwner       = GetActiveWindow();
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
        s_modo    = obj->modo;
        s_cor     = obj->cor;
        s_alfa    = obj->mat.alpha;
        s_brilho  = obj->mat.shininess;
    }
    s_luzLigada = g_luz.ligada ? 1 : 0;
    g_glui->sync_live();
    atualizarInfo();
}
