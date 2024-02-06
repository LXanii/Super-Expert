#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <string>

using namespace geode::prelude;

int lives = 30;
bool super_expert = true;
bool first_init = true;

void resetLives() {
	lives = 30;
}

class $modify(PlayLayer) {

CCSprite* lives_img;
CCLabelBMFont* lives_text;

	bool init(GJGameLevel* level, bool first, bool second) {
		bool result = PlayLayer::init(level, first, second);
		log::info("Player has {} lives. init", lives);
		if (super_expert) {

			m_fields->lives_text = CCLabelBMFont::create(std::to_string(lives + 1).c_str(), "gjFont59.fnt");

			m_fields->lives_img = CCSprite::create("lives_count.png"_spr);
			m_fields->lives_img->setScale(0.6);
			m_fields->lives_img->setOpacity(200);
			m_fields->lives_img->setPosition({17,12});

			m_fields->lives_text->setScale(0.6);
			m_fields->lives_text->setOpacity(200);
			m_fields->lives_text->setPosition({m_fields->lives_img->getPositionX() + 26,m_fields->lives_img->getPositionY() - 1.8f});

			addChild(m_fields->lives_img);
			addChild(m_fields->lives_text);
			
		}
		return result;
	}

	void resetLevel() {
		PlayLayer::resetLevel();
		if (first_init) {
			first_init = false; // fat retard rob
		}
		else {
			m_fields->lives_text->setString(std::to_string(lives).c_str());
		}
		log::info("Player has {} lives. resetLevel", lives);
		lives--;
	}

	void onQuit() {
		PlayLayer::onQuit();
		resetLives(); // FOR TESTING REMOVE LATER WHEN DONE <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		first_init = true;
		log::info("quit true");
	}
};

class $modify(PauseLayer) {
	void onPracticeMode(cocos2d::CCObject* sender) {
		if (super_expert) {
			FLAlertLayer::create("Sorry!", "<cg>Practice Mode</c> isn't available during a <cp>Super Expert</c> run!", "OK")->show();
		}
		else {
			onPracticeMode(sender);
		}
	}
};
