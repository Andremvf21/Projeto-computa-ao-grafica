// Interface.h — painel GLUI e diálogos de arquivo
#pragma once
#include "Common.h"

void criarInterface(int janelaPrincipal);

// Caixa "Abrir arquivo" do Windows. Retorna false se o usuário cancelar.
bool abrirDialogo(const char* filtro, std::string& caminho, const char* pastaInicial = nullptr);

// Copia os valores do objeto aberto para os controles.
void sincronizarPainel();
