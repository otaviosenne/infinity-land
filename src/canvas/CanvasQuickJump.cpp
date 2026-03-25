#include "CanvasQuickJump.hpp"
#include "CanvasViewport.hpp"
#include "../Compositor.hpp"
#include "../render/Renderer.hpp"
#include "../render/pass/RectPassElement.hpp"
#include "../render/pass/TexPassElement.hpp"
#include "../render/OpenGL.hpp"
#include <algorithm>

void CCanvasQuickJump::toggle() {
    m_active = !m_active;
    if (m_active) {
        m_query.clear();
        m_selectedIndex = 0;
        updateMatches();
    }
    g_pCanvasViewport->damageAllMonitors();
}

void CCanvasQuickJump::close() {
    m_active = false;
    g_pCanvasViewport->damageAllMonitors();
}

bool CCanvasQuickJump::isActive() const {
    return m_active;
}

int CCanvasQuickJump::fuzzyScore(const std::string& query, const std::string& target) const {
    if (query.empty())
        return 0;

    std::string lowerQuery  = query;
    std::string lowerTarget = target;
    std::ranges::transform(lowerQuery, lowerQuery.begin(), ::tolower);
    std::ranges::transform(lowerTarget, lowerTarget.begin(), ::tolower);

    const auto pos = lowerTarget.find(lowerQuery);
    if (pos == std::string::npos)
        return -1;

    if (pos == 0)
        return 100;

    return 50 - static_cast<int>(pos);
}

void CCanvasQuickJump::updateMatches() {
    m_matches.clear();

    for (const auto& w : g_pCompositor->m_windows) {
        if (w->isHidden() || !w->m_isMapped)
            continue;

        const auto& title    = w->m_title;
        const auto& appClass = w->m_class;

        if (m_query.empty()) {
            m_matches.push_back({w, title, appClass, 0});
            continue;
        }

        const int titleScore = fuzzyScore(m_query, title);
        const int classScore = fuzzyScore(m_query, appClass);
        const int bestScore  = std::max(titleScore, classScore);

        if (bestScore >= 0)
            m_matches.push_back({w, title, appClass, bestScore});
    }

    std::ranges::sort(m_matches, [](const auto& a, const auto& b) { return a.score > b.score; });

    if (m_matches.empty())
        m_selectedIndex = 0;
    else
        m_selectedIndex = std::clamp(m_selectedIndex, 0, static_cast<int>(m_matches.size()) - 1);
}

bool CCanvasQuickJump::handleKey(uint32_t keycode, bool pressed, char32_t character) {
    if (!m_active || !pressed)
        return false;

    constexpr uint32_t KEYCODE_ESCAPE    = 1;
    constexpr uint32_t KEYCODE_ENTER     = 28;
    constexpr uint32_t KEYCODE_ARROW_UP  = 103;
    constexpr uint32_t KEYCODE_ARROW_DN  = 108;
    constexpr uint32_t KEYCODE_BKSP      = 14;

    switch (keycode) {
        case KEYCODE_ESCAPE:
            close();
            return true;
        case KEYCODE_ENTER:
            jumpToSelected();
            close();
            return true;
        case KEYCODE_ARROW_UP:
            m_selectedIndex = std::max(0, m_selectedIndex - 1);
            g_pCanvasViewport->damageAllMonitors();
            return true;
        case KEYCODE_ARROW_DN:
            if (!m_matches.empty())
                m_selectedIndex = std::min(static_cast<int>(m_matches.size()) - 1, m_selectedIndex + 1);
            g_pCanvasViewport->damageAllMonitors();
            return true;
        case KEYCODE_BKSP:
            if (!m_query.empty()) {
                m_query.pop_back();
                updateMatches();
                g_pCanvasViewport->damageAllMonitors();
            }
            return true;
        default: break;
    }

    if (character >= 32 && character < 127) {
        m_query += static_cast<char>(character);
        updateMatches();
        g_pCanvasViewport->damageAllMonitors();
        return true;
    }

    return false;
}

