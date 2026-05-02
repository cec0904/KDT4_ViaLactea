#include "Monster/Boss/VL_Boss1.h"
#include "Base/Component/VL_AggroComponent.h"
#include "Base/Component/VL_CooldownComponent.h"
#include "Base/Data/Character/VL_BossMonsterDataAsset.h"
#include "Base/Data/VFX/VL_VFXDataAsset.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Kismet/KismetMathLibrary.h"


// 피직스에셋 접근
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Engine/DamageEvents.h"

//Gimmick
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"   
#include "GameFramework/Actor.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/World.h"           // GetWorld 및 World 관련 기능
#include "TimerManager.h"// SetTimer, ClearTimer 등 타이머 직접 제어
#include "Gimmick/VL_BossGimmickActor.h"
#include "Gimmick/VL_MeteorProjectile.h"
#include "Gimmick/VL_Solo_Projectile.h"
#include "Gimmick/VL_LaserProjectile.h"
#include "Gimmick/VL_LightningProjectile.h"
#include "Gimmick/VL_FinalBossAxe.h"
#include "NavigationSystem.h"


//페이즈 전환 타임라인 컴포넌트
#include "Components/TimelineComponent.h"

#include "GameFramework/CharacterMovementComponent.h"

// 비동기로드 헤더
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
// 필수 엔진 라이브러리
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"

// 애니메이션 접근
#include "Base/Character/VL_CharacterAnimInstanceBase.h"

#include "../../Player/MainCharacterBase.h"


#include "CustomLog/CustomLog.h"

AVL_Boss1::AVL_Boss1()
{
    PrimaryActorTick.bCanEverTick = false;
    SetReplicatingMovement(true);

	AggroComponent = CreateDefaultSubobject<UVL_AggroComponent>(TEXT("AggroComponent"));
    CooldownComponent = CreateDefaultSubobject<UVL_CooldownComponent>(TEXT("CooldownComponent"));

    CooldownComponent->SetIsReplicated(true);

    /////////////////////
    RushDamageDetector = CreateDefaultSubobject<USphereComponent>(TEXT("RushDamageDetector"));
    RushDamageDetector->SetupAttachment(GetCapsuleComponent());
    //RushDamageDetector->SetSphereRadius(20.0f); // 에디터에서설정

    RushDamageDetector->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RushDamageDetector->SetCollisionResponseToAllChannels(ECR_Overlap);

    //CurrentPhase = EBossPhase::None;
   
    EmergeTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("EmergeTimeline"));

    if (WeaponMesh)
    {
        // 레버넌트의 오른손 본/소켓에 부착
        WeaponMesh->SetupAttachment(GetMesh(), TEXT("WeaponPoint"));
    }

    Tags.Add(FName("Boss1"));
}

void AVL_Boss1::ResetCombat()
{
    if (AggroComponent)
    {
        AggroComponent->ClearAllThreat(); // 전체 맵을 비우는 함수 추가
    }
}

FText AVL_Boss1::GetBossName() const
{
    if (CachedBossData)
    {
        return CachedBossData->BossDisplayName;
    }

    return FText::FromString(TEXT("Unknown Boss"));
}

UVL_BossMonsterDataAsset* AVL_Boss1::GetBossDataAsset() const
{
    return CachedBossData;
}


void AVL_Boss1::Multicast_HideBones_Implementation(const TArray<FName>& BoneNames, bool bHide)
{
    if (!GetMesh()) return; 

    for (const FName& BoneName : BoneNames)
    {
        if (bHide)
        {
            // PBO_None: 본만 숨김 / PBO_Term: 자식 본까지 물리 포함 처리
            GetMesh()->HideBoneByName(BoneName, EPhysBodyOp::PBO_None);
        }
        else
        {
            GetMesh()->UnHideBoneByName(BoneName);
        }
    }

}

void AVL_Boss1::OnRep_BonesHidden()
{
    UpdateBoneVisibility();

}

void AVL_Boss1::UpdateBoneVisibility()
{
    if (!GetMesh()) return;

    TArray<FName> TargetBones = { TEXT("weapon_tail_l_01"), TEXT("weapon_tail_r_01") };

    for (const FName& BoneName : TargetBones)
    {
        if (bBonesHidden)
        {
            // PBO_None을 사용해야 물리 시뮬레이션 영향 없이 렌더링만 제거됩니다.
            GetMesh()->HideBoneByName(BoneName, EPhysBodyOp::PBO_None);
        }
        else
        {
            GetMesh()->UnHideBoneByName(BoneName);
        }
    }
}


void AVL_Boss1::ThrowAxe(bool bRight)
{
    if (!CachedBossData || CachedBossData->AxeClass.IsNull()) return;

    TSubclassOf<AVL_FinalBossAxe> AxeClass = CachedBossData->AxeClass.LoadSynchronous();
    if (!AxeClass) return;

    // 2. 스폰 위치 결정 (소켓 이름 오타 주의: Socket)
    const FName SocketName = bRight ? TEXT("weapon_r_Soket") : TEXT("weapon_l_Soket");
    FVector SpawnLocation = GetMesh()->GetSocketLocation(SocketName);

    AActor* Target = GetTargetCharacter();
    FRotator SpawnRotation;
    FVector LaunchDirection;

    if (Target)
    {
        FVector TargetLocation = Target->GetActorLocation();
        // 타겟을 바라보는 정확한 회전값 계산
        SpawnRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, TargetLocation);
        // [수정] 보스의 정면이 아닌, 타겟을 향한 방향 벡터를 추출
        LaunchDirection = SpawnRotation.Vector();
    }
    else
    {
        SpawnRotation = GetActorRotation();
        LaunchDirection = GetActorForwardVector();
    }


    if (HasAuthority())
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = GetInstigator();
        // 스폰 시 충돌로 인해 위치가 틀어지는 것을 방지
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AVL_FinalBossAxe* Axe = GetWorld()->SpawnActor<AVL_FinalBossAxe>(AxeClass, SpawnLocation, SpawnRotation, SpawnParams);
        if (Axe)
        {
            // [수정] 계산된 방향(타겟 방향)으로 발사
            Axe->FireInDirection(LaunchDirection);
        }
    }
}

float AVL_Boss1::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (!HasAuthority()) return 0.f;

    float FinalDamage = DamageAmount;

    // 화살체는 ApplyPointDamage를 통한 데미지 접근 방법을 사용해야 보스 약점 공략 가능
    // 1. 포인트 데미지(총알 등)인지 확인
    if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
    {
        const FPointDamageEvent* PointDamageEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);

        // 2. 피지컬 머티리얼 확인   HitPhysMat == PM_Head 에셋 정보 
        UPhysicalMaterial* HitPhysMat = PointDamageEvent->HitInfo.PhysMaterial.Get();
        if (HitPhysMat)
        {
            EPhysicalSurface HitSurface = UPhysicalMaterial::DetermineSurfaceType(HitPhysMat);
            const UEnum* SurfaceEnum = StaticEnum<EPhysicalSurface>();
            FString HitSurfaceName = SurfaceEnum->GetNameStringByValue((int64)HitSurface);
            FString ActiveWeakPointName = SurfaceEnum->GetNameStringByValue((int64)ActiveWeakPoint);

            // 3. 머리(SurfaceType1)를 맞았고, 현재 보스가 약점 노출 상태
            if (HitSurface == ActiveWeakPoint && ActiveWeakPoint != EPhysicalSurface::SurfaceType_Default) 
            {
                FinalDamage *= 2.0f; // 동적 데미지 2배 적용!
                //CUSTOM_LOG("보스 약점 타격! 2배 데미지!");
            }
            
        }
    }
    Multicast_PlayHitFlash();

    // 4. 최종 계산된 데미지를 부모 클래스에게 전달 (체력 감소 등 실제 처리는 부모가 함)
    return Super::TakeDamage(FinalDamage, DamageEvent, EventInstigator, DamageCauser);
}

