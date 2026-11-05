# Parameter map

Stable version-1 APVTS IDs and ranges are defined in `Source/PluginProcessor.cpp::layout`.

- `plate_select`
- `reverb_time_a`
- `reverb_time_b`
- `reverb_time_c`
- `input_filter_model`
- `input_filter_emt_db`
- `input_filter_martech_hz`
- `width`
- `balance`
- `eq_enable`
- `eq_low_freq`
- `eq_low_gain`
- `eq_high_freq`
- `eq_high_gain`
- `mod_rate`
- `mod_depth`
- `predelay`
- `mix`
- `wet_solo`
- `power`

Defaults follow supplied numeric table: 2 s per tank, A, EMT −10 dB, width 100%, balance center, EQ on/flat at 120 Hz and 8 kHz, mod off, 50 ms predelay, 15% stored mix, Wet Solo on, power on. Predelay and mix use exact center-point tapers. Filter includes explicit OFF. State root PLATE140, version 1; normalized host values use JUCE range conversion.

## Physical knobs and EQ gains

The UI uses logarithmic frequency dials (200 Hz/2 kHz at noon) and a rate dial
with 0.1 Hz at noon, 0 Hz at the OFF detent. These are UI-only scales; normalized
host automation retains the existing APVTS mapping. Both GAIN controls are wet
return shelf gains, not a master volume. They affect audio only with EQ IN;
WET SOLO isolates the return, while a low INSERT mix reduces their influence on
the combined output. Double-click uses the declared parameter default, including
when the editor opens after a factory/user preset. Exact tests and measured
+/-12 dB responses are recorded in validation/panel-controls-linux.txt.