void CCanvasQuickJump::jumpToSelected() {
    if (m_matches.empty())
        return;

    const auto w = m_matches[m_selectedIndex].window.lock();
    if (!w)
        return;

    const auto center = w->m_realPosition->goal() + w->m_realSize->goal() / 2.0;
    g_pCanvasViewport->setViewCenter(center);
}

void CCanvasQuickJump::render(PHLMONITOR pMonitor, const CRegion& damage) {
    if (!m_active)
        return;

    if (pMonitor != g_pCompositor->m_lastMonitor.lock())
        return;

    const double centerX   = pMonitor->m_position.x + (pMonitor->m_size.x - OVERLAY_WIDTH) / 2.0;
    const double topY      = pMonitor->m_position.y + pMonitor->m_size.y / 4.0;
    const int    matchCount = std::min(static_cast<int>(m_matches.size()), MAX_MATCHES);
    const double totalH    = OVERLAY_HEIGHT + matchCount * MATCH_HEIGHT;

    CRectPassElement::SRectData bgData;
    bgData.box   = CBox{centerX, topY, OVERLAY_WIDTH, totalH};
    bgData.color = CHyprColor(0.1, 0.1, 0.15, 0.9);
    bgData.round = 8;
    g_pHyprRenderer->m_renderPass.add(makeUnique<CRectPassElement>(bgData));

    CRectPassElement::SRectData inputBg;
    inputBg.box   = CBox{centerX, topY, OVERLAY_WIDTH, OVERLAY_HEIGHT};
    inputBg.color = CHyprColor(0.15, 0.15, 0.2, 0.95);
    inputBg.round = 8;
    g_pHyprRenderer->m_renderPass.add(makeUnique<CRectPassElement>(inputBg));

    const std::string displayText = m_query.empty() ? "Search windows..." : m_query;
    const auto        textColor   = m_query.empty() ? CHyprColor(0.5, 0.5, 0.5, 0.8) : CHyprColor(1.0, 1.0, 1.0, 1.0);
    auto              queryTex    = g_pHyprOpenGL->renderText(displayText, textColor, 14, false, "", OVERLAY_WIDTH - 20);

    if (queryTex) {
        CTexPassElement::SRenderData texData;
        texData.tex = queryTex;
        texData.box = CBox{centerX + 10, topY + (OVERLAY_HEIGHT - queryTex->m_size.y) / 2.0, queryTex->m_size.x, queryTex->m_size.y};
        texData.a   = 1.0f;
        g_pHyprRenderer->m_renderPass.add(makeUnique<CTexPassElement>(texData));
    }

    for (int i = 0; i < matchCount; ++i) {
        const double rowY      = topY + OVERLAY_HEIGHT + i * MATCH_HEIGHT;
        const bool   selected  = (i == m_selectedIndex);

        if (selected) {
            CRectPassElement::SRectData selBg;
            selBg.box   = CBox{centerX, rowY, OVERLAY_WIDTH, MATCH_HEIGHT};
            selBg.color = CHyprColor(0.3, 0.4, 0.7, 0.6);
            selBg.round = (i == matchCount - 1) ? 8 : 0;
            g_pHyprRenderer->m_renderPass.add(makeUnique<CRectPassElement>(selBg));
        }

        const auto& match   = m_matches[i];
        const auto  label   = match.title + " (" + match.appClass + ")";
        auto        rowTex  = g_pHyprOpenGL->renderText(label, CHyprColor(0.9, 0.9, 0.9, 1.0), 12, false, "", OVERLAY_WIDTH - 20);

        if (rowTex) {
            CTexPassElement::SRenderData texData;
            texData.tex = rowTex;
            texData.box = CBox{centerX + 10, rowY + (MATCH_HEIGHT - rowTex->m_size.y) / 2.0, rowTex->m_size.x, rowTex->m_size.y};
            texData.a   = 1.0f;
            g_pHyprRenderer->m_renderPass.add(makeUnique<CTexPassElement>(texData));
        }
    }
}
