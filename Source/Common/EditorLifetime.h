#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

namespace plateui {
// Message-thread only. Complete JUCE's real mouse capture and Slider drag before
// attachments or LookAndFeel objects disappear. Popup deletion can change the tree.
inline void finishInteractions(juce::Component& root) {
    jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());
    std::vector<juce::Component::SafePointer<juce::Slider>> sliders;
    const auto collect = [&](juce::Component& c, const auto& self) -> void {
        if (auto* s = dynamic_cast<juce::Slider*>(&c)) sliders.emplace_back(s);
        for (auto* child : c.getChildren()) self(*child, self);
    };
    collect(root, collect);
    juce::Component::SafePointer<juce::Component> safeRoot(&root);
    // A copy avoids invalidating the list if dispatch creates/removes a source.
    const auto sources = juce::Desktop::getInstance().getMouseSources();
    for (auto source : sources) {
        auto* target = source.getComponentUnderMouse();
        if (!safeRoot) return;
        if (source.isDragging() && target && (target == safeRoot.getComponent() || safeRoot->isParentOf(target))) {
            if (auto* peer = target->getPeer()) {
                auto position = peer->globalToLocal(source.getScreenPosition());
                peer->handleMouseEvent(source.getType(), position,
                    source.getCurrentModifiers().withoutMouseButtons(), source.getCurrentPressure(),
                    source.getCurrentOrientation(), juce::Time::currentTimeMillis(), {}, source.getIndex());
            }
            source.enableUnboundedMouseMovement(false);
        }
    }
    for (auto safe : sliders) {
        if (!safe) continue;
        safe->setPopupDisplayEnabled(false, false, nullptr);
        // mouseExit closes even a hover-only popup through Slider's own owner.
        auto source = juce::Desktop::getInstance().getMainMouseSource();
        const auto now = juce::Time::getCurrentTime();
        safe->mouseExit(juce::MouseEvent(source, {}, {}, 0, 0, 0, 0, 0,
            safe.getComponent(), safe.getComponent(), now, {}, now, 0, false));
        if (safe) safe->hideTextBox(true);
    }
}
}
