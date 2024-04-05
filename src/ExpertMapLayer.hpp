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
    CCLabelBMFont* lives_text;
    CCSprite* islandSprite;
    CCMenu* end_run_btn_menu;
    CCMenu* settings_menu;
    CCMenu* showLevelMenu;
    CCMenu* map_deco;
    CCMenuItemSpriteExtra* startBtn;
    LoadingCircle* loading_circle;
};

void ExpertMapLayer::keyBackClicked() {
    this->onGoBack(nullptr);
}

void ExpertMapLayer::downloadLevel(CCObject* self) {
    if (downloading) return;
    //log::info("{}", self->getTag());
    GameLevelManager::sharedState()->m_levelDownloadDelegate = this;
    GameLevelManager::sharedState()->downloadLevel(self->getTag(), true); // fuck you rob

    // put sfx here
    if (!Mod::get()->getSettingValue<bool>("level-info")) {
        FMODAudioEngine::sharedEngine()->stopAllMusic();
        FMODAudioEngine::sharedEngine()->stopAllActions();
        FMODAudioEngine::sharedEngine()->stopAllEffects();
        FMODAudioEngine::sharedEngine()->playMusic("playSound_01.ogg", false, 1, 0);
    }
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

    if (Mod::get()->getSettingValue<bool>("map-music")) {
        FMODAudioEngine::sharedEngine()->stopAllMusic();
        FMODAudioEngine::sharedEngine()->stopAllActions();
        FMODAudioEngine::sharedEngine()->stopAllEffects();

        FMODAudioEngine::sharedEngine()->playMusic("super_expert_music.mp3"_spr, true, 1, 0);
    }

    GameManager* manager = GameManager::sharedState();
    auto director = CCDirector::sharedDirector();
	auto size = director->getWinSize();
    int current_level_display = current_level;
    
    this->setKeypadEnabled(true);

    CCSprite* expert_run_bg = CCSprite::create("se_bg_1.png"_spr);
    CCSprite* expert_run_fg = CCSprite::create("se_bg_2.png"_spr);
    CCSprite* lives_holder = CCSprite::create("se_lives_bg.png"_spr);
    CCSprite* super_expert_lbl = CCSprite::create("se_logo.png"_spr);
	CCLabelBMFont* lives_text_x = CCLabelBMFont::create("x", "gjFont59.fnt");
    CCLabelBMFont* start_game_text = CCLabelBMFont::create("Start Expert Run", "bigFont.fnt");
    lives_text = CCLabelBMFont::create(std::to_string(lives).c_str(), "gjFont59.fnt");
    skips_left = CCLabelBMFont::create(fmt::format("Skips: {}/{}", skips, Mod::get()->getSettingValue<int64_t>("skips")).c_str(), "bigFont.fnt");
    dl_txt = CCLabelBMFont::create("Fetching Level ID...", "bigFont.fnt");
    islandSprite = CCSprite::create("se_island.png"_spr);

    if (!super_expert) islandSprite->setColor({ 0, 0, 0 });
    islandSprite->setScale(1.2);
    
    CCLabelBMFont* lvls_completed = CCLabelBMFont::create(fmt::format("Levels Complete: {}/15", current_level_display).c_str(), "chatFont.fnt");
    if (current_level_display > 15) current_level_display = 15;
    dl_count = 0;

    dl_txt->setPosition({size.width/ 2, size.height/ 2 + 65.f});
    dl_txt->setVisible(false);
    dl_txt->setScale(0.6);

    loading_circle = LoadingCircle::create();
    addChild(loading_circle);
    loading_circle->setVisible(false);

	auto bottomLeft = CCSprite::createWithSpriteFrameName("gauntletCorner_001.png");
	auto bottomRight = CCSprite::createWithSpriteFrameName("gauntletCorner_001.png");

	bottomRight->setFlipX(true);

	CCSize img_size = bottomLeft->getScaledContentSize();

	bottomLeft->setPosition({ img_size.width / 2, img_size.height / 2 });
	bottomRight->setPosition({ (size.width) - (img_size.width / 2), img_size.height / 2 });

	addChild(bottomLeft, 1);
	addChild(bottomRight, 1); // thanks fig

    CCMenu* back_btn_menu = CCMenu::create(); // my perfect order :(
    back_btn_menu->setPosition(img_size.width / 2 - 10, size.height - 30); // exit button position

    lives_holder->setPosition({((size.width) - (img_size.width / 2)) - 10, back_btn_menu->getPositionY()});
    lives_holder->setScale(0.35);

    // menus & assets below

    CCMenu* start_btn_menu = CCMenu::create();
    CCMenu* discord_btn_menu = CCMenu::create();
    CCMenu* label_menu = CCMenu::create();

    label_menu->setPosition({0,0});
    label_menu->setID("super-expert-label");
    
    showLevelMenu = CCMenu::create();
    end_run_btn_menu = CCMenu::create();
    settings_menu = CCMenu::create();

    CCSprite* settingsGear = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
    settingsGear->setScale(0.6);

    CCSprite* discordSprite = CCSprite::createWithSpriteFrameName("gj_discordIcon_001.png");
    discordSprite->setScale(0.87);

    CCSprite* showLevels = CCSprite::create("levels.png"_spr);
    showLevels->setScale(0.65);

    auto backBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png"), this, menu_selector(ExpertMapLayer::onGoBack));
    auto settingsBtn = CCMenuItemSpriteExtra::create(settingsGear, this, menu_selector(ExpertMapLayer::openSettings));
    auto discordBtn = CCMenuItemSpriteExtra::create(discordSprite, this, menu_selector(ExpertMapLayer::openDiscord));
    auto showLevelBtn = CCMenuItemSpriteExtra::create(showLevels, this, menu_selector(ExpertMapLayer::showAllLevels));
    auto labelBtn = CCMenuItemSpriteExtra::create(super_expert_lbl, this, menu_selector(ExpertMapLayer::openDevs));
    startBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_longBtn03_001.png"), this, menu_selector(ExpertMapLayer::start_expert_run));

    addChild(discord_btn_menu);
    discord_btn_menu->addChild(discordBtn);
    discord_btn_menu->setID("discord-button");
    discord_btn_menu->setPosition({lives_holder->getPositionX() + 20, lives_holder->getPositionY() - 40});

    addChild(settings_menu);
    settings_menu->addChild(settingsBtn);
    settings_menu->setID("settings-button");
    settings_menu->setPosition(discord_btn_menu->getPositionX(), discord_btn_menu->getPositionY() - 33);
    
    addChild(showLevelMenu);
    showLevelMenu->addChild(showLevelBtn);
    showLevelMenu->setID("levels-button");
    showLevelMenu->setPosition(settings_menu->getPositionX(), settings_menu->getPositionY() - 36);
    showLevelMenu->setVisible(false);
    
    end_run_btn_menu->setPosition({back_btn_menu->getPositionX(), back_btn_menu->getPositionY() - 20});

    // end of menus

    skips_left->setPosition({size.width / 2, 15});
    skips_left->setScale(0.65);
    addChild(skips_left, 2);
    skips_left->setVisible(false);

    labelBtn->setPosition({size.width / 2, back_btn_menu->getPositionY()});
    labelBtn->setScale(0.825);

    lvls_completed->setPosition({super_expert_lbl->getPositionX(), super_expert_lbl->getPositionY() - 30});

    expert_run_bg->setPosition({size.width / 2, size.height / 2});
    expert_run_fg->setPosition({size.width / 2, size.height / 2});
    expert_run_bg->setScale(1.2);
    expert_run_fg->setScale(1.2);
    expert_run_bg->setColor({ 189, 189, 189 });

    end_run_btn_menu->setScale(0.7);
    end_run_btn_menu->setPosition({end_run_btn_menu->getPositionX() - 85, end_run_btn_menu->getPositionY() - 70});

    SimplePlayer* player = SimplePlayer::create(manager->getPlayerFrame());
    player->m_firstLayer->setColor(manager->colorForIdx(manager->getPlayerColor()));
    player->m_secondLayer->setColor(manager->colorForIdx(manager->getPlayerColor2()));
    player->setGlowOutline(manager->colorForIdx(manager->getPlayerGlowColor()));
			
    if (manager->getPlayerGlow()) player->enableCustomGlowColor(manager->colorForIdx(manager->getPlayerGlowColor()));
    else player->disableGlowOutline();
    
    player->updateColors();

    player->setPosition({lives_holder->getPositionX() - 18, lives_holder->getPositionY()});
    player->setScale(0.725);


    addChild(player, 2);

    lives_text->setScale(0.65);
    lives_text->setOpacity(200);
    lives_text->setPosition({player->getPositionX() + 38, player->getPositionY() - 1.8f});

    lives_text_x->setScale(0.65);
    lives_text_x->setOpacity(200);

    if (lives >= 100) {
        player->setPosition({player->getPositionX() - 7, player->getPositionY()});
        lives_text->setPosition({lives_text->getPositionX() - 3, lives_text->getPositionY()});
        lives_text_x->setPosition({lives_text->getPositionX() - 23, lives_text->getPositionY()});
    }
    else lives_text_x->setPosition({lives_text->getPositionX() - 18, lives_text->getPositionY()}); // og pos

    start_btn_menu->setPosition({size.width/ 2, size.height/ 2 - 30});

    islandSprite->setPosition({size.width/ 2, size.height/ 2}); // edit for map pos

    // xaniis deco factory

    map_deco = CCMenu::create();

    CCParticleFire* fire_1 = CCParticleFire::create(); // right eye
    CCParticleFire* fire_2 = CCParticleFire::create(); // left eye
    CCParticleFire* fire_3 = CCParticleFire::create(); // under map
    CCParticleRain* rain_1 = CCParticleRain::create();

    fire_1->setPosition({islandSprite->getPositionX() - 110, islandSprite->getPositionY() + 34});
    fire_2->setPosition({islandSprite->getPositionX() - 137, islandSprite->getPositionY() + 30});
    fire_3->setPosition({skips_left->getPositionX(), skips_left->getPositionY() - 157});

    fire_1->setOpacity(100);
    fire_1->setScale(0.1);
    fire_2->setOpacity(100);
    fire_2->setScale(0.1);
    fire_3->setScaleX(2.500);
    rain_1->setOpacity(50);
    map_deco->addChild(fire_1, 1);
    map_deco->addChild(fire_2, 1);
    map_deco->setPosition({0,0});
    addChild(fire_3, 0);
    addChild(rain_1, 0);
    addChild(map_deco, 1);
    if (!super_expert) map_deco->setVisible(false);

    // END OF DECO FACTORY!!!!

    start_game_text->setPosition(85,17);
    start_game_text->setScale(0.5);

    addChild(expert_run_bg, -10); // run first cuz bg thanks everyone
    addChild(expert_run_fg, -9);
    addChild(lives_text, 1);
    addChild(lives_text_x, 1);
    addChild(lives_holder, 0);
    addChild(islandSprite);
    addChild(back_btn_menu);
    addChild(lvls_completed);
    addChild(end_run_btn_menu);
    
    label_menu->addChild(labelBtn);
    back_btn_menu->addChild(backBtn);
    addChild(label_menu);
    
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

    CCTextureCache::sharedTextureCache()->addImage("WorldSheet.png", false);
    CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile("WorldSheet.plist");

    // thanks chatgpt
    std::vector<CCPoint> stageCoordinates = {
        {-247, -16}, {-212, -15}, {-183, -28}, {-150, -20}, {-116, -32}, {-82, -25},
        {-50, -17}, {-26, 3}, {7, 8}, {14, -18}, {36, -36},
        {69, -27}, {106, -30}, {142, -28}, {174, -16}
    };

    std::vector<CCPoint> blankCoordinates = {
        {-231, -14}, {-198, -21}, {-167, -22}, {-135, -25},
        {-99, -27}, {-66, -20}, {-37, -7}, {-10, 7}, {9, -3},
        {23, -28}, {51, -30}, {87, -27}, {124, -28}, {159, -22}
    };

    int i = 0;
    for (const auto& coord : stageCoordinates) {
        if (current_level >= i) {
            stage_sprite = CCSprite::createWithSpriteFrameName("worldLevelBtn_001.png");
            CCMenuItemSpriteExtra* stageBtn;
            if (current_level == i) {
                stageBtn = CCMenuItemSpriteExtra::create(stage_sprite, this, menu_selector(ExpertMapLayer::downloadLevel));
                stage_sprite->setScale(0.9);
            }
            else {
                stageBtn = CCMenuItemSpriteExtra::create(stage_sprite, this, NULL);
                stageBtn->setColor({ 92, 92, 92 });
                stage_sprite->setScale(0.8);
            }
            stageBtn->setPosition(coord);
            stageBtn->setTag(ids);
            dotsmenu->addChild(stageBtn);
        }
        else {
            stage_sprite = CCSprite::createWithSpriteFrameName("worldLevelBtn_locked_001.png");
            stage_sprite->setPosition(coord);
            stage_sprite->setScale(0.8);
            stage_sprite->setTag(ids);
            dotsmenu->addChild(stage_sprite);
        }
        
        i++;
    }

    for (const auto& coord : blankCoordinates) {

        CCSprite* stageBlank = CCSprite::createWithSpriteFrameName("uiDot_001.png");
        stageBlank->setPosition(coord);
        stageBlank->setScale(0.5);
        dotsmenu->addChild(stageBlank);
    }

    dotsmenu->setPosition({islandSprite->getPositionX(), islandSprite->getPositionY()});

    addChild(dotsmenu);
    dotsmenu->setID("courses");
    skips_left->setVisible(true);
    showLevelMenu->setVisible(true);
}

