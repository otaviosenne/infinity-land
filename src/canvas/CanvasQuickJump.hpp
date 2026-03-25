#pragma once
#include "../defines.hpp"
#include "../helpers/Monitor.hpp"

struct SQuickJumpMatch {
    PHLWINDOWREF window;
    std::string  title;
    std::string  appClass;
    int          score;
};

class CCanvasQuickJump {
  public:
    void render(PHLMONITOR pMonitor, const CRegion& damage);
    void toggle();
    void close();
    bool isActive() const;
    bool handleKey(uint32_t keycode, bool pressed, char32_t character = 0);

  private:
    bool                        m_active        = false;
    std::string                 m_query;
    int                         m_selectedIndex = 0;
    std::vector<SQuickJumpMatch> m_matches;

    void updateMatches();
    int  fuzzyScore(const std::string& query, const std::string& target) const;
    void jumpToSelected();

    static constexpr int OVERLAY_WIDTH  = 400;
    static constexpr int OVERLAY_HEIGHT = 40;
    static constexpr int MATCH_HEIGHT   = 32;
    static constexpr int MAX_MATCHES    = 5;
};

inline UP<CCanvasQuickJump> g_pCanvasQuickJump;
