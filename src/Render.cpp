// Render.cpp — Responsável: Isabel Lugon (Sprints 1 e 2)
#include "Render.h"

void configurarLuz(const Luz& luz) {
    // Versão mínima para o teste da Sprint 0 — Isabel completa na Sprint 1.
    glEnable(GL_LIGHTING);
    if (luz.ligada) glEnable(GL_LIGHT0); else glDisable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, luz.pos);
}

void desenharIndicadorLuz(const Luz& luz) { (void)luz; /* TODO Sprint 1 */ }
void desenharObjeto(const Objeto3D& obj)  { (void)obj; /* TODO Sprint 1 */ }

void desenharCena() {
    for (const auto& o : g_cena) if (o.visivel) desenharObjeto(o);  // TODO Sprint 2: ordenação
}
