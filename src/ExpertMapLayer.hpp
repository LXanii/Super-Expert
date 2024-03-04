#pragma once

#include <Geode/DefaultInclude.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/utils/web.hpp>
#include <cocos2d.h>
#include <Geode/Geode.hpp>
#include <random>
#include <string>

#include "ExpertStartupLayer.hpp"

using namespace geode::prelude;

int current_level = 0;
extern int lives;
bool super_expert = false;
std::vector<int> ids;
extern bool downloading;
std::string sharelevels;

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
    static ExpertMapLayer* replaceScene();

    void onGoBack(CCObject*);
    void start_expert_run(CCObject*);
    void end_expert_run(CCObject*);
    void downloadLevel(CCObject*);
    void copyRunID(CCObject*);
    void openSettings(CCObject*);
    void downloadLevels();
    void addMap();
    void ondownloadfinished(std::string const&);
    void expertReset();
    void stopAllAudio();

    int dl_count;
    CCLabelBMFont* dl_txt;
    CCLabelBMFont* lvls_completed;
    CCMenu* end_run_btn_menu;
    CCMenu* copy_btn_menu;
    CCMenu* settings_menu;
    CCMenuItemSpriteExtra* startBtn;
    CCMenuItemSpriteExtra* copyBtn;
};

void ExpertMapLayer::downloadLevel(CCObject* self) {
    log::info("{}", self->getTag());
    GameLevelManager::sharedState()->m_levelDownloadDelegate = this;
    GameLevelManager::sharedState()->downloadLevel(self->getTag(), true); // fuck you rob

    ExpertMapLayer::stopAllAudio();
    // put sfx here
}

void ExpertMapLayer::levelDownloadFinished(GJGameLevel* level) {
    ExpertStartupLayer::scene(level);
}

void ExpertMapLayer::levelDownloadFailed(int p0) {
    downloading = false;
}

bool ExpertMapLayer::init() {
    if (!CCLayer::init())
        return false;

    if (lives <= 0) expertReset();

    GameManager* manager = GameManager::sharedState();
    auto director = CCDirector::sharedDirector();
	auto size = director->getWinSize();
    
    this->setKeypadEnabled(true);

    CCSprite* expert_run_bg = CCSprite::create("game_bg_36_001.png");
    CCLabelBMFont* lives_text = CCLabelBMFont::create(std::to_string(lives).c_str(), "gjFont59.fnt");
	CCLabelBMFont* lives_text_x = CCLabelBMFont::create("x", "gjFont59.fnt");
    CCLabelBMFont* start_game_text = CCLabelBMFont::create("Start Expert Run", "bigFont.fnt");
    CCLabelBMFont* run_id = CCLabelBMFont::create("Share Run ID", "bigFont.fnt");
    CCLabelBMFont* super_expert_lbl = CCLabelBMFont::create("Super Expert Run", "goldFont.fnt");
    CCLabelBMFont* lvls_completed = CCLabelBMFont::create(fmt::format("Levels Complete: {}/15", current_level).c_str(), "chatFont.fnt");
    dl_count = 0;

    dl_txt = CCLabelBMFont::create("Levels Downloaded: 0/15", "bigFont.fnt");
    dl_txt->setPosition({size.width/ 2, size.height/ 2});
    dl_txt->setVisible(false);
    dl_txt->setScale(0.75);

    CCMenu* back_btn_menu = CCMenu::create();
    CCMenu* start_btn_menu = CCMenu::create();
    
    end_run_btn_menu = CCMenu::create();
    copy_btn_menu = CCMenu::create();
    settings_menu = CCMenu::create();

    auto backBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png"),
        this, menu_selector(ExpertMapLayer::onGoBack));

    auto settingsBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png"),
        this, menu_selector(ExpertMapLayer::openSettings));

    startBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_longBtn03_001.png"),
        this, menu_selector(ExpertMapLayer::start_expert_run));

    copyBtn = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_longBtn03_001.png"),
        this, menu_selector(ExpertMapLayer::copyRunID));


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

    lvls_completed->setPosition({super_expert_lbl->getPositionX(), super_expert_lbl->getPositionY() - 30});

    expert_run_bg->setPosition({size.width / 2, size.height / 2});
    expert_run_bg->setScale(1.2);
    expert_run_bg->setColor(ccColor3B(43,57,96));

    end_run_btn_menu->setScale(0.7);
    end_run_btn_menu->setPosition({end_run_btn_menu->getPositionX() - 85, end_run_btn_menu->getPositionY() - 70});


    SimplePlayer* player = SimplePlayer::create(manager->getPlayerFrame());
    player->m_firstLayer->setColor(manager->colorForIdx(manager->getPlayerColor()));
    player->m_secondLayer->setColor(manager->colorForIdx(manager->getPlayerColor2()));
    player->setGlowOutline(manager->colorForIdx(manager->getPlayerGlowColor()));
			
    if (manager->getPlayerGlow()) player->enableCustomGlowColor(manager->colorForIdx(manager->getPlayerGlowColor()));
    else player->disableGlowOutline();
    
    player->updateColors();

    player->setPosition({91,12});
    player->setScale(0.65);
    addChild(player, 2);

    copy_btn_menu->setScale(0.8);
    copy_btn_menu->setPosition({player->getPositionX() + 260, player->getPositionY() - 30});

    lives_text->setScale(0.6);
    lives_text->setOpacity(200);
    lives_text->setPosition({player->getPositionX() + 35,player->getPositionY() - 1.8f});

    lives_text_x->setScale(0.6);
    lives_text_x->setOpacity(200);
    lives_text_x->setPosition({lives_text->getPositionX() - 18, lives_text->getPositionY()});

    start_btn_menu->setPosition({size.width/ 2, size.height/ 2});

    start_game_text->setPosition(85,17);
    start_game_text->setScale(0.5);

    run_id->setScale(0.6);
    run_id->setPosition({85, 16.5});

    addChild(expert_run_bg, -10); // run first cuz bg thanks everyone
    addChild(lives_text);
    addChild(lives_text_x);
    addChild(back_btn_menu);
    addChild(lvls_completed);
    addChild(dl_txt);
    addChild(end_run_btn_menu);
    addChild(super_expert_lbl);

    addChild(copy_btn_menu);

    copy_btn_menu->addChild(copyBtn);
    copyBtn->addChild(run_id);
    copyBtn->setVisible(false);

    addChild(settings_menu);
    settings_menu->addChild(settingsBtn);
    settingsBtn->setPosition({0, -50});

    settings_menu->setVisible(false); // remove in future for update thanks me

    back_btn_menu->addChild(backBtn);
    if (!super_expert) {
        end_run_btn_menu->setVisible(false);
        lvls_completed->setVisible(false);
        addChild(start_btn_menu);
        start_btn_menu->addChild(startBtn);
        startBtn->addChild(start_game_text);
        lvls_completed->setVisible(false);
    }
    else {
        addMap();
    }

    return true;
}

