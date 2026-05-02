#include "Base/Character/VL_AICharacterBase.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

#include "AIController.h"                // AICast 및 컨트롤러 기본 기능
#include "BehaviorTree/BlackboardComponent.h" // 블랙보드 값(GetValueAsObject 등) 읽기용
#include "BehaviorTree/BehaviorTree.h"

#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

// 서버 HP 헤더
#include "Net/UnrealNetwork.h"
// 회전 로직 함수용 헤더
#include "Kismet/KismetMathLibrary.h"

#include "NavigationSystem.h"
#include "Monster/Boss/VL_Boss1.h"
#include "Base/Data/Character/VL_BossMonsterDataAsset.h"
#include "Player/MainCharacterBase.h"
#include "CustomLog/CustomLog.h"

#include "Components/WidgetComponent.h"

// Sets default values
AVL_AICharacterBase::AVL_AICharacterBase()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    // 1. 무기 메시 컴포넌트 생성
    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    // 부모 메시(캐릭터 몸)에 부착
    WeaponMesh->SetupAttachment(GetMesh());

    // AI가 이동할 때 회전 방향으로 캐릭터가 부드럽게 돌아가도록 설정
    GetCharacterMovement()->bOrientRotationToMovement = true;
    bUseControllerRotationYaw = false; // 컨트롤러 회전에 몸 전체가 즉각 반응하지 않게 함 (BT의 Rotate 노드와 충돌 방지)


    // --- 메시 위치 및 회전 정렬 복원 ---
    // 1. 위치: 캡슐 절반 높이만큼 아래로 (보통 캡슐 Half Height가 90 내외)
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));

    // 2. 회전: 캐릭터 정면(화살표 방향)과 일치시키기 위해 Z축으로 -90도 회전
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));


    // 서버 HP 동기화
    bReplicates = true; // 캐릭터 복제 활성화
}

// Called when the game starts or when spawned
void AVL_AICharacterBase::BeginPlay()
{
    Super::BeginPlay();

    UVL_MonsterDataAsset* Data = GetMonsterDataAsset();
    if (!Data) return;

    // 1. 데이터 에셋을 기반으로 캐릭터 초기화


    if (!Data->WeaponSkeletalMesh.IsNull())
    {
        UStaticMesh* LoadedWeapon = Data->WeaponSkeletalMesh.LoadSynchronous();
        if (LoadedWeapon)
        {
            // 컴포넌트가 없다면 여기서 생성 (런타임 생성)
            if (!WeaponMesh)
            {
                WeaponMesh = NewObject<UStaticMeshComponent>(this, TEXT("DynamicWeaponMesh"));
                WeaponMesh->SetupAttachment(GetMesh()); // 임시 부착
                WeaponMesh->RegisterComponent();        // 월드에 등록 (필수!)
            }

            WeaponMesh->SetStaticMesh(LoadedWeapon);

            FName SocketName = Data->WeaponSocketName.IsNone() ? TEXT("WeaponSocket") : Data->WeaponSocketName;
            WeaponMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
            WeaponMesh->SetHiddenInGame(false);
        }
    }
    // 몽타주 메모리에 로드
    if (!Data->AttackMontage.IsNull())
    {
        Data->AttackMontage.LoadSynchronous();
    }
    if (!Data->HitMontage.IsNull())
    {
        Data->HitMontage.LoadSynchronous();
    }
    if (!Data->MonsterGroggyMontage.IsNull())
    {
        Data->MonsterGroggyMontage.LoadSynchronous();
    }
    if (!Data->DeathMontage.IsNull())
    {
        Data->DeathMontage.LoadSynchronous(); // 여기서 확실히 메모리에 올림
    }
    //// 무기 몽타주 메모리 로드 현재 무기 미존재라 주석처리
    //if (!Data->WeaponAttackMontage.IsNull())
    //{
    //    Data->WeaponAttackMontage.LoadSynchronous();
    //}
    //if (!Data->WeaponHitMontage.IsNull())
    //{
    //    Data->WeaponHitMontage.LoadSynchronous();
    //}
    //if (!Data->WeaponGroggyMontage.IsNull())
    //{
    //    Data->WeaponGroggyMontage.LoadSynchronous();
    //}
    //if (!Data->WeaponDeathMontage.IsNull())
    //{
    //    Data->WeaponDeathMontage.LoadSynchronous();
    //}


    if (!Data->WeaponAttackMontage.IsNull())
    {
        Data->WeaponAttackMontage.LoadSynchronous();
    }

    // 몬스터 크기 설정
    SetActorScale3D(FVector(Data->MonsterScale));

    // 스탯 설정
    if (HasAuthority()) // 서버에서만 실행
    {
        MaxHP = Data->BaseStats.MaxHP;
        CurrentHP = MaxHP;
    }

    BaseDamage = Data->BaseStats.AttackPower;
    BaseDefense = Data->BaseStats.Defense;

    RotationSpeed = Data->RotationRate;
    GetCharacterMovement()->RotationRate = FRotator(0.f, RotationSpeed, 0.f);

    UCapsuleComponent* MonsterCapsule = GetCapsuleComponent();
    if (MonsterCapsule)
    {
        // 물리 질량 설정
        MonsterCapsule->SetMassOverrideInKg(NAME_None, Data->Mass, true);

        MonsterCapsule->CanCharacterStepUpOn = ECB_No;
    }
    // 2. 캐릭터 무브먼트 컴포넌트 설정 (가장 중요)
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        // 플레이어나 다른 캐릭터가 밀 때 안 밀리게 함
        MoveComp->PushForceFactor = Data->PushForceFactor;

        if (Data->Mass > 500)
        {

            MoveComp->bEnablePhysicsInteraction = false;

            MoveComp->MaxTouchForce = 0.0f;

            MoveComp->InitialPushForceFactor = 0.0f;
            MoveComp->AvoidanceWeight = 1.0f;

            MoveComp->bIgnoreClientMovementErrorChecksAndCorrection = false;

            MoveComp->MaxDepenetrationWithPawn = 0.0f;
            MoveComp->MaxDepenetrationWithPawnAsProxy = 0.0f;
        }
    }

    HPWidgetComponent = FindComponentByClass<UWidgetComponent>();

    if (HPWidgetComponent)
    {
        GetWorldTimerManager().SetTimer(
            HPBarVisibilityTimerHandle,
            this,
            &AVL_AICharacterBase::UpdateHPBarVisibility,
            0.2f,
            true
        );

        UpdateHPBarVisibility();
    }
}