void AVL_Boss1::SetHitFlashValue(float Value)
{
    if (BossDynamicMaterial)
    {
        // 머티리얼의 HitFlash 파라미터를 조절 (1.0이면 하양, 0.0이면 원래대로)
        BossDynamicMaterial->SetScalarParameterValue(FName("HitFlash"), Value);
    }
}

void AVL_Boss1::ResetHitFlash()
{
    SetHitFlashValue(0.0f);
}

void AVL_Boss1::Multicast_PlayHitFlash_Implementation()
{
    SetHitFlashValue(1.0f); // 

    // 0.1초 뒤에 ResetHitFlash 호출
    GetWorldTimerManager().SetTimer(HitFlashTimerHandle, this, &AVL_Boss1::ResetHitFlash, 0.1f, false);
}

// 30초 마다 실행
void AVL_Boss1::StartWeakPointCycle() 
{
    if (!HasAuthority()) return;

    // 1~7 사이의 랜덤한 SurfaceType 선택 
    int32 RandomIdx = FMath::RandRange(1, 5);
    ActiveWeakPoint = static_cast<EPhysicalSurface>(RandomIdx);
    
    OnRep_ActiveWeakPoint();

    // 10초 뒤에 약점을 끌 타이머 설정
    GetWorldTimerManager().SetTimer(WeakPointDurationHandle, this, &AVL_Boss1::DeactivateWeakPoint, 10.0f, false);

    //CUSTOM_LOG("보스 약점 노출! 번호: %d", RandomIdx);
}

void AVL_Boss1::DeactivateWeakPoint() 
{
    if (!HasAuthority()) return;

    ActiveWeakPoint = EPhysicalSurface::SurfaceType_Default;

    OnRep_ActiveWeakPoint();
}

void AVL_Boss1::ShowWeakPointVisual()
{
    GetWorldTimerManager().ClearTimer(WeakPointLocationTimerHandle);

    if (ActiveWeakPoint != EPhysicalSurface::SurfaceType_Default)
    {
        // 0.1초~0.2초 정도가 시각적으로 부드러움 (0.5초는 약간 끊겨 보일 수 있음)
        float UpdateInterval = 0.1f;

        // 반복 타이머 설정
        GetWorldTimerManager().SetTimer(WeakPointLocationTimerHandle, this, &AVL_Boss1::UpdateWeakPointLocation, UpdateInterval, true);

        // 최초 즉시 실행 (타이머는 설정된 시간 후에 첫 실행되므로)
        UpdateWeakPointLocation();
    }
    else
    {
        // 약점이 해제되면 타이머 중지
        GetWorldTimerManager().ClearTimer(WeakPointLocationTimerHandle);

        // 비주얼 초기화 (원래 색으로 복귀)
        UpdateMaterialVisual(FVector::ZeroVector, false);
    }
}

//void AVL_Boss1::UpdateWarpTarget()
//{
//
//    AActor* Target = GetTargetCharacter();
//
//    if (Target)
//    {
//        // 몽타주의 Warp Notify에서 설정한 Warp Target Name과 일치해야 합니다 (예: "CombatTarget")
//        // 위치와 회전값을 동시에 타겟 플레이어 기준으로 업데이트합니다.
//        MotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(
//            FName("CombatTarget"),
//            Target->GetActorLocation(),
//            Target->GetActorRotation()
//        );
//        CUSTOM_LOG("Warp Target Updated! Target Location: %s", *Target->GetActorLocation().ToString());
//    }
//    else
//    {
//        CUSTOM_LOG("Warp Target Update Failed: Target is NULL!");
//    }
//
//}


void AVL_Boss1::UpdateWeakPointLocation()
{
    if (ActiveWeakPoint == EPhysicalSurface::SurfaceType_Default)
    {
        GetWorldTimerManager().ClearTimer(WeakPointLocationTimerHandle);
        return;
    }
    if (!BossDynamicMaterial || !GetMesh() || !GetMesh()->GetPhysicsAsset()) return;

    for (const UBodySetup* BodySetup : GetMesh()->GetPhysicsAsset()->SkeletalBodySetups)
    {
        // 2. 해당 바디에 설정된 물리 재질(SurfaceType)이 현재 활성화된 약점과 같은지 확인
        if (BodySetup && BodySetup->PhysMaterial && BodySetup->PhysMaterial->SurfaceType == ActiveWeakPoint)
        {

            FBodyInstance* BodyInst = GetMesh()->GetBodyInstance(BodySetup->BoneName);

            if (BodyInst)
            {
                // 4. 피직스 바디의 정중앙(질량 중심) 좌표를 가져와 머티리얼에 쏩니다.
                FVector CenterLocation = BodyInst->GetCOMPosition();
                UpdateMaterialVisual(CenterLocation, true);

                return; // 찾았으니 함수 종료 (성능 최적화)
            }
        }
    }
}

void AVL_Boss1::OnRep_ActiveWeakPoint()
{
    ShowWeakPointVisual();
}


void AVL_Boss1::UpdateMaterialVisual(FVector Location, bool bIsActive)
{
    if (BossDynamicMaterial)
    {
        // 머티리얼 파라미터 업데이트
        BossDynamicMaterial->SetVectorParameterValue(FName("WeakPointPos"), Location);
        BossDynamicMaterial->SetScalarParameterValue(FName("IsWeakPointActive"), bIsActive ? 1.0f : 0.0f);

        FLinearColor BaseColor = FLinearColor::Black; // 평소 색상
        FLinearColor TargetColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);

        FLinearColor FinalColor = bIsActive ? FLinearColor::LerpUsingHSV(BaseColor, TargetColor, 0.5f) : BaseColor;

        BossDynamicMaterial->SetVectorParameterValue(FName("WeakPointColor"), FinalColor);

        ////  약점 부위 색상 설정
        //FLinearColor Color = bIsActive ? FLinearColor::Red : FLinearColor::Black;
        //BossDynamicMaterial->SetVectorParameterValue(FName("WeakPointColor"), Color);
    }
}

void AVL_Boss1::StartCombo()
{
    if (!HasAuthority() || bIsAttacking)
    {
        return;
    }

    UVL_BossMonsterDataAsset* BossData = GetBossDataAsset();

    int32 PatternsNum = BossData->ComboPatterns.Num();
    if (!BossData || PatternsNum == 0)
    {
        CUSTOM_LOG("Error: BossDataAsset is Null or ComboPatterns is Empty");
        return;
    }

    CurrentPatternIndex = FMath::RandHelper(PatternsNum);
    bIsAttacking = true;
    bCanContinueCombo = true;
    CurrentComboIndex = 0;
    bCanDash = true;

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        // 1. 진행 중이던 관성 및 이동 명령 강제 정지
        MoveComp->StopMovementImmediately();

    }
    //CUSTOM_LOG("현재 콤보 : %d", CurrentPatternIndex);
    const FComboData& SelectedPattern = BossData->ComboPatterns[CurrentPatternIndex];
    if (SelectedPattern.ComboSteps.Num() == 0)
    {
        CUSTOM_LOG("Error: Selected Pattern %d has no ComboSteps!", CurrentPatternIndex);
        bIsAttacking = false; // 상태 복구
        return;
    }
    FName FirstSectionName = SelectedPattern.ComboSteps[0].SectionName;
    Multi_PlayComboStep(FirstSectionName, true);
}

