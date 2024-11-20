#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/CreatorLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/EndLevelLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

#include "ExpertMapLayer.hpp"
#include "ExpertManager.hpp"

#include <string>

using namespace geode::prelude;

bool first_init = true;
int extra_lives;
bool level_started = false;
int coin_lives = 0;

class $modify(PlayLayer) {
	struct Fields {
        CCLabelBMFont* lives_text;
		CCLabelBMFont* lives_text_x;
		CCLabelBMFont* lives_bracket;
    };

	bool init(GJGameLevel* level, bool first, bool second) {
		if (!PlayLayer::init(level, first, second))
			return false;

		int lives = Mod::get()->getSavedValue<int64_t>("lives-left");
		if (ExpertManager::get().running) {
			if (!level_started) {
				level_started = true;
				extra_lives = 1;
			}

			//log::info("{}", extra_lives);

			m_fields->lives_text = CCLabelBMFont::create(std::to_string(lives + 1).c_str(), "gjFont59.fnt");
			m_fields->lives_text_x = CCLabelBMFont::create("x", "gjFont59.fnt");

			GameManager* manager = GameManager::sharedState();
    
			SimplePlayer* player = SimplePlayer::create(manager->getPlayerFrame());
			player->m_firstLayer->setColor(manager->colorForIdx(manager->getPlayerColor()));
			player->m_secondLayer->setColor(manager->colorForIdx(manager->getPlayerColor2()));
			player->setGlowOutline(manager->colorForIdx(manager->getPlayerGlowColor()));
			
			if (manager->getPlayerGlow()) player->enableCustomGlowColor(manager->colorForIdx(manager->getPlayerGlowColor()));
			else player->disableGlowOutline();
			
			player->updateColors();

			player->setPosition({11,12});
			player->setScale(0.65);
			player->setOpacity(200);
			addChild(player, 2);

			m_fields->lives_text->setScale(0.6);
			m_fields->lives_text->setOpacity(200);
			m_fields->lives_text->setPosition({player->getPositionX() + 35,player->getPositionY() - 1.8f});

			m_fields->lives_text_x->setScale(0.6);
			m_fields->lives_text_x->setOpacity(200);
			m_fields->lives_text_x->setPosition({m_fields->lives_text->getPositionX() - 18, m_fields->lives_text->getPositionY()});
			
			if (lives >= 99) m_fields->lives_text->setPosition({m_fields->lives_text->getPositionX() + 5, m_fields->lives_text->getPositionY()});
			log::info("{}", lives);
			addChild(m_fields->lives_text, 100);
			addChild(m_fields->lives_text_x, 100);
			
		}

		return true;
	}

	void resetLevel() {
		int lives = Mod::get()->getSavedValue<int64_t>("lives-left");

		coin_lives = 0;
		if (ExpertManager::get().running) {
			if (!ExpertManager::get().levelEnd) {
				if (first_init) {
					first_init = false; // fat retard rob
				} else {
					if (lives >= 0) m_fields->lives_text->setString(std::to_string(lives).c_str());
				}
				//log::info("Player has {} lives. resetLevel", lives);
				Mod::get()->setSavedValue<int64_t>("lives-left", lives - 1);
				
				if ((lives - 1) + 2 <= 0) {
					ExpertManager::get().running = false;
					first_init = true;
					ExpertMapLayer::create()->replaceScene();
				} else PlayLayer::resetLevel();

			} else PlayLayer::onQuit();
		} else PlayLayer::resetLevel();
	} 

	void onQuit() {
		first_init = true;
		if (ExpertManager::get().running) {
			ExpertMapLayer::create()->replaceScene();
		}
		else PlayLayer::onQuit();
	}
};

class $modify(LevelInfoLayer) {
	void onBack(CCObject* obj) {
		if (ExpertManager::get().running) {
			ExpertMapLayer::create()->replaceScene();
			return;
		}

		LevelInfoLayer::onBack(obj);
	}

	void onPlay(CCObject* obj) {
		if (ExpertManager::get().running) {
			auto layer = ExpertStartupLayer::create(m_level);
			auto scene = CCScene::create();
			scene->addChild(layer);
			CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5f, scene));
			return;
		}
		LevelInfoLayer::onPlay(obj);
	}
};

class $modify(ExpertPauseLayer, PauseLayer) {
	void onPracticeMode(cocos2d::CCObject* sender) {
		if (ExpertManager::get().running) FLAlertLayer::create("Unavailable", "<cg>Practice Mode</c> isn't available during a <cp>Super Expert</c> run!", "OK")->show();
		else PauseLayer::onPracticeMode(sender);
	}