UAnimMontage* AVL_AICharacterBase::GetAttackMontage() const
{
    UVL_MonsterDataAsset* Data = GetMonsterDataAsset();

    if (!Data) return nullptr;

    UAnimMontage* Montage = Data->AttackMontage.LoadSynchronous();

    return Montage;
}

UVL_MonsterDataAsset* AVL_AICharacterBase::GetMonsterDataAsset() const
{
    return Cast<UVL_MonsterDataAsset>(CharacterDataAsset);
}

float AVL_AICharacterBase::GetMaxHP() const
{
    return MaxHP;
}
// maxhp랑 currenthp 이 값을 받으실건지 아니면 
float AVL_AICharacterBase::GetHPRatio() const
{
    float MaxHPVal = GetMaxHP();
    if (MaxHPVal > 0.f)
    {
        return FMath::Clamp(CurrentHP / MaxHPVal, 0.f, 1.0f);
    }
    return 0.f;
}

bool AVL_AICharacterBase::Attack()
{
    if (!HasAuthority())
    {
        return false;
    }
    if (bIsHit || bIsGroggy || bIsAttacking)
    {
        return false;
    }

    AAIController* AIC = Cast<AAIController>(GetController());
    if (!AIC)
    {
        CUSTOM_LOG("컨트롤러없음");
        return false;
    }

    if (AIC->GetBlackboardComponent())
    {
        AActor* Target = Cast<AActor>(AIC->GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")));
        if (Target)
        {
            float Distance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
            float AttackRange = GetMonsterDataAsset()->AttackRange;

            // 사거리를 벗어났다면 공격하지 않고 실패 리턴
            if (Distance > AttackRange)
            {
                //CUSTOM_LOG("Distance: %.2f, AttackRange : %.2f", Distance, AttackRange);
                return false;
            }
        }
        else
        {
            // 타겟이 없다면 공격 불가
            //CUSTOM_LOG("target is not");

            return false;
        }
    }

    bIsAttacking = true;
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately();
        GetCharacterMovement()->MaxWalkSpeed = 0.f;
    }

    const FName BossTag = FName(TEXT("Boss1"));
    if (!ActorHasTag(BossTag))
    {
        AIC->ClearFocus(EAIFocusPriority::Gameplay);

    }

    Multicast_PlayAttack();

    return true;
}

void AVL_AICharacterBase::Multicast_PlayAttack_Implementation()
{
    UAnimMontage* Montage = GetAttackMontage();
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

    if (Montage && AnimInstance)
    {
        AnimInstance->Montage_Play(Montage);

        // 종료 델리게이트 설정 (공격이 끝나면 다시 움직일 수 있게 함)
        if (HasAuthority())
        {
            FOnMontageEnded EndDelegate;
            EndDelegate.BindUObject(this, &AVL_AICharacterBase::HandleAttackMontageEnded);
            AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
        }
    }


}