void AVL_Boss1::CheckNextCombo()
{

    if (!bIsAttacking || !HasAuthority()) return;

    if (!bCanContinueCombo)
    {
        EndCombo();
        return;
    }

    UVL_BossMonsterDataAsset* BossData = GetBossDataAsset();
    if (!BossData)
        return;

    const FComboData& ComboInfo = BossData->ComboPatterns[CurrentPatternIndex];

    int32 NextIndex = CurrentComboIndex + 1;

    AActor* Target = GetTargetCharacter();
    if (!ComboInfo.ComboSteps.IsValidIndex(NextIndex) || !IsValid(Target))
    {
        EndCombo();
        return;
    }

    float Distance = FVector::Distance(GetActorLocation(), Target->GetActorLocation());

    if (Distance > ComboInfo.ComboAttackRange)
    {
        //CUSTOM_LOG("Distance : %f, 거리 벗어나서 콤보 종료", Distance);
        EndCombo();
        return;
    }
    if (ComboInfo.ComboUseRootMotion != true)
    {
        GetWorldTimerManager().SetTimer(CombatMoveTimerHandle, this, &AVL_Boss1::UpdateCombatMovement, 0.01f, true);
    }
    CurrentComboIndex = NextIndex;
    FName NextSectionName = ComboInfo.ComboSteps[CurrentComboIndex].SectionName;

    // 서버에서 계산된 다음 섹션으로 모든 클라이언트를 점프시킴
    Multi_PlayComboStep(NextSectionName, false);
}

void AVL_Boss1::EndCombo()
{
    if (!HasAuthority() || !bIsAttacking) return;

    UAnimMontage* ResetMontageToPlay = nullptr;

    if (CachedBossData && CachedBossData->ComboPatterns.IsValidIndex(CurrentComboIndex))
    {
        const auto& CurrentSteps = CachedBossData->ComboPatterns[CurrentPatternIndex].ComboSteps;
        if (CurrentSteps.IsValidIndex(CurrentComboIndex))
        {
            ResetMontageToPlay = CurrentSteps[CurrentComboIndex].ComboResetMontage.LoadSynchronous();
        }
    }
    CurrentComboIndex = 0;
    bIsAttacking = false;
    bCanContinueCombo = false;

    Multi_EndCombo(ResetMontageToPlay);

    TArray<FName> Bones = { FName(TEXT("weapon_l")), FName(TEXT("weapon_r")) };
    Multicast_HideBones(Bones, false);

    if (OnComboEnded.IsBound())
    {
        //CUSTOM_LOG("콤보 태스크 종료");
        OnComboEnded.Broadcast();
    }
}

void AVL_Boss1::Multi_PlayComboStep_Implementation(FName SectionName, bool bIsFirstStep)
{
    UVL_BossMonsterDataAsset* BossData = GetBossDataAsset();
    if (!BossData || !BossData->ComboPatterns.IsValidIndex(CurrentPatternIndex)) return;

    const FComboData& SelectedPattern = BossData->ComboPatterns[CurrentPatternIndex];

    UAnimMontage* MontagePtr = SelectedPattern.ComboMontage.LoadSynchronous();
    if (!MontagePtr)
    {
        CUSTOM_LOG("Error: ComboMontage is not loaded yet!");
        return;

    }
    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
    if (!AnimInst) return;

    if (bIsFirstStep) 
    {

        PlayAnimMontage(MontagePtr);
    }

    if (AnimInst->Montage_IsPlaying(MontagePtr))
    {
        AnimInst->Montage_JumpToSection(SectionName, MontagePtr);
    }

    // 클라이언트에서도 애니메이션 변수 동기화
    if (UVL_CharacterAnimInstanceBase* BossAnimInst = Cast<UVL_CharacterAnimInstanceBase>(AnimInst))
    {
        BossAnimInst->bFullBody = true;
    }
}

void AVL_Boss1::Multi_EndCombo_Implementation(UAnimMontage* ResetMontage)
{
    bCanDash = false;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance)
    {
        if (UVL_CharacterAnimInstanceBase* BossAnimInst = Cast<UVL_CharacterAnimInstanceBase>(AnimInstance))
        {
            BossAnimInst->bFullBody = false;
        }

        if (ResetMontage)
        {
            PlayAnimMontage(ResetMontage);
            //AnimInstance->Montage_Play(ResetMontage);
        }
        else
        {
            // 설정된 리셋 몽타주가 없다면
            StopAllMontage(0.2f);
        }
        


    }
}

void AVL_Boss1::UpdateCombatMovement()
{
    AActor* Target = GetTargetCharacter();
    if (!bIsAttacking || !bCanRotate || !Target) return;

    //  회전 
    FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target->GetActorLocation());
    TargetRot.Pitch = 0.f;
    TargetRot.Roll = 0.f;
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRot, 0.01f, 5.0f));

    FVector DashDirection = GetActorForwardVector();

    if (bCanDash)
    {
        // 현재 속도를 서서히 높였다가 줄이는 방식 (변수 선언 필요: CurrentDashSpeed)
        // 시작할 때 DashSpeed까지 가속, 끝날 때 0으로 감속
        CurrentDashSpeed = FMath::FInterpTo(CurrentDashSpeed, DashSpeed, 0.01f, 10.f);

        AddActorWorldOffset(DashDirection * CurrentDashSpeed * 0.01f, true);
    }
}

bool AVL_Boss1::GetIsRushing() const
{
    return bIsRushing;
}

// 블랙보드 키의 있는 target 받아오기
AActor* AVL_Boss1::GetTargetCharacter() const
{
    if (!CachedAIC) return nullptr;

    UBlackboardComponent* BB = CachedAIC->GetBlackboardComponent();
    if (!BB) return nullptr;

    return Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));
}

void AVL_Boss1::RotateToTarget(float DeltaTime)
{
    if (!HasAuthority()) return;

    if (AActor* Target = GetTargetCharacter())
    {
        if (IsValid(Target))
        {
            FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target->GetActorLocation());
            // Yaw(좌우) 값만 사용하여 보정
            FRotator TargetRotation = FRotator(0.f, LookAtRotation.Yaw, 0.f);

            SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 10.0f));
        }
    }
}


