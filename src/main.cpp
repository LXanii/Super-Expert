#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/CreatorLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>

#include "ExpertMapLayer.hpp"

#include <string>

using namespace geode::prelude;

bool first_init = true;
extern int lives;
extern bool super_expert;

void resetLives() {
	lives = 30;
}

class $modify(PlayLayer) {

CCSprite* lives_img;
CCLabelBMFont* lives_text;
CCLabelBMFont* lives_text_x;

	bool init(GJGameLevel* level, bool first, bool second) {
		bool result = PlayLayer::init(level, first, second);
		log::info("Player has {} lives. init", lives);
		if (super_expert) {

			
			m_fields->lives_text = CCLabelBMFont::create(std::to_string(lives + 1).c_str(), "gjFont59.fnt");
			m_fields->lives_text_x = CCLabelBMFont::create("x", "gjFont59.fnt");

			GameManager* manager = GameManager::sharedState();
    
			SimplePlayer* player = SimplePlayer::create(manager->getPlayerFrame());
			player->m_firstLayer->setColor(manager->colorForIdx(manager->getPlayerColor()));
			player->m_secondLayer->setColor(manager->colorForIdx(manager->getPlayerColor2()));
			player->updateColors();

			player->setPosition({11,12});
			player->setScale(0.65);
			addChild(player, 2);

			m_fields->lives_text->setScale(0.6);
			m_fields->lives_text->setOpacity(200);
			m_fields->lives_text->setPosition({player->getPositionX() + 35,player->getPositionY() - 1.8f});

			m_fields->lives_text_x->setScale(0.6);
			m_fields->lives_text_x->setOpacity(200);
			m_fields->lives_text_x->setPosition({m_fields->lives_text->getPositionX() - 18, m_fields->lives_text->getPositionY()});

			addChild(m_fields->lives_text);
			addChild(m_fields->lives_text_x);
			
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
		}
	}

	void onQuit() {
		PlayLayer::onQuit();
		//resetLives(); // FOR TESTING REMOVE LATER WHEN DONE <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		first_init = true;
		log::info("quit true");
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