void ExpertMapLayer::addMap() {

    CCSprite* stage_sprite;

    CCMenu* dotsmenu = CCMenu::create();

    CCTextureCache::sharedTextureCache()->addImage("TowerSheet.png", false);
    CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile("TowerSheet.plist");

    CCSprite* castleBtn = CCSprite::createWithSpriteFrameName("theTower_01_001.png");
    castleBtn->setPosition({35, 110});
    castleBtn->setScale(0.7);
    dotsmenu->addChild(castleBtn);

    // thanks chatgpt
    std::vector<CCPoint> stageCoordinates = {
        {105, 45}, {160, 45}, {215, 45}, {270, 45}, {315, 72}, {270, 100},
        {215, 100}, {160, 100}, {115, 127}, {160, 155}, {215, 155},
        {270, 155}, {325, 155}, {380, 155}, {435, 155}
    };

    std::vector<CCPoint> stageBlanks = {
        {133, 45}, {188, 45}, {243, 45}, {293, 55},
        {293, 90}, {243, 100}, {188, 100}, {128, 110}, {128, 145},
        {188, 155}, {243, 155}, {298, 155}, {353, 155}, {408, 155}
    };

    CCTextureCache::sharedTextureCache()->addImage("WorldSheet.png", false);
    CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile("WorldSheet.plist");

    int i = 0;
    for (const auto& coord : stageCoordinates) {
        if (current_level >= i) {
            stage_sprite = CCSprite::createWithSpriteFrameName("worldLevelBtn_001.png");
            CCMenuItemSpriteExtra* stageBtn;
            if (current_level == i) stageBtn = CCMenuItemSpriteExtra::create(stage_sprite, this, menu_selector(ExpertMapLayer::downloadLevel));
            else {
                stageBtn = CCMenuItemSpriteExtra::create(stage_sprite, this, NULL);
                stageBtn->setColor({ 92, 92, 92 });
            }
            stageBtn->setPosition(coord);
            stageBtn->setTag(ids[i]);
            dotsmenu->addChild(stageBtn);
        }
        else {
            stage_sprite = CCSprite::createWithSpriteFrameName("worldLevelBtn_locked_001.png");
            stage_sprite->setPosition(coord);
            stage_sprite->setTag(ids[i]);
            dotsmenu->addChild(stage_sprite);
        }
        
        i++;
    }

    for (const auto& coord : stageBlanks) {

        CCSprite* stageBlank = CCSprite::createWithSpriteFrameName("uiDot_001.png");
        stageBlank->setPosition(coord);
        stageBlank->setScale(0.5);
        dotsmenu->addChild(stageBlank);
    }

    CCSprite* castleEndBtn = CCSprite::createWithSpriteFrameName("theTowerDoor_001.png");
    castleEndBtn->setPosition({403, 73});  
    castleEndBtn->setScale(0.6f);
    dotsmenu->addChild(castleEndBtn);

    dotsmenu->setPosition({29, 29});

    // copyBtn->setVisible(true); shared run id button uncomment in future update thanks again me
    settings_menu->setVisible(false);

    addChild(dotsmenu);
}

