# Trabalho 2 – Computação Gráfica (UNICAP)

Sistema Visualizador de Objetos 3D com OpenGL, GLUT e GLUI. Modelo principal: um gato malhado com laço rosa (`assets/modelos/gato.obj`).

**Equipe:** Ana Beatriz, André Mota, Artur Uchôa e Isabel Lugon · **Entrega:** 22/11/2026

---

## Estrutura

```
Trabalho2_CG/
├── src/
│   ├── Common.h        contrato comum (Malha, Material, Objeto3D, Luz, Camera)
│   ├── Cena.cpp        estado global
│   ├── main.cpp        janela, perspectiva, laço principal
│   ├── ObjLoader.*     Artur    – parser .obj
│   ├── Geometria.*     Artur    – normais e UV automática
│   ├── Textura.*       Ana      – stb_image, sphere map, cor do material
│   ├── Render.*        Isabel   – luz, shading, transparência
│   └── Interface.*     André    – painel GLUI e diálogos
├── third_party/        freeglut, GLUI e stb_image (código-fonte, compilados junto)
├── assets/modelos/     .obj (gato + modelos de teste)
├── assets/texturas/    pelo do gato e sphere map
├── Trabalho2_CG.sln    abrir este arquivo no Visual Studio
├── Bibliotecas.props   includes e definições com caminhos relativos
└── .gitignore
```

---

## Sprint 0 – passo a passo

### 1. André – projeto no Visual Studio 2026

O projeto já vem pronto (`Trabalho2_CG.sln`). O **freeglut 3.8** e a **GLUI 2.36** estão em `third_party/` como código-fonte e são compilados junto com o projeto. **Não é preciso instalar nem copiar biblioteca nenhuma.**

1. **Abrir:** dê dois cliques em `Trabalho2_CG.sln` e, se aparecer *Redirecionar Projetos*, aceite.
2. **Plataforma:** escolha **Debug | x64** (x86 também funciona).
3. **Compilar e rodar:** Ctrl+F5. A primeira compilação demora um pouco mais, porque compila as bibliotecas.
4. **Conferir:** deve abrir o **bule laranja** com o painel GLUI à direita. Redimensione a janela: o bule não pode esticar.

### 2. Repositório no GitHub

O repositório já foi criado.
- **Convidar a equipe:** no site, vá em *Settings → Collaborators → Add people* e convide Ana, Artur e Isabel.
- **Cada pessoa:** no Visual Studio, *Git → Clonar Repositório*, cole a URL e abra `Trabalho2_CG.sln`. O bule precisa aparecer na máquina de todos.

**Combinados:**
- Uma branch por tarefa (`feature/parser-obj`, `feature/luz`...), com Pull Request para a `main`.
- Integração no fim de cada sprint.
- Mudanças no `Common.h` são combinadas no grupo antes do push.

> O `.gitignore` ignora `*.obj` do compilador, **mas libera** `assets/**/*.obj`. Sem essa exceção, os modelos 3D não iriam para o GitHub.

### 3. Artur – contrato comum (`Common.h`)

O `Common.h` já tem as estruturas usadas por todos os módulos, e cada módulo tem um `.h` com as funções que precisa implementar. Os stubs compilam, então cada pessoa pode trabalhar na sua parte sem travar as outras.

Os modelos de teste para o parser estão em `assets/modelos/`:

| Arquivo | Testa | Triângulos |
|---|---|---|
| `teste_esfera_vt.obj` | `v`, `vt`, `vn`, faces `v/vt/vn`, triângulos degenerados nos polos | 1920 |
| `teste_toro_sem_vt.obj` | só `v` e `f`, faces em **quads** (triangulação) e geração de UV | 2304 após triangular |

Os dois têm faces em sentido anti-horário, com normais para fora.

### 4. Isabel – modelo do gato

O modelo já está em `assets/modelos/gato.obj`. É um modelo **original** da equipe, sem problema de licença:
- **Malha:** gato malhado sentado, com laço rosa no pescoço, em uma única peça.
- **Tamanho:** **25.024 triângulos** e 12.510 vértices.
- **Formato:** faces `v/vt/vn`, com UV e normais exportadas.
- **Textura:** `assets/texturas/gato_pelo.png` (1024×512), com listras, barriga creme, olhos, nariz e laço.

