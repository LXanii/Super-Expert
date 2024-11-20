#include "ExpertMapLayer.hpp"
#include "ExpertStartPopup.hpp"
#include "ExpertManager.hpp"
#include "utils.hpp"

// thanks chatgpt
const std::vector<CCPoint> stageCoordinates = {
    {-247, -16}, {-212, -15}, {-183, -28}, {-150, -20}, {-116, -32}, {-82, -25},
    {-50, -17}, {-26, 3}, {7, 8}, {14, -18}, {36, -36},
    {69, -27}, {106, -30}, {142, -28}, {174, -16}
};

const std::vector<CCPoint> blankCoordinates = {
    {-231, -14}, {-198, -21}, {-167, -22}, {-135, -25},
    {-99, -27}, {-66, -20}, {-37, -7}, {-10, 7}, {9, -3},
    {23, -28}, {51, -30}, {87, -27}, {124, -28}, {159, -22}
};

void ExpertMapLayer::keyBackClicked() {
    this->onGoBack(nullptr);
}

void ExpertMapLayer::downloadLevel(CCObject* self) {
    if (ExpertManager::get().downloading) return;
    //log::info("{}", self->getTag());
    GameLevelManager::sharedState()->m_levelDownloadDelegate = this;
    GameLevelManager::sharedState()->downloadLevel(self->getTag(), true); // fuck you rob

    // put sfx here
    if (!Mod::get()->getSettingValue<bool>("level-info")) {
        FMODAudioEngine::sharedEngine()->stopAllMusic(false);
        FMODAudioEngine::sharedEngine()->stopAllActions();
        FMODAudioEngine::sharedEngine()->stopAllEffects();
        FMODAudioEngine::sharedEngine()->playMusic("playSound_01.ogg", false, 1, 0);
    }
}

void ExpertMapLayer::levelDownloadFinished(GJGameLevel* level) {
    // I am sorry for this code C++ sucks
    std::string unsplit = Mod::get()->getSaveContainer()["authors"][std::to_string(level->m_userID)].as<std::string>().unwrapOrDefault();
    log::info("it's {}", unsplit.size());
    if (unsplit.empty()) {
        log::error("Authors map was empty!");
    } else {
        std::vector<std::string> authorsplit = splitString(unsplit, ':');
        level->m_creatorName = authorsplit[1];
        level->m_accountID = stoi(authorsplit[2]);
    }
    if (Mod::get()->getSettingValue<bool>("level-info")) {
        CCDirector::get()->replaceScene(CCTransitionFade::create(0.5f, LevelInfoLayer::scene(level, false)));
        return;
    }
    auto layer = ExpertStartupLayer::create(level);
    auto scene = CCScene::create();
    scene->addChild(layer);
    CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, scene));
}

void ExpertMapLayer::levelDownloadFailed(int p0) {
    ExpertManager::get().downloading = false;
}

template <class T>
inline void setSavedDefault(const char* key, T const& val) {
    if (!Mod::get()->hasSavedValue(key)) Mod::get()->setSavedValue<T>(key, val);
}

