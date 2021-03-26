/*  God Egg Duplication
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Clientside/PrettyPrint.h"
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShEggRoutines.h"
#include "PokemonSwSh_GodEggDuplication.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

GodEggDuplication::GodEggDuplication()
    : SingleSwitchProgram(
        FeedbackType::NONE, PABotBaseLevel::PABOTBASE_31KB,
        "God Egg Duplication",
        "NativePrograms/GodEggDuplication.md",
        "Mass duplicate " + STRING_POKEMON + " with the God Egg."
    )
    , MAX_FETCH_ATTEMPTS(
        "<b>Fetch this many times:</b><br>This puts a limit on how many eggs you can get so you don't make a mess of your boxes for fetching too many.",
        2000
    )
    , PARTY_ROUND_ROBIN(
        "<b>Party Round Robin:</b><br>Cycle through this many party members.",
        6, 1, 6
    )
{
    m_options.emplace_back(&MAX_FETCH_ATTEMPTS, "MAX_FETCH_ATTEMPTS");
    m_options.emplace_back(&PARTY_ROUND_ROBIN, "PARTY_ROUND_ROBIN");
}


void GodEggDuplication::collect_godegg(uint8_t party_slot) const{
    pbf_wait(50);
    ssf_press_button1(BUTTON_B, 100);
    ssf_press_button1(BUTTON_B, 100);
    pbf_wait(225);

    //  "You received an Egg from the Nursery worker!"
    ssf_press_button1(BUTTON_B, 300);

    //  "Where do you want to send the Egg to?"
    ssf_press_button1(BUTTON_A, 100);

    //  (extra line of text for French)
    ssf_press_button1(BUTTON_B, 100);

    //  "Please select a Pokemon to swap from your party."
    ssf_press_button1(BUTTON_B, MENU_TO_POKEMON_DELAY);

    //  Select the party member.
    for (uint8_t c = 0; c < party_slot; c++){
        ssf_press_dpad1(DPAD_DOWN, 10);
    }
    ssf_press_button1(BUTTON_A, 300);
    pbf_mash_button(BUTTON_B, 500);
}
void GodEggDuplication::run_program(SingleSwitchProgramEnvironment& env, uint16_t attempts) const{
    if (attempts == 0){
        return;
    }

    uint8_t items = PARTY_ROUND_ROBIN > 6 ? 6 : PARTY_ROUND_ROBIN;
    uint8_t party_slot = 0;

    uint16_t c = 0;

    //  1st Fetch: Get into position.
    {
        env.logger.log("Fetch Attempts: " + tostr_u_commas(c));
        fly_home_collect_egg(true);
        collect_godegg(party_slot++);
        if (party_slot >= items){
            party_slot = 0;
        }

        c++;
        if (c >= attempts){
            return;
        }
    }

    //  Now we are in steady state.
    for (; c < attempts; c++){
        env.logger.log("Fetch Attempts: " + tostr_u_commas(c));
        eggfetcher_loop();
        collect_egg();
        collect_godegg(party_slot++);
        if (party_slot >= items){
            party_slot = 0;
        }
    }
}

void GodEggDuplication::program(SingleSwitchProgramEnvironment& env) const{
    grip_menu_connect_go_home();
    resume_game_back_out(TOLERATE_SYSTEM_UPDATE_MENU_FAST, 400);

    run_program(env, MAX_FETCH_ATTEMPTS);
    ssf_press_button2(BUTTON_HOME, GAME_TO_HOME_DELAY_SAFE, 10);

    end_program_callback();
    end_program_loop();
}



}
}
}
