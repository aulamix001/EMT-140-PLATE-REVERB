# Atualização 0.2.1 — correção de estalos

[Auditoria completa e provas antes/depois](CRACKLE_AUDIT.md). Linux Release
13/13, integração dos 100 presets, pluginval 10 e validator 47/47 executados.
ASan/UBSan: 9/9, 159,59 s; testes explícitos em buffers 100/520; benchmark de CPU 59–66% menor em oito cenários e 100 presets sem clipping no probe −18,42 dBFS. Windows 0.2.1 em andamento; o registro 0.2.0 abaixo é
histórico, não deve ser confundido com a validação do novo binário.
Studio One 8, Reaper e escuta real continuam não executados.

# Validação — PLATE 140 0.2.0

Registro de 05/11/2026. Código, resultados e limitações são separados abaixo. Os registros antigos em `measurements.json` e `validation/audibility` descrevem revisões anteriores; não substituem estas medições. Banco, IDs, faixas, passos e defaults: os 20 parâmetros existentes foram preservados. Não há 25 parâmetros nem MIDI Learn neste produto.

## Executado no Linux

Debian 13 x86-64, GCC 14.2, JUCE 8.0.6, C++17, Python 3.12, NumPy 2.3.5/SciPy 1.17.0; Xvfb :103, 2200×1200. Código de produção rastreado em `metrics/provenance.json`, incluindo blobs do engine/banco e SHA256 do VST3 Linux.

| Verificação | Resultado efetivo |
|---|---|
| Build Release, VST3/Standalone/runners | PASS |
| ctest Release | PASS: 11/11, 101,68 s, com todos os controles e o banco final. |
| Controles e ganhos | PASS: 20 parâmetros alteram o áudio em seu contexto ativo; 10 knobs, hit areas, três RTs/placas e 10 posições de filtro. Ganhos +/-12 dB medidos: graves +/-11,9742 dB, agudos +/-11,9951 dB; EQ OUT e MIX 0 preservam áudio esperado. |
| Integração nativa dos 100 presets | PASS: 100 respostas de áudio diferentes, 100 XML, estado, limites, biblioteca, A/B, clipboard e navegador flutuante |
| Host VST3 externo | PASS: 100 programas; get/setCurrentProgram e restauração de estado; RTs chegam ao processor; 100 respostas diferentes |
| Métricas dos 100 presets | PASS: 4.950 pares e 36 pares de medianas de categorias |
| Modulação, tom estacionário 1 kHz | PASS: energia lateral/portadora 0,833127 modulada; 1,86×10⁻¹⁴ estacionária |
| Placas A/B/C em configuração plana | PASS: brilho A 3,54 / B −0,72 / C 8,47 dB; RT500 próximo de 2,02 s |
| Ciclo de vida nativo | PASS: 100 instâncias e 300 editores por execução, em escala 100% e 125%; áudio em thread separada; 300/300 gestos equilibrados; callback concluído antes de liberar processor |
| Comparação RGBA Linux | PASS: 0/934.305 pixels alterados ao trocar a preferência gráfica no mesmo backend Linux; não compara GDI/Direct2D Windows |
| Regressão de buses e reativação | PASS: mono/estéreo, 0 canais, 0 amostras, pré-prepare/pós-release e rates inválidos |
| Navegador sob reprodução | PASS: 12 ciclos de fechamento, atualização de busca pendente, áudio finito e message thread responsiva |
| Debug ASan + UBSan | PASS: 7/7, 190,26 s; inclui controles/áudio, detect_leaks=1 e halt_on_error=1; sem relatório de memória/UB/leaks nem JUCE Assertion. |
| pluginval 1.0.4, nível 10 | PASS no VST3 final; GUI, 44,1/48/96/192 kHz, blocos 32/64/128/256/512/1024, seed 140 |
| Validator oficial Steinberg 3.8.1 | PASS: 47/47 no Release e 47/47 no Debug instrumentado; SDK fixado em 3cdf9ca5d1f5b1b21e0a86832aa4abe55607bd96 |
| AudioPluginHost oficial sem alterar fontes | PARCIAL: carregou projeto/estado e abriu/fechou o editor; primeira saída 139, repetições normal e GDB saíram sem falha; causa ainda indeterminada |

Logs efetivos em `validation/ctest-audibility-linux.txt`, `preset-integration-audibility-linux.txt`, `debug-asan-linux.txt[.gz]`, `pluginval-audibility-linux.txt` e `steinberg-*-linux.txt`. O log Debug completo preserva os diagnósticos X11 BadAtom produzidos no Xvfb sem window manager. O resumo informa sua contagem, sem ocultar assertions. O host oficial também não teve hardware ALSA/MIDI disponível. Detalhes da saída 139 e das repetições em `validation/official-host-linux.md`; não foi obtida uma stack da queda inicial.

