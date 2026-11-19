// =============================================================================
//  Cena — cena padrão (mesa, gato e bola) e animação
//  Sprint 2
// =============================================================================
#pragma once
#include "Common.h"

// Acha um arquivo da pasta assets mesmo quando o programa é aberto fora do
// Visual Studio (o .exe fica em x64\Debug, dois níveis abaixo do projeto).
// Retorna o caminho que existe, ou o próprio 'relativo' se nenhum existir.
std::string caminhoAsset(const std::string& relativo);

// Carrega o sphere map e monta a cena inicial: mesa (fixa), gato e bola.
// Erros (arquivo não encontrado) vão para 'avisos', sem fechar o programa.
void montarCenaPadrao(std::string& avisos);

// Avança a animação dos objetos marcados como "animado".
// Retorna true se algum objeto se mexeu (precisa redesenhar).
bool atualizarAnimacao(float segundos);
