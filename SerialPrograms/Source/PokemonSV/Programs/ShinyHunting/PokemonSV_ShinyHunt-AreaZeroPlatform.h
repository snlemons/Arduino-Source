/*  Shiny Hunt - Area Zero Platform
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_ShinyHuntAreaZeroPlatform_H
#define PokemonAutomation_PokemonSV_ShinyHuntAreaZeroPlatform_H

#include <functional>
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




class ShinyHuntAreaZeroPlatform_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHuntAreaZeroPlatform_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class ShinyHuntAreaZeroPlatform : public SingleSwitchProgramInstance{
public:
    ShinyHuntAreaZeroPlatform();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    void run_iteration(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context);
    bool clear_in_front(
        ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
        std::function<void(BotBaseContext& context)>&& command
    );

    void run_path0(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context);
    void run_path1(ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context);

private:
    enum class Mode{
        NO_RESET,
        PERIODIC_RESET,
    };
    EnumDropdownOption<Mode> MODE;

    SimpleIntegerOption<uint16_t> RESET_DURATION_MINUTES;

    enum class Path{
        PATH0,
        PATH1,
    };
    EnumDropdownOption<Path> PATH;

    BooleanCheckBoxOption VIDEO_ON_SHINY;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;

    uint64_t m_iterations = 0;
};





}
}
}
#endif