#include "Base/Character/VL_CharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "Base/Data/Character/VL_CharacterDataAsset.h"
#include "CustomLog/CustomLog.h"

// Sets default values
AVL_CharacterBase::AVL_CharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// 스프링암 카메라 라인트레이스가 캐릭터/몬스터에 반응하지 않도록 무시
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

float AVL_CharacterBase::GetCooldownValue(FGameplayTag Tag) const
{
	if (CharacterDataAsset)
	{
		return CharacterDataAsset->GetCooldownValue(Tag);
	}

	return 0.0f;
}

// Called when the game starts or when spawned
void AVL_CharacterBase::BeginPlay()
{
	Super::BeginPlay();
	if (CharacterDataAsset)
	{
		// 2. 메시 로드 및 설정 (소프트 레퍼런스 동기 로드)
		if (!CharacterDataAsset->SkeletalMesh.IsNull())
		{
			USkeletalMesh* LoadedMesh = CharacterDataAsset->SkeletalMesh.LoadSynchronous();
			if (LoadedMesh)
			{
				GetMesh()->SetSkeletalMesh(LoadedMesh);
				//CUSTOM_LOG("%s: 메시 로드 완료", *GetName());
			}
		}

		// 3. 애니메이션 블루프린트(ABP) 클래스 설정
		// 메시를 설정한 후에 호출해야 AnimInstance가 정상적으로 생성됩니다.
		if (CharacterDataAsset->AnimBlueprintClass)
		{
			GetMesh()->SetAnimInstanceClass(CharacterDataAsset->AnimBlueprintClass);
			//CUSTOM_LOG("%s: 애니메이션 블루프린트 설정 완료", *GetName());
		}
		else
		{
			CUSTOM_LOG("%s: 경고 - 데이터 에셋에 AnimBlueprintClass가 없습니다.", *GetName());
		}
	}
	else
	{
		CUSTOM_LOG("%s: 에러 - CharacterDataAsset이 할당되지 않았습니다!", *GetName());
	}
}

// Called every frame
void AVL_CharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AVL_CharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

