// ObjLoader.cpp — parser de arquivos .obj
#include "ObjLoader.h"
#include "Geometria.h"
#include <cstdio>
#include <cstdlib>

namespace {

// Canto de face com índices em base 0 (-1 = ausente)
struct Canto { int v = -1, vt = -1; };

// Índices do .obj começam em 1; negativos contam do fim da lista
int converterIndice(long idx, int tamanhoAtual) {
    if (idx > 0) return (int)idx - 1;
    if (idx < 0) return tamanhoAtual + (int)idx;
    return -1;
}

// Lê "v", "v/vt", "v//vn" ou "v/vt/vn"
bool lerCanto(const char* token, int nV, int nVT, Canto& c) {
    char* fim = nullptr;
    long iv = std::strtol(token, &fim, 10);
    if (fim == token) return false;
    c.v = converterIndice(iv, nV);
    c.vt = -1;
    if (*fim == '/') {
        const char* p = fim + 1;
        if (*p != '/') {
            long ivt = std::strtol(p, &fim, 10);
            if (fim != p) c.vt = converterIndice(ivt, nVT);
        }
        // vn é ignorado: as normais são recalculadas
    }
    return true;
}

}  // namespace

bool carregarOBJ(const std::string& caminho, Malha& saida, std::string& erro) {
    FILE* f = std::fopen(caminho.c_str(), "rb");
    if (!f) {
        erro = "Nao foi possivel abrir o arquivo " + caminho;
        return false;
    }

    Malha m;
    std::vector<Canto> cantos;
    int facesIgnoradas = 0;
    bool todosOsCantosTemVT = true;

    char linha[4096];
    while (std::fgets(linha, sizeof(linha), f)) {
        char* p = linha;
        while (*p == ' ' || *p == '\t') ++p;

        if (p[0] == 'v' && (p[1] == ' ' || p[1] == '\t')) {
            Vec3 v;
            char* q = p + 2;
            v.x = std::strtof(q, &q);
            v.y = std::strtof(q, &q);
            v.z = std::strtof(q, &q);
            m.vertices.push_back(v);
        }
        else if (p[0] == 'v' && p[1] == 't' && (p[2] == ' ' || p[2] == '\t')) {
            Vec2 t;
            char* q = p + 3;
            t.u = std::strtof(q, &q);
            t.v = std::strtof(q, &q);
            m.uvs.push_back(t);
        }
        else if (p[0] == 'f' && (p[1] == ' ' || p[1] == '\t')) {
            cantos.clear();
            bool valida = true;
            char* q = p + 2;
            while (true) {
                while (*q == ' ' || *q == '\t') ++q;
                if (*q == '\0' || *q == '\r' || *q == '\n') break;
                Canto c;
                if (!lerCanto(q, (int)m.vertices.size(), (int)m.uvs.size(), c)) break;
                if (c.v < 0 || c.v >= (int)m.vertices.size()) valida = false;
                if (c.vt >= (int)m.uvs.size()) c.vt = -1;
                cantos.push_back(c);
                while (*q && *q != ' ' && *q != '\t' && *q != '\r' && *q != '\n') ++q;
            }
            if (!valida || cantos.size() < 3) { ++facesIgnoradas; continue; }

            // Polígono de n vértices vira n-2 triângulos em leque
            for (size_t k = 1; k + 1 < cantos.size(); ++k) {
                const Canto* tri[3] = { &cantos[0], &cantos[k], &cantos[k + 1] };
                Triangulo t;
                for (int i = 0; i < 3; ++i) {
                    t.v[i]  = tri[i]->v;
                    t.vt[i] = tri[i]->vt;
                    if (t.vt[i] < 0) todosOsCantosTemVT = false;
                }
                m.triangulos.push_back(t);
            }
        }
    }
    std::fclose(f);

    if (m.triangulos.empty()) {
        erro = "O arquivo nao tem faces validas: " + caminho;
        return false;
    }
    if (facesIgnoradas > 0)
        std::printf("Aviso: %d face(s) com indice invalido foram ignoradas.\n", facesIgnoradas);

    // Usa os 'vt' só se todos os cantos tiverem; senão, gera a UV
    m.temUVdoArquivo = !m.uvs.empty() && todosOsCantosTemVT;

    normalizarMalha(m);
    calcularNormais(m);
    if (!m.temUVdoArquivo) gerarUV(m);

    saida = std::move(m);
    erro.clear();
    return true;
}