void ExpertMapLayer::start_expert_run(CCObject*) {
    skips = Mod::get()->getSettingValue<int64_t>("skips");
    lives = Mod::get()->getSettingValue<int64_t>("lives");

    skips_left->setString(fmt::format("Skips: {}/{}", skips, Mod::get()->getSettingValue<int64_t>("skips")).c_str());
    lives_text->setString(std::to_string(lives).c_str());

    dl_txt->setVisible(true);
    startBtn->setVisible(false);
    loading_circle->show();
    current_level = 0;
    sharelevels.clear();
    downloadLevels();
}

void ExpertMapLayer::openDevs(CCObject*) {
    FLAlertLayer::create("Thank You!", "<cy>Thanks for downloading!</c>\n\n<cr>Note:</c> This mod is still in active development, we ask any bugs or suggestions be sent in the discord!\n\nThanks Again!\n\n<cg>From: Xanii & Adya</c> <cp><3</c>", "OK")->show();
}

void ExpertMapLayer::openDiscord(CCObject*) {
    geode::createQuickPopup("Join Discord", "Would you like to join the\n<cy>Official Discord Server?</c>", "No", "Yes", [] (auto fl, bool btn2) {
                if (btn2) geode::utils::web::openLinkInBrowser("https://discord.gg/W3BkznGTV8");});
}

