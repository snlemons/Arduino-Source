/*  Alpha Music Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_AlphaMusicDetector_H
#define PokemonAutomation_PokemonLA_AlphaMusicDetector_H

#include <QImage>
#include "Common/Cpp/SpinLock.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/Inference/AudioPerSpectrumDetectorBase.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"

#include <memory>

namespace PokemonAutomation{

class ConsoleHandle;
class SpectrogramMatcher;

namespace NintendoSwitch{
namespace PokemonLA{


class AlphaMusicDetector : public AudioPerSpectrumDetectorBase{
public:
    AlphaMusicDetector(ConsoleHandle& console, OnShinyCallback on_shiny_callback);

    // Implement AudioPerSpectrumDetectorBase::get_score_threshold()
    virtual float get_score_threshold() const override;

protected:
    // Implement AudioPerSpectrumDetectorBase::build_spectrogram_matcher()
    virtual std::unique_ptr<SpectrogramMatcher> build_spectrogram_matcher(size_t sampleRate) override;
};



}
}
}
#endif