**Se quiserem trocar por um modelo baixado** (Sketchfab, Poly Pizza, Free3D):
- **Exportar no Blender (File → Export → Wavefront .obj):** marque *Selection Only*, *UV Coordinates*, *Normals* e **Triangulated Mesh**. Deixe *Forward* = `-Z` e *Up* = `Y`.
- **Conferir:** pelo menos 1600 triângulos (veja em *Viewport Overlays → Statistics*). Se a licença for CC-BY, coloque o crédito aqui no README.

### 5. Ana Beatriz – texturas

As texturas provisórias já estão em `assets/texturas/`. Elas foram geradas por script e servem para testar desde já:
- `spheremap.png`: reflexão de ambiente. É uma "bola cromada" refletindo céu, horizonte e chão.

**Pendências:**
- **Textura do gato:** use a que vem com o modelo da Isabel.
- **Tamanhos:** prefira 512 ou 1024 px em potência de 2.

---

## Pronto da Sprint 0

- [ ] Todos clonaram, compilaram e viram o bule com o painel
- [ ] `Bibliotecas.props` aplicado; nenhum caminho absoluto no projeto
- [x] `gato.obj` (25.024 triângulos) e `gato_pelo.png` em `assets/`
- [ ] Texturas e sphere map em `assets/texturas/`
- [ ] Cada pessoa sabe qual arquivo `.cpp` é seu na Sprint 1

---

## Sprint 2 – requisitos do enunciado

O programa é um visualizador de **um objeto**. Abrir outro `.obj` substitui o atual. Implementa **só o que o enunciado pede**, mais três apoios para a demonstração: câmera com o mouse, a esfera amarela que mostra onde está a luz e a cor do modo Sólido.

| Requisito | Painel | Código |
|---|---|---|
| 1 – Nada por linha de comando | todo o painel | `Interface.cpp` |
| 2a – Abrir .obj por caixa de diálogo | Arquivos → Abrir modelo | `Interface.cpp` (`abrirDialogo`), `ObjLoader.cpp` |
| 2b – Perspectiva, janela ≥ 600×600, sem distorcer ao redimensionar | — | `main.cpp` (`reshape`) |
| 2c – Luz liga/desliga e posição | Luz | `Interface.cpp`, `Render.cpp` (`configurarLuz`) |
| 3a – Flat (por face) e Smooth (Gouraud) com seletor | Shading | `Render.cpp`, `Geometria.cpp` (`calcularNormais`) |
| 3b – Alfa, `GL_BLEND`, slider, ordenação dos triângulos e `glDepthMask` | Material → Opacidade | `Render.cpp` (`desenharCena`, `ordenarTriangulos`) |
| 3c – Especular de Phong e shininess | Material → Shininess | `Render.cpp` |
| 3c – Reflexão de ambiente (sphere map), liga/desliga | Modo → Reflexao | `Textura.cpp` (`ligarTextura`) |
| 3d – Carregar .png/.jpg/.bmp | Arquivos → Abrir textura | `Textura.cpp` (`carregarTextura`) |
| 3d – Texturizado / Sólido / Reflexão | Modo | `Textura.cpp` |
| 3d – `vt` do arquivo ou UV automática | (automático; o painel informa qual) | `ObjLoader.cpp`, `Geometria.cpp` (`gerarUV`) |
| 5 – Malha ≥ 1600 triângulos | Arquivos (contador) | `gato.obj`: 25.024 triângulos |

### Roteiro de teste

1. **Abrir o modelo (2a):** *Abrir modelo* → `gato.obj`. Deve mostrar "Triangulos: 25024" e "UV: do arquivo (vt)".
2. **Janela (2b):** redimensione e maximize. O gato não pode esticar.
3. **Luz (2c):** desligue e ligue; mude X, Y e Z e veja a esfera amarela acompanhar.
4. **Shading (3a):** alterne Flat e Smooth. As facetas aparecem no Flat.
5. **Transparência (3b):** baixe a Opacidade e gire a câmera com o mouse. Não podem aparecer faces piscando nem "buracos".
6. **Brilho (3c):** mova o slider de Shininess. O brilho fica mais concentrado ou mais espalhado.
7. **Reflexão (3c):** Modo → Reflexao e depois Solido. É o liga/desliga.
8. **Textura (3d):** *Abrir textura* → `gato_pelo.png`. Alterne Texturizado e Solido e troque a Cor no Sólido.
9. **UV automática (3d):** abra `teste_toro_sem_vt.obj`. Deve mostrar "UV: gerada (esferica)". Com a textura, o toro aparece texturizado.