void AVL_AICharacterBase::HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{

    if (!bIsAttacking)
    {
        return;
    }
    bIsAttacking = false;

    const FName BossTag = FName(TEXT("Boss1"));
    if (!ActorHasTag(BossTag))
    {
        AAIController* AIC = Cast<AAIController>(GetController());
        if (AIC)
        {
            // 블랙보드에서 타겟을 가져와 다시 포커스 설정
            UBlackboardComponent* BB = AIC->GetBlackboardComponent();
            if (BB)
            {
                AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));
                if (Target)
                {
                    AIC->SetFocus(Target);
                }
            }
        }
    }
    if (OnAttackFinished.IsBound())
    {
        OnAttackFinished.Broadcast();
    }
    // 속도 복구 (서버에서 이동 모드를 변경하면 클라이언트로 복제됨)

    EnableMovement();

}

void AVL_AICharacterBase::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    if (!HasAuthority()) return;

    // 보스 예외 처리
    const FName BossTag = FName(TEXT("Boss1"));
    if (ActorHasTag(BossTag)) return;

    // 데이터 확인
    UVL_MonsterDataAsset* Data = GetMonsterDataAsset();
    if (!Data || Data->BehaviorTreeAsset.IsNull()) return;

    AAIController* AIC = Cast<AAIController>(NewController);
    if (!AIC) return;

    // [핵심] 타이머를 사용하여 0.2초 정도 지연 실행 (Standalone 안정성 확보)
    // 람다 캡처에 필요한 변수들을 넘겨줍니다.
    FTimerHandle InitTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(InitTimerHandle, [this, AIC, Data]()
        {
            if (!IsValid(this) || !IsValid(AIC)) return;

            // 소프트 레퍼런스 로드
            UBehaviorTree* BTAsset = Data->BehaviorTreeAsset.LoadSynchronous();
            if (!BTAsset) return;

            UBlackboardComponent* BB = nullptr;
            if (AIC->UseBlackboard(BTAsset->GetBlackboardAsset(), BB))
            {
                FVector CurrentLocation = GetActorLocation();

                // 네비게이션 시스템 확인 및 투영 시도
                UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
                if (NavSys)
                {
                    FNavLocation ProjectedLocation;
                    // Standalone에서는 반경을 500 정도로 넉넉하게 잡는 것이 좋습니다.
                    if (NavSys->ProjectPointToNavigation(CurrentLocation, ProjectedLocation, FVector(500.f)))
                    {
                        BB->SetValueAsVector(TEXT("HomePos"), ProjectedLocation.Location);
                    }
                    else
                    {
                        BB->SetValueAsVector(TEXT("HomePos"), CurrentLocation);
                    }
                }

                AIC->RunBehaviorTree(BTAsset);
            }
        }, 0.5f, false);
}

void AVL_AICharacterBase::EnableMovement()
{
    // 다시 걷기 상태로 전환
    if (bIsGroggy || bIsHit || bIsDead)
    {
        return;
    }

    if (GetCharacterMovement())
    {
        if (GetCharacterMovement()->MovementMode != EMovementMode::MOVE_None)
        {
            GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);

            if (UVL_MonsterDataAsset* Data = GetMonsterDataAsset())
            {
                GetCharacterMovement()->MaxWalkSpeed = Data->BaseStats.MaxMoveSpeed;
            }
            else
            {
                GetCharacterMovement()->MaxWalkSpeed = 600.f; // 폴백(Fallback) 값
            }
        }
    }
    if (HasAuthority())
    {
        AAIController* AIC = Cast<AAIController>(GetController());
        if (AIC)
        {
            UBlackboardComponent* BB = AIC->GetBlackboardComponent();
            if (BB)
            {
                BB->SetValueAsBool(TEXT("bIsLocked"), false);

            }
        }
    }
}

void AVL_AICharacterBase::UpdateHPBarVisibility()
{
    if (!HPWidgetComponent)
    {
        return;
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC || !PC->PlayerCameraManager)
    {
        HPWidgetComponent->SetVisibility(false);
        return;
    }

    const float Distance = FVector::Dist(
        GetActorLocation(),
        PC->PlayerCameraManager->GetCameraLocation()
    );

    const bool bShouldShow = Distance <= HPBarVisibleDistance && CurrentHP > 0.f;

    HPWidgetComponent->SetVisibility(bShouldShow);
}

