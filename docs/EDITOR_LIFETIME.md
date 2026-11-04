# Encerramento e gráficos — PLATE 140

Este repositório contém PLATE 140/JUCE 8.0.6, 20 parâmetros, sem MIDI Learn. O texto de diagnóstico recebido cita também H-Delay/CopperEcho, Feedback/DELAY e 25 parâmetros; esses componentes não existem aqui. A atualização JUCE 9.0.5 solicitada especificamente para H-Delay não foi aplicada a outro produto. A correção upstream de autoscaling citada no pedido não comprova a causa do congelamento nesta DAW.

## Auditoria e mudanças

- O código base capturava `this` em `FileChooser::launchAsync`. O remoto 6cf21fa já acrescentou SafePointer/closing. A integração preserva essas proteções e acrescenta a confirmação da identidade do chooser atual. O editor cancela/destrói o chooser antes dos controles.
- `EditorLifetime.h` libera a captura real via `ComponentPeer::handleMouseEvent`, deixando JUCE executar Slider::mouseUp, finalizar ScopedDragNotification/attachment e destruir o popup. Não chama callbacks de gesto manualmente nem endChangeGesture indiscriminadamente. A varredura armazena SafePointers antes de qualquer fechamento que possa modificar a árvore.
- Depois da interação, fecha popups de hover pelo Slider, destrói attachments, desconecta LookAndFeels e deixa os controles serem destruídos. O timer de 30 Hz para primeiro; o navegador flutuante pertence ao editor e é destruído antes dele. Não há AlertWindow de entrada numérica nem RotaryControl com LookAndFeel próprio neste projeto; não foram inventados controles para simular esses casos.
- Os menus assíncronos que acessam o editor já usam SafePointer; o menu MAP só copia uma string estática e não referencia editor/processor. O processor não tem Timer ou callbacks de componentes.
- Nada desta correção foi acrescentado ao callback de áudio. A preferência gráfica é atômica, fora dos 20 parâmetros APVTS.

## Política Windows

`CHI_WINDOWS_SOFTWARE_GRAPHICS=ON` copia apenas juce_graphics e juce_gui_basics para a pasta de build. Muda a fábrica HWNDComponentPeer para GDI **antes** de criar o peer e NativeImageType para SoftwareImageType. Fontes/includes apontam para as cópias; o checkout JUCE fornecido permanece intacto. A configuração falha se as assinaturas esperadas não forem encontradas. `compatible-graphics.txt` registra a escolha. OFF mantém as fábricas upstream.

O adaptador RAII EditorGraphics observa hierarquia/visibilidade, impede reentrada e aplica a escolha no peer real na message thread. Options → Graphics permite CPU/GDI ou GPU/Direct2D no Windows. graphicsMode=0/1 é metadado de sessão; estados antigos usam o padrão configurado, e presets de fábrica não alteram essa preferência. HiDPI e o redimensionamento proporcional continuam ativos. Essa política não comprova compatibilidade com todas as GPUs.

## Harness e limites

EditorGestureLifetime abre 100 processors e 300 editores nativos. Uma thread separada processa áudio, incluindo buffers zero e modo offline; cada editor fecha durante um arraste real de Width, com attachment e popup. O teste exige pares begin/end equilibrados e captura encerrada. O host de teste para e aguarda o callback antes de destruir o processor. Alterna 44,1/48/96/192 kHz e restaura estado entre instâncias. Watchdog de 30 segundos registra falha e tenta minidump no Windows, com windows.h antes de dbghelp.h.

Checks Windows adicionais verificam peer simples inicialmente GDI, armazenamento CPU, leitura/desenho de pixels e troca CPU→GPU→CPU sem alterar parâmetros. Só contam como executados quando rodarem no Windows. A existência desses testes e do workflow não valida o Studio One.

Ainda pendentes: Studio One/Fender Studio Pro 8.4 no Windows 11, GPU/driver e monitor físico a 125%, escala VST3 anexada pelo SDK, mistura de DPI entre monitores e captura de um congelamento real. Não há stacks/minidump da DAW nem causa raiz demonstrada. Os testes não verificam componentes H-Delay ausentes deste produto.

Para um freeze real: usar Debug/PDB da mesma compilação, anexar Visual Studio Native à DAW, Break All, stacks de todas as threads e dump. Registrar versão da DAW, driver/GPU, escala, sample rate, buffer e ação exata. Não confundir resultados do harness com stacks do Studio One.
