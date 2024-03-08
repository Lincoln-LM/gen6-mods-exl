#include <3ds/types.h>
#include "hook.hpp"
#include "external.hpp"
#include "symbol.hpp"
#include <string>
#include <tuple>

static std::string last_cro;
// Store the name of the last loaded CRO DLL
HOOK_DEFINE(CROCheck)
{
    static void Callback(Registers* regs)
    {
        last_cro.assign(reinterpret_cast<char*>(regs->r3));
        // original instruction
        regs->r2 = 1;
    }
};

// Randomize the encounter slots of a horde encounter
HOOK_DEFINE(HordeRandomizer)
{

    static AlphaSapphire::DLLField::encounter_slot_t horde_slots[5];

    static std::tuple<ushort, ushort> random_pokemon() {
        ushort species = AlphaSapphire::TinyMT::RandMax(721) + 1;
        AlphaSapphire::PersonalInfo::LoadPersonalData(species, 0);
        ushort form_count = AlphaSapphire::PersonalInfo::GetPersonalParam(AlphaSapphire::PersonalInfo::PersonalParamIndex::FORM_COUNT);
        ushort form = AlphaSapphire::TinyMT::RandMax(form_count);

        return std::make_tuple(species, form);
    }

    static u32 VectorOffset;
    static void Callback(Registers* regs)
    {
        auto battle_count = *reinterpret_cast<int*>(regs->r4 + 0x6c);
        // only randomize hordes
        if (battle_count  == 5) {
            int horde_type = AlphaSapphire::TinyMT::RandMax(18);
            for (int i = 0; i < 5; i++) {
                std::tuple<ushort, ushort> mon;
                int type1, type2;
                do {
                    mon = random_pokemon();
                    AlphaSapphire::PersonalInfo::LoadPersonalData(std::get<0>(mon), std::get<1>(mon));
                    type1 = AlphaSapphire::PersonalInfo::GetPersonalParam(AlphaSapphire::PersonalInfo::PersonalParamIndex::TYPE_1);
                    type2 = AlphaSapphire::PersonalInfo::GetPersonalParam(AlphaSapphire::PersonalInfo::PersonalParamIndex::TYPE_2);
                } while (type1 != horde_type && type2 != horde_type);

                horde_slots[i].species_form = std::get<0>(mon) | (std::get<1>(mon) << 11);
                horde_slots[i].min_level = 5;
                horde_slots[i].max_level = 5;
            }
            regs->r6 = reinterpret_cast<u32>(horde_slots);
        }
        else {
            // original instruction
            regs->r6 = regs->r8 + (regs->r0 << 2);
        }
    }
};
u32 HordeRandomizer::VectorOffset = -1;
AlphaSapphire::DLLField::encounter_slot_t HordeRandomizer::horde_slots[5] = {0};

// Patch DLLField.cro whenever loaded to enable the randomizer
HOOK_DEFINE(CROLoaded)
{
    static void Callback(Registers* regs)
    {
        auto dll_offset = regs->r5;
        // patch CRO DLL
        if (last_cro == "rom2:/DllField.cro") {
            HordeRandomizer::InstallAtAddressInline(dll_offset + AlphaSapphire::DLLField::CreateWildBattle_offset + 0x284, &HordeRandomizer::VectorOffset);
        }
        // original instruction
        regs->r0 = dll_offset;
    }
};

extern "C" void install_hooks(PokemonTitle title_id)
{
    if (title_id == GAME_OR || title_id == GAME_AS) {
        // unused memory :pray:
        vector_base = 0x6aec00;
        CROCheck::InstallAtAddressInline(AlphaSapphire::LoadCRO_offset + 0x10);
        CROLoaded::InstallAtAddressInline(AlphaSapphire::LoadCRO_offset + 0xf8);
    }
}