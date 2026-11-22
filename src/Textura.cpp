// Textura.cpp — imagens, sphere map e cor do material
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Textura.h"

// Cada janela GLUT tem seu contexto OpenGL; a textura precisa ser criada na principal
namespace {
struct NaJanelaPrincipal {
    int anterior;
    NaJanelaPrincipal() : anterior(glutGetWindow()) {
        if (g_janela > 0 && anterior != g_janela) glutSetWindow(g_janela);
    }
    ~NaJanelaPrincipal() {
        if (anterior > 0 && anterior != g_janela) glutSetWindow(anterior);
    }
};
}

GLuint carregarTextura(const std::string& caminho, std::string& erro) {
    // No .obj, v = 0 é a base da imagem
    stbi_set_flip_vertically_on_load(1);

    // Sempre RGBA
    int largura = 0, altura = 0, canaisNoArquivo = 0;
    unsigned char* pixels = stbi_load(caminho.c_str(), &largura, &altura, &canaisNoArquivo, 4);
    if (!pixels) {
        const char* motivo = stbi_failure_reason();
        erro = "Nao foi possivel abrir a imagem " + caminho +
               (motivo ? std::string(" (") + motivo + ")" : std::string());
        return 0;
    }

    NaJanelaPrincipal contexto;

    GLuint id = 0;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // Gera os mipmaps e aceita lados que não são potência de 2
    GLint r = gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, largura, altura,
                                GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    stbi_image_free(pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (r != 0) {
        glDeleteTextures(1, &id);
        erro = std::string("Erro ao enviar a textura para o OpenGL: ") +
               reinterpret_cast<const char*>(gluErrorString(r));
        return 0;
    }

    erro.clear();
    return id;
}

bool ligarTextura(const Objeto3D& obj) {
    // Reflexão: sphere map com coordenadas geradas pela normal e pela câmera
    if (obj.modo == MODO_REFLEXAO) {
        if (g_texSphereMap == 0) { glDisable(GL_TEXTURE_2D); return false; }
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, g_texSphereMap);
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        return true;
    }

    bool usar = obj.modo == MODO_TEXTURA && obj.textura != 0 && !obj.malha.uvs.empty();
    if (!usar) {
        glDisable(GL_TEXTURE_2D);
        return false;
    }
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, obj.textura);
    // MODULATE: a textura é multiplicada pela cor iluminada
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    return true;
}

void desligarTextura() {
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

void enviarUV(const Malha& m, const Triangulo& t, int i) {
    int k = t.vt[i];
    if (k >= 0 && k < (int)m.uvs.size())
        glTexCoord2f(m.uvs[k].u, m.uvs[k].v);
}

// Cores do modo Sólido
const CorNomeada CORES[] = {
    { "Cinza",      0.80f, 0.80f, 0.80f },
    { "Branco",     1.00f, 1.00f, 1.00f },
    { "Laranja",    0.95f, 0.58f, 0.25f },
    { "Vermelho",   0.85f, 0.15f, 0.15f },
    { "Azul",       0.25f, 0.45f, 0.90f },
    { "Verde",      0.30f, 0.75f, 0.35f },
    { "Rosa",       0.96f, 0.56f, 0.70f },
};
const int NUM_CORES = (int)(sizeof(CORES) / sizeof(CORES[0]));

void aplicarCor(Objeto3D& obj, int indiceCor) {
    if (indiceCor < 0 || indiceCor >= NUM_CORES) return;
    const CorNomeada& c = CORES[indiceCor];
    obj.cor = indiceCor;
    obj.mat.difusa[0] = c.r;  obj.mat.difusa[1] = c.g;  obj.mat.difusa[2] = c.b;
    // Ambiente com o mesmo tom, mais escura
    obj.mat.ambiente[0] = c.r * 0.25f;  obj.mat.ambiente[1] = c.g * 0.25f;  obj.mat.ambiente[2] = c.b * 0.25f;
}
