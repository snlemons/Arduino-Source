/*  Ingo Battle Grinder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <chrono>
#include <iostream>
#include <QImage>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/Inference/PokemonLA_BattleMenuDetector.h"
#include "PokemonLA/Inference/PokemonLA_BattlePokemonSwitchDetector.h"
#include "PokemonLA/Programs/PokemonLA_BattleRoutines.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA_MagikarpMoveGrinder.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ArcPhoneDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_DialogueEllipseDetector.h"
#include "PokemonLA/Inference/PokemonLA_DialogDetector.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


MagikarpMoveGrinder_Descriptor::MagikarpMoveGrinder_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonLA:MagikarpMoveGrinder",
        STRING_POKEMON + " LA", "Magikarp Move Grinder",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/MagikarpMoveGrinder.md",
        "grind status moves with any style against a Magikarp to finish " + STRING_POKEDEX + " research tasks.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


MagikarpMoveGrinder::MagikarpMoveGrinder(const MagikarpMoveGrinder_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(POKEMON_ACTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);
}



class MagikarpMoveGrinder::Stats : public StatsTracker{
public:
    Stats()
        : magikarp(m_stats["Magikarp"])
        , move_attempts(m_stats["Move Attempts"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Magikarp");
        m_display_order.emplace_back("Move Attempts");
        m_display_order.emplace_back("Errors", true);
    }

    std::atomic<uint64_t>& magikarp;
    std::atomic<uint64_t>& move_attempts;
    std::atomic<uint64_t>& errors;
};

std::unique_ptr<StatsTracker> MagikarpMoveGrinder::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


bool MagikarpMoveGrinder::battle_magikarp(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    Stats& stats = env.stats<Stats>();

    env.console.log("Grinding on Magikarp...");

    // Which pokemon in the party is not fainted
    size_t cur_pokemon = 0;

    while(true){
        const bool stop_on_detected = true;
        BattleMenuDetector battle_menu_detector(env.console, env.console, stop_on_detected);
        BattlePokemonSwitchDetector pokemon_switch_detector(env.console, env.console, stop_on_detected);
        ArcPhoneDetector arc_phone_detector(env.console, env.console, std::chrono::milliseconds(200), stop_on_detected);
        int ret = wait_until(
            env, env.console, context, std::chrono::minutes(2),
            {
                &battle_menu_detector,
                &pokemon_switch_detector,
                &arc_phone_detector
            }
        );
        if (ret < 0){
            env.console.log("Error: Failed to find battle menu after 2 minutes.");
            throw OperationFailedException(env.console, "Failed to find battle menu after 2 minutes.");
        }

        if (ret == 0){
            env.console.log("Our turn!", COLOR_BLUE);

            // Press A to select moves
            pbf_press_button(context, BUTTON_A, 10, 125);
            context.wait_for_all_requests();
            
            const MoveStyle style = POKEMON_ACTIONS.get_style(cur_pokemon);

            const bool check_move_success = true;
            if (use_move(env.console, context, cur_pokemon, 0, style, check_move_success) == false){
                // We are still on the move selection screen. No PP.
                cur_pokemon++;
                if (cur_pokemon >= POKEMON_ACTIONS.num_pokemon()){
                    env.console.log("All pokemon grinded. Stop program.");
                    break;
                }
                env.console.log("No PP. Switch Pokemon.", COLOR_RED);

                // Press B to leave move selection menu
                pbf_press_button(context, BUTTON_B, 10, 125);

                // Go to the switching pokemon screen:
                pbf_press_dpad(context, DPAD_DOWN, 20, 100);
                
                cur_pokemon = switch_pokemon(env.console, context, cur_pokemon, POKEMON_ACTIONS.num_pokemon());
            }
            else{
                stats.move_attempts++;
                env.update_stats();
            }
        }
        else if (ret == 1){
            env.console.log("Pokemon fainted.");
            cur_pokemon++;
            if (cur_pokemon >= POKEMON_ACTIONS.num_pokemon()){
                env.console.log("All pokemon grinded. Stop program.");
                break;
            }
            cur_pokemon = switch_pokemon(env.console, context, cur_pokemon, POKEMON_ACTIONS.num_pokemon());
        }
        else{ // ret is 2
            env.console.log("Battle finished.");
            break;
        }
    }

    stats.magikarp++;
    env.update_stats();

    return false;
}



void MagikarpMoveGrinder::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    Stats& stats = env.stats<Stats>();

    if (POKEMON_ACTIONS.num_pokemon() == 0){
        throw OperationFailedException(env.console, "No Pokemon sepecified to grind.");
    }

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);

    env.update_stats();
    send_program_status_notification(
        env.logger(), NOTIFICATION_STATUS,
        env.program_info(),
        "",
        stats.to_str()
    );
    try{
        battle_magikarp(env, context);
    }catch (OperationFailedException&){
        stats.errors++;
        throw;
    }

    env.update_stats();
    send_program_finished_notification(
        env.logger(), NOTIFICATION_PROGRAM_FINISH,
        env.program_info(),
        "",
        stats.to_str()
    );
}





}
}
}