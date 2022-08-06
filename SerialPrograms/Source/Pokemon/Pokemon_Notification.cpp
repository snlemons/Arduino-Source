/*  Shiny Notification
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "Pokemon/Resources/Pokemon_PokeballNames.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "Pokemon_Notification.h"

namespace PokemonAutomation{
namespace Pokemon{


Color shiny_color(ShinyType shiny_type){
    switch (shiny_type){
    case ShinyType::MAYBE_SHINY:
    case ShinyType::UNKNOWN_SHINY:
    case ShinyType::STAR_SHINY:
        return COLOR_STAR_SHINY;
    case ShinyType::SQUARE_SHINY:
        return COLOR_SQUARE_SHINY;
    default:
        return Color();
    }
}
std::string shiny_symbol(ShinyType shiny_type){
    switch (shiny_type){
    case ShinyType::MAYBE_SHINY:
        return ":question:";
    case ShinyType::UNKNOWN_SHINY:
    case ShinyType::STAR_SHINY:
        return "\u2728";
    case ShinyType::SQUARE_SHINY:
        return "\u2733";    //  Green square asterisk
    default:
        return "";
    }
}
std::string pokemon_to_string(const EncounterResult& pokemon){
    std::string str;

    std::string symbol = shiny_symbol(pokemon.shininess);
    if (!symbol.empty()){
        str += symbol + " ";
    }

    if (pokemon.slug_candidates.empty()){
        str = "Unable to detect.";
    }else if (pokemon.slug_candidates.size() == 1){
        str += get_pokemon_name(*pokemon.slug_candidates.begin()).display_name();
    }else{
        str += "Ambiguous: ";
        bool first1 = true;
        for (const std::string& slug : pokemon.slug_candidates){
            if (!first1){
                str += ", ";
            }
            first1 = false;
            str += get_pokemon_name(slug).display_name();
        }
    }
    return str;
}



void send_encounter_notification(
    ProgramEnvironment& env,
    EventNotificationOption& settings_nonshiny,
    EventNotificationOption& settings_shiny,
    bool enable_names, bool shiny_detected,
    const std::vector<EncounterResult>& results,
    double alpha,
    const ImageViewRGB32& screenshot,
    const EncounterFrequencies* frequencies
){
    ShinyType max_shiny_type = ShinyType::UNKNOWN;
    size_t shiny_count = 0;

    std::string names;

    bool first = true;
    for (const EncounterResult& result : results){
        if (!first){
            names += "\n";
        }
        first = false;
        names += pokemon_to_string(result);
        max_shiny_type = max_shiny_type < result.shininess ? result.shininess : max_shiny_type;
        shiny_count += is_confirmed_shiny(result.shininess) ? 1 : 0;
    }
    if (max_shiny_type == ShinyType::MAYBE_SHINY){
        max_shiny_type = ShinyType::UNKNOWN_SHINY;
    }
    Color color = shiny_color(max_shiny_type);
    bool has_shiny = is_likely_shiny(max_shiny_type) || shiny_detected;

    std::string shinies;
    if (results.size() == 1){
        std::string symbol = shiny_symbol(results[0].shininess);
        switch (results[0].shininess){
        case ShinyType::UNKNOWN:
            shinies = "Unknown";
            break;
        case ShinyType::NOT_SHINY:
            shinies = "Not Shiny";
            break;
        case ShinyType::MAYBE_SHINY:
            shinies = "Maybe Shiny";
            break;
        case ShinyType::UNKNOWN_SHINY:
            shinies = symbol + std::string(" Shiny ") + symbol;
            break;
        case ShinyType::STAR_SHINY:
            shinies = symbol + std::string(" Star Shiny ") + symbol;
            break;
        case ShinyType::SQUARE_SHINY:
            shinies = symbol + std::string(" Square Shiny ") + symbol;
            break;
        }
    }else if (!results.empty()){
        std::string symbol = shiny_symbol(max_shiny_type);
        switch (shiny_count){
        case 0:
            if (shiny_detected){
                symbol = shiny_symbol(ShinyType::UNKNOWN_SHINY);
                shinies = symbol + " Found Shiny! " + symbol + " (Unable to determine which.)";
            }else{
                shinies = "No Shinies";
            }
            break;
        case 1:
            shinies = symbol + " Found Shiny! " + symbol;
            break;
        default:
            shinies += symbol + std::string(" Multiple Shinies! ") + symbol;
            break;
        }
    }

    std::vector<std::pair<std::string, std::string>> embeds;
    if (enable_names && !names.empty()){
        embeds.emplace_back("Species", std::move(names));
    }
    if (!shinies.empty()){
        if (!std::isnan(alpha)){
            shinies += "\n(Detection Alpha = " + tostr_default(alpha) + ")";
        }
        embeds.emplace_back("Shininess", std::move(shinies));
    }
    {
        std::string session_stats_str;
        if (env.current_stats()){
            session_stats_str += env.current_stats()->to_str();
        }
        if (frequencies && !frequencies->empty()){
            if (!session_stats_str.empty()){
                session_stats_str += "\n";
            }
            session_stats_str += frequencies->dump_sorted_map("");
        }
        if (!session_stats_str.empty()){
            embeds.emplace_back("Session Stats", std::move(session_stats_str));
        }
    }
    if (env.historical_stats()){
        embeds.emplace_back("Historical Stats", env.historical_stats()->to_str());
    }

    if (has_shiny){
        send_program_notification(
            env.logger(), settings_shiny,
            color, env.program_info(),
            "Encounter Notification",
            embeds,
            screenshot, true
        );
    }else{
        send_program_notification(
            env.logger(), settings_nonshiny,
            color, env.program_info(),
            "Encounter Notification",
            embeds,
            screenshot, false
        );
    }
}



void send_catch_notification(
    Logger& logger,
    EventNotificationOption& settings_catch_success,
    EventNotificationOption& settings_catch_failed,
    const ProgramInfo& info,
    const std::set<std::string>* pokemon_slugs,
    const std::string& ball_slug, int balls_used,
    bool success
){
    Color color = success ? COLOR_GREEN : COLOR_ORANGE;

    std::vector<std::pair<std::string, std::string>> embeds;

    if (pokemon_slugs){
        std::string str;
        if (pokemon_slugs->empty()){
            str = "None - Unable to detect.";
        }else if (pokemon_slugs->size() == 1){
            str += get_pokemon_name(*pokemon_slugs->begin()).display_name();
        }else{
            str += "Ambiguous: ";
            bool first = true;
            for (const std::string& slug : *pokemon_slugs){
                if (!first){
                    str += ", ";
                }
                first = false;
                str += get_pokemon_name(slug).display_name();
            }
        }
        embeds.emplace_back("Species", std::move(str));
    }
    {
        std::string str;
        if (balls_used >= 0){
            str += std::to_string(balls_used);
        }
        if (!ball_slug.empty()){
            if (!str.empty()){
                str += " x ";
            }
            str += get_pokeball_name(ball_slug).display_name();
        }
        if (!str.empty()){
            embeds.emplace_back("Balls Used", str);
        }
    }

    if (success){
        send_program_notification(
            logger, settings_catch_success,
            color, info,
            STRING_POKEMON + " Caught",
            embeds
        );
    }else{
        send_program_notification(
            logger, settings_catch_failed,
            color, info,
            "Catch Failed",
            embeds
        );
    }
}




}
}
