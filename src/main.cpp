#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/CreatorLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/EndLevelLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

#include "ExpertMapLayer.hpp"

#include <string>

using namespace geode::prelude;

bool first_init = true;
extern int lives;
extern bool super_expert;
int extra_lives;
bool level_started = false;
bool downloading = false;
bool levelEnd = false;
int coin_lives = 0;

extern int skips;
extern int current_level;
extern int ids;

class $modify(PlayLayer) {

CCLabelBMFont* lives_text;
CCLabelBMFont* lives_text_x;
CCLabelBMFont* lives_bracket;

	bool init(GJGameLevel* level, bool first, bool second) {
		bool result = PlayLayer::init(level, first, second);
		// log::info("Player has {} lives. init", lives);
		if (super_expert) {
			if (!level_started) {
				level_started = true;
				extra_lives = 1;
			}

			// log::info("{}", extra_lives);

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
		return result;

	}

	void resetLevel() {
		coin_lives = 0;
		if (super_expert) {
			if (!levelEnd) {
				if (first_init) {
					first_init = false; // fat rob
				}
				else {
					if (lives >= 0) m_fields->lives_text->setString(std::to_string(lives).c_str());
				}
				// log::info("Player has {} lives. resetLevel", lives);
				lives--;
				
				if (lives + 2 <= 0) {
					super_expert = false;
					first_init = true;
					ExpertMapLayer::replaceScene();
				}
				else {
					PlayLayer::resetLevel();
				}
				// log::info("{}", extra_lives);

				if (level_started) {
					// log::info("level_started");
					}
				}
			else PlayLayer::onQuit();
			}
		else PlayLayer::resetLevel();
		} 

	void onQuit() {
		first_init = true;
		if (super_expert) {
			ExpertMapLayer::replaceScene();
		}
		else PlayLayer::onQuit();
	}
};

class $modify(LevelInfoLayer) {
	void onBack(CCObject* obj) {
		if (super_expert) {
			ExpertMapLayer::replaceScene();
			return;
		}

		LevelInfoLayer::onBack(obj);
	}

	void onPlay(CCObject* obj) {
		if (super_expert) {
			ExpertStartupLayer::scene(m_level);
			return;
		}
		LevelInfoLayer::onPlay(obj);
	}
};

class $modify(ExpertPauseLayer, PauseLayer) {
	void onPracticeMode(cocos2d::CCObject* sender) {
		if (super_expert) FLAlertLayer::create("Unavailable", "<cg>Practice Mode</c> isn't available during a <cp>Super Expert</c> run!", "OK")->show();
		else PauseLayer::onPracticeMode(sender);
	}

	void customSetup() {
        PauseLayer::customSetup();

		if (!super_expert) return;
        
        auto centerButtons = this->getChildByID("center-button-menu");
        CCMenuItemSpriteExtra* exitButton = reinterpret_cast<CCMenuItemSpriteExtra*>(centerButtons->getChildByID("exit-button"));

        CCSprite* skipsSprite = CCSprite::create("skipBtn.png"_spr);
        CCMenuItemSpriteExtra* skipsBtn = CCMenuItemSpriteExtra::create(skipsSprite, this, menu_selector(ExpertPauseLayer::skipLevel));

        skipsBtn->setID("xanii.super_expert/skip-button");
        skipsBtn->setPosition({-237.750f,-80});
        centerButtons->addChild(skipsBtn);
    }

	void skipLevel(CCObject* obj) {
		if (skips <= 0) {
			FLAlertLayer::create("SKIP", "You don't have any <cr>skips</c> left!", "OK")->show();
			return;
		}
		std::string plural = " ";
		if (skips > 1) plural = "s ";
		createQuickPopup("SKIP", fmt::format("Would you like to <cr>skip</c> this <cp>level</c>?\nYou have <cy>{} skip{}left</c>.", skips, plural), "NO", "YES", [this, obj](FLAlertLayer*, bool btn2) {
        if (btn2) {
            current_level++;
			if (current_level < 15) downloading = true;
			skips--;
			lives++; // compensation
			PauseLayer::onQuit(obj);
        }
        });
	}
};

class $modify(ExpertCallback, CreatorLayer) {

CCSprite* expertBtnSprite;
CCMenuItemSpriteExtra* expertButton;

	bool init() {
		bool result = CreatorLayer::init();
		auto director = CCDirector::sharedDirector();
		auto size = director->getWinSize();
		auto creatorButtons = this->getChildByID("creator-buttons-menu");
		if (!Loader::get()->getLoadedMod("capeling.goodbye_unnecessary_buttons")) {

			CCMenu* menu = CCMenu::create();
	
			CCMenuItemSpriteExtra* versusButton = reinterpret_cast<CCMenuItemSpriteExtra*>(creatorButtons->getChildByID("versus-button"));
			CCMenuItemSpriteExtra* questButton = reinterpret_cast<CCMenuItemSpriteExtra*>(creatorButtons->getChildByID("quests-button"));
	
			m_fields->expertBtnSprite = CCSprite::create("super_expert_btn.png"_spr);
			m_fields->expertBtnSprite->setScale(0.805);
	
			m_fields->expertButton = CCMenuItemSpriteExtra::create(m_fields->expertBtnSprite, this, menu_selector(ExpertCallback::onExpert));
	
			versusButton->setVisible(false);
	
			m_fields->expertButton->setPosition({questButton->getPositionX() + 92, questButton->getPositionY() - 2.4f});
			m_fields->expertButton->setID("super-expert-button"_spr);
			
		} else {
			CCSprite* emptyBtn = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
			m_fields->expertBtnSprite = CCSprite::create("super_expert_btn_mini.png"_spr);
			m_fields->expertBtnSprite->setPosition(emptyBtn->getContentSize() / 2);
			m_fields->expertBtnSprite->setScale(.625f);
			emptyBtn->addChild(m_fields->expertBtnSprite);
			emptyBtn->setID("super-expert-button"_spr);
			m_fields->expertButton = CCMenuItemSpriteExtra::create(emptyBtn, this, menu_selector(ExpertCallback::onExpert));
			m_fields->expertButton->setPosition((emptyBtn->getContentSize().width / 2) + 2.5f, (size.height / 2));
		}
		creatorButtons->addChild(m_fields->expertButton);

		return result;
	}

	void onExpert(CCObject*) {
        ExpertMapLayer::scene();
    }
};

class $modify(EndLevelLayer) {
	void showLayer(bool p0) { // find whatever gets called when u hit the end
		EndLevelLayer::showLayer(p0);
		levelEnd = true;
		PlayLayer* pl = PlayLayer::get(); // changed to make porting to mac easier :]
		if (super_expert) {
			log::info("coin lives: {}", coin_lives);
			level_started = false;
			lives += 1; // compensate for completion
			if (ids == pl->m_level->m_levelID) current_level++;
			if (current_level < 15) downloading = true;
			if (Mod::get()->getSettingValue<bool>("coin-lives")) lives += coin_lives;
			coin_lives = 0;
		}
	}
};

class $modify(GJBaseGameLayer) { // 1329 user coin pickup | 0x19d100 address make sure to submit l8r
	void pickupItem(EffectGameObject *p0) {
		if (super_expert) {
        	if (p0->m_objectID == 1329) coin_lives++;
		}
        GJBaseGameLayer::pickupItem(p0);
    }
};