/*  Shiny Sound Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QString>
#include "CommonFramework/Inference/SpectrogramMatcher.h"
#include "CommonFramework/Inference/AudioTemplateCache.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/AudioFeed.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA_ShinySoundDetector.h"

#include <sstream>
#include <cfloat>
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



ShinySoundDetector::~ShinySoundDetector(){}
ShinySoundDetector::ShinySoundDetector(ConsoleHandle& console, bool stop_on_detected)
    : AudioInferenceCallback("ShinySoundDetector")
    , m_console(console)
    , m_stop_on_detected(stop_on_detected)
    , m_detected(false)
{}

QImage ShinySoundDetector::consume_screenshot(){
    SpinLockGuard lg(m_lock);
    return std::move(m_screenshot);
}


bool ShinySoundDetector::process_spectrums(
    const std::vector<AudioSpectrum>& newSpectrums,
    AudioFeed& audioFeed
){
    if (newSpectrums.empty()){
        m_detected.store(false, std::memory_order_release);
        return false;
    }

    size_t sampleRate = newSpectrums[0].sample_rate;
    if (m_matcher == nullptr || m_matcher->sampleRate() != sampleRate){
        m_console.log("Loading spectrogram...");
        m_matcher = std::make_unique<SpectrogramMatcher>(
            AudioTemplateCache::instance().get_throw("PokemonLA/ShinySound", sampleRate),
            SpectrogramMatcher::Mode::SPIKE_CONV, sampleRate,
            GameSettings::instance().SHINY_SHOUND_LOW_FREQUENCY
        );
    }

    // Feed spectrum one by one to the matcher:
    // newSpectrums are ordered from newest (largest timestamp) to oldest (smallest timestamp).
    // To feed the spectrum from old to new, we need to go through the vector in the reverse order:
    
    for(auto it = newSpectrums.rbegin(); it != newSpectrums.rend(); it++){
        std::vector<AudioSpectrum> singleSpectrum = {*it};
        float matcherScore = m_matcher->match(singleSpectrum);

        if (matcherScore == FLT_MAX){
            continue; // error or not enough spectrum history
        }

        const float threshold = (float)GameSettings::instance().SHINY_SHOUND_THRESHOLD0;
        bool found = matcherScore <= threshold;
//        cout << matcherScore << endl;

        size_t curStamp = m_matcher->latestTimestamp();
        // std::cout << "(" << curStamp+1-m_matcher->numTemplateWindows() << ", " <<  curStamp+1 << "): " << matcherScore << 
        //     (found ? " FOUND!" : "") << std::endl;

        if (found){
            {
                SpinLockGuard lg(m_lock);
                m_screenshot = m_console.video().snapshot();
            }
            std::ostringstream os;
            os << "Shiny sound find, score " << matcherScore << "/" << threshold << ", scale: " << m_matcher->lastMatchedScale();
            m_console.log(os.str(), COLOR_BLUE);
            audioFeed.add_overlay(curStamp+1-m_matcher->numTemplateWindows(), curStamp+1, COLOR_RED);
            // Tell m_matcher to skip the remaining spectrums so that if `process_spectrums()` gets
            // called again on a newer batch of spectrums, m_matcher is happy.
            m_matcher->skip(std::vector<AudioSpectrum>(
                newSpectrums.begin(),
                newSpectrums.begin() + std::distance(it + 1, newSpectrums.rend())
            ));
            m_detected.store(true, std::memory_order_release);
            return m_stop_on_detected;
        }
    }

    m_detected.store(false, std::memory_order_release);
    return false;
}

void ShinySoundDetector::clear(){
    m_matcher->clear();
}



}
}
}