bool AVL_AICharacterBase::ApplyGroggy(float DamageAmount)
{
    const FName BossTag = FName(TEXT("Boss1"));

    if (!ActorHasTag(BossTag)) return false;


    if (bIsGroggy || bIsDead) return false;

    //UVL_MonsterDataAsset* Data = GetMonsterDataAsset();
    UVL_BossMonsterDataAsset* Data = Cast<UVL_BossMonsterDataAsset>(GetMonsterDataAsset());

    if (Data == nullptr)
    {
        return false;
    }

    // 게이지 누적
    CurrentGroggyGauge += DamageAmount;
    //CUSTOM_LOG("그로기 게이지 : %.2f 맥스 그로기 :%.2f ", CurrentGroggyGauge, Data->MaxGroggyGauge);

    // 그로기 발생 조건 체크
    if (CurrentGroggyGauge >= Data->MaxGroggyGauge)
    {
        CurrentGroggyGauge = 0.f;
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->StopMovementImmediately();
            GetCharacterMovement()->MaxWalkSpeed = 0.f;
        }
        return true;
    }
    return false;
}

void AVL_AICharacterBase::StopAttack()
{
    if (HasAuthority())
    {
        if (bIsAttacking)
        {
            if (OnAttackFinished.IsBound())
            {
                OnAttackFinished.Broadcast();
            }
        }
        bIsAttacking = false;
        EnableMovement(); // 이동 속도 복구
    }
    Multicast_StopAttack();
}

void AVL_AICharacterBase::Multicast_StopAttack_Implementation()
{

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance)
    {
        // 1. 현재 재생 중인 모든 몽타주를 즉시 중단 (블렌드 아웃 시간 0.2초)
        AnimInstance->Montage_Stop(0.2f);
    }
}

void AVL_AICharacterBase::DecreaseGroggyGauge(float Amount)
{
    if (!HasAuthority()) return;

    if (bIsGroggy || bIsDead)
    {
        return;
    }
    if (UVL_MonsterDataAsset* Data = GetMonsterDataAsset())
    {
        if (CurrentGroggyGauge <= 0.f) return;

        CurrentGroggyGauge = FMath::Clamp(CurrentGroggyGauge - Amount, 0.0f, Data->MaxGroggyGauge);

        //CUSTOM_LOG("그로기 게이지 감소 현재 그로기 게이지 : %.2f", CurrentGroggyGauge);
    }
}

void AVL_AICharacterBase::AttackCheck(bool bCanParry, bool bShouldKnockback)
{
    if (!HasAuthority()) return;

    UVL_MonsterDataAsset* Data = GetMonsterDataAsset();
    if (!Data) return;

    //// 1. 공격 위치 및 범위 설정 (박스 형태)
    FVector Forward = GetActorForwardVector();

    FVector BoxExtent = Data->AttackBoxSize;
    FVector Center = GetActorLocation() + Forward * BoxExtent.X;


    // 캐릭터가 바라보는 방향으로 박스 회전 시키기
    FQuat BoxRotation = GetActorQuat();

    TArray<FHitResult> OutHits;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    // 2. 박스 스윕 실행
    bool bHasHit = GetWorld()->SweepMultiByChannel(
        OutHits,
        Center, Center, // 제자리 체크
        BoxRotation,    // 캐릭터 회전값 적용
        ECC_Pawn,
        FCollisionShape::MakeBox(BoxExtent),
        Params
    );

    FColor DrawColor = bHasHit ? FColor::Red : FColor::Green;
    DrawDebugBox(GetWorld(), Center, BoxExtent, BoxRotation, DrawColor, false, 2.0f, 0, 2.0f);

    // 3. 중복 타격 방지 및 데미지 전달
    if (bHasHit)
    {
        BaseDamage = Data->BaseStats.AttackPower;
        TArray<AActor*> HitActors; // 이번 휘두름에 맞은 객체 저장

        for (const FHitResult& Hit : OutHits)
        {
            AMainCharacterBase* Victim = Cast<AMainCharacterBase>(Hit.GetActor());

            // 유효성 검사 + 장부에 없는 몬스터인지 확인
            if (Victim && !HitActors.Contains(Victim))
            {
                HitActors.Add(Victim); // 장부에 이름 적기

                // 실제 데미지 적용 (언리얼 표준 방식)
                UGameplayStatics::ApplyDamage(
                    Victim,           // 맞은 대상
                    BaseDamage,            // 데미지 양
                    GetController(),  // 공격자 컨트롤러
                    this,             // 데미지 원인 액터
                    nullptr           // 데미지 타입
                );
                if (bShouldKnockback)
                {
                    FVector LaunchDir = (Victim->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();

                    // 세기 적용 (수평 힘 + 수직 힘)
                    FVector LaunchVelocity = LaunchDir * Data->KnockbackStrength;
                    LaunchVelocity.Z = Data->KnockbackUpwardForce;

                    if (Victim->GetActionState() != ECharacterActionState::Rolling)
                    {
                        //CUSTOM_LOG("")
                        Victim->ApplyKnockback(LaunchVelocity);
                    }
                }

            }
        }
    }
}

void AVL_AICharacterBase::StopAllMontage(float BlendTime, UAnimMontage* SpecificMontage)
{
    if (HasAuthority())
    {
        Multicast_StopMontage(BlendTime, SpecificMontage);
    }

}

void AVL_AICharacterBase::Multicast_StopMontage_Implementation(float BlendTime, UAnimMontage* SpecificMontage)
{
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance)
    {
        if (SpecificMontage)
        {
            // 특정 몽타주가 재생 중일 때만 중단
            if (AnimInstance->Montage_IsPlaying(SpecificMontage))
            {
                AnimInstance->Montage_Stop(BlendTime, SpecificMontage);
            }
        }
        else
        {
            // 재생 중인 모든 몽타주 중단
            AnimInstance->Montage_Stop(BlendTime);
        }
    }
}

