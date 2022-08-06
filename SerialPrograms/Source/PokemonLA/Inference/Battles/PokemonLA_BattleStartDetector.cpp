/*  Battle Start Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/ImageGradient.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "PokemonLA_BattleStartDetector.h"

#include <vector>
#include <iostream>
#include <algorithm>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


BattleStartDetector::BattleStartDetector(Logger& logger, VideoOverlay& overlay)
    : VisualInferenceCallback("BattleStartDetector")
    , m_upper_boundary  (0.0, 0.113, 1.0, 0.15)
    , m_lower_boundary (0.2, 0.871, 0.63, 0.015)
{}

void BattleStartDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_BLUE, m_upper_boundary);
    items.add(COLOR_BLUE, m_lower_boundary);
}

bool BattleStartDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){

    Color threshold(50, 50, 50);
    
    const ImageViewRGB32 upper_boundary = extract_box_reference(frame, m_upper_boundary);
    
    const size_t upper_border_length = count_horizontal_translucent_border_pixels(upper_boundary, threshold, true);

    bool detected = upper_border_length / (float)upper_boundary.width() > 0.9;

    if (detected == false){
        return false;
    }

    const ImageViewRGB32 lower_boundary = extract_box_reference(frame, m_lower_boundary);

    const size_t lower_border_length = count_horizontal_translucent_border_pixels(lower_boundary, threshold, false);

    return lower_border_length / (float)lower_boundary.width() > 0.9;
}



}
}
}
