#pragma once

#include <Geode/ui/GeodeUI.hpp>
#include <Geode/utils/web.hpp>
#include <cocos2d.h>
#include <Geode/Geode.hpp>

#include "ExpertMapLayer.hpp"

using namespace geode::prelude;

class ExpertStartPopup : public geode::Popup<ExpertMapLayer*> {
public:
    static constexpr float POPUP_WIDTH = 265.f;
    static constexpr float POPUP_HEIGHT = 285.f;

    static ExpertStartPopup* create(ExpertMapLayer* parent);

private:
    bool setup(ExpertMapLayer* parent) override;

    void startRun(CCObject*);
    void settingRating(CCObject*);
    void settingLives(CCObject*);

    ExpertMapLayer* m_parent = nullptr;
};