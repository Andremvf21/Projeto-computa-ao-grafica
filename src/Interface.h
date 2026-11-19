// =============================================================================
//  Interface — painel GLUI, diálogos de arquivo e lista de objetos
//  Responsável: André Mota           Requisitos: 1, 2a, 2b e controles de todos
// =============================================================================
#pragma once
#include "Common.h"

// Cria o painel GLUI à direita da janela principal.
void criarInterface(int janelaPrincipal);

// Abre a caixa "Abrir arquivo" do Windows. 'filtro' no formato
// "Modelos OBJ\0*.obj\0". Retorna false se o usuário cancelar.
bool abrirDialogo(const char* filtro, std::string& caminho);

// Copia os valores do objeto selecionado para os controles do painel.
void sincronizarPainel();