bool ExpertMapLayer::init() { // beware, this code is dog shit holy fuck
    if (!CCLayer::init())
        return false;

    // setup lives and skips

    setSavedDefault<std::string>("share-levels", "");
    setSavedDefault<int64_t>("current-level", 0);
    setSavedDefault<bool>("currently-running", false);
    setSavedDefault<int64_t>("lives-left", Mod::get()->getSettingValue<int64_t>("lives"));
    setSavedDefault<int64_t>("skips-left", Mod::get()->getSettingValue<int64_t>("skips"));
    setSavedDefault<int64_t>("run-seed", 0);

    if (!Mod::get()->hasSavedValue("authors")) // i do this manually cuz i use save container here - ninXout
        Mod::get()->getSaveContainer()["authors"] = std::map<std::string, std::string>{{}, {}};

    int lives = Mod::get()->getSavedValue<int64_t>("lives-left");
    int skips = Mod::get()->getSavedValue<int64_t>("skips-left");
    int current_level = Mod::get()->getSavedValue<int64_t>("current-level");
    ExpertManager::get().running = Mod::get()->getSavedValue<bool>("currently-running");

    // now we get into the good part

    if (lives < 0) {
        expertReset();
        auto showGameOver = CCCallFunc::create(this, callfunc_selector(ExpertMapLayer::showGameOver));
        runAction(CCSequence::createWithTwoActions(CCDelayTime::create(0.2f), showGameOver));
    }

    if (Mod::get()->getSettingValue<bool>("map-music")) {
        FMODAudioEngine::sharedEngine()->stopAllMusic(false);
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

    if (!ExpertManager::get().running) islandSprite->setColor({ 0, 0, 0 });
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
    startBtn = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_longBtn03_001.png"), this, menu_selector(ExpertMapLayer::startExpertRun));

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

    if (lives >= 100) { // TODO: maybe use a CCMenu AxisLayout to have this properly positioned all the time
        player->setPosition({player->getPositionX() - 7, player->getPositionY()});
        lives_text->setPosition({lives_text->getPositionX() - 3, lives_text->getPositionY()});
        lives_text_x->setPosition({lives_text->getPositionX() - 23, lives_text->getPositionY()});
    }
    else lives_text_x->setPosition({lives_text->getPositionX() - 18, lives_text->getPositionY()}); // og pos

    start_btn_menu->setPosition({size.width / 2.f, size.height / 2.f - 30});

    islandSprite->setPosition({size.width / 2.f, size.height / 2.f}); // edit for map pos

    // xaniis deco factory

    map_deco = CCMenu::create();

    CCParticleFire* fire_1 = CCParticleFire::create(); // right eye
    CCParticleFire* fire_2 = CCParticleFire::create(); // left eye
    CCParticleFire* fire_3 = CCParticleFire::create(); // under map
    CCParticleRain* rain_1 = CCParticleRain::create();

    fire_1->setPosition({islandSprite->getPositionX() - 110, islandSprite->getPositionY() + 34});
    fire_2->setPosition({islandSprite->getPositionX() - 137, islandSprite->getPositionY() + 30});
    fire_3->setPosition({skips_left->getPositionX(), skips_left->getPositionY() - 157});

    //fire_1->setOpacity(100);
    fire_1->setScale(0.1);
    //fire_2->setOpacity(100);
    fire_2->setScale(0.1);
    fire_3->setScaleX(2.500);
    //rain_1->setOpacity(50);
    map_deco->addChild(fire_1, 1);
    map_deco->addChild(fire_2, 1);
    map_deco->setPosition({0,0});
    addChild(fire_3, 0);
    addChild(rain_1, 0);
    addChild(map_deco, 1);
    if (!ExpertManager::get().running) map_deco->setVisible(false);

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

    if (!ExpertManager::get().running) {
        end_run_btn_menu->setVisible(false);
        lvls_completed->setVisible(false);
        addChild(start_btn_menu);
        start_btn_menu->addChild(startBtn);
        startBtn->addChild(start_game_text);
        lvls_completed->setVisible(false);
    } else {
        if (Mod::get()->getSavedValue<int64_t>("run-seed") != 0 || ExpertManager::get().downloading) downloadLevels();
        else addMap();
        if (current_level == 15) {
            auto showCongrats = CCCallFunc::create(this, callfunc_selector(ExpertMapLayer::showCongrats));
            runAction(CCSequence::createWithTwoActions(CCDelayTime::create(0.3f), showCongrats));
            Mod::get()->setSavedValue<int64_t>("current-level", current_level + 1);
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
    FLAlertLayer::create("Level List", Mod::get()->getSavedValue<std::string>("share-levels"), "OK")->show();
}

void ExpertMapLayer::addMap() {

    CCSprite* stage_sprite;

    CCMenu* dotsmenu = CCMenu::create();

    CCTextureCache::sharedTextureCache()->addImage("WorldSheet.png", false);
    CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile("WorldSheet.plist");

    int current_level = Mod::get()->getSavedValue<int64_t>("current-level");

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
            stageBtn->setTag(ExpertManager::get().ids);
            dotsmenu->addChild(stageBtn);
        }
        else {
            stage_sprite = CCSprite::createWithSpriteFrameName("worldLevelBtn_locked_001.png");
            stage_sprite->setPosition(coord);
            stage_sprite->setScale(0.8);
            stage_sprite->setTag(ExpertManager::get().ids);
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

void ExpertMapLayer::confirmExpertRun() {
    int skips = Mod::get()->getSavedValue<int64_t>("skips-left");
    int lives = Mod::get()->getSavedValue<int64_t>("lives-left");

    skips_left->setString(fmt::format("Skips: {}/{}", skips, Mod::get()->getSettingValue<int64_t>("skips")).c_str());
    lives_text->setString(std::to_string(lives).c_str());

    dl_txt->setVisible(true);
    startBtn->setVisible(false);
    loading_circle->show();
    Mod::get()->setSavedValue<int64_t>("current-level", 0);
    Mod::get()->setSavedValue<std::string>("share-levels", "");
    downloadLevels();
}

void ExpertMapLayer::startExpertRun(CCObject*) {
    ExpertStartPopup::create(this)->show();
};

void ExpertMapLayer::openDevs(CCObject*) {
    FLAlertLayer::create("Thank You!", "<cy>Thanks for downloading!</c>\n\n<cr>Note:</c> This mod is still in active development, we ask any bugs or suggestions be sent in the discord!\n\nThanks Again!\n\n<cg>From: Xanii & Adya</c> <cp><3</c>", "OK")->show();
}

void ExpertMapLayer::openDiscord(CCObject*) {
    geode::createQuickPopup("Join Discord", "Would you like to join the\n<cy>Official Discord Server?</c>", "No", "Yes", [] (auto fl, bool btn2) {
                if (btn2) geode::utils::web::openLinkInBrowser("https://discord.gg/W3BkznGTV8");});
}

void ExpertMapLayer::downloadLevels() {
    int skips = Mod::get()->getSavedValue<int64_t>("skips-left");

    if (Mod::get()->getSavedValue<int64_t>("run-seed") == 0) 
        Mod::get()->setSavedValue<int64_t>("run-seed", rand() % 3040);
 
    ExpertManager::get().downloading = true;
    log::info("started downloading");
    loading_circle->setVisible(true);
    loading_circle->show();
    dl_txt->setVisible(true);
    islandSprite->setColor({0,0,0});
    map_deco->setVisible(false);
    if (skips != Mod::get()->getSettingValue<int64_t>("skips")) skips_left->setString(fmt::format("Skips: {}/{}", skips, Mod::get()->getSettingValue<int64_t>("skips")).c_str());

    log::info("did the other stuff");

    auto req = web::WebRequest();
    req.userAgent("");
    req.bodyString(fmt::format("diff=5&type=4&page={}&len=5&secret=Wmfd2893gb7", Mod::get()->getSavedValue<int64_t>("run-seed")));
    webreq.bind([this](web::WebTask::Event* e) { // dude fuck this new web request api holy shit
        if (web::WebResponse* res = e->getValue()) {
            std::string resultat = res->string().unwrap();
            ondownloadfinished(resultat);
        }
        else if (e->isCancelled()) {
            log::info("error");
        }
    });
    webreq.setFilter(req.post("http://www.boomlings.com/database/getGJLevels21.php"));
    log::info("finished the thing");
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
        log::info("{} and {}", authorsplit[0], split);

        Mod::get()->getSaveContainer()["authors"][authorsplit[0]] = split;
    }
    try {
        ExpertManager::get().ids = std::stoi(leveldata[1]);
        Mod::get()->setSavedValue<std::string>("share-levels", Mod::get()->getSavedValue<std::string>("share-levels") + fmt::format("{}\n", leveldata[1]));
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
    ExpertManager::get().running = true;
    Mod::get()->setSavedValue<bool>("currently-running", true);
    ExpertManager::get().downloading = false;
    ExpertManager::get().levelEnd = false;
    Mod::get()->setSettingValue<std::string>("run-id", "");
    addMap();
} 

void ExpertMapLayer::expertReset() {
    ExpertManager::get().running = false;
    Mod::get()->getSavedValue<bool>("currently-running", false);
    Mod::get()->getSaveContainer()["authors"] = std::map<std::string, std::string>{{}, {}};
    Mod::get()->setSavedValue<int64_t>("lives-left", Mod::get()->getSettingValue<int64_t>("lives"));
    Mod::get()->setSavedValue<int64_t>("skips-left", Mod::get()->getSettingValue<int64_t>("skips"));
    Mod::get()->setSavedValue<int64_t>("run-seed", 0);
}

void ExpertMapLayer::onGoBack(CCObject*) {
    if (ExpertManager::get().downloading) {
        FLAlertLayer::create("Invalid Action", "You can't leave while downloading!", "OK")->show();
        return;
    }
    Mod::get()->setSavedValue<bool>("currently-running", ExpertManager::get().running); // save the state
    ExpertManager::get().running = false; // stop playing super expert
    CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
    FMODAudioEngine::sharedEngine()->stopAllMusic(false);
    FMODAudioEngine::sharedEngine()->stopAllActions();
    FMODAudioEngine::sharedEngine()->stopAllEffects();
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

void ExpertMapLayer::replaceScene() {
    FMODAudioEngine::sharedEngine()->stopAllMusic(false);
    FMODAudioEngine::sharedEngine()->stopAllActions();
    FMODAudioEngine::sharedEngine()->stopAllEffects();

    auto scene = CCScene::create();
    scene->addChild(this);
    auto transition = CCTransitionFade::create(0.5f, scene);
    CCDirector::sharedDirector()->replaceScene(transition);
}
