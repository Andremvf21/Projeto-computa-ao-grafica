// Geometria.cpp — normalização, normais e coordenadas de textura
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

// Ângulo em volta de Y em 0..1; a costura fica nas costas (-Z).
float anguloU(const Vec3& p) {
    return 0.5f + std::atan2(p.x, p.z) / (2.0f * PI);
}
}  // namespace

void normalizarMalha(Malha& m) {
    if (m.vertices.empty()) return;

    // Centro da caixa envolvente (a média dos vértices puxaria para as partes mais detalhadas)
    Vec3 mn, mx;
    caixaEnvolvente(m, mn, mx);
    Vec3 centro = (mn + mx) * 0.5f;

    float raio = 0.0f;
    for (Vec3& v : m.vertices) {
        v = v - centro;
        raio = std::max(raio, v.comprimento());
    }
    if (raio < 1e-8f) raio = 1.0f;

    float s = 1.0f / raio;
    for (Vec3& v : m.vertices) v = v * s;
}

void calcularNormais(Malha& m) {
    m.normaisVertice.assign(m.vertices.size(), Vec3(0, 0, 0));

    for (Triangulo& t : m.triangulos) {
        const Vec3& a = m.vertices[t.v[0]];
        const Vec3& b = m.vertices[t.v[1]];
        const Vec3& c = m.vertices[t.v[2]];

        Vec3 n = cross(b - a, c - a);
        t.normalFace = n.normalizado();

        // Soma sem normalizar: faces maiores pesam mais na média
        for (int i = 0; i < 3; ++i) m.normaisVertice[t.v[i]] += n;
    }
    for (Vec3& n : m.normaisVertice) n = n.normalizado();
}

void gerarUV(Malha& m) {
    // Uma UV por canto de triângulo, para tratar a costura sem esticar a textura
    m.uvs.clear();
    m.uvs.reserve(m.triangulos.size() * 3);

    for (Triangulo& t : m.triangulos) {
        Vec2 uv[3];
        bool semAngulo[3] = { false, false, false };

        // u = ângulo em volta de Y, v = latitude
        for (int i = 0; i < 3; ++i) {
            const Vec3& p = m.vertices[t.v[i]];
            float r = std::max(p.comprimento(), 1e-8f);
            float y = std::max(-1.0f, std::min(1.0f, p.y / r));
            uv[i] = Vec2(anguloU(p), 0.5f + std::asin(y) / PI);
            semAngulo[i] = std::sqrt(p.x * p.x + p.z * p.z) < 1e-5f;  // polo
        }

        // Triângulo na costura: soma 1 aos u perto de 0 (GL_REPEAT)
        float uMin = 2, uMax = -1;
        for (int i = 0; i < 3; ++i)
            if (!semAngulo[i]) { uMin = std::min(uMin, uv[i].u); uMax = std::max(uMax, uv[i].u); }
        if (uMax - uMin > 0.5f)
            for (int i = 0; i < 3; ++i)
                if (!semAngulo[i] && uv[i].u < 0.5f) uv[i].u += 1.0f;

        // No polo, u é a média dos outros cantos
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
