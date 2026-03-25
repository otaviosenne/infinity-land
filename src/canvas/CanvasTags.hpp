#pragma once
#include "../defines.hpp"
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

class CCanvasTags {
  public:
    void        setTag(PHLWINDOW window, const std::string& colorName);
    std::string getTag(PHLWINDOW window) const;
    void        removeTag(PHLWINDOW window);

    nlohmann::json toJson() const;
    void           fromJson(const nlohmann::json& j);

    static uint32_t colorToRGBA(const std::string& name);

  private:
    std::string windowKey(PHLWINDOW window) const;

    std::unordered_map<std::string, std::string> m_tags;

    static const std::unordered_map<std::string, uint32_t> COLOR_PALETTE;
};

inline UP<CCanvasTags> g_pCanvasTags;
