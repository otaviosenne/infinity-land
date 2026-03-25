#include "CanvasTags.hpp"
#include "../desktop/Window.hpp"
#include <format>

const std::unordered_map<std::string, uint32_t> CCanvasTags::COLOR_PALETTE = {
    {"red", 0xFF4444FF},    {"green", 0x44FF44FF},  {"blue", 0x4488FFFF},
    {"yellow", 0xFFFF44FF}, {"purple", 0xAA44FFFF}, {"orange", 0xFF8844FF},
};

std::string CCanvasTags::windowKey(PHLWINDOW window) const {
    return std::format("{}", (uintptr_t)window.get());
}

void CCanvasTags::setTag(PHLWINDOW window, const std::string& colorName) {
    if (!window)
        return;
    m_tags[windowKey(window)] = colorName;
}

std::string CCanvasTags::getTag(PHLWINDOW window) const {
    if (!window)
        return "";
    const auto it = m_tags.find(windowKey(window));
    return it != m_tags.end() ? it->second : "";
}

void CCanvasTags::removeTag(PHLWINDOW window) {
    if (!window)
        return;
    m_tags.erase(windowKey(window));
}

nlohmann::json CCanvasTags::toJson() const {
    return nlohmann::json(m_tags);
}

void CCanvasTags::fromJson(const nlohmann::json& j) {
    if (!j.is_object())
        return;
    m_tags.clear();
    for (const auto& [key, val] : j.items()) {
        if (val.is_string())
            m_tags[key] = val.get<std::string>();
    }
}

uint32_t CCanvasTags::colorToRGBA(const std::string& name) {
    const auto it = COLOR_PALETTE.find(name);
    return it != COLOR_PALETTE.end() ? it->second : 0xFFFFFFFF;
}