void ExpertMapLayer::downloadLevels() {
    downloading = true;
    loading_circle->setVisible(true);
    loading_circle->show();
    dl_txt->setVisible(true);
    islandSprite->setColor({0,0,0});
    map_deco->setVisible(false);
    if (skips != Mod::get()->getSettingValue<int64_t>("skips")) skips_left->setString(fmt::format("Skips: {}/{}", skips, Mod::get()->getSettingValue<int64_t>("skips")).c_str());

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
    log::debug("level id success: {}", leveldata[1]);
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
        log::debug("downloading level error: {}", ex.what());
        downloadLevels();
    }

    islandSprite->setColor({255,255,255});
    islandSprite->setVisible(true);
    end_run_btn_menu->setVisible(true);
    map_deco->setVisible(true);
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
    lives = Mod::get()->getSettingValue<int64_t>("lives"); // keep this later i know im gonna remove it, its for actually resetting lives & skips when run is ended
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
            FMODAudioEngine::sharedEngine()->stopAllMusic();
            FMODAudioEngine::sharedEngine()->stopAllActions();
            FMODAudioEngine::sharedEngine()->stopAllEffects();
        }
        });
    else if (!super_expert) {
        CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
        FMODAudioEngine::sharedEngine()->stopAllMusic();
        FMODAudioEngine::sharedEngine()->stopAllActions();
        FMODAudioEngine::sharedEngine()->stopAllEffects();
    }
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
