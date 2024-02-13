#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/CreatorLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/EndLevelLayer.hpp>

#include "ExpertMapLayer.hpp"

#include <string>

using namespace geode::prelude;

bool first_init = true;
extern int lives;
extern bool super_expert;
int extra_lives;
bool level_started = false;

void resetLives() {
	lives = 30;
}

class $modify(PlayLayer) {

CCSprite* lives_img;
CCSprite* empty_live_1;
CCSprite* empty_live_2;

CCLabelBMFont* lives_text;
CCLabelBMFont* lives_text_x;
CCLabelBMFont* lives_bracket;

SimplePlayer* lives_1;
SimplePlayer* lives_2;

	bool init(GJGameLevel* level, bool first, bool second) {
		bool result = PlayLayer::init(level, first, second);
		log::info("Player has {} lives. init", lives);
		if (super_expert) {
			if (!level_started) {
				level_started = true;
				extra_lives = 2;
			}

			log::info("{}", extra_lives);

			m_fields->empty_live_1 = CCSprite::createWithSpriteFrameName("uiDot_001.png");
			m_fields->empty_live_1->setPosition({120,12});
			addChild(m_fields->empty_live_1, 10);
			m_fields->empty_live_1->setVisible(false);

			m_fields->empty_live_2 = CCSprite::createWithSpriteFrameName("uiDot_001.png");
			m_fields->empty_live_2->setPosition({93,12});
			addChild(m_fields->empty_live_2, 10);
			m_fields->empty_live_2->setVisible(false);

			m_fields->lives_text = CCLabelBMFont::create(std::to_string(lives + 1).c_str(), "gjFont59.fnt");
			m_fields->lives_text_x = CCLabelBMFont::create("x", "gjFont59.fnt");
			m_fields->lives_bracket = CCLabelBMFont::create("[            ]", "gjFont59.fnt");

			GameManager* manager = GameManager::sharedState();
    
			SimplePlayer* player = SimplePlayer::create(manager->getPlayerFrame());
			player->m_firstLayer->setColor(manager->colorForIdx(manager->getPlayerColor()));
			player->m_secondLayer->setColor(manager->colorForIdx(manager->getPlayerColor2()));
			player->updateColors();

			player->setPosition({11,12});
			player->setScale(0.65);
			player->setOpacity(200);
			addChild(player, 2);

			SimplePlayer* lives_1 = SimplePlayer::create(manager->getPlayerFrame());
			lives_1->m_firstLayer->setColor(manager->colorForIdx(manager->getPlayerColor()));
			lives_1->m_secondLayer->setColor(manager->colorForIdx(manager->getPlayerColor2()));
			lives_1->updateColors();

			lives_1->setPosition({93,12});
			lives_1->setScale(0.65);
			lives_1->setOpacity(200);
			this->addChild(lives_1, 2);
			
			SimplePlayer* lives_2 = SimplePlayer::create(manager->getPlayerFrame());
			lives_2->m_firstLayer->setColor(manager->colorForIdx(manager->getPlayerColor()));
			lives_2->m_secondLayer->setColor(manager->colorForIdx(manager->getPlayerColor2()));
			lives_2->updateColors();

			lives_2->setPosition({120,12});
			lives_2->setScale(0.65);
			lives_2->setOpacity(200);
			addChild(lives_2, 2);
			addChild(m_fields->empty_live_1);

			m_fields->lives_text->setScale(0.6);
			m_fields->lives_text->setOpacity(200);
			m_fields->lives_text->setPosition({player->getPositionX() + 35,player->getPositionY() - 1.8f});

			m_fields->lives_bracket->setScale(0.6);
			m_fields->lives_bracket->setOpacity(200);
			m_fields->lives_bracket->setPosition({m_fields->lives_text->getPositionX() + 60, m_fields->lives_text->getPositionY() + 1});

			m_fields->lives_text_x->setScale(0.6);
			m_fields->lives_text_x->setOpacity(200);
			m_fields->lives_text_x->setPosition({m_fields->lives_text->getPositionX() - 18, m_fields->lives_text->getPositionY()});

			addChild(m_fields->lives_text, 100);
			addChild(m_fields->lives_text_x, 100);
			addChild(m_fields->lives_bracket, 100);
			
		}
		return result;

	}

	void resetLevel() {
		PlayLayer::resetLevel();
		if (super_expert) {
			if (first_init) {
				first_init = false; // fat retard rob
			}
			else {
				m_fields->lives_text->setString(std::to_string(lives).c_str());
			}
			log::info("Player has {} lives. resetLevel", lives);
			lives--;
			log::info("{}", extra_lives);
		if (level_started) {

			SimplePlayer* live_1 = static_cast<SimplePlayer*>(getChildren()->objectAtIndex(8));
			SimplePlayer* live_2 = static_cast<SimplePlayer*>(getChildren()->objectAtIndex(7));
			if (extra_lives != 0) {
				extra_lives--;
				switch(extra_lives) {
					case 0:
						removeChild(live_2);
						m_fields->empty_live_2->setVisible(true);
						break;
					case 1:
						removeChild(live_1);
						m_fields->empty_live_1->setVisible(true);
						break;
				}
			}
		}	
	}
}

	void onQuit() {
		//resetLives(); // FOR TESTING REMOVE LATER WHEN DONE <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		first_init = true;
		log::info("quit true");
		if (super_expert) {
			//ExpertMapLayer::scene();
			PlayLayer::onQuit();
		} else {
			PlayLayer::onQuit();
		}
	}
};

class $modify(PauseLayer) {
	void onPracticeMode(cocos2d::CCObject* sender) {
		if (super_expert) {
			FLAlertLayer::create("Unavailable", "<cg>Practice Mode</c> isn't available during a <cp>Super Expert</c> run!", "OK")->show();
		}
		else {
			PauseLayer::onPracticeMode(sender);
		}
	}
};

class $modify(ExpertCallback, CreatorLayer) {

	bool init() {
		bool result = CreatorLayer::init();
		auto director = CCDirector::sharedDirector();
		auto size = director->getWinSize();
		auto creatorButtons = this->getChildByID("creator-buttons-menu");

		CCMenu* menu = CCMenu::create();

		CCMenuItemSpriteExtra* versusButton = reinterpret_cast<CCMenuItemSpriteExtra*>(creatorButtons->getChildByID("versus-button"));

		versusButton->setVisible(false);

		menu->setScale(0.8);
		menu->setPosition(versusButton->convertToWorldSpace(getPosition()));
		menu->setPosition({menu->getPositionX() - 12.9f, menu->getPositionY() + 7.5f});
		
		menu->addChild(CCMenuItemSpriteExtra::create(
            CCSprite::create("super_expert_btn.png"_spr),
            this, menu_selector(ExpertCallback::onExpert)));

		addChild(menu);
		return result;
	}

	void onExpert(CCObject*) {
        ExpertMapLayer::scene();
    }
};

class $modify(StartupCallback, LevelInfoLayer) {
	void onUpdate(cocos2d::CCObject* sender) {
		StartupCallback::onMap(m_level);
	}

	void onMap(CCObject*) {
		ExpertStartupLayer::scene(m_level);
	}
};

class $modify(EndLevelLayer) {
	void create() { // find whatever gets called when u hit the end
		EndLevelLayer::create();
		log::info("menu");
	}
};
