#pragma once
#include "../defines.hpp"
#include "../helpers/Monitor.hpp"
#include "../helpers/Color.hpp"
#include "../render/Texture.hpp"

#include <string>

struct SAnnotationStroke {
    std::vector<Vector2D> points;
    CHyprColor            color     = CHyprColor(1.0, 0.3, 0.3, 1.0);
    float                 thickness = 4.0f;
    uint64_t              id        = 0;
};

struct SAnnotationArrow {
    Vector2D   start;
    Vector2D   end;
    CHyprColor color     = CHyprColor(1.0, 0.3, 0.3, 1.0);
    float      thickness = 4.0f;
    uint64_t   id        = 0;
};

struct SAnnotationStickyNote {
    Vector2D    position;
    Vector2D    size = Vector2D{200, 150};
    std::string text;
    CHyprColor  color = CHyprColor(1.0, 0.95, 0.6, 1.0);
    uint64_t    id    = 0;
};

struct SAnnotationText {
    Vector2D    position;
    std::string text;
    CHyprColor  color    = CHyprColor(1.0, 1.0, 1.0, 1.0);
    float       fontSize = 16.0f;
    uint64_t    id       = 0;
};

class CCanvasAnnotation {
  public:
    void                                  addStroke(SAnnotationStroke stroke);
    void                                  removeStroke(uint64_t id);
    SAnnotationStroke*                    findNearestStroke(const Vector2D& canvasPos, double threshold = 20.0);
    void                                  render(PHLMONITOR pMonitor, const CRegion& damage);
    void                                  markDirty();
    const std::vector<SAnnotationStroke>& strokes() const;

    void                                       addArrow(SAnnotationArrow arrow);
    void                                       removeArrow(uint64_t id);
    SAnnotationArrow*                          findNearestArrow(const Vector2D& canvasPos, double threshold = 20.0);
    const std::vector<SAnnotationArrow>&       arrows() const;

    void                                       addStickyNote(SAnnotationStickyNote note);
    void                                       removeStickyNote(uint64_t id);
    SAnnotationStickyNote*                     findStickyNoteAt(const Vector2D& canvasPos);
    const std::vector<SAnnotationStickyNote>&  stickyNotes() const;

    void                                       addText(SAnnotationText text);
    void                                       removeText(uint64_t id);
    SAnnotationText*                           findTextAt(const Vector2D& canvasPos, double threshold = 20.0);
    const std::vector<SAnnotationText>&        texts() const;

    void                                       clearAll();
    void                                       clearByType(const std::string& type);

  private:
    std::vector<SAnnotationStroke>    m_strokes;
    std::vector<SAnnotationArrow>     m_arrows;
    std::vector<SAnnotationStickyNote> m_stickyNotes;
    std::vector<SAnnotationText>      m_texts;
    SP<CTexture>                      m_texture;
    bool                              m_dirty      = true;
    uint64_t                          m_nextId     = 1;
    int                               m_lastWidth  = 0;
    int                               m_lastHeight = 0;

    double distanceToSegment(const Vector2D& point, const Vector2D& segA, const Vector2D& segB) const;
};

inline UP<CCanvasAnnotation> g_pCanvasAnnotation;
