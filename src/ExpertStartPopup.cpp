#include <Geode/Geode.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/utils/web.hpp>
#include <cocos2d.h>

#include "ExpertMapLayer.hpp"
#include "ExpertStartPopup.hpp"

using namespace geode::prelude;

const float ExpertStartPopup::POPUP_WIDTH = 265.f;
const float ExpertStartPopup::POPUP_HEIGHT = 285.f;

bool ExpertStartPopup::setup() {
  auto screenSize = CCDirector::sharedDirector()->getWinSize();
  auto mainLayer = static_cast<CCLayer*>(this->getChildren()->objectAtIndex(0));
  auto main = static_cast<CCNode*>(mainLayer->getChildren()->objectAtIndex(0));

  CCLabelBMFont* settingsText = CCLabelBMFont::create("Run Settings", "bigFont.fnt");
  CCLabelBMFont* startText = CCLabelBMFont::create("Start Run", "bigFont.fnt");
  CCScale9Sprite* startSprite = CCScale9Sprite::create("GJ_button_01.png");
  CCMenu* startMenu = CCMenu::create();

  startSprite->setContentSize(CCPoint(120, 35));
  startText->setScale(0.625);
  startText->setPosition({60, 19});
  startSprite->addChild(startText);

  CCMenuItemSpriteExtra* startBtn = CCMenuItemSpriteExtra::create(startSprite, this, menu_selector(ExpertMapLayer::confirmExpertRun));

  startMenu->setPosition({main->getPositionX(), 30});
  settingsText->setPosition({main->getPositionX(), POPUP_HEIGHT - 20});
  startMenu->addChild(startBtn);
  main->addChild(settingsText);
  main->addChild(startMenu);

  return true;
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
