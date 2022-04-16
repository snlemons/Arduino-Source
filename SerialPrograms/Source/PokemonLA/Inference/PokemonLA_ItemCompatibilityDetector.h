/*  Item Compatibility Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_ItemCompatibilityDetector_H
#define PokemonAutomation_PokemonLA_ItemCompatibilityDetector_H

#include <QImage>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

enum class ItemCompatibility{
    NONE,
    COMPATIBLE,
    INCOMPATIBLE,
};

// Detect whether an item can be used on the lead pokemon.
// The game shows blue "Compatible" if it can be used, red "Incompatible" otherwise.
// This function uses the color of the text to detect which one is shown.
// Return ItemCompatibility::NONE if the detection fails.
ItemCompatibility detect_item_compatibility(const QImage& screen);


}
}
}
#endif