void AVL_AICharacterBase::HandleParrySuccess(AMainCharacterBase* Parrier)
{

    ApplyGroggy(10.f);

    // 2. 플레이어: 패링 성공 함수 호출
    if (Parrier)
    {
        // 플레이어 클래스 내에 정의된 성공 로직 실행 (이펙트, 사운드, 다음 연계기 활성화 등)
        //Parrier->OnParrySuccess(this);
    }
}

//void AVL_AICharacterBase::Multicast_PlayParriedReaction()
//{
//}

float AVL_AICharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // 서버(Authority)에서만 로직 수행
    if (HasAuthority() && ActualDamage > 0.f && CurrentHP > 0.f)
    {
        CurrentHP = FMath::Clamp(CurrentHP - ActualDamage, 0.f, MaxHP);
        //UE_LOG(LogTemp, Warning, TEXT("[BossHP] Damage=%.1f / HP=%.1f / MaxHP=%.1f"),
        //    ActualDamage, CurrentHP, MaxHP);
        OnHPChanged.Broadcast(CurrentHP, MaxHP); //서버용

        if (CurrentHP <= 0.f)
        {
            Die();
        }
        else
        {
            AAIController* AIC = Cast<AAIController>(GetController());
            UBlackboardComponent* BB = AIC ? AIC->GetBlackboardComponent() : nullptr;

            const FName BossTag = FName(TEXT("Boss1"));

            if (ApplyGroggy(ActualDamage))
            {
                SetGroggy(true); // 서버에서 상태 기록
                if (BB)
                {
                    BB->SetValueAsEnum(TEXT("AIState"), (uint8)EAIState::Groggy);
                }
                if (ActorHasTag(BossTag))
                {
                    // 보스는 BTTask에서 몽타주 기반으로 종료를 관리하므로 타이머를 설정하지 않거나,
                    // 매우 긴 시간(Safeguard)을 설정합니다.
                    GetWorld()->GetTimerManager().ClearTimer(GroggyRecoveryTimerHandle);
                }
                //// 3. 타이머 설정: 일정 시간 후 그로기 상태 해제
                //float GroggyDuration = 5.0f; // 기본 5초 대기
                //if (UVL_MonsterDataAsset* DataAsset = GetMonsterDataAsset())
                //{
                //    GroggyDuration = DataAsset->GroggyDuration;
                //}
                //GetWorld()->GetTimerManager().ClearTimer(GroggyRecoveryTimerHandle);
                //// 언리얼 타이머 매니저를 이용해 GroggyDuration 이후 EndGroggyState 호출
                //GetWorld()->GetTimerManager().SetTimer(
                //    GroggyRecoveryTimerHandle,
                //    this,
                //    &AVL_AICharacterBase::EndGroggyState,
                //    GroggyDuration,
                //    false // 반복하지 않음 (단발성)
                //);

                //Multicast_PlayGroggy(); // 비헤이비어 트리 태스크로 관리하기위해 주석처리
            }
            else
            {
                // 서버에서 상태 기록

               // 보스가 아닐 때만 hit 애니메이션 자유 실행
                if (!ActorHasTag(BossTag))
                {
                    bIsHit = true;

                    if (DamageCauser)
                    {
                        HitActor = DamageCauser->GetOwner();
                        //CUSTOM_LOG("HitActor name : %s", *HitActor->GetName());
                    }

                    if (BB) BB->SetValueAsBool(TEXT("bIsLocked"), true);

                    if (GetCharacterMovement())
                    {
                        GetCharacterMovement()->StopMovementImmediately();
                        GetCharacterMovement()->MaxWalkSpeed = 0.f;

                        Multicast_PlayHit();

                    }
                    UVL_MonsterDataAsset* Data = GetMonsterDataAsset();

                    if (Data->CanKnockback)
                    {
                        ApplyKnockback(DamageCauser);
                    }

                }
                else
                {
                    bIsHit = false;

                }
            }
        }
    }
    return ActualDamage;
}

