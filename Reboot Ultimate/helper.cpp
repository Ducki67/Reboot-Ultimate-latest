#include "helper.h"

void* Helper::GetCosmeticLoadoutForPawn(UObject* Pawn)
{
    if (!Pawn)
       return nullptr;

    static auto CosmeticLoadoutOffset = Pawn->GetOffset("CosmeticLoadout");

    if (CosmeticLoadoutOffset != 0)
    {
        return (void*)((uint8_t*)Pawn + CosmeticLoadoutOffset);
    }
    else
    {
        static auto CustomizationLoadoutOffset = Pawn->GetOffset("CustomizationLoadout");
        return (void*)((uint8_t*)Pawn + CustomizationLoadoutOffset);
    }
}