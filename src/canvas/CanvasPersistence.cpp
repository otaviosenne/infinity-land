#include "CanvasPersistence.hpp"
#include "../debug/Log.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <cmath>

using json = nlohmann::json;

static constexpr double POSITION_MATCH_THRESHOLD = 1.0;

std::string CCanvasPersistence::configPath() const {
    const auto home = std::getenv("HOME");
    const auto dir  = std::string(home ? home : "/tmp") + "/.config/infinity-land";
    std::filesystem::create_directories(dir);
    return dir + "/canvas-state.json";
}

void CCanvasPersistence::load() {
    const auto path = configPath();

    try {
        std::ifstream file(path);
        if (!file.is_open())
            return;

        const auto data = json::parse(file);

        if (data.contains("windows") && data["windows"].is_array()) {
            for (const auto& w : data["windows"]) {
                m_windows.push_back(SPersistedWindow{
                    .position = Vector2D{w.value("x", 0.0), w.value("y", 0.0)},
                    .size     = Vector2D{w.value("w", 0.0), w.value("h", 0.0)},
                    .appClass = w.value("class", ""),
                });
            }
        }

        if (data.contains("app_defaults") && data["app_defaults"].is_object()) {
            for (const auto& [key, val] : data["app_defaults"].items()) {
                m_appDefaults[key] = SPersistedAppDefault{
                    .size = Vector2D{val.value("w", 0.0), val.value("h", 0.0)},
                };
            }
        }

        Debug::log(LOG, "CanvasPersistence: loaded {} windows, {} app defaults", m_windows.size(), m_appDefaults.size());
    } catch (const std::exception& e) {
        Debug::log(WARN, "CanvasPersistence: failed to load state: {}", e.what());
        m_windows.clear();
        m_appDefaults.clear();
    }
}

void CCanvasPersistence::saveNow() {
    json data;

    json windowsArray = json::array();
    for (const auto& w : m_windows) {
        windowsArray.push_back({
            {"class", w.appClass},
            {"x",     w.position.x},
            {"y",     w.position.y},
            {"w",     w.size.x},
            {"h",     w.size.y},
        });
    }
    data["windows"] = windowsArray;

    json defaults = json::object();
    for (const auto& [key, val] : m_appDefaults) {
        defaults[key] = {{"w", val.size.x}, {"h", val.size.y}};
    }
    data["app_defaults"] = defaults;

    const auto path    = configPath();
    const auto tmpPath = path + ".tmp";

    std::ofstream file(tmpPath);
    file << data.dump(2);
    file.close();

    std::filesystem::rename(tmpPath, path);

    m_lastSaveTime = std::chrono::steady_clock::now();
}

void CCanvasPersistence::scheduleSave() {
    const auto now     = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastSaveTime).count();

    if (elapsed < DEBOUNCE_MS)
        return;

    saveNow();
}

std::optional<SPersistedWindow> CCanvasPersistence::findByClass(const std::string& appClass) const {
    for (const auto& w : m_windows) {
        if (w.appClass == appClass)
            return w;
    }
    return std::nullopt;
}

SPersistedAppDefault CCanvasPersistence::getAppDefault(const std::string& appClass) const {
    const auto it = m_appDefaults.find(appClass);
    if (it != m_appDefaults.end())
        return it->second;
    return SPersistedAppDefault{.size = Vector2D{DEFAULT_WIDTH, DEFAULT_HEIGHT}};
}

void CCanvasPersistence::trackWindow(const std::string& appClass, const Vector2D& position, const Vector2D& size) {
    for (auto& w : m_windows) {
        if (w.appClass == appClass) {
            w.position = position;
            w.size     = size;
            m_appDefaults[appClass] = SPersistedAppDefault{.size = size};
            return;
        }
    }

    m_windows.push_back(SPersistedWindow{.position = position, .size = size, .appClass = appClass});
    m_appDefaults[appClass] = SPersistedAppDefault{.size = size};
}

void CCanvasPersistence::removeWindow(const std::string& appClass, const Vector2D& position) {
    std::erase_if(m_windows, [&](const SPersistedWindow& w) {
        return w.appClass == appClass && std::abs(w.position.x - position.x) < POSITION_MATCH_THRESHOLD &&
            std::abs(w.position.y - position.y) < POSITION_MATCH_THRESHOLD;
    });
}
