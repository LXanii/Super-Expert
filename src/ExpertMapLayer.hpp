#pragma once

#include <Geode/ui/GeodeUI.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/Geode.hpp>
#include <random>
#include <string>

#include "ExpertStartupLayer.hpp"

using namespace geode::prelude;

class ExpertMapLayer : public CCLayer, LevelDownloadDelegate {
public:
    static ExpertMapLayer* create();
    static ExpertMapLayer* scene();
    
    void replaceScene();
    void confirmExpertRun();

protected:
    bool init() override;
    void keyBackClicked() override;
    virtual void levelDownloadFinished(GJGameLevel*) override;
    virtual void levelDownloadFailed(int) override;

    void onGoBack(CCObject*);
    void startExpertRun(CCObject*);
    void downloadLevel(CCObject*);
    void openSettings(CCObject*);
    void openDevs(CCObject*);
    void openDiscord(CCObject*);
    void showAllLevels(CCObject*);
    void downloadLevels();
    void addMap();
    void ondownloadfinished(std::string const&);
    void expertReset();
    void showCongrats();
    void showGameOver();

    int dl_count;
    CCLabelBMFont* dl_txt;
    CCLabelBMFont* lvls_completed;
    CCLabelBMFont* skips_left;
    CCLabelBMFont* lives_text;
    CCSprite* islandSprite;
    CCMenu* end_run_btn_menu;
    CCMenu* settings_menu;
    CCMenu* showLevelMenu;
    CCMenu* map_deco;
    CCMenuItemSpriteExtra* startBtn;
    LoadingCircle* loading_circle;
    EventListener<web::WebTask> webreq;
};