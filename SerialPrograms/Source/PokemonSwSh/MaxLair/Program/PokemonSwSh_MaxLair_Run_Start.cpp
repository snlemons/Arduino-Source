/*  Max Lair Run Start
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_DigitEntry.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "PokemonSwSh/Programs/PokemonSwSh_Internet.h"
#include "PokemonSwSh/Programs/Hosting/PokemonSwSh_DenTools.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonReader.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_Lobby.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_Entrance.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_Notifications.h"
#include "PokemonSwSh_MaxLair_Run_EnterLobby.h"
#include "PokemonSwSh_MaxLair_Run_Start.h"
#include "PokemonSwSh_MaxLair_Run_StartSolo.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


bool abort_if_error(MultiSwitchProgramEnvironment& env, CancellableScope& scope, const std::atomic<size_t>& errors){
    if (errors.load(std::memory_order_acquire)){
        env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
            pbf_press_button(context, BUTTON_B, 10, TICKS_PER_SECOND);
            pbf_press_button(context, BUTTON_A, 10, TICKS_PER_SECOND);
            pbf_mash_button(context, BUTTON_B, 8 * TICKS_PER_SECOND);
        });
        return true;
    }
    return false;
}

bool wait_for_all_join(
    ConsoleHandle& console, BotBaseContext& context,
    const ImageViewRGB32& entrance,
    size_t start_players
){
    LobbyJoinedDetector joined_detector(start_players, false);
    EntranceDetector entrance_detector(entrance);
    int result = wait_until(
        console, context,
        std::chrono::seconds(10),
        {
            {joined_detector},
            {entrance_detector},
        },
        INFERENCE_RATE
    );
    switch (result){
    case 0:
        console.log("Detected " + std::to_string(start_players) + " players in lobby!");
        return true;
    case 1:
        console.log("Detected entrance... Did you get disconnected?", COLOR_RED);
//        dump_image(console, MODULE_NAME, "wait_for_all_join", console.video().snapshot());
        return false;
    default:
        console.log("Timed out waiting for everyone to join.", COLOR_RED);
//        dump_image(console, MODULE_NAME, "wait_for_all_join", console.video().snapshot());
        return false;
    }
}

class AllJoinedTracker final : public Cancellable{
public:
    AllJoinedTracker(
        CancellableScope& scope, size_t consoles,
        WallClock time_limit
    )
        : m_time_limit(time_limit)
        , m_consoles(consoles)
        , m_counter(0)
    {
        attach(scope);
    }
    virtual ~AllJoinedTracker(){}

    bool report_joined(){
        std::unique_lock<std::mutex> lg(m_lock);
        m_counter++;
        if (m_counter >= m_consoles){
            m_cv.notify_all();
            return true;
        }
        while (true){
            m_cv.wait_until(lg, m_time_limit);
            throw_if_cancelled();
            if (m_counter >= m_consoles){
                return true;
            }
            if (current_time() > m_time_limit){
                return false;
            }
        }
    }

    virtual bool cancel(std::exception_ptr exception) noexcept override{
        if (Cancellable::cancel(std::move(exception))){
            return true;
        }
        std::lock_guard<std::mutex> lg(m_lock);
        m_cv.notify_all();
        return false;
    }

private:
    std::mutex m_lock;
    std::condition_variable m_cv;

    WallClock m_time_limit;
    size_t m_consoles;
    size_t m_counter;
};






bool start_raid_local(
    MultiSwitchProgramEnvironment& env, CancellableScope& scope,
    GlobalStateTracker& state_tracker,
    std::shared_ptr<const ImageRGB32> entrance[4],
    ConsoleHandle& host, size_t boss_slot,
    const HostingSettings& settings,
    ConsoleRuntime console_stats[4]
){
    if (env.consoles.size() == 1){
        BotBaseContext context(scope, host.botbase());
        return start_raid_self_solo(
            host, context,
            state_tracker, entrance[0], boss_slot, console_stats[0].ore
        );
    }

    env.log("Entering lobby...");

    uint8_t code[8];
    bool raid_code = settings.RAID_CODE.get_code(code);

    std::atomic<size_t> errors(0);
    env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
        size_t index = console.index();
        bool is_host = index == host.index();

        entrance[index] = enter_lobby(
            console, context,
            is_host ? boss_slot : 0,
            settings.MODE == HostingMode::HOST_ONLINE,
            console_stats[index].ore
        );
        if (!*entrance[index]){
            errors.fetch_add(1);
            return;
        }
    });
    if (errors.load(std::memory_order_acquire) != 0){
        env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
            pbf_mash_button(context, BUTTON_B, 8 * TICKS_PER_SECOND);
        });
        return false;
    }

    env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
        size_t index = console.index();
        GlobalState& state = state_tracker[index];
        bool is_host = index == host.index();

        //  Read boss.
        if (is_host){
            state.boss = read_boss_sprite(console);
        }

        //  Enter code.
        if (raid_code && env.consoles.size() > 1){
            pbf_press_button(context, BUTTON_PLUS, 10, TICKS_PER_SECOND);
            enter_digits(context, 8, code);
            pbf_wait(context, 2 * TICKS_PER_SECOND);
            pbf_press_button(context, BUTTON_A, 10, TICKS_PER_SECOND);
        }
    });

    //  Open lobby.
    env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
        //  Delay to prevent the Switches from forming separate lobbies.
        if (env.consoles.size() > 1 && console.index() != host.index()){
            pbf_wait(context, 3 * TICKS_PER_SECOND);
        }
        pbf_press_button(context, BUTTON_A, 10, TICKS_PER_SECOND);
    });

    auto time_limit = current_time() +
        std::chrono::milliseconds(settings.LOBBY_WAIT_DELAY * 1000 / TICKS_PER_SECOND);

    AllJoinedTracker joined_tracker(scope, env.consoles.size(), time_limit);

    //  Wait for all Switches to join.
    env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
        size_t index = console.index();

        //  Wait for a player to show up. This lets you ready up.
        if (!wait_for_a_player(console, context, *entrance[index], time_limit)){
            errors.fetch_add(1);
            return;
        }
    });
    if (abort_if_error(env, scope, errors)){
        return false;
    }

    env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
        //  Wait for all consoles to join.
        if (!joined_tracker.report_joined()){
            console.log("Not everyone was able to join.", COLOR_RED);
            errors.fetch_add(1);
            return;
        }
    });
    if (abort_if_error(env, scope, errors)){
        return false;
    }

    env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
        size_t index = console.index();
        if (!wait_for_all_join(console, context, *entrance[index], env.consoles.size())){
            console.log("Switches joined into different raids.", COLOR_RED);
            errors.fetch_add(1);
            return;
        }
    });
    if (abort_if_error(env, scope, errors)){
        return false;
    }

    //  Ready up and wait for lobby to be ready.
    env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
        //  Ready up.
        context.wait_for(std::chrono::seconds(1));
        pbf_press_button(context, BUTTON_A, 10, TICKS_PER_SECOND);
        context.wait_for_all_requests();

        //  Wait
        size_t index = console.index();
        if (!wait_for_lobby_ready(console, context, *entrance[index], env.consoles.size(), env.consoles.size(), time_limit)){
            errors.fetch_add(1);
            pbf_mash_button(context, BUTTON_B, 10 * TICKS_PER_SECOND);
            return;
        }
    });
    if (abort_if_error(env, scope, errors)){
        return false;
    }

    env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
        //  Start
        if (!start_adventure(console, context, env.consoles.size())){
            errors.fetch_add(1);
            pbf_mash_button(context, BUTTON_B, 10 * TICKS_PER_SECOND);
            return;
        }
    });
    if (abort_if_error(env, scope, errors)){
        return false;
    }

    return true;
}

bool start_raid_host(
    MultiSwitchProgramEnvironment& env, CancellableScope& scope,
    GlobalStateTracker& state_tracker,
    std::shared_ptr<const ImageRGB32> entrance[4],
    ConsoleHandle& host, size_t boss_slot,
    HostingSettings& settings,
    const PathStats& path_stats,
    const StatsTracker& session_stats,
    ConsoleRuntime console_stats[4]
){
    if (env.consoles.size() == 1){
        BotBaseContext context(scope, host.botbase());
        return start_raid_host_solo(
            env, host, context,
            state_tracker,
            entrance[0], boss_slot,
            settings,
            path_stats, session_stats,
            console_stats[0].ore
        );
    }

    env.log("Entering lobby...");

    uint8_t code[8];
    bool has_code = settings.RAID_CODE.get_code(code);
    std::string boss;

    std::atomic<size_t> errors(0);
    env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
        size_t index = console.index();
        bool is_host = index == host.index();

        entrance[index] = enter_lobby(
            console, context,
            is_host ? boss_slot : 0,
            settings.MODE == HostingMode::HOST_ONLINE,
            console_stats[index].ore
        );
        if (!*entrance[index]){
            errors.fetch_add(1);
            return;
        }
    });
    if (errors.load(std::memory_order_acquire) != 0){
        env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
            pbf_mash_button(context, BUTTON_B, 8 * TICKS_PER_SECOND);
        });
        return false;
    }

    env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
        size_t index = console.index();
        GlobalState& state = state_tracker[index];
        bool is_host = index == host.index();

        //  Read boss.
        if (is_host){
            boss = read_boss_sprite(console);
            state.boss = boss;
        }

        //  Enter Code
        if (has_code){
            pbf_press_button(context, BUTTON_PLUS, 10, TICKS_PER_SECOND);
            enter_digits(context, 8, code);
            pbf_wait(context, 2 * TICKS_PER_SECOND);
            pbf_press_button(context, BUTTON_A, 10, TICKS_PER_SECOND);
        }
    });

    //  Start delay.
    scope.wait_for(std::chrono::milliseconds(settings.START_DELAY * 1000 / TICKS_PER_SECOND));

    //  Open lobby.
    env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
        //  If you start the raids at the same time, they won't find each other.
        if (console.index() != host.index()){
            pbf_wait(context, 3 * TICKS_PER_SECOND);
        }
        pbf_press_button(context, BUTTON_A, 10, TICKS_PER_SECOND);
    });

    auto time_limit = current_time() +
        std::chrono::milliseconds(settings.LOBBY_WAIT_DELAY * 1000 / TICKS_PER_SECOND);

    AllJoinedTracker joined_tracker(scope, env.consoles.size(), time_limit);

    //  Wait for all Switches to join.
    env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
        //  Wait for a player to show up. This lets you ready up.
        size_t index = console.index();
        if (!wait_for_a_player(console, context, *entrance[index], time_limit)){
            errors.fetch_add(1);
            return;
        }
    });
    if (abort_if_error(env, scope, errors)){
        return false;
    }

    env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
        //  Wait for all consoles to join.
        if (!joined_tracker.report_joined()){
            console.log("Not everyone was able to join.", COLOR_RED);
            errors.fetch_add(1);
            return;
        }
    });
    if (abort_if_error(env, scope, errors)){
        return false;
    }

    env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
        size_t index = console.index();
        if (!wait_for_all_join(console, context, *entrance[index], env.consoles.size())){
            console.log("Switches joined into different raids.", COLOR_RED);
            errors.fetch_add(1);
            return;
        }
    });
    if (abort_if_error(env, scope, errors)){
        return false;
    }

    send_raid_notification(
        env,
        host,
        settings.NOTIFICATIONS,
        has_code, code,
        boss,
        path_stats, session_stats
    );

    //  Ready up and wait for lobby to be ready.
    env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
        //  Ready up.
        context.wait_for(std::chrono::seconds(1));
        pbf_press_button(context, BUTTON_A, 10, TICKS_PER_SECOND);
        context.wait_for_all_requests();

        //  Wait
        size_t index = console.index();
        if (!wait_for_lobby_ready(console, context, *entrance[index], env.consoles.size(), 4, time_limit)){
            errors.fetch_add(1);
            pbf_mash_button(context, BUTTON_B, 10 * TICKS_PER_SECOND);
            return;
        }
    });
    if (abort_if_error(env, scope, errors)){
        return false;
    }

    env.run_in_parallel(scope, [&](ConsoleHandle& console, BotBaseContext& context){
        //  Start
        if (!start_adventure(console, context, env.consoles.size())){
            errors.fetch_add(1);
            pbf_mash_button(context, BUTTON_B, 10 * TICKS_PER_SECOND);
            return;
        }
    });
    if (abort_if_error(env, scope, errors)){
        return false;
    }

    return true;
}



bool start_adventure(
    MultiSwitchProgramEnvironment& env, CancellableScope& scope,
    GlobalStateTracker& state_tracker,
    std::shared_ptr<const ImageRGB32> entrance[4],
    ConsoleHandle& host, size_t boss_slot,
    HostingSettings& settings,
    const PathStats& path_stats,
    const StatsTracker& session_stats,
    ConsoleRuntime console_stats[4]
){
    switch (settings.MODE){
    case HostingMode::NOT_HOSTING:
        return start_raid_local(env, scope, state_tracker, entrance, host, boss_slot, settings, console_stats);
    case HostingMode::HOST_LOCALLY:
    case HostingMode::HOST_ONLINE:
        return start_raid_host(
            env, scope,
            state_tracker,
            entrance,
            host, boss_slot,
            settings,
            path_stats, session_stats,
            console_stats
        );
    }
    throw InternalProgramError(&env.logger(), PA_CURRENT_FUNCTION, "Invalid mode enum.");
}










}
}
}
}
