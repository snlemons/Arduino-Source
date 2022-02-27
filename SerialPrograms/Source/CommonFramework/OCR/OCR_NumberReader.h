/*  OCR Number Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_OCR_NumberReader_H
#define PokemonAutomation_OCR_NumberReader_H

#include <QImage>
#include "CommonFramework/Logging/LoggerQt.h"

namespace PokemonAutomation{
namespace OCR{


//  Returns -1 if no number is found.
int read_number(LoggerQt& logger, const QImage& image);


}
}
#endif
