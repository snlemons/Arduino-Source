/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Globals.h"
#include "PokemonBDSP_EncounterFilterEnums.h"
#include "PokemonBDSP_EncounterFilterOption.h"
#include "PokemonBDSP_EncounterFilterWidget.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{




EncounterFilterOption::EncounterFilterOption(bool enable_overrides, bool allow_autocatch)
    : m_label("<b>Encounter Filter:</b>")
    , m_enable_overrides(enable_overrides)
    , m_allow_autocatch(allow_autocatch)
    , m_shiny_filter_default(ShinyFilter::SHINY)
    , m_shiny_filter_current(m_shiny_filter_default)
    , m_factory(allow_autocatch)
    , m_table(
        "<b>Overrides:</b><br>"
        "The game language must be properly set to read " + STRING_POKEMON + " names. "
        "If multiple overrides apply and are conflicting, the program will stop." +
        (allow_autocatch
            ? "<br>Auto-catching only applies in single battles. The program will stop if asked to auto-catch in a double-battle."
            : ""
        ),
        m_factory
    )
{}
std::vector<EncounterFilterOverride> EncounterFilterOption::overrides() const{
    std::vector<EncounterFilterOverride> ret;
    for (size_t c = 0; c < m_table.size(); c++){
        ret.emplace_back(static_cast<const EncounterFilterOverride&>(m_table[c]));
    }
    return ret;
}
void EncounterFilterOption::load_json(const JsonValue& json){
    using namespace Pokemon;

    const JsonObject* obj = json.get_object();
    if (obj == nullptr){
        return;
    }

    const std::string* str = obj->get_string("ShinyFilter");
    if (str != nullptr){
        auto iter = ShinyFilter_MAP.find(*str);
        if (iter != ShinyFilter_MAP.end()){
            m_shiny_filter_current.store(iter->second, std::memory_order_release);
        }
    }

    if (m_enable_overrides){
        const JsonValue* array = obj->get_value("Overrides");
        if (array != nullptr){
            m_table.load_json(*array);
        }
    }
}
JsonValue EncounterFilterOption::to_json() const{
    JsonObject obj;
    obj["ShinyFilter"] = ShinyFilter_NAMES[(size_t)m_shiny_filter_current.load(std::memory_order_acquire)];

    if (m_enable_overrides){
        obj["Overrides"] = m_table.to_json();
    }

    return obj;
}
void EncounterFilterOption::restore_defaults(){
    m_shiny_filter_current.store(m_shiny_filter_default, std::memory_order_release);
    m_table.restore_defaults();
}
ConfigWidget* EncounterFilterOption::make_ui(QWidget& parent){
    return new EncounterFilterWidget(parent, *this);
}




}
}
}
