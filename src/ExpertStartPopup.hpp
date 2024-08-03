#pragma once

#include <Geode/ui/GeodeUI.hpp>
#include <Geode/utils/web.hpp>
#include <cocos2d.h>
#include <Geode/Geode.hpp>

#include "ExpertMapLayer.hpp"

using namespace geode::prelude;

bool ratedLevels;
bool coinLives;

class ExpertMapLayer;

class ExpertStartPopup : public geode::Popup<> {

public:
    static constexpr float POPUP_WIDTH = 265.f;
    static constexpr float POPUP_HEIGHT = 285.f;

    static ExpertStartPopup* create();

    void startRun(CCObject*);
    void settingRating(CCObject*);
    void settingLives(CCObject*);

private:
    bool setup() override;
};

bool ExpertStartPopup::setup() {

    auto screenSize = CCDirector::sharedDirector()->getWinSize();
    auto mainLayer = static_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));
    auto main = static_cast<CCNode*>(mainLayer->getChildren()->objectAtIndex(0));

    CCLabelBMFont* settingsText = CCLabelBMFont::create("Run Settings", "bigFont.fnt");
    CCLabelBMFont* startText = CCLabelBMFont::create("Start Run", "bigFont.fnt");
    CCLabelBMFont* ratingText = CCLabelBMFont::create("Rated Levels", "bigFont.fnt");
    CCLabelBMFont* livesText = CCLabelBMFont::create("Coin Lives", "bigFont.fnt");
    CCScale9Sprite* startSprite = CCScale9Sprite::create("GJ_button_01.png");
    CCScale9Sprite* settingsBG = CCScale9Sprite::create("square02_001-uhd.png");
    CCMenu* startMenu = CCMenu::create();

    CCMenu* btnMenu = CCMenu::create();
    CCMenu* textMenu = CCMenu::create();
    CCMenuItemToggler* ratedBtn = CCMenuItemToggler::create(
        CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"),
        CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"),
        this,
        menu_selector(ExpertStartPopup::settingRating));

    CCMenuItemToggler* livesBtn = CCMenuItemToggler::create(
        CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"),
        CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"),
        this,
        menu_selector(ExpertStartPopup::settingLives));

    textMenu->setLayout(
        ColumnLayout::create()
        ->setGap(15.f)
        ->setAutoScale(false)
        ->setAxisAlignment(AxisAlignment::End)
    );

    textMenu->setContentHeight(POPUP_WIDTH - 85);
    textMenu->setScale(0.85);

    ratingText->setScale(0.8);
    livesText->setScale(0.9);

    textMenu->addChild(ratingText);
    textMenu->addChild(livesText);
    textMenu->updateLayout();

    btnMenu->setContentHeight(POPUP_WIDTH - 85);
    btnMenu->setScale(0.85);

    btnMenu->setLayout(
        ColumnLayout::create()
        ->setGap(15.f)
        ->setAutoScale(true)
        ->setAxisAlignment(AxisAlignment::End)
    );

    btnMenu->addChild(ratedBtn);
    btnMenu->addChild(livesBtn);
    btnMenu->updateLayout();
    
    startSprite->setContentSize(CCPoint(120, 35));
    settingsBG->setContentSize(CCPoint(POPUP_HEIGHT - 40, POPUP_WIDTH - 85)); // i cannot have fucked this up worse but im not fixing it
    settingsBG->setOpacity(80);
    startText->setScale(0.625);
    startText->setPosition({60, 19});
    startSprite->addChild(startText);
    
    CCMenuItemSpriteExtra* startBtn = CCMenuItemSpriteExtra::create(startSprite, this, menu_selector(ExpertStartPopup::startRun));

    startMenu->setPosition({main->getPositionX(), 30});
    settingsBG->setPosition({main->getPositionX(), main->getPositionY() + 5});
    btnMenu->setPosition({(main->getPositionX() * 2) - 55, main->getPositionY() + 8});
    textMenu->setPosition({main->getPositionX() - 25, main->getPositionY() + 8});
    settingsText->setPosition({main->getPositionX(), POPUP_HEIGHT - 20});

    startMenu->addChild(startBtn);
    main->addChild(settingsText);
    main->addChild(settingsBG);
    main->addChild(startMenu);
    main->addChild(btnMenu);
    main->addChild(textMenu);

    return true;
};

void ExpertStartPopup::settingRating(CCObject*) {
    ratedLevels = !ratedLevels;
};

void ExpertStartPopup::settingLives(CCObject*) {
    coinLives = !coinLives;
};

void ExpertStartPopup::startRun(CCObject*) {
    ExpertMapLayer::confirmExpertRun(nullptr);
};

ExpertStartPopup* ExpertStartPopup::create() {
    auto ret = new ExpertStartPopup;
    auto screenSize = CCDirector::sharedDirector()->getWinSize();
    if (ret->initAnchored(POPUP_WIDTH, POPUP_HEIGHT)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}