void AVL_AICharacterBase::Multicast_PlayHit_Implementation()
{
    UAnimMontage* HitMontage = GetMonsterDataAsset()->HitMontage.LoadSynchronous();


    Internal_PlayMontage(HitMontage, false);
}

void AVL_AICharacterBase::StartGroggyDuration(float Duration)
{
    if (!HasAuthority()) return;

    // 1. 상태 설정 및 이동 정지
    SetGroggy(true);

    // 2. 애니메이션 실행 (멀티캐스트)
    Multicast_PlayGroggy();

    // 3. 기존에 혹시 돌고 있을지 모르는 회복 타이머는 제거
    GetWorld()->GetTimerManager().ClearTimer(GroggyRecoveryTimerHandle);

    // 4. 그로기 지속 시간 타이머 시작 (GroggyingTimerHandle 사용)
    GetWorld()->GetTimerManager().SetTimer(
        GroggyingTimerHandle,
        this,
        &AVL_AICharacterBase::EndGroggyState,
        Duration,
        false
    );
}

void AVL_AICharacterBase::Multicast_PlayGroggy_Implementation()
{
    UAnimMontage* GroggyAnim = GetMonsterDataAsset()->MonsterGroggyMontage.LoadSynchronous();
    if (bIsDead || !GroggyAnim) return;

    if (bIsAttacking) StopAttack();

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance)
    {
        AnimInstance->Montage_Play(GroggyAnim);
    }

}

void AVL_AICharacterBase::Internal_PlayMontage(UAnimMontage* MontageToPlay, bool bIsGroggyType)
{
    if (bIsDead) return;

    if (bIsAttacking)
    {
        StopAttack();
    }

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (MontageToPlay && AnimInstance)
    {
        AnimInstance->Montage_Play(MontageToPlay);

        FOnMontageEnded EndDelegate;
        if (!bIsGroggyType)
        {
            EndDelegate.BindUObject(this, &AVL_AICharacterBase::OnHitMontageEnded);
        }
        //Groggy 태스크가 없을 때
        else
        {
            CUSTOM_LOG("BindUObject(this, &AVL_AICharacterBase::OnGroggyMontageEnded");
            EndDelegate.BindUObject(this, &AVL_AICharacterBase::OnGroggyMontageEnded);
        }

        AnimInstance->Montage_SetEndDelegate(EndDelegate, MontageToPlay);
    }
    else
    {
        if (!bIsGroggyType)
        {
            OnHitMontageEnded(nullptr, false); // 즉시 피격 상태 종료 및 AI 재가동
        }
        else
        {
            // 그로기 몽타주가 없어도 태스크 종료 신호는 보내줘야 함
            OnGroggyMontageEnded(nullptr, false);
        }
    }
}

float AVL_AICharacterBase::PlayGroggyMontage()
{
    if (bIsDead) return 0.f;

    UAnimMontage* GroggyAnim = GetMonsterDataAsset()->MonsterGroggyMontage.LoadSynchronous();
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

    if (GroggyAnim && AnimInstance)
    {
        if (bIsAttacking) StopAttack();

        // 애니메이션 재생 (멀티캐스트는 태스크에서 호출하거나 여기서 수행)
        return AnimInstance->Montage_Play(GroggyAnim);
    }
    return 0.f;
}

void AVL_AICharacterBase::OnHitMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    HitEnded();
}
// 비헤이비어 트리 groggy 태스크가 없다면,(몬스터 내부 그로기 정리 함수)
void AVL_AICharacterBase::OnGroggyMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
//    if (HasAuthority())
//    {
//        bIsGroggy = false;
//        CurrentGroggyGauge = 0.f; // 안전하게 게이지 초기화
//        if (AAIController* AIC = Cast<AAIController>(GetController()))
//        {
//            if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
//            {
//                // 그로기가 끝났으므로 상태를 초기화하여 서비스가 새로 판단하게 함
//                BB->SetValueAsEnum(TEXT("AIState"), (uint8)EAIState::Idle);
//            }
//        }
//    }
//    EnableMovement();
}

