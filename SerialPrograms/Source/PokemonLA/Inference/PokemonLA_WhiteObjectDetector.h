/*  White Object Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_WhiteObjectDetector_H
#define PokemonAutomation_PokemonLA_WhiteObjectDetector_H

#include <vector>
#include <list>
#include <set>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Tools/VideoOverlaySet.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{
    class WaterfillObject;
}
}
namespace NintendoSwitch{
namespace PokemonLA{



class WhiteObjectDetector{
protected:
    using WaterfillObject = Kernels::Waterfill::WaterfillObject;

public:
    virtual ~WhiteObjectDetector() = default;
    WhiteObjectDetector(Color color, std::set<Color> thresholds)
        : m_color(color)
        , m_thresholds(std::move(thresholds))
    {}

    Color color() const{ return m_color; }
    const std::set<Color>& thresholds() const{ return m_thresholds; }

    const std::vector<ImagePixelBox>& detections() const{ return m_detections; }
    void clear(){ m_detections.clear(); }

    virtual void process_object(const QImage& screen, const WaterfillObject& object) = 0;
    virtual void finish(){}

protected:
    void merge_heavily_overlapping(double tolerance = 0.2);

protected:
    Color m_color;
    std::set<Color> m_thresholds;
    std::vector<ImagePixelBox> m_detections;
};






void find_overworld_white_objects(
    const std::vector<std::pair<WhiteObjectDetector&, bool>>& detectors,
    const QImage& screen
);

class WhiteObjectWatcher : public VisualInferenceCallback{
public:
    WhiteObjectWatcher(
        VideoOverlay& overlay,
        std::vector<std::pair<WhiteObjectDetector&, bool>> detectors
    );

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  Return true if the inference session should stop.
    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override;


private:
    ImageFloatBox m_box;
    VideoOverlaySet m_overlays;

    std::vector<std::pair<WhiteObjectDetector&, bool>> m_detectors;
};






}
}
}
#endif