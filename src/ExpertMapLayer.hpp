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
int ids;
int skips = Mod::get()->getSettingValue<int64_t>("skips");
bool super_expert = false;
std::unordered_map<int, std::string> authors;
std::string sharelevels;
extern int lives;
extern bool levelEnd;
extern bool downloading;

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
    CCMenu* end_run_btn_menu;
    CCMenu* settings_menu;
    CCMenu* devs_menu;
    CCMenu* showLevelMenu;
    CCMenuItemSpriteExtra* startBtn;
    CCMenuItemSpriteExtra* devBtn;
    LoadingCircle* loading_circle;
};

void ExpertMapLayer::keyBackClicked() {
    this->onGoBack(nullptr);
}

void ExpertMapLayer::downloadLevel(CCObject* self) {
    if (downloading) return;
    log::info("{}", self->getTag());
    GameLevelManager::sharedState()->m_levelDownloadDelegate = this;
    GameLevelManager::sharedState()->downloadLevel(self->getTag(), true); // fuck you rob

    FMODAudioEngine::sharedEngine()->stopAllMusic();
    FMODAudioEngine::sharedEngine()->stopAllActions();
    FMODAudioEngine::sharedEngine()->stopAllEffects();

    // put sfx here
    if (!Mod::get()->getSettingValue<bool>("level-info")) FMODAudioEngine::sharedEngine()->playMusic("playSound_01.ogg", false, 1, 0);
}

void ExpertMapLayer::levelDownloadFinished(GJGameLevel* level) {
    // I am sorry for this code C++ sucks
    std::vector<std::string> authorsplit = splitString(authors[level->m_userID], ':');
    level->m_creatorName = authorsplit[1];
    level->m_accountID = stoi(authorsplit[2]);
    if (Mod::get()->getSettingValue<bool>("level-info")) {
        CCDirector::get()->replaceScene(CCTransitionFade::create(0.5f, LevelInfoLayer::scene(level, false)));
        return;
    }
    ExpertStartupLayer::scene(level);
}

void ExpertMapLayer::levelDownloadFailed(int p0) {
    downloading = false;
}

