// =============================================================================
//  main.cpp — Trabalho 2 de Computação Gráfica (UNICAP, Prof. Robson Lins)
//  Sistema Visualizador de Objetos 3D com OpenGL, GLUT e GLUI
//  Equipe: Ana Beatriz, André Mota, Artur Uchôa e Isabel Lugon
// =============================================================================
#include "Common.h"
#include "Interface.h"
#include "Render.h"
#include <GL/glui.h>

// Sprint 0: enquanto não há objetos carregados, mostra um bule de teste
// para confirmar que janela, perspectiva, luz e GLUI funcionam juntos.
static void desenharPlaceholder() {
    GLfloat cor[] = { 0.9f, 0.55f, 0.25f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cor);
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
    GLUI_Master.set_glutReshapeFunc(reshape);  // com GLUI, use sempre GLUI_Master.set_*

    criarInterface(g_janela);
    GLUI_Master.set_glutIdleFunc(nullptr);

    glutMainLoop();
    return 0;
}
