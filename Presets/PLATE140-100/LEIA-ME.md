# PLATE 140 — banco original de 100 presets, receitas de timbre versao 2 (schema XML 1)

Os 100 presets ja estao embutidos no projeto atualizado. Nenhuma conversao de parametros e necessaria: selecionar um preset aplica os 20 IDs reais do plugin, incluindo os tempos independentes das placas A/B/C.

## Como usar
1. Abra PLATE 140 no host. Clique no nome do preset na barra superior.
2. O navegador abre em uma janela **flutuante, arrastavel e nao modal**. Continue mexendo no painel com a janela aberta.
3. Procure pelo nome, escolha a categoria ou uma tag (Short, Long, Dark, Bright, Guitar, Vocal, Keys, Drums). Uma tag por vez pode combinar com a busca e a categoria. Best Of mostra favoritos.
4. Clique no preset para carregar; as setas do teclado tambem carregam imediatamente; Enter reaplica a selecao. A estrela ao lado de cada nome adiciona/remove favoritos.
5. Para um preset pessoal, ajuste os controles, digite o nome no navegador e clique SAVE AS. RENAME e DELETE so funcionam nos presets do usuario. Presets de fabrica sao protegidos.
6. A/B guardam ajustes separados durante a sessao do editor. COPY/PASTE copiam o estado XML completo. IN acompanha o Power/bypass. Setas navegam pelos 100 presets de fabrica.
7. O botao MAP lista os IDs estaveis dos parametros; escolher um copia seu ID para usar como referencia no mapeamento de automacao do host. Ele **nao** faz MIDI Learn.
8. A janela pode ser fechada pelo X e reaberta pelo nome do preset. Filtros ficam mantidos enquanto a janela existir.

## Arquivos avulsos e compatibilidade Windows
Os 100 arquivos numerados .xml sao multiplataforma. Para carregar em uma versao anterior deste mesmo plugin que ja tenha importacao XML: menu `...` > Import XML (ou botao direito > Import preset), selecione UM arquivo numerado. Todos os controles sao mapeados automaticamente. Nao importe Bank.json nem Mapeamento.csv como preset.

Bank.json e o manifesto completo; Mapeamento.csv contem os valores brutos dos 20 parametros, com categorias e uso AUX/INSERT. Os enums usam indices: placa 0=A,1=B,2=C; filtro 0=EMT,1=MARTECH,2=OFF; EMT 0=-4,1=-10,2=-16 dB; Martech 0=90,1=125,2=180,3=250,4=270,5=360 Hz. Outros valores sao unidades reais (s, ms, %, Hz, dB, cents); nao sao normalizados 0..1.

O banco e destinado ao **PLATE 140 deste projeto**, nao aos plugins comerciais EMT/UAD. XML nao e um arquivo nativo .vstpreset de outro fabricante. O VST3 Windows precisa ser compilado com o projeto atualizado; nao ha DLL/EXE Windows validado neste pacote de presets.

## AUX versus INSERT
73 presets AUX: Wet Solo ON, mix armazenado 100%. Use em canal auxiliar/FX, enviando sinal por send.
27 presets INSERT: Wet Solo OFF, mix especifico de 8% a 25% (piso de audibilidade preservado da revisao remota). Use diretamente no canal. Nao use um preset INSERT em auxiliar sem ligar Wet Solo, para evitar retorno de sinal seco duplicado.

Vocais: 15; Bateria: 14; Guitarras: 10; Teclas: 10; Cordas: 10; Sopros: 8; Atmosferas: 12; Cinema: 11; Mix: 10. Total: 100. Nomes e estados sao distintos. Os 12 nomes iniciais e seu uso AUX/INSERT foram preservados; o EQ foi recalibrado para maior contraste.

## Persistencia
No Windows, favoritos e presets pessoais ficam em `%APPDATA%\Aulamix\PLATE140\library.xml`. O plugin grava por arquivo temporario e substituicao atomica; erros sao mostrados no rodape do navegador. Biblioteca invalida e preservada, sem sobrescrita. Em outros sistemas, usa o diretorio de dados de usuario informado pelo JUCE. A variavel opcional PLATE140_PRESET_HOME permite selecionar uma pasta de dados em testes.

## Referencias e limites
O banco foi desenhado a partir da especificacao de parametros fornecida pelo usuario e dos IDs/faixas efetivamente implementados em Source/PluginProcessor.cpp. Tentativas de consultar a documentacao oficial UA (help.uaudio.com e media.uaudio.com) foram bloqueadas pelo proxy HTTP 403; o manual online nao foi lido nesta sessao. Nao sao presets oficiais UA, nem uma transcricao ou equivalencia sonora da UAD/EMT.

As receitas sao intencionais por uso: caudas curtas para ataques, predelay maior para separar voz/cauda, EQ no retorno para controlar graves e brilho, modulacao moderada nas atmosferas. Sao pontos de partida originais. Validacao automatica cobre mapeamento, limites, estado, audio finito, busca, A/B, clipboard e persistencia. Nao houve avaliacao auditiva com gravacoes de referencia nem validacao no Studio One 8/Reaper. A revisao final passou build e testes nativos Windows; detalhes em docs/VALIDATION.md.

A etiqueta AUX/INSERT aparece em cada linha do navegador e no painel. Buscar AUX ou INSERT filtra respectivamente 73 ou 27 receitas. A medicao dos 100 presets e a tabela antes/depois estao em docs/PRESET_METRICS_BEFORE_AFTER.md; contraste numerico nao substitui escuta real.
