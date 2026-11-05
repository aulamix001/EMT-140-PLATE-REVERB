# Auditoria de estalos — PLATE 140 0.2.1

A correção foi feita sobre a versão Windows v0.2.0-dev.2 / código e6b647b.
Os estalos relatados ocorrem parados e durante alterações, inclusive com mudanças
de buffer; os demais plugins da DAW não apresentam o mesmo problema. Não há
acesso a áudio, driver ASIO ou às instalações Studio One 8/Reaper do usuário.
Os buffers 100 e 520 informados foram incluídos explicitamente nos testes.

## Defeitos reproduzidos e alterações

| Caminho | Evidência anterior | Correção / verificação |
|---|---|---|
| Reset da DAW | AudioProcessor::reset alterava o programa 7 para 100 (Default). O VST3 real também falhou ao parar/reiniciar. | reset apenas solicita limpeza do DSP; resetToDefaults é a ação explícita da interface. Os 100 programas são testados através de stop/start real do VST3. |
| Inicialização MIX 0 | Entrada 0,2 retornava 0,0000708 na primeira amostra. | Smoothers iniciam com os parâmetros efetivos, não com os defaults de outra configuração; agora retorna exatamente 0,2. |
| Inicialização em bypass | Entrada 0,2 retornava 0,0000833. | Bypass retorna exatamente o sinal seco desde a primeira amostra, inclusive após reset. |
| Pre-delay automatizado | Leitura varria a memória durante a mudança; probe 997 Hz, 137→0 ms, energia acima de 3 kHz subia a −11,73 dB. | Dois taps estacionários com crossfade de 30 ms e destino pendente; mesma medida fica em −48,36 dB, próxima do piso estacionário. Pico de passo permanece próximo de 1× o estacionário nas 3 placas/4 sample rates, limite 1,3×. |
| Processamento contínuo | Todas as 3 placas completas processavam o tempo inteiro, além de recalcular coeficientes estáticos. | Somente a selecionada e as transições ainda audíveis processam. Placa inativa é limpa após fade abaixo de −140 dB; nenhum eco antigo é retomado. Cache de filtros/decay e snap subaudível dos smoothers. Benchmark comparativo em validation/click-audit/benchmark-linux.json. |
| EQ/filtro sob automação | Coeficientes eram substituídos em degraus de 32 amostras. | Interpolação amostra a amostra entre coeficientes, mantendo a suavização dos valores físicos. Stress de frequências/ganhos/placa/modulação/pre-delay permanece finito e limitado. |
| Troca de preset concorrente | Os 20 parâmetros eram escritos separadamente e podiam ser lidos no meio da atualização. | Leitura limitada, sem espera/retry, mantém o último snapshot coerente enquanto uma escrita está aberta. Teste suspende o loader real no meio e exige áudio bit-idêntico ao controle. |
| Interface parada | Painel completo e 3 medidores eram invalidados 30 vezes/s. | Timer 20 Hz; valores parados invalidam somente o pequeno retângulo do VU quando seu nível muda. Imagens continuam em cache CPU, sem trabalho gráfico no callback. |

## Demais causas verificadas e limites

- Fronteiras de buffer: áudio real do processor bit-idêntico com buffers 0/1/7/32/100/127/512/520/1024/2048. Sem tamanho de FFT ou bloco fixo no motor.
- Sample rates: regressões em 44,1/48/96/192 kHz, mono/estéreo, buses vazios e reativação.
- Denormais: ScopedNoDenormals no callback, zeros exatos/snap de estados pequenos e teste de silêncio. Reset do DSP não faz alocações; teste de new cobre reset e processamento nas quatro taxas.
- Memória/índices: wrap fracionário seguro mantido; ASan/UBSan da revisão atual em andamento. Resultados finais em VALIDATION.md.
- Valores inválidos: snapshot de parâmetros conserva o último valor finito e limita faixas; NaN/Inf no áudio não contaminam a rede. Teste no processor real.
- Clipping de ganho/retorno, picos do master e underruns ASIO/DPC continuam dependentes da sessão/driver. A cadeia usa ponto flutuante e não ganhou limiter ou normalização ocultos. Não se apresenta medição offline como prova de ausência de underruns na DAW.
- O teste cobre renderizações, estado, transições, finitude e custo de CPU; não substitui escuta ou validação nas duas DAWs com a interface de áudio do usuário.

## Validação executada

Linux Release: 13/13 ctests, 58,71 s; teste adicional dos buffers informados,
0,07 s; 100 presets reais distintos, importação, recall, A/B, clipboard e browser;
pluginval nível 10 em quatro taxas; Steinberg 47/47. Os 4.950 pares do banco
continuam passando o contraste mínimo existente; nenhum ID/faixa/default foi
alterado e as receitas de fábrica não foram modificadas.

A medição de CPU compara o mesmo probe C++ e o motor exato da versão lançada,
em processos separados, três repetições alternadas, oito configurações e blocos
64. O percentual é fração de um núcleo por segundo de áudio, incluindo geração
dos tons/cronômetro; não é porcentagem global da máquina nem teste ASIO. P99/max
não são gates absolutos em VM. O fixture antigo só é compilado no benchmark.
O áudio estacionário deve manter a mesma energia dentro da tolerância registrada.

Windows 0.2.1: compilação/validação nativa será executada no workflow associado
ao commit da correção. O ZIP só é publicado após os gates Debug e Release.
Studio One 8, Reaper e escuta real NÃO foram executados nesta auditoria.