void AVL_Boss1::BeginPlay()
{
    Super::BeginPlay();

    GetCapsuleComponent()->SetUseCCD(true);

    // 회전
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;
    //이동방향에 고개 돌리기
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 180.f, 0.f);

    CachedBossData = Cast<UVL_BossMonsterDataAsset>(CharacterDataAsset);
    // VFX데이터만 일단 비동기로드 
    // 렉걸리면 몽타주도 비동기로드 사용하기
    if (CachedBossData)
    {
        TSet<FSoftObjectPath> UniqueAssets;

        if (!CachedBossData->AxeClass.IsNull())
        {
            UniqueAssets.Add(CachedBossData->AxeClass.ToSoftObjectPath());
        }

        auto AddToSet = [&](const TSoftObjectPtr<UObject>& SoftPtr) {
            if (!SoftPtr.IsNull())
            {
                UniqueAssets.Add(SoftPtr.ToSoftObjectPath());
            }
            };

        if (CachedBossData->VFXDataAsset)
        {
            for (auto& Pair : CachedBossData->VFXDataAsset->BossVFXMap)
            {
                const auto& Data = Pair.Value;
                // 캐스팅 없이 바로 넣으면 컴파일러가 알아서 타입을 추론
                AddToSet(Data.NiagaraSystem);
                AddToSet(Data.Sound);
                AddToSet(Data.ImpactNiagara);
                AddToSet(Data.ImpactSound);
            }
        }

        for (const FBossPatternData& Pattern : CachedBossData->BossPatterns)
        {
            if (!Pattern.PatternMontage.IsNull())
            {
                UniqueAssets.Add(Pattern.PatternMontage.ToSoftObjectPath());
            }
        }

        for (const FComboData& Combo : CachedBossData->ComboPatterns)
        {
            if (!Combo.ComboMontage.IsNull())
            {
                UniqueAssets.Add(Combo.ComboMontage.ToSoftObjectPath());
                for (const FComboStepData& Step : Combo.ComboSteps)
                {
                    if (!Step.ComboResetMontage.IsNull())
                    {
                        UniqueAssets.Add(Step.ComboResetMontage.ToSoftObjectPath());
                    }
                }
            }
        }
        TArray<FSoftObjectPath> AssetsToLoad = UniqueAssets.Array();
        if (AssetsToLoad.Num() > 0)
        {
            TWeakObjectPtr<AVL_Boss1> WeakThis(this);

            // 중요: FStreamableDelegate::CreateLambda를 사용해야 인자 형식이 맞습니다.
            UAssetManager::GetStreamableManager().RequestAsyncLoad(
                AssetsToLoad,
                FStreamableDelegate::CreateLambda([WeakThis]() {
                    if (AVL_Boss1* StrongThis = WeakThis.Get())
                    {
                        // 로드 완료 후 실행할 로직 (예: AI 가동)
                        StrongThis->StartBossAI();
                    }
                    })
            );
        }
    }

    if (RushDamageDetector)
    {
        RushDamageDetector->IgnoreActorWhenMoving(this, true);
    }

    if (CachedBossData)
    {
        // 2. 타임라인 업데이트 함수 연결 (매 프레임 호출)
        FOnTimelineFloat ProgressFunction;
        ProgressFunction.BindUFunction(this, FName("HandleTimelineProgress"));
        EmergeTimeline->AddInterpFloat(CachedBossData->EmergeCurve, ProgressFunction);

        // 3. 타임라인 종료 함수 연결
        FOnTimelineEvent FinishedFunction;
        FinishedFunction.BindUFunction(this, FName("OnTimelineFinished"));
        EmergeTimeline->SetTimelineFinishedFunc(FinishedFunction);
    }

    UMaterialInterface* BaseMat = GetMesh()->GetMaterial(0);
    if (BaseMat)
    {
        BossDynamicMaterial = GetMesh()->CreateDynamicMaterialInstance(0, BaseMat);
    }

    if (CachedBossData && CachedBossData->PhaseSettings.Contains(EBossPhase::Phase_1))
    {
        // 고정형(MOVE_None)일 때만 구체 콜리전 생성
        if (CachedBossData->PhaseSettings[EBossPhase::Phase_1].MovementMode == MOVE_None)
        {
            // 1. 런타임 동적 생성 (NewObject 사용)
            USphereComponent* MonsterBlockSphere = NewObject<USphereComponent>(this, TEXT("DynamicBlockSphere"));

            if (MonsterBlockSphere)
            {
                // 2. 컴포넌트 부착 전 기본 세팅
                MonsterBlockSphere->SetSphereRadius(22.f); // 보스 크기에 맞게 조절
                MonsterBlockSphere->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

                MonsterBlockSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                MonsterBlockSphere->SetCollisionResponseToAllChannels(ECR_Ignore);

                MonsterBlockSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
                MonsterBlockSphere->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Block);

                // 내비메시에 구멍 뚫기
                MonsterBlockSphere->SetCanEverAffectNavigation(true);
                // 에디터 내 시각화
#if WITH_EDITOR
                MonsterBlockSphere->SetVisibility(true);
#endif

                // 4. 컴포넌트 등록 및 부착 (런타임 생성 시 필수)
                MonsterBlockSphere->RegisterComponent();
                MonsterBlockSphere->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

                UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
                if (NavSys)
                {
                    NavSys->UpdateComponentInNavOctree(*MonsterBlockSphere);
                }

                // 5. 보스 자신과의 충돌 원천 무시
                if (GetCapsuleComponent())
                {
                    GetCapsuleComponent()->IgnoreComponentWhenMoving(MonsterBlockSphere, true);
                    MonsterBlockSphere->IgnoreComponentWhenMoving(GetCapsuleComponent(), true);
                }
            }
        }

    }

    if (!HasAuthority())
    {
        return;
    }

    if (RushDamageDetector)
    {
        RushDamageDetector->OnComponentBeginOverlap.AddDynamic(this, &AVL_Boss1::OnRushOverlap);
    }

    // 고정형 보스모드일 때 weakpointcycle 타이머를 실행함
    if (CachedBossData->PhaseSettings[EBossPhase::Phase_1].MovementMode == EMovementMode::MOVE_None)
    {
        GetWorldTimerManager().SetTimer(WeakPointTimerHandle, this, &AVL_Boss1::StartWeakPointCycle, 10.0f, true);
    }
    InitializeGimmicks();

    if (CachedBossData && WeaponMesh)
    {
        // 데이터 에셋에 칼 SkeletalMesh 포인터가 있다고 가정
        // WeaponMesh->SetSkeletalMesh(CachedBossData->SwordMesh);

        // 시작할 때는 칼을 숨겨둘 수도 있습니다 (사격 페이즈라면)
        // WeaponMesh->SetHiddenInGame(true);
    }

}

void AVL_Boss1::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // 모든 클라이언트에게 이 변수들을 복제함
    DOREPLIFETIME(AVL_Boss1, ActiveWeakPoint);
    DOREPLIFETIME(AVL_Boss1, CurrentPhase);
    DOREPLIFETIME(AVL_Boss1, bBonesHidden);
    DOREPLIFETIME(AVL_Boss1, bIsRushing);

    DOREPLIFETIME(AVL_Boss1, CurrentPatternIndex);    // 헤더에 Replicated 선언된 것 반영
    DOREPLIFETIME(AVL_Boss1, CurrentComboIndex);     // 콤보 진행 상황 동기화
}

void AVL_Boss1::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    // AI 컨트롤러 저장 (나중에 비동기 로드 완료 후 사용)
    CachedAIC = Cast<AAIController>(NewController);

    //  비동기 로드 로직이 완료될 때까지 대기
}
//void AVL_Boss1::Tick(float DeltaTime)
//{
//    Super::Tick(DeltaTime);
//
//    // 공격 중일 때만 이동 및 회전 로직 실행
//    if (bIsAttacking)
//    {
//        UpdateCombatMovement(DeltaTime);
//    }
//
//}


