/*  Environment (Linux)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Environment_Linux_H
#define PokemonAutomation_Environment_Linux_H

namespace PokemonAutomation{


enum class ThreadPriority{
    Max,
    Min,
};

constexpr ThreadPriority DEFAULT_PRIORITY_REALTIME  = ThreadPriority::Max;
constexpr ThreadPriority DEFAULT_PRIORITY_INFERENCE = ThreadPriority::Max;
constexpr ThreadPriority DEFAULT_PRIORITY_COMPUTE   = ThreadPriority::Min;



}
#endif