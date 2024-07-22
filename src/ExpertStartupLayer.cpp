#include "ExpertStartupLayer.hpp"
#include <Geode/DefaultInclude.hpp>
#include <Geode/modify/GameLevelManager.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <cocos2d.h>

using namespace geode::prelude;

int lives = Mod::get()->getSettingValue<int64_t>("lives");

bool ExpertStartupLayer::init(GJGameLevel* level) {
  if (!CCLayer::init()) {
    return false;
  }

  m_level = level;

  auto director = CCDirector::sharedDirector();
  auto size = director->getWinSize();

  this->setKeypadEnabled(true);

  CCLabelBMFont* level_name = CCLabelBMFont::create(level->m_levelName.c_str(), "gjFont59.fnt");
  level_name->setPosition({size.width / 2, size.height / 2 + 130});

  CCLabelBMFont* level_author = CCLabelBMFont::create(level->m_creatorName.c_str(), "chatFont.fnt");
  level_author->setPosition({size.width / 2, size.height / 2 + 100});
  level_author->setScale(0.6f);

  CCLabelBMFont* level_id = CCLabelBMFont::create(fmt::format("ID: {}", std::to_string(level->m_levelID)).c_str(), "chatFont.fnt");
  level_id->setPosition({size.width / 2, size.height / 2 + 80});
  level_id->setScale(0.6f);

  CCSprite* shadow = CCSprite::create("player_shadow.png"_spr);
  shadow->setPosition({size.width / 2, size.height / 2});
  shadow->setScale(0.775f);

  GameManager* manager = GameManager::sharedState();
  SimplePlayer* player = SimplePlayer::create(manager->getPlayerFrame());

  player->m_firstLayer->setColor(manager->colorForIdx(manager->getPlayerColor()));
  player->m_secondLayer->setColor(manager->colorForIdx(manager->getPlayerColor2()));
  player->setGlowOutline(manager->colorForIdx(manager->getPlayerGlowColor()));

  if (manager->getPlayerGlow()) {
    player->enableCustomGlowColor(manager->colorForIdx(manager->getPlayerGlowColor()));
  } else {
    player->disableGlowOutline();
  }

  player->updateColors();

  if (manager->getPlayerGlow()) {
    player->enableCustomGlowColor(manager->colorForIdx(manager->getPlayerGlowColor()));
  }

  player->setScale(2.205f);
  player->setPosition({size.width / 2, size.height / 2 + 30.f});

  CCLabelBMFont* lives_x = CCLabelBMFont::create("x", "gjFont59.fnt");
  lives_x->setPosition({size.width / 2 + 59.f, size.height / 2 + 26.f});
  lives_x->setScale(0.750f);

  CCLabelBMFont* lives_txt = CCLabelBMFont::create(std::to_string(lives).c_str(), "gjFont59.fnt");
  lives_txt->setPosition({size.width / 2 + 99.5f, size.height / 2 + 22.5f});

  if (lives >= 100) {
    lives_txt->setPosition({lives_txt->getPositionX() + 10, lives_txt->getPositionY()}); // thanks mfs who get 100 lives
  }

  lives_txt->setScale(1.625f);

  this->addChild(level_name);
  this->addChild(level_author);
  this->addChild(level_id);
  this->addChild(lives_x);
  this->addChild(lives_txt);
  this->addChild(shadow);
  this->addChild(player);

  lives_x->setOpacity(0);
  auto fadeInAction = CCFadeIn::create(0.5f);
  lives_x->runAction(CCSequence::createWithTwoActions(CCDelayTime::create(1.f), fadeInAction));

  lives_txt->setScale(0.f);
  auto scaleUpAction = CCScaleTo::create(0.5f, 1.625f);
  auto bounceOutAction = CCEaseBackOut::create(scaleUpAction);
  lives_txt->runAction(CCSequence::createWithTwoActions(CCDelayTime::create(1.f), bounceOutAction));

  auto loadLevel = CCCallFunc::create(this, callfunc_selector(ExpertStartupLayer::loadLevel));
  runAction(CCSequence::createWithTwoActions(CCDelayTime::create(3.f), loadLevel));

  return true;
}

void ExpertStartupLayer::loadLevel() {
  CCDirector::get()->replaceScene(CCTransitionFade::create(0.5f, PlayLayer::scene(m_level, false, false)));
}

void ExpertStartupLayer::keyBackClicked() {
  // this->onGoBack(nullptr);
}

void ExpertStartupLayer::onGoBack(CCObject*) {
  CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
}

ExpertStartupLayer* ExpertStartupLayer::create(GJGameLevel* level) {
  auto ret = new ExpertStartupLayer;
  if (ret->init(level)) {
    ret->autorelease();
    return ret;
  }
  CC_SAFE_DELETE(ret);
  return nullptr;
}

ExpertStartupLayer* ExpertStartupLayer::scene(GJGameLevel* level) {
  auto layer = ExpertStartupLayer::create(level);
  auto scene = CCScene::create();
  scene->addChild(layer);
  auto transition = CCTransitionFade::create(0.5f, scene);
  CCDirector::sharedDirector()->replaceScene(transition);

  return layer;
}
