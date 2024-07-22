#pragma once

class ExpertStartPopup : public geode::Popup<> {

public:
  static const float POPUP_WIDTH;
  static const float POPUP_HEIGHT;

  static ExpertStartPopup* create();

  void startRun(CCObject*);

private:
  bool setup() override;
};
