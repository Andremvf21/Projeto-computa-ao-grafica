// Geometria.cpp — normais, coordenadas de textura e malhas procedurais
#include "Geometria.h"
#include <algorithm>

namespace {
const float PI = 3.14159265358979f;

void caixaEnvolvente(const Malha& m, Vec3& mn, Vec3& mx) {
    mn = mx = m.vertices.empty() ? Vec3() : m.vertices[0];
    for (const Vec3& v : m.vertices) {
        mn.x = std::min(mn.x, v.x); mx.x = std::max(mx.x, v.x);
        mn.y = std::min(mn.y, v.y); mx.y = std::max(mx.y, v.y);
        mn.z = std::min(mn.z, v.z); mx.z = std::max(mx.z, v.z);
    }
}

// Ângulo em volta do eixo Y convertido para 0..1.
// atan2(x, z) vale 0 na frente (+Z) e ±PI atrás (-Z): assim a "costura",
// onde u volta de 1 para 0, fica nas costas do modelo, longe da câmera.
float anguloU(const Vec3& p) {
    return 0.5f + std::atan2(p.x, p.z) / (2.0f * PI);
}
}  // namespace

void normalizarMalha(Malha& m) {
    if (m.vertices.empty()) return;

    // Usa o centro da caixa envolvente, e não a média dos vértices: a média
    // puxa o centro para onde a malha é mais detalhada (a cabeça do gato,
    // por exemplo) e o modelo ficaria fora do centro da tela.
    Vec3 mn, mx;
    caixaEnvolvente(m, mn, mx);
    Vec3 centro = (mn + mx) * 0.5f;

    float raio = 0.0f;
    for (Vec3& v : m.vertices) {
        v = v - centro;
        raio = std::max(raio, v.comprimento());
    }
    if (raio < 1e-8f) raio = 1.0f;

    // Escala para raio 1: qualquer modelo, de qualquer tamanho no arquivo,
    // aparece do mesmo tamanho com a câmera na distância padrão.
    float s = 1.0f / raio;
    for (Vec3& v : m.vertices) v = v * s;

    m.centro = Vec3(0, 0, 0);
    m.raio = 1.0f;
}

void calcularNormais(Malha& m) {
    m.normaisVertice.assign(m.vertices.size(), Vec3(0, 0, 0));

    for (Triangulo& t : m.triangulos) {
        const Vec3& a = m.vertices[t.v[0]];
        const Vec3& b = m.vertices[t.v[1]];
        const Vec3& c = m.vertices[t.v[2]];

        // Com os vértices em sentido anti-horário, o produto vetorial aponta
        // para fora. O comprimento dele é o dobro da área do triângulo.
        Vec3 n = cross(b - a, c - a);
        t.normalFace = n.normalizado();   // Flat: uma normal por face

        // Smooth: soma sem normalizar, então faces maiores pesam mais na
        // média. Triângulos degenerados (área zero) não atrapalham.
        for (int i = 0; i < 3; ++i) m.normaisVertice[t.v[i]] += n;
    }
    for (Vec3& n : m.normaisVertice) n = n.normalizado();
}

void gerarUV(Malha& m) {
    // A UV é gerada por canto de triângulo (3 por triângulo), e não por
    // vértice. Assim um vértice na costura pode ter u perto de 0 num
    // triângulo e perto de 1 no vizinho, sem a faixa esticada que aparece
    // quando um triângulo atravessa a costura.
    m.uvs.clear();
    m.uvs.reserve(m.triangulos.size() * 3);

    for (Triangulo& t : m.triangulos) {
        Vec2 uv[3];
        bool semAngulo[3] = { false, false, false };

        // Projeção esférica a partir do centro (o modelo já está normalizado
        // em volta da origem): u = ângulo em volta do eixo Y, v = altura angular.
        for (int i = 0; i < 3; ++i) {
            const Vec3& p = m.vertices[t.v[i]];
            float r = std::max(p.comprimento(), 1e-8f);
            float y = std::max(-1.0f, std::min(1.0f, p.y / r));
            uv[i] = Vec2(anguloU(p), 0.5f + std::asin(y) / PI);
            semAngulo[i] = std::sqrt(p.x * p.x + p.z * p.z) < 1e-5f;  // polo
        }

        // Costura: se os u do triângulo estão a mais de meia volta de
        // distância, ele atravessa a costura. Soma 1 aos que estão perto
        // de 0 (o GL_REPEAT da textura trata u > 1 como continuação).
        float uMin = 2, uMax = -1;
        for (int i = 0; i < 3; ++i)
            if (!semAngulo[i]) { uMin = std::min(uMin, uv[i].u); uMax = std::max(uMax, uv[i].u); }
        if (uMax - uMin > 0.5f)
            for (int i = 0; i < 3; ++i)
                if (!semAngulo[i] && uv[i].u < 0.5f) uv[i].u += 1.0f;

        // No polo o ângulo não existe: usa a média dos outros cantos.
        float soma = 0; int n = 0;
        for (int i = 0; i < 3; ++i) if (!semAngulo[i]) { soma += uv[i].u; ++n; }
        for (int i = 0; i < 3; ++i) if (semAngulo[i]) uv[i].u = n ? soma / n : 0.5f;

        int base = (int)m.uvs.size();
        for (int i = 0; i < 3; ++i) {
            m.uvs.push_back(uv[i]);
            t.vt[i] = base + i;
        }
    }
}
