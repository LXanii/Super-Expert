#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class ExpertStartupLayer : public CCLayer {
protected:
    bool init(GJGameLevel*);

public:
    static ExpertStartupLayer* create(GJGameLevel* level);

    void loadLevel();

    GJGameLevel* m_level;
};