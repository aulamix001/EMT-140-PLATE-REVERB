# Removal after playback: compatibility fix and regression coverage

Reported behavior: insert and use presets successfully; removing the plugin hides
its UI but leaves the DAW unresponsive while audio continues. The earlier empty
bus guard addresses a different, independently reproduced processor crash.

## Changes

- Windows plugin-owned editor, browser and popup windows use JUCE's supported
  software/GDI renderer (engine 0). JUCE 8.0.6's Direct2D window backend owns a
  presentation thread and joins it during window teardown. This compatibility
  path avoids that GPU presentation-thread teardown. It does not change the
  host's own renderer or any parameter, preset, DSP recipe or artwork file.
- Cached panel and knob images are software/CPU images, so their static lifetime
  does not retain Direct2D image resources until DLL unload. The knob's alpha
  trim is computed once rather than once per editor.
- Editor teardown marks the instance as closing, stops its timer, cancels the
  file chooser, disconnects parameter attachments while controls still exist,
  and hides/destroys the floating browser. File chooser and menu callbacks check
  a weak owner and the closing flag before accessing processor/editor state.
- Time-meter value popups belong to the editor, rather than separate windows.

## Tests

Local direct-editor teardown passed 12 cycles while a separate thread processed
finite audio. Alternate cycles open the preset browser; search notifications are
queued immediately before removal. Weak references verify destruction of editor
and browser, and a later timer dispatch verifies message-thread responsiveness.
CPU-backed artwork is asserted; Windows additionally asserts software editor and
browser peers. Native DSP, 100-preset/state/audio/UI integration, clipboard, A/B
and the previous host-lifecycle regression also passed locally.

Windows CI additionally builds PlateRemovalHost, a separate VST3 host with no
static Plate140 linkage. It loads the actual bundle, changes host programs while
playing, destroys the editor while processing continues, then stops processing
and releases the instance. Twelve cycles must complete with finite audio and a
subsequent UI timer dispatch after every removal. A 90-second process watchdog
fails the run if removal hangs. Teardown and host logs are retained as artifacts.

An exploratory Linux VST3 host test encountered a host-side JUCE/Linux RunLoop
callback use-after-free after removal (debugger stack in RunLoop::Impl's fd
callback). That is not evidence for the reported Studio One Windows hang; the
actual-DLL removal gate is therefore Windows-specific. Direct editor/browser
teardown is tested on both platforms.

Studio One 8 and the user's graphics driver are unavailable here. This is a
Windows teardown compatibility fix with targeted regression tests, not a claim
that a Studio One call stack has been obtained or the user's exact hang was
reproduced. Confirmation in Studio One remains required.
