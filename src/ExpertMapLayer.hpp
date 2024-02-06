#pragma once

#include <Geode/DefaultInclude.hpp>
#include <cocos2d.h>

using namespace geode::prelude;

class ExpertMapLayer : public CCLayer {
protected:
    bool init() override;
    void keyBackClicked() override;

public:
    static ExpertMapLayer* create();
    static ExpertMapLayer* scene();

    void onGoBack(CCObject*);
};

bool ExpertMapLayer::init() {
    if (!CCLayer::init())
        return false;
    
    this->setKeypadEnabled(true);

    this->addChild(createLayerBG());

    return true;
}

void ExpertMapLayer::keyBackClicked() {
    this->onGoBack(nullptr);
}

void ExpertMapLayer::onGoBack(CCObject*) {
    CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
}

ExpertMapLayer* ExpertMapLayer::create() {
    auto ret = new ExpertMapLayer;
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

ExpertMapLayer* ExpertMapLayer::scene() {
    auto layer = ExpertMapLayer::create();
    auto scene = CCScene::create();
    scene->addChild(layer);
    auto transition = CCTransitionFade::create(0.5f, scene);
    CCDirector::sharedDirector()->pushScene(transition);

    return layer;
}