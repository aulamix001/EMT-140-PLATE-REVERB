# Validation and release gates
Run CTest DSP tests for silence, finite outputs across 44.1–192 kHz and block sizes 32–2048, impulse energy/tail, plate distinction, width mono, balance, bypass, dry mix, wet-solo invariance, predelay delay accuracy, filter/EQ response, and deterministic reset. Native editor rendering must be captured from the actual JUCE editor.

Separate pending release gates: RT60 ±8% at 500 Hz and band decay measurements, measured frequency response against legitimate reference, THD/aliasing, CPU <3%, allocation instrumentation, automation stress, pluginval level 10, Studio One session recall and Windows x64 build. Never interpret a build or screenshot as proof of these gates.
