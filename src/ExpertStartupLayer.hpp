#pragma once

#include <cocos2d.h>

using namespace cocos2d;

class ExpertStartupLayer : public CCLayer {
protected:
  bool init(GJGameLevel*);
  void keyBackClicked() override;

public:
  static ExpertStartupLayer* create(GJGameLevel* level);
  static ExpertStartupLayer* scene(GJGameLevel* level);

  void onGoBack(CCObject*);
  void loadLevel();

  GJGameLevel* m_level;
};