void AVL_AICharacterBase::HitEnded()
{
    if (HasAuthority())
    {
        bIsHit = false;
    }
    const FName BossTag = FName(TEXT("Boss1"));
    // 보스가 아닐 때만 맞고 바라보는 로직
    if (!ActorHasTag(BossTag))
    {
        StartRotatingToTarget();
    }

    EnableMovement();
}

void AVL_AICharacterBase::SetGroggy(bool IsGroggy)
{
    bIsGroggy = IsGroggy;

    if (IsGroggy)
    {
        // 그로기 시작 시
        if (GetCharacterMovement())
        {
            // 즉시 정지 및 이동 불가 처리
            GetCharacterMovement()->StopMovementImmediately();
            GetCharacterMovement()->MaxWalkSpeed = 0.f;
            //GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
        }

        // 공격 중이었다면 중단
        if (bIsAttacking)
        {
            StopAttack();
        }
    }
    else
    {
        CurrentGroggyGauge = 0.f;
        EnableMovement();
    }
}

void AVL_AICharacterBase::ApplyKnockback(AActor* DamageCauser)
{
    // 서버에서만 실행
    if (!HasAuthority() || !DamageCauser || !GetCharacterMovement())
    {
        return;
    }

    // 1. 방향 계산: 공격자로부터 나에게로 향하는 벡터
    FVector KnockbackDirection = GetActorLocation() - DamageCauser->GetActorLocation();
    KnockbackDirection.Z = 0.f; // 위로 솟구치지 않게 Z축 보정
    KnockbackDirection.Normalize();

    // 2. 힘 조절 (데이터 에셋에 KnockbackForce 변수를 추가하면 더 좋습니다)
    UVL_MonsterDataAsset* Data = GetMonsterDataAsset();
    if (Data->KnockbackResistance == 1.f)
    {
        return;
    }

    float FinalForce = Data->KnockbackForce * (1.0f - Data->KnockbackResistance);



    FVector LaunchVelocity = KnockbackDirection * FinalForce;

    // 3. 캐릭터 날리기
    // bXYOverride: true면 기존 속도를 무시하고 대입, false면 기존 속도에 더함
    LaunchCharacter(LaunchVelocity, true, false);
}

void AVL_AICharacterBase::Multicast_OnDeath_Implementation()
{

    // 1. 충돌 끄기 (캡슐 컴포넌트)
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    //GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    UVL_MonsterDataAsset* Data = GetMonsterDataAsset();

    if (Data)
    {
        // 보스가 공격 중이었다면 중단하고 사망 애니메이션 우선 재생
        UAnimMontage* DeathAnim = Data->DeathMontage.LoadSynchronous();
        if (DeathAnim)
        {
            StopAnimMontage();

            PlayAnimMontage(DeathAnim);

        }
    }
    else
    {
        // 몽타주가 없을 경우를 대비한 기본 Ragdoll 혹은 애니메이션 상태 설정
        GetMesh()->SetSimulatePhysics(true);
    }
}

void AVL_AICharacterBase::Die()
{
    if (!HasAuthority()) return;

    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->StopMovementImmediately();        // 현재 속도 즉시 정지
        MoveComp->GravityScale = 0.0f;              // 중력 영향을 0으로 설정
        MoveComp->SetMovementMode(MOVE_None);       // 이동 로직 자체를 끔
    }

    // 1. AI 정지 (서버에서만 관리)
    if (AAIController* AIC = Cast<AAIController>(GetController()))
    {
        if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
        {
            BB->SetValueAsBool(TEXT("IsDead"), true);
        }
        AIC->StopMovement();

        if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(AIC->GetBrainComponent()))
        {
            BTComp->StopTree(EBTStopMode::Safe);
        }
    }

    // 2. 모든 클라이언트에게 사망 연출 지시
    Multicast_OnDeath();

    if (UWorld* World = GetWorld())
    {
        // 1. 모든 타이머 제거 (자식 클래스 포함)
        World->GetTimerManager().ClearAllTimersForObject(this);

        // 반드시 Clear 이후에 호출
        SetLifeSpan(2.0f);
    }
}


// 네트워크 복제 규칙 정의
void AVL_AICharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // CurrentHP를 모든 클라이언트에게 복제함
    DOREPLIFETIME(AVL_AICharacterBase, CurrentHP);
    DOREPLIFETIME(AVL_AICharacterBase, MaxHP);
    DOREPLIFETIME(AVL_AICharacterBase, bIsAttacking);
    DOREPLIFETIME(AVL_AICharacterBase, bIsHit);
    DOREPLIFETIME(AVL_AICharacterBase, bIsGroggy);
    DOREPLIFETIME(AVL_AICharacterBase, bIsDead);
}

