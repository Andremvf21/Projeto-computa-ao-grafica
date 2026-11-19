// Render.cpp — iluminação, shading e desenho dos objetos
#include "Render.h"
#include "Textura.h"

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

void desenharObjeto(const Objeto3D& obj) {
    const Malha& m = obj.malha;
    if (m.triangulos.empty()) return;

    glPushMatrix();
    glTranslatef(obj.pos[0], obj.pos[1], obj.pos[2]);
    glRotatef(obj.rot[0], 1, 0, 0);
    glRotatef(obj.rot[1], 0, 1, 0);
    glRotatef(obj.rot[2], 0, 0, 1);
    glScalef(obj.escala, obj.escala, obj.escala);

    // O alfa do material vai na componente difusa (usado na Sprint 2).
    GLfloat difusa[4] = { obj.mat.difusa[0], obj.mat.difusa[1], obj.mat.difusa[2], obj.mat.alpha };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   obj.mat.ambiente);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   difusa);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  obj.mat.especular);
    glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, obj.mat.shininess);

    // Flat: uma normal por face e GL_FLAT, então o triângulo inteiro recebe
    // uma cor só e as facetas aparecem. Smooth (Gouraud): uma normal por
    // vértice (média das faces vizinhas) e GL_SMOOTH, que interpola a cor
    // calculada em cada vértice pelo triângulo.
    bool flat = obj.shading == SHADING_FLAT;
    bool temNormaisVertice = m.normaisVertice.size() == m.vertices.size();
    glShadeModel(flat ? GL_FLAT : GL_SMOOTH);

    bool comTextura = ligarTextura(obj);

    glBegin(GL_TRIANGLES);
    for (const Triangulo& t : m.triangulos) {
        if (flat || !temNormaisVertice)
            glNormal3f(t.normalFace.x, t.normalFace.y, t.normalFace.z);
        for (int i = 0; i < 3; ++i) {
            if (!flat && temNormaisVertice) {
                const Vec3& n = m.normaisVertice[t.v[i]];
                glNormal3f(n.x, n.y, n.z);
            }
            if (comTextura) enviarUV(m, t, i);
            const Vec3& v = m.vertices[t.v[i]];
            glVertex3f(v.x, v.y, v.z);
        }
    }
    glEnd();

    if (comTextura) desligarTextura();
    glPopMatrix();
}

void desenharCena() {
    for (const auto& o : g_cena) if (o.visivel) desenharObjeto(o);  // TODO Sprint 2: ordenação
}
