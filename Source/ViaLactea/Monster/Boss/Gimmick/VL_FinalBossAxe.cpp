#include "Monster/Boss/Gimmick/VL_FinalBossAxe.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/MainCharacterBase.h"
#include "Base/Character/VL_AICharacterBase.h"

#include "CustomLog/CustomLog.h"

// Sets default values
AVL_FinalBossAxe::AVL_FinalBossAxe()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(true); // 위치 동기화 활성화
    //NetUpdateFrequency = 100.0f;
    // 1. 충돌체 설정
    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComponent->InitSphereRadius(30.0f);
    CollisionComponent->SetCollisionProfileName(TEXT("Projectile")); 
    CollisionComponent->SetGenerateOverlapEvents(true);
    CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);

    RootComponent = CollisionComponent;

    // 2. 메쉬 설정
    AxeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AxeMesh"));
    AxeMesh->SetupAttachment(RootComponent);

    AxeMesh->SetRelativeRotation(FRotator(0.f, 0.0f, -90.f));
    AxeMesh->SetRelativeLocation(FVector(- 60.f, 110.0f, -110.f));

    AxeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 충돌은 구체 컴포넌트가 담당
    
    AxeMesh->SetCastShadow(false);
    
    // 3. 발사체 컴포넌트
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
    ProjectileMovement->UpdatedComponent = CollisionComponent;
    ProjectileMovement->InitialSpeed = 1000.f;
    ProjectileMovement->MaxSpeed = 2000.f;
    ProjectileMovement->bRotationFollowsVelocity = false; // 회전 컴포넌트와 충돌 방지
    ProjectileMovement->ProjectileGravityScale = 0.0f;     // 필요시 중력 적용 (0이면 직선)

    // 4. 회전 컴포넌트 
    RotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingComp"));
    RotatingMovement->RotationRate = FRotator(-360.0f, 0, 0); // 초당 2바퀴 회전 (Roll/Pitch/Yaw는 메쉬 방향에 맞춰 조정)
}

void AVL_FinalBossAxe::BeginPlay()
{
	Super::BeginPlay();

    if (HasAuthority())
    {
        CollisionComponent->OnComponentBeginOverlap.RemoveDynamic(this, &AVL_FinalBossAxe::OnAxeOverlap);

        CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AVL_FinalBossAxe::OnAxeOverlap);

        CollisionComponent->OnComponentHit.AddDynamic(this, &AVL_FinalBossAxe::OnAxeHit);
    }

    // 5초 뒤 자동 파괴 (허공에 날아갔을 때 대비)
    SetLifeSpan(5.0f);
}

//// Called every frame
//void AVL_FinalBossAxe::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}

void AVL_FinalBossAxe::FireInDirection(const FVector& ShootDirection)
{
    if (ProjectileMovement)
    {
        ProjectileMovement->Velocity = ShootDirection * ProjectileMovement->InitialSpeed;
    }
}

void AVL_FinalBossAxe::OnAxeOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    // 1. 자기 자신이나 주인이 아닌 경우에만 처리
    if (OtherActor && OtherActor != this && OtherActor != GetOwner())
    {
        if (AMainCharacterBase* Player = Cast<AMainCharacterBase>(OtherActor))
        {
            if (Player->GetActionState() == ECharacterActionState::Rolling)
            {
                //CUSTOM_LOG("구르기 상태라 통과");
                return;
            }
            // 3. 언리얼 표준 데미지 시스템 호출
            UGameplayStatics::ApplyDamage(
                OtherActor,           // 데미지를 받을 액터
                DamageAmount,         // 데미지 양
                GetInstigatorController(), // 데미지를 가한 컨트롤러 (보스)
                this,                 // 데미지 유발자 (도끼)
                UDamageType::StaticClass() // 데미지 타입
            );

            // 4. 충돌 후 도끼 제거
            Destroy();
        }
    }
}

void AVL_FinalBossAxe::OnAxeHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (OtherActor && OtherActor != GetOwner())
    {
        Destroy();
    }
}

//void AVL_FinalBossAxe::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//}