void AVL_Boss1::StartBossAI()
{
    if (!HasAuthority() || !CachedAIC) return;

    FTimerHandle BossInitTimer;
    GetWorld()->GetTimerManager().SetTimer(BossInitTimer, [this]()
        {
            // 0.5초 뒤 유효성 검사 (보스가 그사이 죽거나 파괴되지 않았는지)
            if (!IsValid(this) || !IsValid(CachedAIC)) return;

            if (UVL_BossMonsterDataAsset* BossData = GetBossDataAsset())
            {
                // 1. BT 로드 (이미 비동기로 로드했을 것이므로 즉시 반환됨)
                UBehaviorTree* BTAsset = BossData->BehaviorTreeAsset.LoadSynchronous();
                if (!BTAsset) return;

                // 2. 블랙보드 초기화
                UBlackboardComponent* BB = nullptr;
                if (CachedAIC->UseBlackboard(BTAsset->GetBlackboardAsset(), BB))
                {
                    BB->SetValueAsEnum(FName("BossPhase"), (uint8)CurrentPhase);

                    FVector HomeLocation = GetActorLocation();
                    if (auto* MoveComp = GetCharacterMovement())
                    {
                        HomeLocation = MoveComp->GetActorFeetLocation();
                    }

                    // 3. 네비게이션 메시 투영 시도
                    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
                    FNavLocation ProjectedLocation;
                    if (NavSys && NavSys->ProjectPointToNavigation(HomeLocation, ProjectedLocation, FVector(500.f)))
                    {
                        BB->SetValueAsVector(TEXT("HomePos"), ProjectedLocation.Location);
                    }
                    else
                    {
                        // 실패 시 로그를 남겨서 디버깅에 활용
                        CUSTOM_LOG("Warning: Boss HomePos fallback to ActorLoc in Standalone");
                        BB->SetValueAsVector(TEXT("HomePos"), HomeLocation);
                    }

                    // 4. 모든 준비 후 실행
                    CachedAIC->RunBehaviorTree(BTAsset);

                    bBonesHidden = true;
                    UpdateBoneVisibility();

                    CUSTOM_LOG("Boss AI Started Safely");
                }
            }
        }, 0.5f, false);
}

bool AVL_Boss1::IsPatternReady(EAIBossPattern PatternType)
{/*
    if (!CooldownComponent || !CachedBossData) return true;*/

    if (!CooldownComponent)
    {
        CUSTOM_LOG(" 쿨다운 컴포넌트 없음");
        return true;
    }
    else if (!CachedBossData)
    {
        CUSTOM_LOG("데이터 에셋 없음");
        return true;
    }

    // 1. 데이터 에셋에서 이 PatternType(Enum)에 해당하는 태그를 찾습니다.
    FGameplayTag TargetTag;
    for (const FBossPatternData& Pattern : CachedBossData->BossPatterns)
    {
        if (Pattern.PatternType == PatternType)
        {
            TargetTag = Pattern.PatternTag;
            if (TargetTag.IsValid())
            {
                // CooldownComponent->IsReady(TargetTag)를 호출하면 
                // 내부적으로 다시 Boss1에게 GetCooldownDataProvider()를 호출하여 
                // 데이터 에셋의 쿨타임 수치를 가져와 계산
                return CooldownComponent->IsReady(TargetTag);
            }
            break;
        }
    }
    return true; 
}

void AVL_Boss1::OnPatternStarted(EAIBossPattern PatternType)
{
    if (!CooldownComponent || !CachedBossData) return;

    for (const FBossPatternData& Pattern : CachedBossData->BossPatterns)
    {
        if (Pattern.PatternType == PatternType && Pattern.PatternTag.IsValid())
        {

            CooldownComponent->StartCooldown(Pattern.PatternTag);
            break;
        }
    }
}

void AVL_Boss1::MoveToTarget(float DeltaTime, float MoveSpeed)
{
    AActor* Target = GetTargetCharacter();
    if (!Target) return;

    FVector Direction = (Target->GetActorLocation() - GetActorLocation());
    float Distance = Direction.Size();
    Direction.Z = 0.0f; // 높이 차이는 무시
    Direction.Normalize();

    // 플레이어와 너무 가깝지 않을 때만 전진 (예: 150 유닛 이상일 때만)
    const float SafeDistance = 150.0f;
    if (Distance > SafeDistance)
    {
        // AddActorWorldOffset을 사용하거나 CharacterMovement의 Velocity를 조절합니다.
        // Sweep을 true로 설정해야 벽이나 플레이어를 뚫지 않습니다.
        AddActorWorldOffset(Direction * MoveSpeed * DeltaTime, true);
    }
}

void AVL_Boss1::InitializeGimmicks()
{
    if (!HasAuthority() || !CachedBossData) return;

    const float Radius = CachedBossData->LoseTargetRange; // 보스의 에셋 할당 사거리
    const float RadiusSq = FMath::Square(Radius);
    const FVector BossLocation = GetActorLocation();

    TArray<AActor*> FoundActors;

    for (const FGimmickInfo& Info : CachedBossData->GimmickList)
    {
        if (!Info.GimmickClass) continue;

        FoundActors.Reset();
        // 특정 클래스의 액터를 월드에서 검색
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), Info.GimmickClass, FoundActors);

        for (AActor* Actor : FoundActors)
        {
            if (!Actor) continue;

            // 거리 제곱 비교 (최적화)
            if (FVector::DistSquared(BossLocation, Actor->GetActorLocation()) <= RadiusSq)
            {
                if (!ManagedGimmicks.Contains(Actor))
                {
                    AVL_BossGimmickActor* Gimmick = Cast<AVL_BossGimmickActor>(Actor);
                    if (Gimmick)
                    {
                        // TWeakObjectPtr 배열에 안전하게 추가

                        Gimmick->OnGimmickDestroyed.AddDynamic(this, &AVL_Boss1::HandleGimmickDestroyed);
                        Gimmick->OnStateChanged.AddDynamic(this, &AVL_Boss1::HandleGimmickStateChanged);

                        ManagedGimmicks.Emplace(Actor);

                    }
                }
            }
        }
    }

    //CUSTOM_LOG("보스 기믹 초기화 완료: 총 %d개의 기믹 관리 중", ManagedGimmicks.Num());

}

//SetGimmickState 호출시 함수 호출
void AVL_Boss1::HandleGimmickStateChanged(EGimmickState NewState)
{
    if (NewState == EGimmickState::Active)
    {
        // 약점 노출 시 로직
    }
}
// 기믹 체력이 0보다 작을 때 호출
void AVL_Boss1::HandleGimmickDestroyed(AActor* DestroyedGimmick)
{
}

void AVL_Boss1::OnRep_CurrentPhase(EBossPhase OldPhase)
{

    if (!CachedBossData) {
        CUSTOM_LOG("클라이언트: CachedBossData가 없습니다!");
        return;
    }
    const FPhaseSetting& Setting = CachedBossData->PhaseSettings[CurrentPhase];

    // 고정형보스이면 RelativeLocation 변경 하는 함수 넣어줌
    if (Setting.bUseTransitionTimeline)
    {
        StartEmergeTimeline();
        UCharacterMovementComponent* MoveComp = GetCharacterMovement();
        if (CurrentPhase == EBossPhase::Phase_1)
        {
            MoveComp->bUseControllerDesiredRotation = false;
            MoveComp->bOrientRotationToMovement = false;
        }
        else
        {
            GetCharacterMovement()->RotationRate = FRotator(0.f, 90.f, 0.f);
            GetCharacterMovement()->bOrientRotationToMovement = false;
            MoveComp->bUseControllerDesiredRotation = true;
        }

    }
    else
    {
        if (Setting.PhaseTransitionMontage)
        {
             PlayAnimMontage(Setting.PhaseTransitionMontage);

            //PlayAnimMontage(Setting.PhaseTransitionMontage);
            //float Duration = PlayAnimMontage(Setting.PhaseTransitionMontage);
             if (HasAuthority())
             {
                 UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
                 if (AnimInstance)
                 {
                     FOnMontageEnded EndDelegate;
                     EndDelegate.BindUObject(this, &AVL_Boss1::OnPhaseMontageEnded);
                     AnimInstance->Montage_SetEndDelegate(EndDelegate, Setting.PhaseTransitionMontage);
                 }
             }
 
        }
        else
        {
            // 연출이 아예 없는 보스라면 즉시 완료 처리
            if (HasAuthority() && OnPhaseChangeFinished.IsBound())
            {
                OnPhaseChangeFinished.Broadcast();
            }
        }
    }
}

