/*  Raid Catch Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_RaidCatchDetector_H
#define PokemonAutomation_PokemonSwSh_RaidCatchDetector_H

#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Inference/VisualInferenceCallback.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class RaidCatchDetector : public VisualInferenceCallback{
public:
    RaidCatchDetector(VideoOverlay& overlay);

    bool detect(const QImage& screen);

    virtual bool process_frame(
        const QImage& frame,
        std::chrono::system_clock::time_point timestamp
    ) override final;


private:
    ImageFloatBox m_left0;
    ImageFloatBox m_right0;
//    ImageFloatBox m_left1;
//    ImageFloatBox m_right1;
    ImageFloatBox m_text0;
    ImageFloatBox m_text1;
    SelectionArrowFinder m_arrow;

    size_t m_trigger_count = 0;
};


}
}
}
#endif
