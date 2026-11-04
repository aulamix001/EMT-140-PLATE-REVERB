# AudioPluginHost oficial — Linux

Compilado diretamente do checkout JUCE 8.0.6, sem alterar seus fontes, Debug (-O0 -g1). Dependência adicional: ladspa-sdk extraído no sysroot local. VST3 real carregado por projeto .filtergraph com estado binário capturado do programa Ambient Wide. A captura `../images/official-host-linux.png` mostra RT A 4,5 s, pre-delay 40 ms e modulação 0,25 Hz/8,0.

- Carregamento do projeto/estado e abertura da interface: PASS, observado na janela X11 nativa.
- Fechamento pelo botão X do editor: PASS, conferido com xwininfo; a janela filha VST3 foi destruída e o host permaneceu aberto.
- Primeira tentativa de saída: FAIL, exit 139. Antes disso, o harness enviara mensagens WM_DELETE_WINDOW em Xvfb sem window manager e também acionara botões/atalho. Não foi obtida stack dessa primeira falha; não há causa raiz comprovada.
- Repetição sob GDB com X do editor e Ctrl+Q: processo saiu normalmente, sem SIGSEGV. Assim, não existe stack de queda capturada nessa repetição.
- Repetição normal, mesmo projeto, X do editor e Ctrl+Q: exit 0.

Classificação conservadora: **PARCIAL**, pois houve uma saída 139 ainda sem diagnóstico. Não apresentar essa experiência como uma validação irrestrita do encerramento do host.

Ambiente: Xvfb :101, 2200×1200, sem window manager nem dispositivo ALSA/MIDI. Logs contêm BadAtom do X11 e ausência de /dev/snd/seq. O teste não inclui reprodução por hardware de áudio. O processamento simultâneo de áudio foi verificado pelos harnesses próprios separados. Não foram coletadas stacks do Studio One, nem foi executado o Studio One.