void ExpertMapLayer::start_expert_run(CCObject*) {
    std::string run_id_val = Mod::get()->getSettingValue<std::string>("run-id");

    dl_txt->setVisible(true);
    startBtn->setVisible(false);
    settings_menu->setVisible(false);
    current_level = 0;
    //end_run_btn_menu->setVisible(true);
    /*if (run_id_val != "") {
        std::vector<std::string> levelids = splitString(run_id_val, ';');

        for (int i = 0; i < levelids.size(); i++) {
            log::info("{}", levelids[i]);
        }

    }
    else {
        downloading = true;
        downloadLevels();
    } */
    downloading = true;
    downloadLevels();
}

void ExpertMapLayer::copyRunID(CCObject*) {
    clipboard::write(sharelevels);
    Notification::create("Run ID Copied to Clipboard.", CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png"), 2.0f)->show();
}

void ExpertMapLayer::downloadLevels() {
    web::AsyncWebRequest()
        .userAgent("")
		.postRequest()
		.bodyRaw(fmt::format("diff=5&type=4&page={}&len=5&secret=Wmfd2893gb7", rand() % 3040))
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
    std::vector<std::string> levelvect = splitString(string, '|');
    std::string level = levelvect[rand() % 10];
    std::vector<std::string> leveldata = splitString(level, ':');
    log::info("{}", leveldata[1]);
    dl_count++;
    dl_txt->setString(fmt::format("Levels Downloaded: {}/15", dl_count).c_str());
    if (dl_count < 15) {
        downloadLevels();
        sharelevels += leveldata[1] + ";"; // pls dont remove need it for multi runs thx
        ids.push_back(std::stoi(leveldata[1]));
    }
    else {
        sharelevels += leveldata[1];
        ids.push_back(std::stoi(leveldata[1]));
        dl_txt->setVisible(false);
        super_expert = true;
        downloading = false;
        end_run_btn_menu->setVisible(true);
        Mod::get()->setSettingValue<std::string>("run-id", "");
        addMap();
    }
} 

void ExpertMapLayer::expertReset() {
    super_expert = false;
    ids.clear();
    lives = 30;
    
    ExpertMapLayer::keyBackClicked();
}


void ExpertMapLayer::keyBackClicked() {
    this->onGoBack(nullptr);
}

void ExpertMapLayer::onGoBack(CCObject*) {
    if (downloading) {
        FLAlertLayer::create("Invalid Action", "You can't leave while downloading!", "OK")->show();
        return;
    }
    if (super_expert && !downloading) createQuickPopup("End Expert Run", "Would you like to <cr>end</c> your <cp>expert run</c>?", "NO", "YES", [this](FLAlertLayer*, bool btn2) {
        if (btn2) {
            expertReset(); 
            CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
        }
        });
    else if (!super_expert) {
        FMODAudioEngine* fm = FMODAudioEngine::sharedEngine();
        
        CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade); 
        ExpertMapLayer::stopAllAudio();

        fm->playMusic("menuLoop.mp3", true, fm->m_musicVolume, 0);
    }
}

void ExpertMapLayer::openSettings(CCObject*) {
    openSettingsPopup(Mod::get());
}

void ExpertMapLayer::stopAllAudio() {

    FMODAudioEngine::sharedEngine()->stopAllMusic();
    FMODAudioEngine::sharedEngine()->stopAllActions();
    FMODAudioEngine::sharedEngine()->stopAllEffects();

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

    FMODAudioEngine* fm = FMODAudioEngine::sharedEngine();

    auto layer = ExpertMapLayer::create();
    auto scene = CCScene::create();
    scene->addChild(layer);
    auto transition = CCTransitionFade::create(0.5f, scene);
    CCDirector::sharedDirector()->pushScene(transition);

    FMODAudioEngine::sharedEngine()->stopAllMusic();
    FMODAudioEngine::sharedEngine()->stopAllActions();
    FMODAudioEngine::sharedEngine()->stopAllEffects();

    fm->playMusic("super_expert_music.mp3"_spr, true, fm->m_musicVolume, 1);

    return layer;
}

ExpertMapLayer* ExpertMapLayer::replaceScene() {

    FMODAudioEngine* fm = FMODAudioEngine::sharedEngine();

    auto layer = ExpertMapLayer::create();
    auto scene = CCScene::create();
    scene->addChild(layer);
    auto transition = CCTransitionFade::create(0.5f, scene);
    CCDirector::sharedDirector()->replaceScene(transition);
    
    FMODAudioEngine::sharedEngine()->stopAllMusic();
    FMODAudioEngine::sharedEngine()->stopAllActions();
    FMODAudioEngine::sharedEngine()->stopAllEffects(); // dont make fun of me this is the only way it would work :(

    fm->playMusic("super_expert_music.mp3"_spr, true, fm->m_musicVolume, 1);

    log::info("e");

    return layer;
}
