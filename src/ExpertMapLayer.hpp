#pragma once

#include <Geode/DefaultInclude.hpp>
#include <Geode/utils/web.hpp>
#include <cocos2d.h>
#include <Geode/Geode.hpp>
#include <random>

#include "ExpertStartupLayer.hpp"

using namespace geode::prelude;

extern int lives;
bool super_expert = false;

std::vector<std::string> splitString(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream ss(s);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

class ExpertMapLayer : public CCLayer {

protected:
    bool init() override;
    void keyBackClicked() override;
    

public:
    static ExpertMapLayer* create();
    static ExpertMapLayer* scene();

    void onGoBack(CCObject*);
    void start_expert_run(CCObject*);
    void end_expert_run(CCObject*);
    void downloadLevels();
    void ondownloadfinished(std::string const&);

    CCLabelBMFont* dl_txt;
    CCMenuItemSpriteExtra* startBtn;
    int dl_count;
};

bool ExpertMapLayer::init() {
    if (!CCLayer::init())
        return false;

    auto director = CCDirector::sharedDirector();
	auto size = director->getWinSize();
    
    this->setKeypadEnabled(true);

    CCSprite* expert_run_bg = CCSprite::create("game_bg_08_001.png");
    CCLabelBMFont* lives_text = CCLabelBMFont::create(std::to_string(lives).c_str(), "gjFont59.fnt");
    CCSprite* lives_img = CCSprite::create("lives_count.png"_spr);
    CCLabelBMFont* start_game_text = CCLabelBMFont::create("Start Expert Run", "goldFont.fnt");
    dl_count = 0;

    dl_txt = CCLabelBMFont::create("Levels Downloaded: 0/15", "bigFont.fnt");
    dl_txt->setPosition({size.width/2, size.height/2});
    dl_txt->setVisible(false);
    dl_txt->setScale(0.5);

    CCMenu* back_btn_menu = CCMenu::create();
    CCMenu* start_btn_menu = CCMenu::create();
    CCMenu* end_run_btn_menu = CCMenu::create();

    auto backBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png"),
        this, menu_selector(ExpertMapLayer::onGoBack));

    startBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_longBtn03_001.png"),
        this, menu_selector(ExpertMapLayer::start_expert_run));

    auto endRunBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_deleteBtn_001.png"),
        this, menu_selector(ExpertMapLayer::end_expert_run));

    back_btn_menu->setPosition(size.width / 2 - 261, size.height - 30); // exit button position
    end_run_btn_menu->setPosition({back_btn_menu->getPositionX(), back_btn_menu->getPositionY() - 20});


    expert_run_bg->setPosition({size.width / 2, size.height / 2});
    expert_run_bg->setScale(1.2);
    expert_run_bg->setColor(ccColor3B(167,15,224));

    end_run_btn_menu->setScale(0.7);
    end_run_btn_menu->setPosition({end_run_btn_menu->getPositionX() - 85, end_run_btn_menu->getPositionY() - 70});

    lives_img->setScale(0.6);
    lives_img->setPosition({17,12});

    lives_text->setScale(0.6);
    lives_text->setPosition({lives_img->getPositionX() + 26,lives_img->getPositionY() - 1.8f});

    start_btn_menu->setPosition({462, lives_text->getPositionY() + 8});
    start_game_text->setPosition(85,18);
    start_game_text->setScale(0.68);

    addChild(expert_run_bg, -10); // run first cuz bg thanks everyone
    addChild(lives_text);
    addChild(lives_img);
    addChild(back_btn_menu);
    addChild(dl_txt);
    back_btn_menu->addChild(backBtn);
    if (!super_expert) {
        addChild(start_btn_menu);
        start_btn_menu->addChild(startBtn);
        startBtn->addChild(start_game_text);
    }
    else {
        addChild(end_run_btn_menu);
        end_run_btn_menu->addChild(endRunBtn);
    }

    return true;
}

void ExpertMapLayer::start_expert_run(CCObject*) {
    super_expert = true;
    dl_txt->setVisible(true);
    startBtn->setVisible(false);

    downloadLevels();
}

void ExpertMapLayer::downloadLevels() {
    int rng = rand() % 3040;  
    web::AsyncWebRequest()
        .userAgent("")
		.postRequest()
		.bodyRaw(fmt::format("diff=5&type=4&page={}&len=5&secret=Wmfd2893gb7", rng))
        .fetch("http://www.boomlings.com/database/getGJLevels21.php")
        .text()
        .then([this](std::string const& resultat) {
			//log::info("yay: {}", resultat);
            ondownloadfinished(resultat);
		})
        .expect([](std::string const& error) {
			log::info("nay: {}", error);
        });
}

void ExpertMapLayer::ondownloadfinished(std::string const& string) {
    int rng = rand() % 10;
    std::vector<std::string> levelvect = splitString(string, '|');
    std::string level = levelvect[rng];
    std::vector<std::string> leveldata = splitString(level, ':');
    log::info("{}", leveldata[1]);
    dl_count++;
    dl_txt->setString(fmt::format("Levels Downloaded: {}/15", dl_count).c_str());
    if (dl_count < 15) {
        downloadLevels();
    }
}

void ExpertMapLayer::end_expert_run(CCObject*) {
    super_expert = false;
    FLAlertLayer::create("End Run", "Would you like to <cr>end your run</c>?", "OK")->show();
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
