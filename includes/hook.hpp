#pragma once
#include "common.h"
#include "plgldr.h"
#include <3ds/errf.h>
extern "C" {
#include "csvc.h"
}

// needs to be set to an unused portion of memory to store hook vectors in
u32 vector_base = -1;

#define HOOK_DEFINE(name) \
    struct name : public Hook<name>

#define ENSURE_VECTOR_BASE() \
    if (vector_base == -1) { \
        ERRF_ThrowResult(0xBAD0FF); \
    }

struct Registers {
    u32 r0;
    u32 r1;
    u32 r2;
    u32 r3;
    u32 r4;
    u32 r5;
    u32 r6;
    u32 r7;
    u32 r8;
    u32 r9;
    u32 r10;
    u32 r11;
    u32 r12;
    u32 lr;
};

static u32 hook_ofs = 0;
template <typename Derived>
struct Hook
{
    // Install a hook that replaces all logic from {address} until the end of the function with the defined callback
    // Callback should return the same type as the original function and must specify its own arguments based on the context of the original instruction
    static void InstallAtAddress(u32 address, u32 *stored_offset = nullptr)
    {
        ENSURE_VECTOR_BASE();
        u32 vector_offset;
        if (stored_offset != nullptr && *stored_offset != -1) {
            vector_offset = *stored_offset;
        }
        else {
            vector_offset = hook_ofs;
            hook_ofs += 8;
        }
        if (stored_offset != nullptr) {
            *stored_offset = vector_offset;
        }
        u32 vector_address = vector_base + vector_offset;
        u32 *vector_memory = (u32 *)PA_FROM_VA_PTR(vector_address);
        u32 *target_memory = (u32 *)PA_FROM_VA_PTR(address);
        u32 ofs = ((vector_address - (address + 8)) >> 2) & 0xffffff;
        // b vector_address (from hook)
        target_memory[0] = (0b1110 << 28) | (0b101 << 25) | (0b0 << 24) | ofs;
        // ldr pc, [pc, #-4] (pc = Callback)
        vector_memory[0] = 0xe51ff004;
        vector_memory[1] = reinterpret_cast<u32>(Derived::Callback);
        svcInvalidateEntireInstructionCache();
    }
    // Install a hook that replaces a specific instruction with the defined callback
    // Callback must be ``static void Callback(Registers* regs)`` and is expected to reimplement the original instruction if neccesary
    static void InstallAtAddressInline(u32 address, u32 *stored_offset = nullptr)
    {
        ENSURE_VECTOR_BASE();
        u32 vector_offset;
        if (stored_offset != nullptr && *stored_offset != -1) {
            vector_offset = *stored_offset;
        }
        else {
            vector_offset = hook_ofs;
            hook_ofs += 24;
        }
        if (stored_offset != nullptr) {
            *stored_offset = vector_offset;
        }
        u32 vector_address = vector_base + vector_offset;
        u32 *vector_memory = (u32 *)PA_FROM_VA_PTR(vector_address);
        u32 *target_memory = (u32 *)PA_FROM_VA_PTR(address);
        u32 ofs = ((vector_address - (address + 8)) >> 2) & 0xffffff;
        // bl vector_address (from hook)
        target_memory[0] = (0b1110 << 28) | (0b101 << 25) | (0b1 << 24) | ofs;
        // stmdb sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, lr}
        vector_memory[0] = 0xe92d5fff;
        // mov r0, sp
        vector_memory[1] = 0xe1a0000d;
        // mov lr, pc (lr = vector_memory[4])
        vector_memory[2] = 0xe1a0e00f;
        // ldr pc, [pc, #0] (pc = Callback)
        vector_memory[3] = 0xe51ff000;
        // ldmia sp!, {r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, pc}
        vector_memory[4] = 0xe8bd9fff;
        vector_memory[5] = reinterpret_cast<u32>(Derived::Callback);
        svcInvalidateEntireInstructionCache();
    }
};