## Defeitos comprovados e correções

1. **Acesso fora do atraso circular:** ASan reproduziu heap-buffer-overflow em Delay::read quando um pequeno valor negativo, somado à capacidade, arredondava para o tamanho exato do vetor. Agora o wrap usa índices inteiros, preservando a interpolação linear. Teste com nextafter(1,2) e regressão completa passaram sob ASan; stack anterior em `validation/diagnostics/asan-delay-before-fix.txt`.
2. **Gesto aberto ao destruir o editor:** uma cópia isolada retirando apenas finishInteractions falha com `gesture left open after editor destruction`. A versão corrigida termina a captura real e o popup enquanto slider/attachment/processor existem. Não dispara callbacks artificiais de gesto; o teste usa eventos reais via ComponentPeer. Evidência em `validation/diagnostics/gesture-without-finish.txt`.
3. **Assertion UTF-8:** Debug reproduziu juce_String.cpp:327 no tooltip com pontos médios. String::fromUTF8 com bytes escapados removeu a assertion sem alterar o texto exibido.
4. **Programa 0 do VST3:** o estado inicial era Default mas reportava Vocal Long/índice 0; a seleção do mesmo programa podia ser ignorada pelo wrapper. Default agora é o índice 100; as 100 fábricas mantêm 0–99. Notificação ChangeDetails inclui programChanged e nonParameterStateChanged. Estados válidos sem version explícita, com filhos reordenados e metadados extras são aceitos. O teste externo confirma programa restaurado e parâmetros efetivos.

O chooser já recebeu SafePointer/closing na revisão remota 6cf21fa; essa proteção foi mantida e estendida para conferir a identidade do chooser atual. A entrada side estéreo, trim de retorno ×3 e proteção de buses remotos foram preservados. O painel foi reconstruído conforme a referência EMT original enviada em 05/11, com corpo dos knobs e iluminação fixos, índices móveis, agulhas e chaves interativas; as imagens em CPU são decodificadas e recortadas uma vez. Não há geometria 3D ou geração procedural de textura no repaint. A captura é do editor compilado; não se afirma equivalência pixel a pixel nem aparência Windows sem captura própria.

## Presets e métricas antes/depois

[Tabela dos 12 presets](PRESET_METRICS_BEFORE_AFTER.md), JSON/CSV de todos os 100 em `metrics/before` e `metrics/after`. Baseline e banco são do commit e3fe3b3; o candidato inclui a integração remota e o conserto ASan. Render: 48 kHz, impulso estéreo 0,5, 2 s de assentamento, 10 s de cauda wet. Presets INSERT mantêm seu mix no plugin; somente a medição isola wet para não confundir o ataque seco com a cauda.

O teste exige por par pelo menos um eixo: RT500 ≥20%, brilho ≥2,5 dB ou graves ≥2,5 dB. Também exige início intrínseco ≤2 ms, densidade inicial ≥70% e ajuste de decay confiável. Diferenças apenas de largura/pre-delay não são suficientes. Antes, reanalisado com esses mesmos limites: 33 pares internos, 225 entre categorias e 5 pares de medianas de categorias falharam; depois: zero. Início intrínseco final 0,25–0,375 ms; RT500 0,521–5,517 s. Nenhuma dessas medidas constitui prova de percepção auditiva.

Routing: **73 AUX / 27 INSERT**. A integração remota manteve piso de 8% nos INSERT (máximo 25%). Browser/toolbar mostram o modo e buscas AUX/INSERT retornam as contagens corretas. Rate zero/depth zero têm render estacionário bit-idêntico quando preparados nessa configuração; ao desligar modulação em uso, offsets suavizam até zero exato. O callback não ganhou alocações/locks/I/O; a instrumentação de new passou nas quatro sample rates. ASan não é detector completo de data races.

## Windows e testes não executados

