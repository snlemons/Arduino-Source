/*  Egg Hatch Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EggHatchFilter_H
#define PokemonAutomation_PokemonBDSP_EggHatchFilter_H

#include "Common/Cpp/Options/DropdownOption.h"
#include "Common/Cpp/Options/EditableTableOption2.h"
#include "Pokemon/Pokemon_IVChecker.h"
#include "Pokemon/Options/Pokemon_IVCheckerOption.h"
#include "Pokemon/Inference/Pokemon_IVCheckerReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
using namespace Pokemon;


enum class EggHatchAction{
    StopProgram,
    Keep,
    Release,
};

enum class EggHatchShinyFilter{
    Anything,
    NotShiny,
    Shiny,
};

enum class EggHatchGenderFilter{
    Any,
    Male,
    Female,
    Genderless
};




class EggHatchFilterRow : public EditableTableRow2{
public:
    EggHatchFilterRow();
    EggHatchFilterRow(EggHatchShinyFilter p_shiny);
    virtual std::unique_ptr<EditableTableRow2> clone() const override;

public:
    DropdownCell action;
    DropdownCell shiny;
    DropdownCell gender;
    IVCheckerFilterCell iv_hp;
    IVCheckerFilterCell iv_atk;
    IVCheckerFilterCell iv_def;
    IVCheckerFilterCell iv_spatk;
    IVCheckerFilterCell iv_spdef;
    IVCheckerFilterCell iv_speed;
};

class EggHatchFilterTable : public EditableTableOption_t<EggHatchFilterRow>{
public:
    EggHatchFilterTable();
    virtual std::vector<std::string> make_header() const override;
    static std::vector<std::unique_ptr<EditableTableRow2>> make_defaults();

    EggHatchAction get_action(bool shiny, const IVCheckerReader::Results& IVs, EggHatchGenderFilter gender) const;
};










}
}
}
#endif
