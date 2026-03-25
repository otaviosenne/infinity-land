#include "CanvasViews.hpp"
#include "CanvasViewport.hpp"
#include "../desktop/Window.hpp"
#include "../Compositor.hpp"
#include "../layout/CanvasLayout.hpp"
#include "../managers/LayoutManager.hpp"

void CCanvasViews::createView(const std::string& name) {
    if (name.empty() || name == "all")
        return;

    auto& view = m_views[name];
    view.clear();

    const auto pLayout = dynamic_cast<CCanvasLayout*>(g_pLayoutManager->getCurrentLayout());
    if (!pLayout)
        return;

    for (const auto& ref : pLayout->windows()) {
        const auto w = ref.lock();
        if (w)
            view.push_back(w->m_initialClass);
    }
}

void CCanvasViews::switchView(const std::string& name) {
    if (name != "all" && m_views.find(name) == m_views.end())
        return;

    m_activeView = name;
    applyViewVisibility();
}

void CCanvasViews::addToView(const std::string& viewName, PHLWINDOW window) {
    if (!window || viewName == "all")
        return;

    auto it = m_views.find(viewName);
    if (it == m_views.end())
        return;

    it->second.push_back(window->m_initialClass);
}

void CCanvasViews::removeFromView(const std::string& viewName, PHLWINDOW window) {
    if (!window || viewName == "all")
        return;

    auto it = m_views.find(viewName);
    if (it == m_views.end())
        return;

    std::erase(it->second, window->m_initialClass);
}

std::string CCanvasViews::activeView() const {
    return m_activeView;
}

bool CCanvasViews::isWindowVisible(PHLWINDOW window) const {
    if (m_activeView == "all" || !window)
        return true;

    const auto it = m_views.find(m_activeView);
    if (it == m_views.end())
        return true;

    const auto& classes = it->second;
    return std::ranges::find(classes, window->m_initialClass) != classes.end();
}

void CCanvasViews::applyViewVisibility() {
    const auto pLayout = dynamic_cast<CCanvasLayout*>(g_pLayoutManager->getCurrentLayout());
    if (!pLayout)
        return;

    for (const auto& ref : pLayout->windows()) {
        const auto w = ref.lock();
        if (!w)
            continue;
        w->setHidden(!isWindowVisible(w));
    }

    if (g_pCanvasViewport)
        g_pCanvasViewport->damageAllMonitors();
}

nlohmann::json CCanvasViews::toJson() const {
    nlohmann::json j;
    j["active"] = m_activeView;
    j["views"]  = nlohmann::json(m_views);
    return j;
}

void CCanvasViews::fromJson(const nlohmann::json& j) {
    if (!j.is_object())
        return;

    m_activeView = j.value("active", "all");

    if (j.contains("views") && j["views"].is_object()) {
        m_views.clear();
        for (const auto& [key, val] : j["views"].items()) {
            if (!val.is_array())
                continue;
            for (const auto& cls : val)
                if (cls.is_string())
                    m_views[key].push_back(cls.get<std::string>());
        }
    }
}