bool ExpertMapLayer::init() { //beware, this code is dog shit holy fuck
    if (!CCLayer::init())
        return false;

    if (lives < 0) {
        expertReset();
        auto showGameOver = CCCallFunc::create(this, callfunc_selector(ExpertMapLayer::showGameOver));
        runAction(CCSequence::createWithTwoActions(CCDelayTime::create(0.2f), showGameOver));
    }

    GameManager* manager = GameManager::sharedState();
    auto director = CCDirector::sharedDirector();
	auto size = director->getWinSize();
    int current_level_display = current_level;
    
    this->setKeypadEnabled(true);

    CCSprite* expert_run_bg = CCSprite::create("game_bg_08_001.png");
    CCLabelBMFont* lives_text = CCLabelBMFont::create(std::to_string(lives).c_str(), "gjFont59.fnt");
	CCLabelBMFont* lives_text_x = CCLabelBMFont::create("x", "gjFont59.fnt");
    CCLabelBMFont* start_game_text = CCLabelBMFont::create("Start Expert Run", "bigFont.fnt");
    CCLabelBMFont* super_expert_lbl = CCLabelBMFont::create("Super Expert Run", "goldFont.fnt");
    skips_left = CCLabelBMFont::create(fmt::format("Skips Left: {}/{}", skips, Mod::get()->getSettingValue<int64_t>("skips")).c_str(), "chatFont.fnt");
    dl_txt = CCLabelBMFont::create("Fetching Level ID...", "bigFont.fnt");
    
    CCLabelBMFont* lvls_completed = CCLabelBMFont::create(fmt::format("Levels Complete: {}/15", current_level_display).c_str(), "chatFont.fnt");
    if (current_level_display > 15) current_level_display = 15;
    dl_count = 0;

    dl_txt->setPosition({size.width/ 2, size.height/ 2 + 65.f});
    dl_txt->setVisible(false);
    dl_txt->setScale(0.6);

    loading_circle = LoadingCircle::create();
    addChild(loading_circle);
    loading_circle->setVisible(false);

    // menus & assets below

    CCMenu* back_btn_menu = CCMenu::create();
    CCMenu* start_btn_menu = CCMenu::create();
    CCMenu* discord_btn_menu = CCMenu::create();
    
    showLevelMenu = CCMenu::create();
    end_run_btn_menu = CCMenu::create();
    settings_menu = CCMenu::create();
    devs_menu = CCMenu::create();

    CCSprite* settingsGear = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
    settingsGear->setScale(0.5);

    CCSprite* discordSprite = CCSprite::createWithSpriteFrameName("gj_discordIcon_001.png");
    discordSprite->setScale(0.75);

    CCSprite* devsSprite = CCSprite::create("devs.png"_spr);
    devsSprite->setScale(0.6);

    CCSprite* showLevels = CCSprite::create("levels.png"_spr);
    showLevels->setScale(0.55);

    auto backBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png"), this, menu_selector(ExpertMapLayer::onGoBack));
    auto settingsBtn = CCMenuItemSpriteExtra::create(settingsGear, this, menu_selector(ExpertMapLayer::openSettings));
    auto discordBtn = CCMenuItemSpriteExtra::create(discordSprite, this, menu_selector(ExpertMapLayer::openDiscord));
    auto showLevelBtn = CCMenuItemSpriteExtra::create(showLevels, this, menu_selector(ExpertMapLayer::showAllLevels));    
    startBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_longBtn03_001.png"), this, menu_selector(ExpertMapLayer::start_expert_run));
    devBtn = CCMenuItemSpriteExtra::create(devsSprite, this, menu_selector(ExpertMapLayer::openDevs));

    addChild(discord_btn_menu);
    discord_btn_menu->addChild(discordBtn);
    discord_btn_menu->setPosition({527.5f,245});

    addChild(showLevelMenu);
    showLevelMenu->addChild(showLevelBtn);
    showLevelMenu->setPosition(discord_btn_menu->getPositionX(), discord_btn_menu->getPositionY() - 33);
    showLevelMenu->setVisible(false);

    addChild(devs_menu);
    devs_menu->addChild(devBtn);
    devs_menu->setPosition({93, 290});

    addChild(settings_menu);
    settings_menu->addChild(settingsBtn);
    settingsBtn->setPosition({243, 117});
    
    // end of menus

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

    skips_left->setPosition({bottomRight->getPositionX() - 100.f, lives_text->getPositionY() + 10});
    addChild(skips_left);
    skips_left->setVisible(false);

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
    addChild(lvls_completed);
    addChild(end_run_btn_menu);
    addChild(super_expert_lbl);


    back_btn_menu->addChild(backBtn);
    addChild(dl_txt);
    if (!super_expert) {
        end_run_btn_menu->setVisible(false);
        lvls_completed->setVisible(false);
        addChild(start_btn_menu);
        start_btn_menu->addChild(startBtn);
        startBtn->addChild(start_game_text);
        lvls_completed->setVisible(false);
    }
    else {
        (downloading) ? downloadLevels() : addMap();
        if (current_level == 15) {
            auto showCongrats = CCCallFunc::create(this, callfunc_selector(ExpertMapLayer::showCongrats));
            runAction(CCSequence::createWithTwoActions(CCDelayTime::create(0.3f), showCongrats));
            current_level++;
        }
    }

    handleTouchPriority(this);

    return true;
}

void ExpertMapLayer::showCongrats() {
    FLAlertLayer::create("Congratulations", "You have successfully completed\n<cp>Super Expert</c>!", "OK")->show();
}

void ExpertMapLayer::showGameOver() {
    FLAlertLayer::create("Out of Lives!", "It looks like you've <cr>ran out of lives</c>!", "OK")->show();
}

void ExpertMapLayer::showAllLevels(CCObject*) {
    FLAlertLayer::create("Level List", sharelevels, "OK")->show();
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
            stageBtn->setTag(ids);
            dotsmenu->addChild(stageBtn);
        }
        else {
            stage_sprite = CCSprite::createWithSpriteFrameName("worldLevelBtn_locked_001.png");
            stage_sprite->setPosition(coord);
            stage_sprite->setTag(ids);
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

    addChild(dotsmenu);
    skips_left->setVisible(true);
    showLevelMenu->setVisible(true);
}