void AVL_Boss1::OnPhaseMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (OnPhaseChangeFinished.IsBound())
    {
        OnPhaseChangeFinished.Broadcast();
    }
}

FVector AVL_Boss1::GetRandomNavLocation(float Radius)
{
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    FVector BossLoc = GetActorLocation();
    if (!NavSys)
    {
        return BossLoc + FVector(0.f, 100.f, 0.f);
    }


    FNavLocation OutLocation;
    if (NavSys->GetRandomReachablePointInRadius(GetActorLocation(), Radius, OutLocation))
    {
        
        for (int32 i = 0; i < 5; ++i)
        {
            float Dist = FVector::Dist(OutLocation, GetActorLocation());

            if (Dist > 100.f && Dist < CachedBossData->DetectionRange)
            {
                return OutLocation.Location;
            }
            NavSys->GetRandomReachablePointInRadius(GetActorLocation(), Radius, OutLocation);
        }

        return OutLocation.Location;
    }
    return BossLoc + FVector(0.f, 100.f, 0.f);
}

void AVL_Boss1::SpawnMinions(FGameplayTag SummonTag)
{
    if (!HasAuthority() || !CachedBossData) return;

    const FMinionSpawnData* FoundData = CachedBossData->MinionSpawnInfos.FindByPredicate(
        [&](const FMinionSpawnData& Info) { return Info.SpawnTag.MatchesTagExact(SummonTag); }
    );

    const FMinionSpawnData& Data = FoundData ? *FoundData : CachedBossData->MinionSpawnInfos[0];

    // 1. 데이터 에셋에서 해당 태그에 맞는 소환 정보를 찾음
    // 여기서는 예시로 0번 데이터를 쓴다고 가정
    
    TArray<FVector> ValidSpawnLocations;

    const float BossRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();

    const float SafeDistance = BossRadius + 100.0f; // 보스 캡슐 이외의 범위 설정
    const float SafeDistanceSq = FMath::Square(SafeDistance);

    const float MinSeparationDistanceSq = FMath::Square(Data.SpawnRadius / (float)(Data.SpawnCount * 2));
    const FVector BossLoc = GetActorLocation();
    for (int32 i = 0; i < Data.SpawnCount; ++i)
    {
        FVector SelectedLoc = FVector::ZeroVector;

        bool bFoundValidLocation = false;
        for (int32 Attempt = 0; Attempt < 10; ++Attempt)
        {
            SelectedLoc = GetRandomNavLocation(Data.SpawnRadius); // 일단 좌표 저장
            //float BossHalfHeight =  GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

            //SelectedLoc.Z = GetActorLocation().Z - BossHalfHeight;

            float DistToBossSq = FVector::DistSquaredXY(SelectedLoc, BossLoc);
            if (DistToBossSq < SafeDistanceSq)
            {

                continue; // 너무 가까우면 다시 찾기
            }

            bool bTooCloseToOthers = false;
            for (const FVector& SavedLoc : ValidSpawnLocations)
            {
                if (FVector::DistSquaredXY(SelectedLoc, SavedLoc) < FMath::Square(MinSeparationDistanceSq))
                {
                    bTooCloseToOthers = true;
                    break;
                }
            }
            if (!bTooCloseToOthers)
            {
                bFoundValidLocation = true;
                break;
            }
            else if (Attempt == 9)
            {
 
                //CUSTOM_LOG("최적 위치를 찾지 못했지만, 마지막 좌표에 강제 소환합니다.");
                bFoundValidLocation = true;
                break;
            }
        }
        if (bFoundValidLocation)
        {
            ValidSpawnLocations.Add(SelectedLoc);

            FActorSpawnParameters Params;
            Params.Owner = this;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

            AVL_AICharacterBase* Minion = GetWorld()->SpawnActor<AVL_AICharacterBase>(Data.MinionClass, SelectedLoc, FRotator::ZeroRotator, Params);
            if (Minion)
            {
                SummonedMinions.Add(Minion);
            }
        }
    }
    if (GetWorld())
    {
        // 소환 이후 30초 후에 데미지를 입는 함수
        GetWorldTimerManager().SetTimer(MinionPenaltyTimerHandle, this, &AVL_Boss1::CheckMinionsAndApplyPenalty, 30.0f, false);
    }
}

void AVL_Boss1::CheckMinionsAndApplyPenalty()
{
    // 1. 살아있는 미니언이 있는지 확인
    // SummonedMinions에서 무효한(이미 죽은) 포인터 제거
    SummonedMinions.RemoveAll([](TWeakObjectPtr<AVL_AICharacterBase> Minion) {
        return !Minion.IsValid();
        });

    // 2. 미니언이 남아있다면 전멸기/데미지 로직 실행
    if (SummonedMinions.Num() > 0)
    {
        if (AggroComponent)
        {
            // ThreatMap 순회하며 데미지 전달
            for (auto& Elem : *AggroComponent->GetThreatMap())
            {
                AActor* Victim = Elem.Key;
                if (IsValid(Victim))
                {
                    float Dist = FVector::Distance(GetActorLocation(), Victim->GetActorLocation());
                    if (Dist < CachedBossData->DetectionRange)
                    {
                        // 모든 적에게 강력한 데미지 (예: 9999 혹은 기획 수치)
                        UGameplayStatics::ApplyDamage(Victim, 5.0f, GetController(), this, UDamageType::StaticClass());

                        CUSTOM_LOG("%s에게 미니언 처리 실패 페널티 데미지 부여!", *Victim->GetName());
                    }
                }
            }
        }
    }

    // 타이머 핸들 초기화
    if (UWorld* World = GetWorld())
    {
        // 이 함수가 핸들을 0(Invalid)으로 만들고 엔진 스케줄러에서도 제거
        World->GetTimerManager().ClearTimer(MinionPenaltyTimerHandle);
    }
}

