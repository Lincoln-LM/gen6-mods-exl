#pragma once
extern "C" {
#include "csvc.h"
}
template <typename Ret, typename... Args>
inline static Ret external(long addr, Args... args) {
    return reinterpret_cast<Ret(*)(Args...)>(PA_FROM_VA_PTR(addr))(args...);
}