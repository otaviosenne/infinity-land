#include "CanvasToolbar.hpp"
#include "CanvasViewport.hpp"
#include "../render/OpenGL.hpp"

#include <cairo/cairo.h>
#include <GLES3/gl32.h>
#include <array>
#include <cmath>

static const std::array<CHyprColor, 8> PALETTE_COLORS = {{
    CHyprColor(1.0, 0.3, 0.3, 1.0),
    CHyprColor(1.0, 0.6, 0.2, 1.0),
    CHyprColor(1.0, 0.9, 0.2, 1.0),
    CHyprColor(0.3, 0.8, 0.3, 1.0),
    CHyprColor(0.3, 0.5, 1.0, 1.0),
    CHyprColor(0.7, 0.3, 1.0, 1.0),
    CHyprColor(1.0, 1.0, 1.0, 1.0),
    CHyprColor(0.1, 0.1, 0.1, 1.0),
}};

static const std::array<float, 3> THICKNESS_OPTIONS = {2.0f, 4.0f, 8.0f};

static void cairoRoundedRect(cairo_t* cr, double x, double y, double w, double h, double r) {
    cairo_new_sub_path(cr);
    cairo_arc(cr, x + w - r, y + r, r, -M_PI / 2.0, 0);
    cairo_arc(cr, x + w - r, y + h - r, r, 0, M_PI / 2.0);
    cairo_arc(cr, x + r, y + h - r, r, M_PI / 2.0, M_PI);
    cairo_arc(cr, x + r, y + r, r, M_PI, 3.0 * M_PI / 2.0);
    cairo_close_path(cr);
}

void CCanvasToolbar::buildLayout(double monitorX, double monitorY, int monitorW, int monitorH) {
    m_buttons.clear();

    constexpr int TOOL_COUNT      = 2;
    constexpr int COLOR_COUNT     = 8;
    constexpr int THICKNESS_COUNT = 3;
    constexpr int TOTAL_BUTTONS   = TOOL_COUNT + COLOR_COUNT + THICKNESS_COUNT;

    const int totalWidth  = TOOLBAR_PADDING * 2 + TOTAL_BUTTONS * BUTTON_SIZE + (TOTAL_BUTTONS - 1) * BUTTON_GAP + 2 * SEPARATOR_GAP;
    const int totalHeight = TOOLBAR_PADDING * 2 + BUTTON_SIZE;

    const double toolbarX = monitorX + monitorW - TOOLBAR_MARGIN - totalWidth;
    const double toolbarY = monitorY + monitorH - TOOLBAR_MARGIN - totalHeight - 140;

    m_bounds = CBox{toolbarX, toolbarY, (double)totalWidth, (double)totalHeight};

    double curX = toolbarX + TOOLBAR_PADDING;
    const double btnY = toolbarY + TOOLBAR_PADDING;

    auto addButton = [&](eButtonType type, eDrawTool tool, CHyprColor color, float thickness) {
        SToolbarButton btn;
        btn.box       = CBox{curX, btnY, (double)BUTTON_SIZE, (double)BUTTON_SIZE};
        btn.type      = type;
        btn.tool      = tool;
        btn.color     = color;
        btn.thickness = thickness;
        m_buttons.push_back(btn);
        curX += BUTTON_SIZE + BUTTON_GAP;
    };

    addButton(TBTN_BRUSH, DRAW_BRUSH, {}, 0);
    addButton(TBTN_ERASER, DRAW_ERASER, {}, 0);

    curX += SEPARATOR_GAP - BUTTON_GAP;

    for (const auto& color : PALETTE_COLORS)
        addButton(TBTN_COLOR, DRAW_BRUSH, color, 0);

    curX += SEPARATOR_GAP - BUTTON_GAP;

    for (const auto& thick : THICKNESS_OPTIONS)
        addButton(TBTN_THICKNESS, DRAW_BRUSH, {}, thick);
}

static void renderButtonContent(cairo_t* cr, const SToolbarButton& btn, double localX, double localY, bool isActive) {
    cairoRoundedRect(cr, localX, localY, BUTTON_SIZE, BUTTON_SIZE, 4);

    switch (btn.type) {
        case TBTN_BRUSH:
            cairo_set_source_rgba(cr, 0.6, 0.6, 0.6, 1.0);
            cairo_fill_preserve(cr);
            break;
        case TBTN_ERASER:
            cairo_set_source_rgba(cr, 0.5, 0.4, 0.4, 1.0);
            cairo_fill_preserve(cr);
            break;
        case TBTN_COLOR:
            cairo_set_source_rgba(cr, btn.color.r, btn.color.g, btn.color.b, btn.color.a);
            cairo_fill_preserve(cr);
            break;
        case TBTN_THICKNESS:
            cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1.0);
            cairo_fill_preserve(cr);
            cairo_new_path(cr);
            cairo_set_source_rgba(cr, 0.9, 0.9, 0.9, 1.0);
            cairo_set_line_width(cr, btn.thickness);
            cairo_move_to(cr, localX + 6, localY + BUTTON_SIZE / 2.0);
            cairo_line_to(cr, localX + BUTTON_SIZE - 6, localY + BUTTON_SIZE / 2.0);
            cairo_stroke(cr);
            break;
    }

    cairo_new_path(cr);

    if (isActive) {
        cairoRoundedRect(cr, localX, localY, BUTTON_SIZE, BUTTON_SIZE, 4);
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
        cairo_set_line_width(cr, 2.0);
        cairo_stroke(cr);
    }
}

