#pragma once

#include <Geode/DefaultInclude.hpp>
#include <Geode/utils/web.hpp>
#include <cocos2d.h>
#include <Geode/Geode.hpp>
#include <random>
#include <string>

#include "ExpertStartupLayer.hpp"

using namespace geode::prelude;

extern int lives;
bool super_expert = false;
std::vector<int> ids;

std::vector<std::string> splitString(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream ss(s);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

class ExpertMapLayer : public CCLayer, LevelDownloadDelegate {
protected:
    bool init() override;
    void keyBackClicked() override;
    

public:
    virtual void levelDownloadFinished(GJGameLevel*);
    virtual void levelDownloadFailed(int);

    static ExpertMapLayer* create();
    static ExpertMapLayer* scene();

    void onGoBack(CCObject*);
    void start_expert_run(CCObject*);
    void end_expert_run(CCObject*);
    void downloadLevel(CCObject*);
    void downloadLevels();
    void addMap();
    void ondownloadfinished(std::string const&);

    CCLabelBMFont* dl_txt;
    CCMenuItemSpriteExtra* startBtn;
    int dl_count;
    std::string sharelevels;
    CCMenu* end_run_btn_menu;
};

void ExpertMapLayer::downloadLevel(CCObject* self) {
    log::info("{}", self->getTag());
    GameLevelManager::sharedState()->m_levelDownloadDelegate = this;
    GameLevelManager::sharedState()->downloadLevel(self->getTag(), true); // fuck you rob
}

void ExpertMapLayer::levelDownloadFinished(GJGameLevel* level) {
    log::info("yay");
    ExpertStartupLayer::scene(level);
}

void ExpertMapLayer::levelDownloadFailed(int p0) {
    log::info("nay");
}

bool ExpertMapLayer::init() {
    if (!CCLayer::init())
        return false;

    GameManager* manager = GameManager::sharedState();
    auto director = CCDirector::sharedDirector();
	auto size = director->getWinSize();
    
    this->setKeypadEnabled(true);

    CCSprite* expert_run_bg = CCSprite::create("game_bg_08_001.png");
    CCLabelBMFont* lives_text = CCLabelBMFont::create(std::to_string(lives).c_str(), "gjFont59.fnt");
	CCLabelBMFont* lives_text_x = CCLabelBMFont::create("x", "gjFont59.fnt");
    CCLabelBMFont* start_game_text = CCLabelBMFont::create("Start Expert Run", "bigFont.fnt");
    CCLabelBMFont* super_expert_lbl = CCLabelBMFont::create("Super Expert Run", "goldFont.fnt");
    dl_count = 0;

    dl_txt = CCLabelBMFont::create("Levels Downloaded: 0/15", "bigFont.fnt");
    dl_txt->setPosition({size.width/ 2, size.height/ 2});
    dl_txt->setVisible(false);
    dl_txt->setScale(0.75);

    CCMenu* back_btn_menu = CCMenu::create();
    CCMenu* start_btn_menu = CCMenu::create();
    
    end_run_btn_menu = CCMenu::create();

    auto backBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png"),
        this, menu_selector(ExpertMapLayer::onGoBack));

    startBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_longBtn03_001.png"),
        this, menu_selector(ExpertMapLayer::start_expert_run));

    auto endRunBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_deleteBtn_001.png"),
        this, menu_selector(ExpertMapLayer::end_expert_run));

	auto bottomLeft = CCSprite::createWithSpriteFrameName("GJ_sideArt_001.png");
	auto topRight = CCSprite::createWithSpriteFrameName("GJ_sideArt_001.png");
	auto bottomRight = CCSprite::createWithSpriteFrameName("GJ_sideArt_001.png");

	topRight->setFlipY(true);
	topRight->setFlipX(true);
	bottomRight->setFlipX(true);

	CCSize img_size = bottomLeft->getScaledContentSize();

	bottomLeft->setPosition({ img_size.width / 2, img_size.height / 2 });
	topRight->setPosition({ (size.width) - (img_size.width / 2), (size.height) - (img_size.height / 2) });
	bottomRight->setPosition({ (size.width) - (img_size.width / 2), img_size.height / 2 });

	addChild(bottomLeft, 1);
	addChild(topRight, 1);
	addChild(bottomRight, 1); // thanks fig

    back_btn_menu->setPosition(size.width / 2 - 261, size.height - 30); // exit button position
    end_run_btn_menu->setPosition({back_btn_menu->getPositionX(), back_btn_menu->getPositionY() - 20});

    super_expert_lbl->setPosition({start_btn_menu->getPositionX(), back_btn_menu->getPositionY()});
    super_expert_lbl->setScale(1.2);

    expert_run_bg->setPosition({size.width / 2, size.height / 2});
    expert_run_bg->setScale(1.2);
    expert_run_bg->setColor(ccColor3B(167,15,224));

    end_run_btn_menu->setScale(0.7);
    end_run_btn_menu->setPosition({end_run_btn_menu->getPositionX() - 85, end_run_btn_menu->getPositionY() - 70});


    SimplePlayer* player = SimplePlayer::create(manager->getPlayerFrame());
    player->m_firstLayer->setColor(manager->colorForIdx(manager->getPlayerColor()));
    player->m_secondLayer->setColor(manager->colorForIdx(manager->getPlayerColor2()));
    player->updateColors();

    player->setPosition({91,12});
    player->setScale(0.65);
    addChild(player, 2);

    lives_text->setScale(0.6);
    lives_text->setOpacity(200);
    lives_text->setPosition({player->getPositionX() + 35,player->getPositionY() - 1.8f});

    lives_text_x->setScale(0.6);
    lives_text_x->setOpacity(200);
    lives_text_x->setPosition({lives_text->getPositionX() - 18, lives_text->getPositionY()});

    start_btn_menu->setPosition({size.width/ 2, size.height/ 2});
    start_game_text->setPosition(85,17);
    start_game_text->setScale(0.5);

    addChild(expert_run_bg, -10); // run first cuz bg thanks everyone
    addChild(lives_text);
    addChild(lives_text_x);
    addChild(back_btn_menu);
    addChild(dl_txt);
    addChild(end_run_btn_menu);
    addChild(super_expert_lbl);
    end_run_btn_menu->addChild(endRunBtn);
    end_run_btn_menu->setVisible(false);
    back_btn_menu->addChild(backBtn);
    if (!super_expert) {
        addChild(start_btn_menu);
        start_btn_menu->addChild(startBtn);
        startBtn->addChild(start_game_text);
    }
    else {
        addMap();
    }

    return true;
}

