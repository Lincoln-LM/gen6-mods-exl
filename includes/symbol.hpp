#pragma once
#include <3ds/types.h>

typedef enum PokemonTitle
{
    GAME_X = 0x0004000000055D00,
    GAME_Y = 0x0004000000055E00,
    GAME_OR = 0x000400000011C400,
    GAME_AS = 0x000400000011C500,
    GAME_S = 0x0004000000164800,
    GAME_M = 0x0004000000175E00,
    GAME_US = 0x00040000001B5000,
    GAME_UM = 0x00040000001B5100,
} PokemonTitle;

namespace AlphaSapphire {
    u32 LoadCRO_offset = 0x36f518;

    namespace DLLField {
        u32 CreateWildBattle_offset = 0x9ab30;
        struct encounter_slot_t {
            u16 species_form;
            u8 min_level;
            u8 max_level;
        };

    }

    namespace TinyMT {
        u32 RandMax_offset = 0x48af80;
        ushort RandMax(ushort rand_max) {
            return external<ushort>(RandMax_offset, rand_max);
        }
    }

    namespace PersonalInfo {
        u32 LoadPersonalData_offset = 0x6f5488;
        u32 GetPersonalParam_offset = 0x14eb40;
        enum PersonalParamIndex {
            // TODO: other params
            TYPE_1 = 6,
            TYPE_2 = 7,
            FORM_COUNT = 32,
        };
        void LoadPersonalData(ushort species, ushort form) {
            external<void>(LoadPersonalData_offset, species, form);
        }
        int GetPersonalParam(PersonalParamIndex param_index) {
            return external<int>(GetPersonalParam_offset, param_index);
        }
    }
}