void AVL_AICharacterBase::ApplyHealing()
{
    // 리슨 서버(Authority)에서만 체력을 직접 수정함
    if (!HasAuthority()) return;
    //테스트 빼기
    CurrentHP = FMath::Clamp(CurrentHP + 10.f, 0.0f, MaxHP);

    // 리슨 서버 본인의 UI 갱신을 위해 직접 호출 (OnRep은 서버 본인에겐 호출 안됨)

    OnRep_CurrentHP();

}

void AVL_AICharacterBase::ApplyHealing(float HealAmount)
{
    // 리슨 서버(Authority)에서만 체력을 직접 수정함
    if (!HasAuthority()) return;
    //테스트 빼기
    CurrentHP = FMath::Clamp(CurrentHP + HealAmount, 0.0f, MaxHP);

    // 리슨 서버 본인의 UI 갱신을 위해 직접 호출 (OnRep은 서버 본인에겐 호출 안됨)
    if (HealAmount != 0.0f)
    {
        OnRep_CurrentHP();

    }
}

void AVL_AICharacterBase::EndGroggyState()
{
    if (HasAuthority())
    {
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().ClearTimer(GroggyingTimerHandle);
        }

        bIsGroggy = false;
        CurrentGroggyGauge = 0.f;
    }
    if (AAIController* AIC = Cast<AAIController>(GetController()))
    {
        if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
        {
            // 상태를 Combat으로 변경 -> BT Decorator가 감지하여 Task를 Abort시킴
            CUSTOM_LOG("그로기 끝 combat");
            BB->SetValueAsEnum(TEXT("AIState"), (uint8)EAIState::Combat);
        }
    }

    // 애니메이션 중단 및 이동 복구
    StopAllMontage(0.25f, GetMonsterDataAsset()->MonsterGroggyMontage.LoadSynchronous());

    EnableMovement();
}

void AVL_AICharacterBase::StartRotatingToTarget()
{
    // 노티파이에 의해 호출되는 시점부터 회전 로직 활성화
    if (HitActor)
    {
        AAIController* AIC = Cast<AAIController>(GetController());
        if (AIC)
        {
            // 1. 상태 변경 (BT가 Combat 섹션의 로직을 타게 함)
            if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
            {
                BB->SetValueAsEnum(TEXT("AIState"), (uint8)EAIState::Chase);
                // 필요하다면 타겟 정보도 블랙보드에 전달
                BB->SetValueAsObject(TEXT("TargetActor"), HitActor);
            }

            // 2. 이동 중단 (중요: 가던 길을 멈춰야 제자리에서 회전에 집중함)
            AIC->StopMovement();
        }

        bIsRotating = true;
        RotationCurrentTime = 0.0f;
        SetActorTickEnabled(true);

    }
}

void AVL_AICharacterBase::OnRep_CurrentHP()
{
    OnHPChanged.Broadcast(CurrentHP, MaxHP); //클라용
}

void AVL_AICharacterBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (bIsRotating && HitActor)
    {
        RotationCurrentTime += DeltaTime; // 누적 시간 업데이트

        FRotator CurrentRot = GetActorRotation();
        // HitActor가 Tick 도중 Destroy될 가능성에 대비해 IsValid 체크 권장
        if (!IsValid(HitActor))
        {
            StopRotationLogic();
            return;
        }

        FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), HitActor->GetActorLocation());
        TargetRot.Pitch = 0.f;
        TargetRot.Roll = 0.f;

        FRotator NewRot = FMath::RInterpConstantTo(CurrentRot, TargetRot, DeltaTime, RotationSpeed);
        SetActorRotation(NewRot);

        // 종료 조건 1: 목표 각도 도달 (오차 범위 5~10도)
        float AngleDelta = FMath::Abs(FMath::FindDeltaAngleDegrees(CurrentRot.Yaw, TargetRot.Yaw));

        // 종료 조건 2: 최대 허용 시간 초과 (타임아웃)
        if (AngleDelta < 10.0f || RotationCurrentTime > MaxRotationTime)
        {
            StopRotationLogic();
        }
    }
}

void AVL_AICharacterBase::StopRotationLogic()
{
    //CUSTOM_LOG("회전 로직 종료");
    bIsRotating = false;
    HitActor = nullptr;
    RotationCurrentTime = 0.0f; // 시간 초기화
    SetActorTickEnabled(false);
}
