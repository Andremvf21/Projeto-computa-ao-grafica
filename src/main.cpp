#include "Common.h"
#include "Interface.h"
#include "Render.h"
#include "Textura.h"
#include <GL/glui.h>
#include <algorithm>
#include <cstdio>

// Fundo em degradê, desenhado sem profundidade
static void desenharFundo() {
    glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_CURRENT_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);  glPushMatrix(); glLoadIdentity();
    glBegin(GL_QUADS);
    glColor3f(0.97f, 0.96f, 0.94f); glVertex2f(-1, -1); glVertex2f(1, -1);
    glColor3f(0.78f, 0.84f, 0.92f); glVertex2f(1, 1);   glVertex2f(-1, 1);
    glEnd();
    glPopMatrix();
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopAttrib();
}

static void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    desenharFundo();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -g_camera.distancia);
    glRotatef(g_camera.rotX, 1, 0, 0);
    glRotatef(g_camera.rotY, 0, 1, 0);

    configurarLuz(g_luz);  // depois da câmera: luz fixa no mundo
    desenharIndicadorLuz(g_luz);
    desenharCena();

    glutSwapBuffers();
}

// Perspectiva com o aspect ratio da área sem o painel
static void reshape(int, int) {
    int tx, ty, tw, th;
    GLUI_Master.get_viewport_area(&tx, &ty, &tw, &th);
    if (th == 0) th = 1;
    glViewport(tx, ty, tw, th);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(g_camera.fov, (double)tw / (double)th, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glutPostRedisplay();
}

// Câmera: botão esquerdo gira; botão direito ou roda aproxima
static int s_botao = -1;
static int s_xAnt = 0, s_yAnt = 0;

static void aproximar(float fator) {
    g_camera.distancia = std::max(1.5f, std::min(30.0f, g_camera.distancia * fator));
    glutPostRedisplay();
}

static void mouse(int botao, int estado, int x, int y) {
    // Roda do mouse chega como botões 3 e 4
    if (botao == 3 || botao == 4) {
        if (estado == GLUT_DOWN) aproximar(botao == 3 ? 0.9f : 1.1f);
        return;
    }
    s_botao = (estado == GLUT_DOWN) ? botao : -1;
    s_xAnt = x;
    s_yAnt = y;
}

static void movimento(int x, int y) {
    int dx = x - s_xAnt, dy = y - s_yAnt;
    s_xAnt = x;
    s_yAnt = y;

    if (s_botao == GLUT_LEFT_BUTTON) {
        g_camera.rotY += dx * 0.5f;
        g_camera.rotX = std::max(-89.0f, std::min(89.0f, g_camera.rotX + dy * 0.5f));
        glutPostRedisplay();
    } else if (s_botao == GLUT_RIGHT_BUTTON) {
        aproximar(1.0f + dy * 0.01f);
    }
}

static void inicializarGL() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(JANELA_LARGURA, JANELA_ALTURA);
    glutInitWindowPosition(80, 40);
    g_janela = glutCreateWindow("Trabalho 2 - Visualizador 3D");

    inicializarGL();

    // Sphere map da reflexão (precisa da janela já criada)
    std::string erro;
    g_texSphereMap = carregarTextura("assets/texturas/spheremap.png", erro);
    if (g_texSphereMap == 0) std::printf("Aviso: %s\n", erro.c_str());

    glutDisplayFunc(display);
    glutMotionFunc(movimento);
    // Com GLUI, reshape e mouse são registrados pelo GLUI_Master
    GLUI_Master.set_glutReshapeFunc(reshape);
    GLUI_Master.set_glutMouseFunc(mouse);

    criarInterface(g_janela);
    GLUI_Master.set_glutIdleFunc(nullptr);

    glutMainLoop();
    return 0;
}