void AVL_Boss1::SetRushMode(bool Enable)
{
    if (HasAuthority())
    {
        bIsRushing = Enable;
        // 서버(호스트)는 직접 실행
        OnRep_IsRushing();


        if (Enable)
        {
            // 충돌 판정 활성화
            HitActors.Empty(); // 새로 돌진할 때마다 목록 초기화
            RushDamageDetector->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        }
        else
        {
            // 충돌 판정 비활성화
            RushDamageDetector->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
    }

}

bool AVL_Boss1::GetRushMode()
{
    return bIsRushing;
}

void AVL_Boss1::Multicast_PlayBossMontage_Implementation(UAnimMontage* Montage, float InPlayRate)
{
    if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
    {
        if (UVL_CharacterAnimInstanceBase* BossAnimInst = Cast<UVL_CharacterAnimInstanceBase>(AnimInst))
        {
            BossAnimInst->bFullBody = true; // 양쪽 모두 갱신
        }
        PlayAnimMontage(Montage, InPlayRate);
    }
}

void AVL_Boss1::StopBossMontage(float BlendOutTime)
{
    // 서버에서 멈춤
    if (GetMesh() && GetMesh()->GetAnimInstance())
    {
        GetMesh()->GetAnimInstance()->Montage_Stop(BlendOutTime);
    }
    // 클라이언트들도 멈추도록 멀티캐스트 호출
    Multicast_StopBossMontage(BlendOutTime);
}

void AVL_Boss1::Multicast_StopBossMontage_Implementation(float BlendOutTime)
{
    if (GetMesh() && GetMesh()->GetAnimInstance())
    {
        GetMesh()->GetAnimInstance()->Montage_Stop(BlendOutTime);
    }
}


FVector AVL_Boss1::CalculateRushKnockbackVelocity(ACharacter* TargetChar)
{
    if (!TargetChar) return FVector::ZeroVector;

    FVector BossLoc = GetActorLocation();
    FVector TargetLoc = TargetChar->GetActorLocation();

    FVector KnockbackDir = TargetLoc - BossLoc;
    KnockbackDir.Z = 0.0f;
    KnockbackDir.Normalize();

    float PushForce = 300.0f;
    float UpForce = 300.0f;

    return (KnockbackDir * PushForce) + (FVector::UpVector * UpForce);
}


void AVL_Boss1::StartEmergeTimeline()
{
    if (EmergeTimeline && EmergeTimeline->IsPlaying())
    {
        return;
    }

    UVL_BossMonsterDataAsset * Data =GetBossDataAsset();
    // 1. 데이터 에셋 및 커브 유효성 체크
    if (!Data || !Data->EmergeCurve) return;

    if (!Data->PhaseSettings.Contains(CurrentPhase))
    {
        CUSTOM_LOG("Error: PhaseSettings에 현재 페이즈 데이터가 없습니다!");
        return;
    }

    PlayAnimMontage(Data->PhaseSettings[CurrentPhase].PhaseTransitionMontage);

    if (EmergeTimeline)
    {
        EmergeTimeline->Stop();
        LerpStartZ = GetMesh()->GetRelativeLocation().Z;


        LerpEndZ = Data->PhaseSettings[CurrentPhase].MeshZOffset;

        // 3. 재생
        EmergeTimeline->PlayFromStart();
    }
}

void AVL_Boss1::HandleTimelineProgress(float Value)
{
    FVector NewRelativeLocation = GetMesh()->GetRelativeLocation();
    NewRelativeLocation.Z = FMath::Lerp(LerpStartZ, LerpEndZ, Value);
    GetMesh()->SetRelativeLocation(NewRelativeLocation);
}

void AVL_Boss1::OnTimelineFinished()
{
    if (HasAuthority()) // 서버에서만 물리/상태값 확정
    {
        UVL_BossMonsterDataAsset* Data = GetBossDataAsset();

        const FPhaseSetting& Setting = Data->PhaseSettings[CurrentPhase];
        UCharacterMovementComponent* MoveComp = GetCharacterMovement();

        if (MoveComp)
        {
            MoveComp->SetMovementMode(Setting.MovementMode);
            MoveComp->GravityScale = Setting.bUseGravity ? 1.0f : 0.0f;
        }
        //float CapsuleRadius = Setting.CapsuleRadius;
        //float CapsuleHalfHeight = Setting.CapsuleHalfHeight;

        //GetCapsuleComponent()->SetCapsuleSize(CapsuleRadius, CapsuleHalfHeight);

 

        // 델리게이트 브로드캐스트 
        if (OnPhaseChangeFinished.IsBound())
        {
            OnPhaseChangeFinished.Broadcast();
        }
    }

}

void AVL_Boss1::OnRushOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority()) return;

    //CUSTOM_LOG("Overlap Detected: %s", *OtherActor->GetName());

    if (!bIsRushing || !OtherActor || OtherActor == this) return;

    //CUSTOM_LOG("OnRushOverlap");
    // 이미 이번 돌진에 맞은 액터는 무시 (다단히트 방지)
    if (HitActors.Contains(OtherActor)) return;
    
    if (ACharacter* TargetChar = Cast<ACharacter>(OtherActor))
    {
        // 대상이 캐릭터라면 일단 목록에 추가 (무조건 한 번만 실행되도록)
        HitActors.Add(OtherActor);

        // 공통 로직: 넉백 실행
        FVector LaunchVel = CalculateRushKnockbackVelocity(TargetChar);

        // 4. 세부 판정: 플레이어(MainCharacter)인 경우에만 대미지 및 로그 처리
        if (AMainCharacterBase* Player = Cast<AMainCharacterBase>(TargetChar))
        {
            UGameplayStatics::ApplyDamage(Player, 10.0f, GetController(), this, UDamageType::StaticClass());
            //CUSTOM_LOG("%s(Player)가 보스의 돌진에 맞았습니다!", *Player->GetName());

            Player->ApplyKnockback(LaunchVel);
        }
        else
        {
            TargetChar->LaunchCharacter(LaunchVel, true, true);
        }
    }
}

void AVL_Boss1::OnRep_IsRushing()
{
    if (!CachedBossData) return;
    auto* MoveComp = GetCharacterMovement();
    if (!MoveComp) return;

    if (bIsRushing)
    {
        MoveComp->bOrientRotationToMovement = true;

        // 1. 속도 동기화 (예측 정확도 향상)
        MoveComp->MaxWalkSpeed = CachedBossData->BaseStats.MaxMoveSpeed * 3;

        // 2. 콜리전 동기화 (캐릭터 끼임/떨림 방지)
        GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    }
    else
    {
        // 4. 원래 상태로 복구
        MoveComp->MaxWalkSpeed = CachedBossData->BaseStats.MaxMoveSpeed;
        GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
        MoveComp->bOrientRotationToMovement = false;
    }
}

bool AVL_Boss1::ApplyGroggy(float DamageAmount)
{
    if (CurrentPhase == EBossPhase::Phase_1 && CachedBossData->PhaseSettings[EBossPhase::Phase_1].MovementMode == EMovementMode::MOVE_None)
    {
        return false;
    }
    return Super::ApplyGroggy(DamageAmount);
}

void AVL_Boss1::Die()
{
    if (!HasAuthority()) return;

    for (TWeakObjectPtr<AVL_AICharacterBase> MinionPtr : SummonedMinions)
    {
        if (MinionPtr.IsValid())
        {
            MinionPtr->Die();
            // 만약 미니언 클래스에도 Die()가 있다면 MinionPtr->Die() 호출 권장
        }
    }
    SummonedMinions.Empty();

    for (TWeakObjectPtr<AActor> GimmickPtr : ManagedGimmicks)
    {
        if (GimmickPtr.IsValid())
        {
            // 기믹 상태를 '파괴'나 '비활성화'로 변경
            GimmickPtr->Destroy();
        }
    }
    ManagedGimmicks.Empty();

    OnBossDeathEvent.Broadcast();

    Super::Die();
}

float AVL_Boss1::GetCooldownValue(FGameplayTag Tag) const
{
    if (CachedBossData)
    {
        return CachedBossData->GetCooldownValue(Tag);
    }
    else
    {
        //CUSTOM_LOG("보스의 데이터에셋 함수에 접근 못함");
    }
    // 보스 데이터가 없으면 부모의 기본 로직이라도 따릅니다.
    return Super::GetCooldownValue(Tag);
}

void AVL_Boss1::SetCurrentPhase(EBossPhase NewPhase)
{
    if (!HasAuthority() || CurrentPhase == NewPhase || !CachedBossData) return;
    if (!CachedBossData->PhaseSettings.Contains(NewPhase))
    {
        UE_LOG(LogTemp, Error, TEXT("PhaseSettings에 %d 페이즈 데이터가 없습니다!"), (int32)NewPhase);
        return;
    }
    EBossPhase OldPhase = CurrentPhase;
    CurrentPhase = NewPhase;

    OnRep_CurrentPhase(OldPhase);
}