void ExpertMapLayer::start_expert_run(CCObject*) {
    skips = Mod::get()->getSettingValue<int64_t>("skips");

    skips_left->setString(fmt::format("Skips Left: {}/{}", skips, Mod::get()->getSettingValue<int64_t>("skips")).c_str());

    dl_txt->setVisible(true);
    startBtn->setVisible(false);
    loading_circle->show();
    current_level = 0;
    sharelevels.clear();
    downloadLevels();
}

void ExpertMapLayer::openDevs(CCObject*) {
    FLAlertLayer::create("Thank You!", "<cy>Thanks for downloading!</c>\n\n<cr>Note:</c> This mod is currently in beta, as there is more we plan to add!\nAny suggestions / feedback is appreciated!\n\n<cg>From: Xanii & Adya</c> <cp><3</c>", "OK")->show();
}

void ExpertMapLayer::openDiscord(CCObject*) {
    geode::createQuickPopup("Join Discord", "Would you like to join the <cy>Official Discord Server?</c>", "No", "Yes", [] (auto fl, bool btn2) {
                if (btn2) geode::utils::web::openLinkInBrowser("https://discord.gg/W3BkznGTV8");});
}

void ExpertMapLayer::downloadLevels() {
    downloading = true;
    loading_circle->setVisible(true);
    loading_circle->show();
    dl_txt->setVisible(true);
    if (skips != Mod::get()->getSettingValue<int64_t>("skips")) skips_left->setString(fmt::format("Skips Left: {}/{}", skips, Mod::get()->getSettingValue<int64_t>("skips")).c_str());

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
    int rng = rand() % 10;
    std::vector<std::string> levelvect = splitString(string, '|');
    std::string level = levelvect[rng];
    std::vector<std::string> leveldata = splitString(level, ':');
    log::info("{}", leveldata[1]);
    if (loading_circle->isVisible()) loading_circle->setVisible(false);
    for (int i = 0; i < splitString(splitString(string, '#')[1], '|').size(); i++) {
        std::string split = splitString(splitString(string, '#')[1], '|')[i];
        std::vector<std::string> authorsplit = splitString(split, ':');
        authors.insert({stoi(authorsplit[0]), split });
    }
    try {
        ids = std::stoi(leveldata[1]);
        sharelevels += fmt::format("{}\n", leveldata[1]);
    }
    catch (std::invalid_argument const& ex) // FIX THIS SHIT FOR ANDROID IMA KMSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS
    {
        log::info("{}", ex.what());
        downloadLevels();
    }

    end_run_btn_menu->setVisible(true);
    dl_txt->setVisible(false);
    super_expert = true;
    downloading = false;
    levelEnd = false;
    Mod::get()->setSettingValue<std::string>("run-id", "");
    addMap();
} 

void ExpertMapLayer::expertReset() {
    super_expert = false;
    authors.clear();
    lives = 30; // keep this later i know im gonna remove it, its for actually resetting lives & skips when run is ended
    skips = Mod::get()->getSettingValue<int64_t>("skips");
}

void ExpertMapLayer::onGoBack(CCObject*) {
    if (downloading) {
        FLAlertLayer::create("Invalid Action", "You can't leave while downloading!", "OK")->show();
        return;
    }
    if (super_expert && !downloading) createQuickPopup("End Expert Run", "Would you like to <cr>end</c> your <cp>expert run</c>?", "NO", "YES", [this](FLAlertLayer*, bool btn2) {
        if (btn2) {
            expertReset(); 
            this->onGoBack(nullptr);
            CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
        }
        });
    else if (!super_expert) CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
}

void ExpertMapLayer::openSettings(CCObject*) {
    openSettingsPopup(Mod::get());
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

ExpertMapLayer* ExpertMapLayer::replaceScene() {
    
    FMODAudioEngine::sharedEngine()->stopAllMusic();
    FMODAudioEngine::sharedEngine()->stopAllActions();
    FMODAudioEngine::sharedEngine()->stopAllEffects();

    auto layer = ExpertMapLayer::create();
    auto scene = CCScene::create();
    scene->addChild(layer);
    auto transition = CCTransitionFade::create(0.5f, scene);
    CCDirector::sharedDirector()->replaceScene(transition);

    return layer;
}
