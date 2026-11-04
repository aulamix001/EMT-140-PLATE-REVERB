# Comparacao antes/depois — 12 presets

Numeros medidos, nao estimados: impulso estereo identico de 0,5, 48 kHz, 2 s de assentamento e 10 s de cauda. Caminho wet isolado em ambos os bancos; largura, filtros, EQ e pre-delay mantidos de cada preset. Valores mostram **antes → depois**.

| Preset | RT60 banda larga (s) | Inicio total (ms) | Brilho (dB) | Graves (dB) | Largura S/M |
|---|---:|---:|---:|---:|---:|
| Vocal Long | 2.40 → 2.21 | 49.65 → 20.33 | 3.53 → 5.06 | -12.82 → -16.32 | 0.99 → 1.01 |
| Vocal Tight | 1.36 → 1.15 | 39.65 → 10.33 | 4.85 → 5.41 | -11.43 → -18.77 | 0.99 → 1.01 |
| Drum Room | 0.76 → 0.75 | 31.75 → 0.25 | 6.13 → 1.24 | -15.73 → -16.38 | 0.80 → 0.79 |
| Percussion Short | 0.48 → 0.39 | 31.75 → 0.25 | 6.64 → 3.61 | -16.15 → -22.90 | 0.60 → 0.60 |
| Cinematic 5s | 4.33 → 4.72 | 111.75 → 80.25 | 2.35 → -6.15 | -15.92 → -13.13 | 1.00 → 0.95 |
| Dark Jazz | 1.62 → 1.81 | 48.85 → 15.38 | 3.92 → 0.49 | -12.56 → -8.44 | 1.00 → 0.97 |
| Ambient Wide | 3.92 → 3.87 | 69.65 → 40.33 | 1.65 → 3.32 | -10.45 → -15.61 | 0.99 → 0.99 |
| Snare Chrome | 0.97 → 0.84 | 43.75 → 12.25 | 7.02 → 3.70 | -24.94 → -29.04 | 0.90 → 0.90 |
| Kick Upper Plate | 0.48 → 0.42 | 33.75 → 2.25 | 6.81 → 2.80 | -28.75 → -34.64 | 0.40 → 0.40 |
| Infinite Steel | 4.61 → 4.61 | 139.65 → 110.33 | 1.36 → 4.22 | -19.77 → -24.08 | 0.99 → 0.99 |
| Mono Ghost | 2.55 → 2.71 | 156.75 → 125.25 | 3.28 → -3.19 | -13.55 → -10.16 | 0.00 → 0.00 |
| Strings Bright | 2.03 → 2.33 | 58.85 → 25.38 | 5.85 → 11.50 | -20.01 → -20.74 | 1.00 → 0.99 |

## Definicoes e criterios

- RT60: regressao T30 (-5 a -35 dB da integral reversa de Schroeder), extrapolada a 60 dB. A tabela mostra banda larga. JSON/CSV tambem incluem RT60 em 354–707 Hz, usada para testar contraste de decay sem confundir EQ com tempo de reverb.
- Inicio total: primeiro frame com energia >= -60 dB do pico, incluindo o pre-delay armazenado. Inicio intrinseco = total menos pre-delay; limite novo <= 2 ms.
- Brilho: 10 log10(energia 3–12 kHz / energia 300–2000 Hz). Graves: 10 log10(energia 20–180 Hz / energia 300–2000 Hz). As larguras das bandas nao sao normalizadas; sao razoes espectrais, nao os ganhos do EQ.
- Largura: RMS Side / RMS Mid, com M=(L+R)/2 e S=(L-R)/2. Zero indica mono.
- Densidade: fracao dos primeiros 10 ms apos o inicio com energia >= -40 dB do pico. Exige-se >= 70% para todos os presets.
- Cada um dos 4.950 pares precisa diferir por **pelo menos um** eixo: RT60 de referencia >=15%, inicio >=8 ms, brilho >=1,5 dB, graves >=1,5 dB ou largura >=0,08. O mesmo teste vale para os 36 pares de medianas das nove categorias.
- A distancia normalizada e o maximo dessas cinco diferencas divididas pelos respectivos limites. Exige-se >=1, sem ajustar o limite para fazer o banco passar. Valores completos e falhas anteriores: `metrics/before` e `metrics/after`.

Esses criterios demonstram contraste objetivo; nao constituem teste psicoacustico, escuta real ou validacao no Reaper/Studio One 8. Os presets INSERT preservam seu mix e Wet Solo OFF no plugin; o isolamento wet e exclusivo da medicao para evitar que o ataque dry esconda a cauda.