Execuções CI anteriores falharam no build do renderer de métricas por falta de include explícito de string no MSVC; esse erro foi corrigido em dd5c795. A execução seguinte passou 8/9 testes Debug, incluindo os 100 programas VST3; o harness de gesto não iniciou o arraste. O harness final traz sua janela à frente, aguarda pintura no Windows e testa também escala global 125%, sem reduzir os critérios. A execução fa45a1e passou os oito testes de áudio/estado/métricas, mas os dois harnesses de gestos terminaram por timeout, bloqueando Release e publicação. A revisão e6b13a3 removeu adaptadores duplicados, aguardou o hit-test real de forma assíncrona e passou todos os gates Windows: Debug/Release ctest, validator oficial, remoção real do VST3, integração e pluginval 10. Execução 42047210753 publicou v0.2.0-dev.1. O job de PR falhou sem causa detalhada disponível publicamente; seu diagnóstico foi ampliado. Essa versão antecede o painel e o novo critério de contraste deste registro. A revisão final **e6b647b748eac85aefa16d31f83259d04ffee449** também passou todos os gates no [run 42061095886](https://github.com/aulamix001/EMT-140-PLATE-REVERB/actions/runs/42061095886): build/ctest/validator oficial em Debug, build/ctest/integração dos 100 presets/remoção VST3/pluginval 10 em Release. A publicação **v0.2.0-dev.2** foi baixada e conferida: PE32+ AMD64, CRC do ZIP, SHA256, commit e igualdade dos 100 XML/Bank.json com o fonte final. Registro em `validation/windows-package.json` e `windows-ci.txt`; captura nativa Windows em `images/plate140-windows.png`.

O status geral do workflow ficou vermelho **somente no job open-pr**: `GitHub Actions is not permitted to create or approve pull requests (createPullRequest)`. Os jobs build e release passaram; o VST3 foi publicado. O PR não foi aberto porque a permissão do repositório bloqueia sua criação pelo token do Actions. A branch e a descrição do PR estão prontas para revisão.

[Download Windows x64 v0.2.0-dev.2](https://github.com/aulamix001/EMT-140-PLATE-REVERB/releases/download/v0.2.0-dev.2/PLATE140-Windows-x64.zip) — 15.706.992 bytes; SHA256 `adc130fd12e2aa8cdafc59fd27b27735e546f432db7706ef4ba83450aa48f14d`.


**NÃO EXECUTADO neste ambiente:** Reaper; Studio One 8/Studio One ou Fender Studio Pro 8.4 no Windows 11; escuta real; teste físico de NVIDIA/Intel/AMD; monitores em DPI misto a 125%/100%; sequência SDK de escalas anexadas ao host (100/125/100/150/175/200/250/300/400/137/125/100); minidump de um congelamento da DAW. Não foi comprovada a causa do freeze relatado nem equivalência de aparência Windows usando capturas Linux.

A política CPU/GDI com imagens em CPU é uma escolha de compatibilidade, não garantia para todas as GPUs. JUCE 9.0.5 foi citado no pedido para H-Delay, produto ausente deste repositório; PLATE 140 mantém JUCE 8.0.6. Não se atribui o congelamento à correção upstream de autoscaling sem evidência na DAW. O manual oficial UA permaneceu inacessível por HTTP 403; não foi apresentado como lido.

## Comandos executados

```sh
. /workspace/tools/build-env.sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/workspace/tools/sysroot/usr
cmake --build build --parallel 3
DISPLAY=:103 ctest --test-dir build -V --output-on-failure
PLATE140_PRESET_HOME=/workspace/pasta-de-teste DISPLAY=:103 build/PlatePreview_artefacts/Release/PlatePreview /workspace/plate-final.png
```

Debug usou -O0 -g1 -fsanitize=address,undefined -fno-omit-frame-pointer e flags de link equivalentes. ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 e UBSAN_OPTIONS=halt_on_error=1; ctest selecionou PlateDSP, PlateSoundBehaviour, EditorGestureLifetime, EditorGestureLifetime125, HostLifecycle e EditorBrowserTeardown. O sandbox inicial impediu o LeakSanitizer de inspecionar threads; executar fora dele resolveu a restrição, mantendo detect_leaks=1. O cache Ninja perdeu dependências entre execuções neste ambiente; a recompilação final usou os comandos de compile/link emitidos por ninja -t commands, seguida das suítes acima, sem alterar o código para evitar testes.

Validator SDK foi compilado em Debug com VSTGUI/plugin/hosting examples OFF; validator permanece disponível nessa configuração. Para carregar o plugin instrumentado, o executável recebeu LD_PRELOAD do libasan do mesmo GCC. AudioPluginHost foi compilado pelo CMake oficial JUCE com JUCE_BUILD_EXTRAS=ON, target AudioPluginHost, e um projeto com estado VST3 real; o encerramento normal foi acionado por X do editor e Ctrl+Q. Esses comandos não substituem o reteste na DAW afetada.

## Ambiente gráfico e repetição final (05/11)

Uma primeira repetição perdeu o servidor Xvfb enquanto os validadores ainda
executavam. XOpenDisplay(:102) retornou nullptr; pluginval caiu dentro do próprio
XEmbedComponent ao criar a janela, e o runner de integração ficou sem clipboard.
A stack foi preservada em validation/diagnostics/pluginval-no-display.txt.
Nenhum código do produto foi alterado para ignorar esses testes. Uma única sessão
manteve o Xvfb :103 vivo até terminar, verificou a conexão antes de começar e
encerrou o display depois: Release 11/11, ASan/UBSan 7/7, runner dos 100 presets
e pluginval 10 com GUI, seed 140, quatro sample rates e seis block sizes: exit 0.
As capturas finais estão em images/plate140.png e images/preset-browser.png.
A medição de CPU em máquinas reais, escuta e validação nas DAWs continuam pendentes.