void ExpertMapLayer::addMap() {
    CCMenu* dotsmenu = CCMenu::create();

    CCMenuItemSpriteExtra* castleBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("theTower_01_001.png"), this, NULL);
    castleBtn->setPosition({95, 90});
    castleBtn->setScale(0.75);
    dotsmenu->addChild(castleBtn);

    // thanks chatgpt
    std::vector<CCPoint> stageCoordinates = {
        {135, 45}, {175, 45}, {215, 45}, {255, 45}, {282, 72}, {255, 100},
        {215, 100}, {175, 100}, {147, 127}, {175, 155}, {215, 155},
        {255, 155}, {295, 155}, {335, 155}, {375, 155}
    };

    std::vector<CCMenuItemSpriteExtra*> stageButtons;
    int i = 0;

    for (const auto& coord : stageCoordinates) {
        CCMenuItemSpriteExtra* stageBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("uiDot_001.png"), this, menu_selector(ExpertMapLayer::downloadLevel));
        stageBtn->setPosition(coord);
        stageBtn->setScale(1.5);
        stageBtn->setTag(ids[i]);
        stageButtons.push_back(stageBtn);
        dotsmenu->addChild(stageBtn);
        i++;
    }

    CCMenuItemSpriteExtra* castleEndBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("uiDot_001.png"), this, NULL);
    castleEndBtn->setPosition({420, 195});
    dotsmenu->addChild(castleEndBtn);

    dotsmenu->setPosition({29, 29});
    addChild(dotsmenu);
}

void ExpertMapLayer::start_expert_run(CCObject*) {
    super_expert = true;
    dl_txt->setVisible(true);
    startBtn->setVisible(false);
    end_run_btn_menu->setVisible(true);

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
        sharelevels += leveldata[1] + ";";
        ids.push_back(std::stoi(leveldata[1]));
    }
    else {
        sharelevels += leveldata[1];
        ids.push_back(std::stoi(leveldata[1]));
        dl_txt->setVisible(false);
        addMap();
    }
}

void ExpertMapLayer::end_expert_run(CCObject*) {
    super_expert = false;
    //FLAlertLayer* end_run = FLAlertLayer::create("End Run", "Press <cy>OK</c> to <cr>end your run</c>.", "OK");
    //end_run->show();
    lives = 30;
    ids.clear();
    ExpertMapLayer::keyBackClicked();
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