	void customSetup() {
        PauseLayer::customSetup();

		if (!ExpertManager::get().running) return;
        
        auto centerButtons = this->getChildByID("center-button-menu");
        CCMenuItemSpriteExtra* exitButton = reinterpret_cast<CCMenuItemSpriteExtra*>(centerButtons->getChildByID("exit-button"));

        CCSprite* skipsSprite = CCSprite::create("skipBtn.png"_spr);
        CCMenuItemSpriteExtra* skipsBtn = CCMenuItemSpriteExtra::create(skipsSprite, this, menu_selector(ExpertPauseLayer::skipLevel));

        skipsBtn->setID("skip-button"_spr);
        skipsBtn->setPosition({-237.750f,-80});
        centerButtons->addChild(skipsBtn);
    }

	void skipLevel(CCObject* obj) {
		int lives = Mod::get()->getSavedValue<int64_t>("lives-left");
		int skips = Mod::get()->getSavedValue<int64_t>("skips-left");
		int current_level = Mod::get()->getSavedValue<int64_t>("current-level");

		if (skips <= 0) {
			FLAlertLayer::create("SKIP", "You don't have any <cr>skips</c> left!", "OK")->show();
			return;
		}

		createQuickPopup("SKIP", fmt::format("Would you like to <cr>skip</c> this <cp>level</c>?\nYou have <cy>{} skip{}left</c>.", skips, (skips > 1 ? "s " : " ")), "NO", "YES", [this, obj, lives, skips, current_level](FLAlertLayer*, bool btn2) {
			if (btn2) {
				Mod::get()->setSavedValue<int64_t>("current-level", current_level + 1);
				if ((current_level + 1) < 15) ExpertManager::get().downloading = true;
				Mod::get()->setSavedValue<int64_t>("skips-left", skips - 1);
				Mod::get()->setSavedValue<int64_t>("lives-left", lives + 1); // compensation
				PauseLayer::onQuit(obj);
			}
        });
	}
};

class $modify(ExpertCallback, CreatorLayer) {
	struct Fields {
		CCSprite* expertBtnSprite;
		CCMenuItemSpriteExtra* expertButton;
	};

	bool init() {
		if (!CreatorLayer::init())
			return false;

		auto director = CCDirector::sharedDirector();
		auto size = director->getWinSize();
		auto creatorButtons = this->getChildByID("creator-buttons-menu");

		CCMenu* menu = CCMenu::create();

		CCMenuItemSpriteExtra* versusButton = reinterpret_cast<CCMenuItemSpriteExtra*>(creatorButtons->getChildByID("versus-button"));
		CCMenuItemSpriteExtra* questButton = reinterpret_cast<CCMenuItemSpriteExtra*>(creatorButtons->getChildByID("quests-button"));

		m_fields->expertBtnSprite = CCSprite::create("super_expert_btn.png"_spr);
		m_fields->expertBtnSprite->setScale(0.805);

		m_fields->expertButton = CCMenuItemSpriteExtra::create(m_fields->expertBtnSprite, this, menu_selector(ExpertCallback::onExpert));
		versusButton->setVisible(false);

		if(Mod::get()->getSettingValue<bool>("show-versus-button") && Loader::get()->isModLoaded("alphalaneous.pages_api")) {
			versusButton->setVisible(true);
		}

		m_fields->expertButton->setPosition({questButton->getPositionX() + 92, questButton->getPositionY() - 2.4f});
		m_fields->expertButton->setID("super-expert-button");
		
		creatorButtons->addChild(m_fields->expertButton);

		return true;
	}

	void onExpert(CCObject*) {
        auto layer = ExpertMapLayer::create();
		auto scene = CCScene::create();
		scene->addChild(layer);
		auto transition = CCTransitionFade::create(0.5f, scene);
		CCDirector::sharedDirector()->pushScene(transition);
    }
};

class $modify(EndLevelLayer) {
	void showLayer(bool p0) { // find whatever gets called when u hit the end
		EndLevelLayer::showLayer(p0);

		int lives = Mod::get()->getSavedValue<int64_t>("lives-left");
		int current_level = Mod::get()->getSavedValue<int64_t>("current-level");
		PlayLayer* pl = PlayLayer::get(); // changed to make porting to mac easier :]

		ExpertManager::get().levelEnd = true;
		if (ExpertManager::get().running) {
			log::info("coin lives: {}", coin_lives);
			level_started = false;
			Mod::get()->setSavedValue<int64_t>("lives-left", lives + 1); // compensate for completion
			if (ExpertManager::get().ids == pl->m_level->m_levelID) Mod::get()->setSavedValue<int64_t>("current-level", current_level + 1);
			if ((current_level + 1) < 15) ExpertManager::get().downloading = true;
			if (Mod::get()->getSettingValue<bool>("coin-lives")) Mod::get()->setSavedValue<int64_t>("lives", Mod::get()->getSavedValue<int64_t>("lives") + coin_lives);
			coin_lives = 0;
		}
	}
};

class $modify(GJBaseGameLayer) { // 1329 user coin pickup | 0x19d100 address make sure to submit l8r
	void pickupItem(EffectGameObject *p0) {
		if (ExpertManager::get().running) {
        	if (p0->m_objectID == 1329) coin_lives++;
		}
        GJBaseGameLayer::pickupItem(p0);
    }
};
