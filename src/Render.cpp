// Render.cpp — iluminação, shading, transparência e desenho da cena
#include "Render.h"
#include "Textura.h"
#include <algorithm>
#include <numeric>

void configurarLuz(const Luz& luz) {
    glEnable(GL_LIGHTING);
    if (luz.ligada) glEnable(GL_LIGHT0); else glDisable(GL_LIGHT0);

    // Um pouco de luz ambiente na própria lâmpada clareia o lado de sombra.
    // Como ela pertence à GL_LIGHT0, some junto quando a luz é desligada,
    // e a diferença entre ligada e desligada fica bem visível.
    static const GLfloat ambiente[]  = { 0.25f, 0.25f, 0.25f, 1.0f };
    static const GLfloat difusa[]    = { 1.0f, 1.0f, 1.0f, 1.0f };
    static const GLfloat especular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT,  ambiente);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  difusa);
    glLightfv(GL_LIGHT0, GL_SPECULAR, especular);

    // Especular de Phong (requisito 3c):
    //  - LOCAL_VIEWER: o brilho é calculado com a direção real até a câmera,
    //    então o ponto brilhante "anda" na superfície quando a câmera gira.
    //  - SEPARATE_SPECULAR_COLOR: o brilho é somado depois da textura. Sem
    //    isso, a textura (MODULATE) escurece o brilho e ele quase some.
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);

    // A posição passa pela matriz MODELVIEW atual. Como esta função é chamada
    // depois da câmera, a luz fica parada no mundo e não "gira junto" com ela.
    glLightfv(GL_LIGHT0, GL_POSITION, luz.pos);
}

void desenharIndicadorLuz(const Luz& luz) {
    // A esfera é desenhada sem iluminação, só com cor, para parecer acesa:
    // amarela com a luz ligada, cinza com ela desligada.
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    if (luz.ligada) glColor3f(1.0f, 0.85f, 0.1f);
    else            glColor3f(0.35f, 0.35f, 0.35f);
    glPushMatrix();
    glTranslatef(luz.pos[0], luz.pos[1], luz.pos[2]);
    glutSolidSphere(0.08, 16, 12);
    glPopMatrix();
    glPopAttrib();
}

namespace {

// Ordem dos triângulos do mais distante para o mais próximo da câmera
// (requisito 3b). Usa a profundidade do centro de cada triângulo no espaço da
// câmera: z = linha 3 da MODELVIEW vezes o ponto. No OpenGL a câmera olha para -z, então z mais negativo = mais longe.
void ordenarTriangulos(const Malha& m, std::vector<int>& ordem) {
    GLfloat mv[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mv);   // matriz em colunas: z = mv[2]x + mv[6]y + mv[10]z + mv[14]

    static std::vector<float> prof;
    const size_t n = m.triangulos.size();
    prof.resize(n);
    for (size_t k = 0; k < n; ++k) {
        const Triangulo& t = m.triangulos[k];
        const Vec3& a = m.vertices[t.v[0]];
        const Vec3& b = m.vertices[t.v[1]];
        const Vec3& c = m.vertices[t.v[2]];
        // Soma dos 3 vértices em vez da média: a ordem é a mesma e poupa a divisão.
        float x = a.x + b.x + c.x, y = a.y + b.y + c.y, z = a.z + b.z + c.z;
        prof[k] = mv[2] * x + mv[6] * y + mv[10] * z;
    }
    ordem.resize(n);
    std::iota(ordem.begin(), ordem.end(), 0);
    std::sort(ordem.begin(), ordem.end(), [](int i, int j) { return prof[i] < prof[j]; });
}

void desenhar(const Objeto3D& obj, bool ordenar) {
    const Malha& m = obj.malha;
    if (m.triangulos.empty()) return;

    // O alfa do material vai na componente difusa: é ela que define o alfa
    // da cor final calculada pela iluminação.
    GLfloat difusa[4] = { obj.mat.difusa[0], obj.mat.difusa[1], obj.mat.difusa[2], obj.mat.alpha };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   obj.mat.ambiente);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   difusa);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  obj.mat.especular);
    glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, std::max(0.0f, std::min(128.0f, obj.mat.shininess)));

    // Flat: uma normal por face e GL_FLAT, então o triângulo inteiro recebe
    // uma cor só e as facetas aparecem. Smooth (Gouraud): uma normal por
    // vértice (média das faces vizinhas) e GL_SMOOTH, que interpola a cor
    // calculada em cada vértice pelo triângulo.
    bool flat = obj.shading == SHADING_FLAT;
    bool temNormaisVertice = m.normaisVertice.size() == m.vertices.size();
    glShadeModel(flat ? GL_FLAT : GL_SMOOTH);

    bool comTextura = ligarTextura(obj);

    static std::vector<int> ordem;
    if (ordenar) ordenarTriangulos(m, ordem);
    const size_t n = m.triangulos.size();

    glBegin(GL_TRIANGLES);
    for (size_t k = 0; k < n; ++k) {
        const Triangulo& t = m.triangulos[ordenar ? ordem[k] : k];
        if (flat || !temNormaisVertice)
            glNormal3f(t.normalFace.x, t.normalFace.y, t.normalFace.z);
        for (int i = 0; i < 3; ++i) {
            if (!flat && temNormaisVertice) {
                const Vec3& nv = m.normaisVertice[t.v[i]];
                glNormal3f(nv.x, nv.y, nv.z);
            }
            if (comTextura) enviarUV(m, t, i);
            const Vec3& v = m.vertices[t.v[i]];
            glVertex3f(v.x, v.y, v.z);
        }
    }
    glEnd();

    if (comTextura) desligarTextura();
}

}  // namespace

void desenharObjeto(const Objeto3D& obj) {
    desenhar(obj, false);
}

void desenharCena() {
    const Objeto3D* obj = objetoSelecionado();
    if (!obj) return;

    if (obj->mat.alpha >= 0.999f) {   // opaco: desenho normal
        desenhar(*obj, false);
        return;
    }

    // Transparente (requisito 3b)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Sem gravar no depth buffer: uma face transparente da frente não pode
    // "apagar" uma de trás. Além disso os triângulos são desenhados do mais
    // longe para o mais perto (ordenar = true), para a mistura sair na ordem
    // certa de qualquer ângulo.
    glDepthMask(GL_FALSE);
    // As faces de dentro aparecem através do objeto: com TWO_SIDE elas são
    // iluminadas com a normal invertida, em vez de ficarem escuras.
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    desenhar(*obj, true);

    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
