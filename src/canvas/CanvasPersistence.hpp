#pragma once
#include "../defines.hpp"
#include <string>
#include <unordered_map>
#include <chrono>
#include <optional>
#include <vector>

struct SPersistedWindow {
    Vector2D    position;
    Vector2D    size;
    std::string appClass;
};

struct SPersistedAppDefault {
    Vector2D size;
};

class CCanvasPersistence {
  public:
    void load();
    void scheduleSave();

    std::optional<SPersistedWindow> findByClass(const std::string& appClass) const;
    SPersistedAppDefault            getAppDefault(const std::string& appClass) const;
    void                            trackWindow(const std::string& appClass, const Vector2D& position, const Vector2D& size);
    void                            removeWindow(const std::string& appClass, const Vector2D& position);

  private:
    void        saveNow();
    std::string configPath() const;

    std::vector<SPersistedWindow>                          m_windows;
    std::unordered_map<std::string, SPersistedAppDefault>  m_appDefaults;
    std::chrono::steady_clock::time_point                  m_lastSaveTime;

    static constexpr int     DEFAULT_WIDTH  = 1200;
    static constexpr int     DEFAULT_HEIGHT = 800;
    static constexpr int64_t DEBOUNCE_MS    = 1000;
};

inline UP<CCanvasPersistence> g_pCanvasPersistence;
