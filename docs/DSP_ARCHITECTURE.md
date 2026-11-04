# Architecture

Independent double-precision 16-line orthogonal Householder feedback-delay networks, six serial input allpass diffusers and per-line feedback allpasses. The three tanks have distinct incommensurate lengths, diffusion coefficients, two-band decay losses and pickup shelves: A balanced, B dark/dense, C bright/metallic. This is an independent plate-inspired approximation, not a finite-element solver or a proprietary algorithm.

Stereo mid/side excitation preserves opposite-polarity input energy. The integrated remote return trim is ×3 (+9.54 dB); it does not alter the feedback decay.

Input → input shelf → restrained tanh driver → diffusion → three continuously running tanks and 12 dense early taps → smoothed plate selection → fractional wet predelay → two-band shelf EQ → mid/side width → balance attenuation → linear dry/wet → RMS VU. Dry latency is zero. Wet Solo overrides mix without overwriting it; power bypass is smoothed. All tanks retain their independent RT controls.

Early pickup starts 0.25–0.375 ms after input at 48 kHz, before the user predelay. Shorter feedback lines and dispersive allpasses supply immediate density. Two-band stable feedback gains follow the selected RT including the allpass reference-band group delay. Model-dependent high-band loss and pickup EQ create distinct brightness without making short-loop fixed low-pass loss dominate RT. An 8 Hz DC blocker prevents DC accumulation.

Modulation changes fractional feedback delay reads with independent line phases, not RT. Excursion derives from depth in cents and rate, limited to min(6 ms, 35% of the nominal line delay). Offsets smooth over 50 ms. Rate zero freezes phase and fades offsets to exactly zero, using integer stationary reads thereafter. A fresh zero-rate render is bit-identical to zero-depth stationary output.

All delay storage is allocated in prepare. Callback storage is fixed; no allocation, locking, file I/O, UI access or renderer calls. C++ allocation instrumentation and numerical stability checks run at 44.1/48/96/192 kHz. IDs, ranges, steps and defaults of all 20 audio parameters remain unchanged. Revised factory recipes intentionally sound different; old sessions retain saved values, though the revised algorithm changes their resulting sound.

Graphics preference is separate atomic session metadata, not an audio parameter. See EDITOR_LIFETIME.md and VALIDATION.md for lifecycle policy and actual executed checks.
