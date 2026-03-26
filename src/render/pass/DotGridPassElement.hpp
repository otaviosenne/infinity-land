#pragma once

#include "PassElement.hpp"
#include "../../defines.hpp"

class CCanvasBackground;

class CDotGridPassElement : public IPassElement {
  public:
    struct SDotGridData {
        PHLMONITORREF monitor;
    };

    CDotGridPassElement(const SDotGridData& data);

    void                draw(const CRegion& damage) override;
    bool                needsLiveBlur() override;
    bool                needsPrecomputeBlur() override;
    const char*         passName() override;
    bool                disableSimplification() override;
    std::optional<CBox> boundingBox() override;

  private:
    SDotGridData m_data;
};
