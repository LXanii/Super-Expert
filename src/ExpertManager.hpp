#pragma once

class ExpertManager {
public:
    static ExpertManager& get() {
        static ExpertManager instance;
        return instance;
    }
    
    bool running = false;
    bool downloading = false;
    int ids = 0;
    bool levelEnd = false;

protected:
    ExpertManager() {};
};