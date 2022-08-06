/*  Encounter Dialog Tracker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EncounterDialogTracker_H
#define PokemonAutomation_PokemonSwSh_EncounterDialogTracker_H

#include <chrono>
#include <atomic>
#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/Inference/VisualDetector.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

enum class EncounterState{
    BEFORE_ANYTHING,
    WILD_ANIMATION,
    YOUR_ANIMATION,
    POST_ENTRY,
};


class EncounterDialogTracker : public VisualInferenceCallback{
public:
    EncounterDialogTracker(
        Logger& logger,
        StaticScreenDetector& dialog_detector
    );

    //  Thread-safe
    bool dialog_on() const{ return m_dialog_on.load(std::memory_order_acquire); }
    EncounterState encounter_state() const{ return m_state.load(std::memory_order_acquire); }

    //  Not thread-safe
    std::chrono::milliseconds wild_animation_duration() const{ return m_wild_animation_duration; }
    std::chrono::milliseconds your_animation_duration() const{ return m_your_animation_duration; }

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;
    void push_end(WallClock timestamp = current_time());

private:
    Logger& m_logger;
    StaticScreenDetector& m_dialog_detector;
    WallClock m_end_dialog;

    std::atomic<bool> m_dialog_on;
    std::atomic<EncounterState> m_state;

    std::chrono::milliseconds m_wild_animation_duration;
    std::chrono::milliseconds m_your_animation_duration;
};


}
}
}
#endif
