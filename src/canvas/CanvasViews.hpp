#pragma once
#include "../defines.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>

class CCanvasViews {
  public:
    void        createView(const std::string& name);
    void        switchView(const std::string& name);
    void        addToView(const std::string& viewName, PHLWINDOW window);
    void        removeFromView(const std::string& viewName, PHLWINDOW window);
    std::string activeView() const;
    bool        isWindowVisible(PHLWINDOW window) const;

    nlohmann::json toJson() const;
    void           fromJson(const nlohmann::json& j);

  private:
    void applyViewVisibility();

    std::unordered_map<std::string, std::vector<std::string>> m_views;
    std::string                                               m_activeView = "all";
};

inline UP<CCanvasViews> g_pCanvasViews;
