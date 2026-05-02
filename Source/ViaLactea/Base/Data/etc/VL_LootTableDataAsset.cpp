#include "Base/Data/etc/VL_LootTableDataAsset.h"
#include "Base/Data/Item/VL_ItemDataAsset.h"

TArray<FLootResult> UVL_LootTableDataAsset::RollLoot() const
{
    TArray<FLootResult> FinalLoot;

    for (const FLootEntry& Entry : LootEntries)
    {
        // 1. 유효성 검사 (아이템 데이터가 비어있으면 스킵)
        if (Entry.ItemData.IsNull()) continue;

        // 2. 확률 계산 (0.0 ~ 1.0 난수 발생)
        const float RandomRoll = FMath::FRand();

        // 3. RandomRoll이 확률 값보다 낮으면 당첨
        if (RandomRoll <= Entry.DropChance)
        {
            FLootResult Result;
            Result.ItemData = Entry.ItemData;

            // 3. 개수 결정 (최소 ~ 최대 사이 랜덤)
            Result.Quantity = FMath::RandRange(Entry.MinCount, Entry.MaxCount);

            if (Result.Quantity > 0)
            {
                bool bFound = false;
                for (FLootResult& ExistingResult : FinalLoot)
                {
                    if (ExistingResult.ItemData == Result.ItemData)
                    {
                        // 1.해당 아이템의  데이터 테이블에 접근해서(id이든 인덱스이든) 해당 아이템의
                        // max 값을 확인하고 합쳐지고 남은 잉여아이템을 가방 확인 후 남았다면, 옆 칸에 넣어주는 로직도 필요
                        // 인벤토리 컴포넌트에 있을 수도 있어서 일단 보류
                        
                        // 2. 먹었는데 아이템 칸이 꽉찼다면 떨어트릴 것인지 아니면 못먹는다고 ui 띄울 것인지
                        // 개인적으로는 땅에 떨어트리기가 UI추가 작업 필요 없어서 좋아보임

                        ExistingResult.Quantity += Result.Quantity; // 수량만 병합
                        bFound = true;
                        break;
                    }
                }

                // 없다면 새로 추가
                if (!bFound)
                {
                    FinalLoot.Add(Result);
                }
            }
        }
    }
    return FinalLoot;
}
