// =============================================================================
//  main.cpp — Trabalho 2 de Computação Gráfica (UNICAP, Prof. Robson Lins)
//  Sistema Visualizador de Objetos 3D com OpenGL, GLUT e GLUI
//  Equipe: Ana Beatriz, André Mota, Artur Uchôa e Isabel Lugon
// =============================================================================
#include "Common.h"
#include "Interface.h"
#include "Render.h"
#include <GL/glui.h>
#include <algorithm>

// Enquanto nenhum modelo foi aberto, mostra um bule para a tela não ficar vazia.
static void desenharPlaceholder() {
    GLfloat cor[] = { 0.9f, 0.55f, 0.25f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cor);
    glShadeModel(GL_SMOOTH);
    glutSolidTeapot(1.0);
}

static void display() {
    glClearColor(0.93f, 0.91f, 0.87f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -g_camera.distancia);
    glRotatef(g_camera.rotX, 1, 0, 0);
    glRotatef(g_camera.rotY, 0, 1, 0);

    configurarLuz(g_luz);  // depois da câmera: a luz fica fixa no mundo
    desenharIndicadorLuz(g_luz);

    if (g_cena.empty()) desenharPlaceholder();
    else                desenharCena();

    glutSwapBuffers();
}

// Requisito 2b: perspectiva + aspect ratio da área útil (descontando o painel)
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

// -----------------------------------------------------------------------------
// Câmera com o mouse: botão esquerdo gira, botão direito ou roda aproxima
// -----------------------------------------------------------------------------
static int s_botao = -1;
static int s_xAnt = 0, s_yAnt = 0;

static void aproximar(float fator) {
    // Multiplica em vez de somar: o zoom fica com a mesma "velocidade"
    // de perto e de longe.
    g_camera.distancia = std::max(1.5f, std::min(30.0f, g_camera.distancia * fator));
    glutPostRedisplay();
}

static void mouse(int botao, int estado, int x, int y) {
    // Sem callback de roda registrado, o freeglut entrega a roda do mouse
    // como os botões 3 (para frente) e 4 (para trás).
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
        // Limita a inclinação para a câmera não passar por cima e virar
        // o modelo de cabeça para baixo.
        g_camera.rotX = std::max(-89.0f, std::min(89.0f, g_camera.rotX + dy * 0.5f));
        glutPostRedisplay();
    } else if (s_botao == GLUT_RIGHT_BUTTON) {
        aproximar(1.0f + dy * 0.01f);
    }
}

static void inicializarGL() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);  // normais corretas mesmo com glScale
    glShadeModel(GL_SMOOTH);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(JANELA_LARGURA, JANELA_ALTURA);
    glutInitWindowPosition(80, 40);
    g_janela = glutCreateWindow("Trabalho 2 - Visualizador 3D");

    inicializarGL();
    glutDisplayFunc(display);
    glutMotionFunc(movimento);
    // Com GLUI, os callbacks de reshape, mouse, teclado e idle são registrados
    // pelo GLUI_Master, e antes de criar o painel (a janela atual ainda é a
    // principal).
    GLUI_Master.set_glutReshapeFunc(reshape);
    GLUI_Master.set_glutMouseFunc(mouse);

    criarInterface(g_janela);
    GLUI_Master.set_glutIdleFunc(nullptr);

    glutMainLoop();
    return 0;
}
