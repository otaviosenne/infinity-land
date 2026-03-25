#include "CanvasPersistence.hpp"
#include "CanvasAnnotation.hpp"
#include "CanvasTags.hpp"
#include "CanvasViews.hpp"
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

        if (data.contains("tags") && g_pCanvasTags)
            g_pCanvasTags->fromJson(data["tags"]);

        if (data.contains("views") && g_pCanvasViews)
            g_pCanvasViews->fromJson(data["views"]);

        if (data.contains("annotations") && g_pCanvasAnnotation) {
            const auto& ann = data["annotations"];

            if (ann.contains("strokes") && ann["strokes"].is_array()) {
                for (const auto& s : ann["strokes"]) {
                    SAnnotationStroke stroke;
                    if (s.contains("points") && s["points"].is_array()) {
                        for (const auto& p : s["points"])
                            stroke.points.push_back(Vector2D{p[0].get<double>(), p[1].get<double>()});
                    }
                    if (s.contains("color") && s["color"].is_array()) {
                        const auto& c = s["color"];
                        stroke.color = CHyprColor(c[0].get<double>(), c[1].get<double>(), c[2].get<double>(), c[3].get<double>());
                    }
                    stroke.thickness = s.value("thickness", 4.0f);
                    g_pCanvasAnnotation->addStroke(std::move(stroke));
                }
            }

            if (ann.contains("arrows") && ann["arrows"].is_array()) {
                for (const auto& a : ann["arrows"]) {
                    SAnnotationArrow arrow;
                    if (a.contains("start") && a["start"].is_array())
                        arrow.start = Vector2D{a["start"][0].get<double>(), a["start"][1].get<double>()};
                    if (a.contains("end") && a["end"].is_array())
                        arrow.end = Vector2D{a["end"][0].get<double>(), a["end"][1].get<double>()};
                    if (a.contains("color") && a["color"].is_array()) {
                        const auto& c = a["color"];
                        arrow.color = CHyprColor(c[0].get<double>(), c[1].get<double>(), c[2].get<double>(), c[3].get<double>());
                    }
                    arrow.thickness = a.value("thickness", 4.0f);
                    g_pCanvasAnnotation->addArrow(std::move(arrow));
                }
            }

            if (ann.contains("sticky_notes") && ann["sticky_notes"].is_array()) {
                for (const auto& n : ann["sticky_notes"]) {
                    SAnnotationStickyNote note;
                    if (n.contains("position") && n["position"].is_array())
                        note.position = Vector2D{n["position"][0].get<double>(), n["position"][1].get<double>()};
                    if (n.contains("size") && n["size"].is_array())
                        note.size = Vector2D{n["size"][0].get<double>(), n["size"][1].get<double>()};
                    note.text = n.value("text", "");
                    if (n.contains("color") && n["color"].is_array()) {
                        const auto& c = n["color"];
                        note.color = CHyprColor(c[0].get<double>(), c[1].get<double>(), c[2].get<double>(), c[3].get<double>());
                    }
                    g_pCanvasAnnotation->addStickyNote(std::move(note));
                }
            }

            if (ann.contains("texts") && ann["texts"].is_array()) {
                for (const auto& t : ann["texts"]) {
                    SAnnotationText text;
                    if (t.contains("position") && t["position"].is_array())
                        text.position = Vector2D{t["position"][0].get<double>(), t["position"][1].get<double>()};
                    text.text     = t.value("text", "");
                    text.fontSize = t.value("fontSize", 16.0f);
                    if (t.contains("color") && t["color"].is_array()) {
                        const auto& c = t["color"];
                        text.color = CHyprColor(c[0].get<double>(), c[1].get<double>(), c[2].get<double>(), c[3].get<double>());
                    }
                    g_pCanvasAnnotation->addText(std::move(text));
                }
            }

            g_pCanvasAnnotation->markDirty();
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

    if (g_pCanvasTags)
        data["tags"] = g_pCanvasTags->toJson();

    if (g_pCanvasViews)
        data["views"] = g_pCanvasViews->toJson();

    if (g_pCanvasAnnotation) {
        json annotations;

        json strokesArray = json::array();
        for (const auto& s : g_pCanvasAnnotation->strokes()) {
            json pts = json::array();
            for (const auto& p : s.points)
                pts.push_back({p.x, p.y});
            strokesArray.push_back({
                {"points",    pts},
                {"color",     {s.color.r, s.color.g, s.color.b, s.color.a}},
                {"thickness", s.thickness},
            });
        }
        annotations["strokes"] = strokesArray;

        json arrowsArray = json::array();
        for (const auto& a : g_pCanvasAnnotation->arrows()) {
            arrowsArray.push_back({
                {"start",     {a.start.x, a.start.y}},
                {"end",       {a.end.x, a.end.y}},
                {"color",     {a.color.r, a.color.g, a.color.b, a.color.a}},
                {"thickness", a.thickness},
            });
        }
        annotations["arrows"] = arrowsArray;

        json notesArray = json::array();
        for (const auto& n : g_pCanvasAnnotation->stickyNotes()) {
            notesArray.push_back({
                {"position", {n.position.x, n.position.y}},
                {"size",     {n.size.x, n.size.y}},
                {"text",     n.text},
                {"color",    {n.color.r, n.color.g, n.color.b, n.color.a}},
            });
        }
        annotations["sticky_notes"] = notesArray;

        json textsArray = json::array();
        for (const auto& t : g_pCanvasAnnotation->texts()) {
            textsArray.push_back({
                {"position", {t.position.x, t.position.y}},
                {"text",     t.text},
                {"color",    {t.color.r, t.color.g, t.color.b, t.color.a}},
                {"fontSize", t.fontSize},
            });
        }
        annotations["texts"] = textsArray;

        data["annotations"] = annotations;
    }

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