void CCanvasToolbar::render(PHLMONITOR pMonitor, const CRegion& damage) {
    if (!g_pCanvasDrawMode || !g_pCanvasDrawMode->isActive())
        return;

    const auto monitorW = pMonitor->m_transformedSize.x;
    const auto monitorH = pMonitor->m_transformedSize.y;

    buildLayout(pMonitor->m_position.x, pMonitor->m_position.y, monitorW, monitorH);

    const int texW = (int)m_bounds.w;
    const int texH = (int)m_bounds.h;

    auto* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, texW, texH);
    auto* cr      = cairo_create(surface);

    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    cairoRoundedRect(cr, 0, 0, texW, texH, 8);
    cairo_set_source_rgba(cr, 0.15, 0.15, 0.15, 0.85);
    cairo_fill(cr);

    const auto activeTool      = g_pCanvasDrawMode->activeTool();
    const auto activeColor     = g_pCanvasDrawMode->activeColor();
    const auto activeThickness = g_pCanvasDrawMode->activeThickness();

    int separatorIndex = 0;
    for (size_t i = 0; i < m_buttons.size(); ++i) {
        const auto& btn    = m_buttons[i];
        const double localX = btn.box.x - m_bounds.x;
        const double localY = btn.box.y - m_bounds.y;

        bool isActive = false;
        if (btn.type == TBTN_BRUSH || btn.type == TBTN_ERASER)
            isActive = (activeTool == btn.tool);
        else if (btn.type == TBTN_COLOR)
            isActive = (btn.color.r == activeColor.r && btn.color.g == activeColor.g && btn.color.b == activeColor.b);
        else if (btn.type == TBTN_THICKNESS)
            isActive = (btn.thickness == activeThickness);

        renderButtonContent(cr, btn, localX, localY, isActive);

        if ((i == 1 || i == 9) && separatorIndex < 2) {
            double sepX = localX + BUTTON_SIZE + (SEPARATOR_GAP + BUTTON_GAP) / 2.0;
            cairo_set_source_rgba(cr, 0.4, 0.4, 0.4, 0.5);
            cairo_set_line_width(cr, 1.0);
            cairo_move_to(cr, sepX, 4);
            cairo_line_to(cr, sepX, texH - 4);
            cairo_stroke(cr);
            ++separatorIndex;
        }
    }

    cairo_surface_flush(surface);

    if (!m_texture)
        m_texture = makeShared<CTexture>();

    m_texture->allocate();
    m_texture->m_size = {texW, texH};
    m_texture->bind();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_BLUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, cairo_image_surface_get_data(surface));

    m_texture->unbind();

    g_pHyprOpenGL->renderTexture(m_texture, m_bounds, {.a = 1.0f});

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}

bool CCanvasToolbar::handleClick(const Vector2D& screenPos) {
    if (!containsPoint(screenPos))
        return false;

    for (const auto& btn : m_buttons) {
        if (screenPos.x >= btn.box.x && screenPos.x <= btn.box.x + btn.box.w && screenPos.y >= btn.box.y && screenPos.y <= btn.box.y + btn.box.h) {

            switch (btn.type) {
                case TBTN_BRUSH:
                case TBTN_ERASER: g_pCanvasDrawMode->setTool(btn.tool); break;
                case TBTN_COLOR: g_pCanvasDrawMode->setColor(btn.color); break;
                case TBTN_THICKNESS: g_pCanvasDrawMode->setThickness(btn.thickness); break;
            }

            m_dirty = true;
            if (g_pCanvasViewport)
                g_pCanvasViewport->damageAllMonitors();
            return true;
        }
    }

    return true;
}

bool CCanvasToolbar::containsPoint(const Vector2D& screenPos) const {
    return screenPos.x >= m_bounds.x && screenPos.x <= m_bounds.x + m_bounds.w && screenPos.y >= m_bounds.y && screenPos.y <= m_bounds.y + m_bounds.h;
}
