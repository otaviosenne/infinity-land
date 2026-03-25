#pragma once
#include "../defines.hpp"
#include "../helpers/math/Math.hpp"
#include "../desktop/Window.hpp"
#include "CanvasAnnotation.hpp"

#include <string>
#include <variant>

class ICanvasCommand {
  public:
    virtual ~ICanvasCommand() = default;
    virtual void execute()    = 0;
    virtual void undo()       = 0;
};

using SAnnotationData = std::variant<SAnnotationStroke, SAnnotationArrow, SAnnotationStickyNote, SAnnotationText>;

class WindowMoveCommand : public ICanvasCommand {
  public:
    WindowMoveCommand(PHLWINDOWREF window, const Vector2D& oldPos, const Vector2D& newPos);
    void execute() override;
    void undo() override;

  private:
    PHLWINDOWREF m_window;
    Vector2D     m_oldPos;
    Vector2D     m_newPos;

    void applyPosition(const Vector2D& pos);
};

class WindowResizeCommand : public ICanvasCommand {
  public:
    WindowResizeCommand(PHLWINDOWREF window, const Vector2D& oldPos, const Vector2D& oldSize, const Vector2D& newPos, const Vector2D& newSize);
    void execute() override;
    void undo() override;

  private:
    PHLWINDOWREF m_window;
    Vector2D     m_oldPos;
    Vector2D     m_oldSize;
    Vector2D     m_newPos;
    Vector2D     m_newSize;

    void applyState(const Vector2D& pos, const Vector2D& size);
};

class AnnotationAddCommand : public ICanvasCommand {
  public:
    AnnotationAddCommand(const std::string& type, uint64_t id, SAnnotationData data);
    void execute() override;
    void undo() override;

  private:
    std::string     m_type;
    uint64_t        m_id;
    SAnnotationData m_data;

    void addAnnotation();
    void removeAnnotation();
};

class AnnotationRemoveCommand : public ICanvasCommand {
  public:
    AnnotationRemoveCommand(const std::string& type, uint64_t id, SAnnotationData data);
    void execute() override;
    void undo() override;

  private:
    std::string     m_type;
    uint64_t        m_id;
    SAnnotationData m_data;

    void addAnnotation();
    void removeAnnotation();
};

class AnnotationClearCommand : public ICanvasCommand {
  public:
    AnnotationClearCommand(std::vector<SAnnotationStroke> strokes, std::vector<SAnnotationArrow> arrows,
                           std::vector<SAnnotationStickyNote> notes, std::vector<SAnnotationText> texts);
    void execute() override;
    void undo() override;

  private:
    std::vector<SAnnotationStroke>     m_strokes;
    std::vector<SAnnotationArrow>      m_arrows;
    std::vector<SAnnotationStickyNote> m_notes;
    std::vector<SAnnotationText>       m_texts;
};
