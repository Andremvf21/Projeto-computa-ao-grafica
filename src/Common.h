// =============================================================================
//  Common.h — "contrato" de estruturas compartilhadas por toda a equipe
//  Trabalho 2 de Computação Gráfica (UNICAP) — Visualizador de Objetos 3D
//
//  Regra da equipe: qualquer mudança neste arquivo é combinada no grupo antes
//  do push, porque todos os módulos dependem dele.
// =============================================================================
#pragma once

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/glut.h>

#include <cmath>
#include <string>
#include <vector>

// -----------------------------------------------------------------------------
// Vetores
// -----------------------------------------------------------------------------
struct Vec2 {
    float u = 0, v = 0;
    Vec2() = default;
    Vec2(float u_, float v_) : u(u_), v(v_) {}
};

struct Vec3 {
    float x = 0, y = 0, z = 0;
    Vec3() = default;
    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    Vec3 operator+(const Vec3& o) const { return { x + o.x, y + o.y, z + o.z }; }
    Vec3 operator-(const Vec3& o) const { return { x - o.x, y - o.y, z - o.z }; }
    Vec3 operator*(float s)       const { return { x * s, y * s, z * s }; }
    Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }

    float comprimento() const { return std::sqrt(x * x + y * y + z * z); }
    Vec3 normalizado() const {
        float c = comprimento();
        return c > 1e-8f ? Vec3(x / c, y / c, z / c) : Vec3(0, 1, 0);
    }
};

inline Vec3  cross(const Vec3& a, const Vec3& b) { return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x }; }
inline float dot(const Vec3& a, const Vec3& b)   { return a.x * b.x + a.y * b.y + a.z * b.z; }

// -----------------------------------------------------------------------------
// Malha (preenchida pelo parser OBJ ou pela geração procedural)
// -----------------------------------------------------------------------------
struct Triangulo {
    int  v[3]  = { 0, 0, 0 };     // índices em Malha::vertices
    int  vt[3] = { -1, -1, -1 };  // índices em Malha::uvs (-1 = sem coordenada)
    Vec3 normalFace;              // usada no Flat Shading (requisito 3a)
};

struct Malha {
    std::vector<Vec3>      vertices;
    std::vector<Vec3>      normaisVertice;  // média das faces vizinhas (Smooth, 3a)
    std::vector<Vec2>      uvs;             // vindas do 'vt' ou geradas (3d)
    std::vector<Triangulo> triangulos;

    bool  temUVdoArquivo = false;  // true se o .obj trouxe 'vt'
    Vec3  centro;                  // após normalizar: (0,0,0)
    float raio = 1.0f;             // raio da esfera envolvente

    int numTriangulos() const { return (int)triangulos.size(); }
    void limpar() { *this = Malha(); }
};

// -----------------------------------------------------------------------------
// Material e modos de visualização
// -----------------------------------------------------------------------------
enum Shading    { SHADING_FLAT = 0, SHADING_SMOOTH = 1 };
enum ModoVisual { MODO_SOLIDO = 0, MODO_TEXTURA = 1, MODO_REFLEXAO = 2 };
enum TipoUV     { UV_ESFERICA = 0, UV_CILINDRICA = 1, UV_PLANAR = 2 };
enum Preset     { PRESET_VIDRO = 0, PRESET_CROMO = 1, PRESET_FOSCO = 2, PRESET_TEXTURA = 3 };

struct Material {
    float ambiente[4]  = { 0.2f, 0.2f, 0.2f, 1.0f };
    float difusa[4]    = { 0.8f, 0.8f, 0.8f, 1.0f };
    float especular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float shininess    = 32.0f;  // 0..128 (requisito 3c)
    float alpha        = 1.0f;   // 0..1   (requisito 3b)
};

// -----------------------------------------------------------------------------
// Objeto da cena (a cena aceita N objetos)
// -----------------------------------------------------------------------------
struct Objeto3D {
    std::string nome = "objeto";
    Malha       malha;
    Material    mat;
    Shading     shading = SHADING_SMOOTH;
    ModoVisual  modo    = MODO_SOLIDO;
    TipoUV      tipoUV  = UV_ESFERICA;
    GLuint      textura = 0;          // 0 = sem textura carregada

    float pos[3] = { 0, 0, 0 };
    float rot[3] = { 0, 0, 0 };       // graus em X, Y, Z
    float escala = 1.0f;
    bool  visivel = true;
    bool  removivel = true;           // a mesa é fixa (false)
};

// -----------------------------------------------------------------------------
// Luz e estado global
// -----------------------------------------------------------------------------
struct Luz {
    bool  ligada  = true;
    float pos[4]  = { 2.0f, 4.0f, 3.0f, 1.0f };  // w = 1 -> luz pontual
};

struct Camera {
    float distancia = 6.0f;
    float rotX = 20.0f, rotY = -30.0f;  // graus
    float fov  = 45.0f;
};

// Definidos em Cena.cpp
extern std::vector<Objeto3D> g_cena;
extern int    g_idSelecionado;
extern Luz    g_luz;
extern Camera g_camera;
extern GLuint g_texSphereMap;
extern int    g_janela;

inline Objeto3D* objetoSelecionado() {
    if (g_idSelecionado < 0 || g_idSelecionado >= (int)g_cena.size()) return nullptr;
    return &g_cena[g_idSelecionado];
}

// Constantes da janela (requisito 2b: mínimo 600x600)
constexpr int JANELA_LARGURA = 1000;
constexpr int JANELA_ALTURA  = 700;
constexpr float ALTURA_MESA  = 0.0f;  // y do tampo da mesa; objetos nascem aqui
