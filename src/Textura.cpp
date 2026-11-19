// Textura.cpp — Responsável: Ana Beatriz (Sprints 1 e 2)
#define STB_IMAGE_IMPLEMENTATION   // só aqui, em nenhum outro .cpp
#include "stb_image.h"
#include "Textura.h"

// -----------------------------------------------------------------------------
// Cada janela do GLUT tem o próprio contexto OpenGL, inclusive o painel da GLUI,
// e uma textura criada em um contexto não existe nos outros. Quando o botão
// "Abrir textura" for clicado, a janela atual será o painel; sem esta troca,
// a textura iria para o painel e o objeto apareceria branco na janela principal.
// O construtor troca para a janela principal e o destrutor devolve a anterior.
// -----------------------------------------------------------------------------
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
    // No .obj, v = 0 é a parte de baixo da imagem. O arquivo de imagem guarda
    // a linha de cima primeiro. Virando na leitura, as duas convenções batem
    // e a textura não aparece de cabeça para baixo.
    stbi_set_flip_vertically_on_load(1);

    // O último argumento (4) pede sempre RGBA, qualquer que seja o arquivo:
    // um formato só para .png, .jpg e .bmp, e o alfa fica disponível para a
    // transparência (3b). Com 4 bytes por pixel, cada linha também tem tamanho
    // múltiplo de 4, que é o alinhamento padrão que o OpenGL espera.
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

    // REPEAT: UVs fora de 0..1 repetem a imagem (útil na madeira da mesa).
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Perto: interpola entre pixels. Longe: usa os mipmaps, o que evita o
    // "chuvisco" nas listras do gato quando ele fica pequeno na tela.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // gluBuild2DMipmaps envia a imagem e gera as versões reduzidas (mipmaps).
    // Também redimensiona imagens cujo lado não é potência de 2, então o
    // usuário pode abrir qualquer foto pela interface.
    GLint r = gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, largura, altura,
                                GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    stbi_image_free(pixels);  // a imagem já foi copiada para o OpenGL
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

void liberarTextura(GLuint& id) {
    if (id == 0) return;
    NaJanelaPrincipal contexto;
    glDeleteTextures(1, &id);
    id = 0;
}

bool ligarTextura(const Objeto3D& obj) {
    // TODO (Sprint 2): MODO_REFLEXAO com g_texSphereMap e glTexGen.
    bool usar = obj.modo == MODO_TEXTURA && obj.textura != 0 && !obj.malha.uvs.empty();
    if (!usar) {
        glDisable(GL_TEXTURE_2D);
        return false;
    }
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, obj.textura);
    // MODULATE multiplica a cor da textura pela cor do material já iluminada.
    // Assim a luz, o flat/smooth e o alfa continuam valendo no objeto
    // texturizado. Com REPLACE a imagem ficaria "chapada", sem sombra.
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    return true;
}

void desligarTextura() {
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

void enviarUV(const Malha& m, const Triangulo& t, int i) {
    int k = t.vt[i];
    if (k >= 0 && k < (int)m.uvs.size())
        glTexCoord2f(m.uvs[k].u, m.uvs[k].v);
}

void aplicarPreset(Objeto3D& obj, Preset p) {
    // TODO (Sprint 2): valores finais de cada preset
    (void)obj; (void)p;
}