void AVL_Boss1::SpawnPatternVFX(FGameplayTag PatternTag, FVector Location)
{
    if (!CachedBossData || !CachedBossData->VFXDataAsset) return;

    // 1. 구조체 통째로 찾기
    const FVLEffectInfo* EffectInfo = CachedBossData->VFXDataAsset->GetEffectInfoByTag(PatternTag);

    if (EffectInfo)
    {
        UNiagaraSystem* NS = EffectInfo->NiagaraSystem.LoadSynchronous();

        // 만약 Get()이 실패할 경우를 대비한 안전장치 (혹시 모를 상황 대비)
  
        if (NS)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                NS,
                Location,
                FRotator::ZeroRotator
            );
        }

        // 4. 사운드도 동일하게 처리
        USoundBase* Sound = EffectInfo->Sound.Get();
        if (!Sound) Sound = EffectInfo->Sound.LoadSynchronous();

        if (Sound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, Sound, Location);
        }
    }
}

void AVL_Boss1::SpawnMeteor(FGameplayTag PatternTag, FVector SpawnLocation)
{
    if (!CachedBossData || !CachedBossData->VFXDataAsset) return;

    // 1. 데이터 에셋에서 정보 찾기
    const FVLEffectInfo* EffectInfo = CachedBossData->VFXDataAsset->GetEffectInfoByTag(PatternTag);
    if (!EffectInfo) return;

   FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);

    // 3. 운석 액터 스폰
    AVL_MeteorProjectile* Meteor = GetWorld()->SpawnActorDeferred<AVL_MeteorProjectile>(
        AVL_MeteorProjectile::StaticClass(),
        SpawnTransform,
        this,
        GetInstigator(),
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn
    );



    if (Meteor)
    {
        float CustomRadius = EffectInfo->CollisionRadius;
        FVector Forward = GetActorForwardVector();

        // 1. Z축을 무시한 수평 벡터 생성
        FVector HorizontalDir = FVector(Forward.X, Forward.Y, 0.f);

        // 2. 다시 단위 벡터로 만듦 (보스가 수직으로 하늘/땅을 보지 않는 한 안전함)
        HorizontalDir = HorizontalDir.GetSafeNormal();

        FVector SpawnOffset = EffectInfo->SpawnOffset;

        // 3. MovingPower를 곱하면 보스의 고개 각도와 상관없이 일정한 전진 속도를 얻음
        FVector FinalVelocity = HorizontalDir * EffectInfo->MovingPower;
        FinalVelocity.Z = EffectInfo->ZSpeed;

        bool CanVFXMoving = EffectInfo->IsVFXMoving;
        // 4. 투사체 초기화 (VFX 정보, 타겟, 속도, 데미지 전달)
        Meteor->InitializeProjectile(CachedBossData->VFXDataAsset, PatternTag, 25.0f, CustomRadius, FinalVelocity, SpawnOffset, CanVFXMoving);

        Meteor->FinishSpawning(SpawnTransform);


    }
}

//void AVL_Boss1::SpawnMeteor(FGameplayTag PatternTag, FVector SpawnLocation)
//{
//    if (!CachedBossData || !CachedBossData->VFXDataAsset) return;
//
//    // 1. 데이터 에셋에서 정보 찾기
//    const FVLEffectInfo* EffectInfo = CachedBossData->VFXDataAsset->GetEffectInfoByTag(PatternTag);
//    if (!EffectInfo) return;
//
//    FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);
//
//    // 3. 운석 액터 스폰
//    AVL_MeteorProjectile* Meteor = GetWorld()->SpawnActorDeferred<AVL_MeteorProjectile>(
//        AVL_MeteorProjectile::StaticClass(),
//        SpawnTransform,
//        this,
//        GetInstigator(),
//        ESpawnActorCollisionHandlingMethod::AlwaysSpawn
//    );
//
//
//    if (Meteor)
//    {
//        // 4. 투사체 초기화 (VFX 정보, 타겟, 속도, 데미지 전달)
//        Meteor->InitializeProjectile(CachedBossData->VFXDataAsset, PatternTag, 25.0f);
//
//        Meteor->FinishSpawning(SpawnTransform);
//    }
//}

void AVL_Boss1::SpawnExplosive(FGameplayTag InLoopTag, FGameplayTag InExplosionTag, FVector SpawnLocation)
{
    if (!CachedBossData || !CachedBossData->VFXDataAsset) return;

    FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);

    AVL_Solo_Projectile* Projectile = GetWorld()->SpawnActorDeferred<AVL_Solo_Projectile>(
        AVL_Solo_Projectile::StaticClass(),
        SpawnTransform,
        this,
        GetInstigator(),
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn
    );

    if (Projectile)
    {
        Projectile->InitializeProjectile(CachedBossData->VFXDataAsset, InLoopTag, InExplosionTag);
        
        Projectile->FinishSpawning(SpawnTransform);
    }
}

void AVL_Boss1::SpawnLaser(FGameplayTag PatternTag, AActor* TargetActor)
{
    if (!CachedBossData || !CachedBossData->VFXDataAsset) return;

    // 1. 데이터 에셋에서 정보 찾기
    const FVLEffectInfo* EffectInfo = CachedBossData->VFXDataAsset->GetEffectInfoByTag(PatternTag);
    if (!EffectInfo) return;

    // 2. 시작 위치 설정 (보스의 손이나 머리 소켓)
    FVector StartLocation = GetMesh()->GetSocketLocation(TEXT("Muzzle_Laser"));

    // Pitch: -90(아래), Yaw: 보스의 현재 바라보는 방향, Roll: 0
    FRotator SpawnRotation = FRotator(-90.f, GetActorRotation().Yaw, 0.f);
    FTransform SpawnTransform(SpawnRotation, StartLocation);

    // 3. 레이저 액터 스폰 (Deferred 방식)
    AVL_LaserProjectile* Laser = GetWorld()->SpawnActorDeferred<AVL_LaserProjectile>(
        AVL_LaserProjectile::StaticClass(),
        SpawnTransform,
        this,
        GetInstigator(),
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn
    );

    if (Laser)
    {

        Laser->InitializeLaser(CachedBossData->VFXDataAsset, PatternTag, TargetActor, 2.0f, 100.f);

        Laser->FinishSpawning(SpawnTransform);
    }
}

void AVL_Boss1::SpawnLightning(FGameplayTag PatternTag, FVector SpawnLocation)
{
    if (!CachedBossData || !CachedBossData->VFXDataAsset) return;

    // 1. 데이터 에셋에서 정보 찾기
    const FVLEffectInfo* EffectInfo = CachedBossData->VFXDataAsset->GetEffectInfoByTag(PatternTag);
    if (!EffectInfo) return;

    FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);

    AVL_LightningProjectile* Lightning = GetWorld()->SpawnActorDeferred<AVL_LightningProjectile>(
        AVL_LightningProjectile::StaticClass(),
        SpawnTransform,
        this,
        GetInstigator(),
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn
    );


    if (Lightning)
    {
        // 4. 투사체 초기화 (VFX 정보, 타겟, 속도, 데미지 전달)
        Lightning->InitializeStrike(CachedBossData->VFXDataAsset, PatternTag, 25.0f);

        Lightning->FinishSpawning(SpawnTransform);
    }
}
