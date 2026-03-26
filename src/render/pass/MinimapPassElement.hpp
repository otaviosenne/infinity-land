#pragma once

#include "PassElement.hpp"
#include "../../defines.hpp"

struct SMinimapRect {
    CBox      box;
    CHyprColor color;
    int        round = 0;
};

class CMinimapPassElement : public IPassElement {
  public:
    struct SMinimapData {
        std::vector<SMinimapRect> rects;
        std::string               zoomText;
        Vector2D                  zoomTextPos;
    };

    CMinimapPassElement(const SMinimapData& data);

    void                draw(const CRegion& damage) override;
    bool                needsLiveBlur() override;
    bool                needsPrecomputeBlur() override;
    const char*         passName() override;
    bool                disableSimplification() override;

  private:
    SMinimapData m_data;
};
