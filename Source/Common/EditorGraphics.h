#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <atomic>
namespace plateui {
class EditorGraphics final : private juce::ComponentListener {
public:
    EditorGraphics(juce::Component& c, std::atomic<bool>& preference)
        : component(c), software(preference) { component.addComponentListener(this); }
    ~EditorGraphics() override { component.removeComponentListener(this); }
    void apply() {
        jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());
       #if JUCE_WINDOWS
        if (applying) return;
        const juce::ScopedValueSetter<bool> guard(applying, true);
        if (auto* peer = component.getPeer()) {
            const int requested = software.load() ? 0 : 1;
            if (peer->getAvailableRenderingEngines().size() > requested
                && peer->getCurrentRenderingEngine() != requested)
                peer->setCurrentRenderingEngine(requested);
        }
       #endif
    }
private:
    juce::Component& component;
    std::atomic<bool>& software;
    bool applying = false;
    void componentParentHierarchyChanged(juce::Component&) override { apply(); }
    void componentVisibilityChanged(juce::Component&) override { apply(); }
    JUCE_DECLARE_NON_COPYABLE(EditorGraphics)
};
}
