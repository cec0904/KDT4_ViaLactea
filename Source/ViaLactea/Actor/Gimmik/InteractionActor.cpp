#include "InteractionActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Base/Component/VL_InteractComponent.h"

//#include "../Player/MainCharacterBase.h"
//#include "../ViaLacteaCharacter.h"
#include "../../CustomLog/CustomLog.h"

// Sets default values
AInteractionActor::AInteractionActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    

    // 1. 루트 컴포넌트 설정
    BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
    RootComponent = BaseMesh;

    BaseMesh->SetCollisionProfileName(TEXT("BlockAll"));

    InteractionWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionWidget"));
    InteractionWidget->SetupAttachment(RootComponent);
    InteractionWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 70.0f));

    InteractionWidget->SetWidgetSpace(EWidgetSpace::World);
    InteractionWidget->SetDrawSize(FVector2D(150.0f, 50.0f));

    InteractionWidget->SetVisibility(false);
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> XRayMaterial(TEXT("/Game/Test/KHB/EngineMaterials/M_Widget_XRay.M_Widget_XRay"));

    if (XRayMaterial.Succeeded())
    {
        // 2. 위젯 컴포넌트에 머티리얼 적용
        // 인덱스 0번은 위젯의 앞면(Face)
        InteractionWidget->SetMaterial(0, XRayMaterial.Object);
    }
    ///////////////////260311
    InteractComponent = CreateDefaultSubobject<UVL_InteractComponent>(TEXT("InteractComponent"));
    InteractComponent->InteractionWidget = InteractionWidget;

}

// Called when the game starts or when spawned
void AInteractionActor::BeginPlay()
{
    Super::BeginPlay();

    //// 서버와 클라이언트 모두에서 Overlap 이벤트를 감지하도록 바인딩
    //if (BaseMesh)
    //{
    //    BaseMesh->OnComponentBeginOverlap.AddDynamic(this, &AInteractionActor::OnOverlapBegin);
    //    BaseMesh->OnComponentEndOverlap.AddDynamic(this, &AInteractionActor::OnOverlapEnd);

    //    // 충돌 설정 확인 (코드에서 처리 주석- 에디터 콜리전 프로파일 세팅)
    //    //BaseMesh->SetCollisionProfileName(TEXT("Trigger")); // 혹은 "OverlapAllDynamic"
    //    //BaseMesh->SetGenerateOverlapEvents(true);
    //}

    // 3. 델리게이트에 함수 바인딩 (컴포넌트에서 상호작용 신호를 보내면 HandleOnInteract가 실행됨)
    if (InteractComponent)
    {
        InteractComponent->OnInteractEvent.AddDynamic(this, &AInteractionActor::HandleOnInteract);
    }
}
void AInteractionActor::HandleOnInteract(AActor* Interactor)
{
    // NPC 고유의 로직: 대화 시작 등
    CUSTOM_LOG("상호작용 성공");
}


//void AInteractionActor::ShowInteractionWidget(bool bShow)
//{
//    if (InteractionWidget)
//    {
//        InteractionWidget->SetVisibility(bShow);
//    }
//}
//
//void AInteractionActor::OnInteract_Implementation() // _Implementation을 반드시 붙여야 함
//{
//    //CUSTOM_LOG("Interaction Executed on Server");
//}
//
//void AInteractionActor::Tick(float DeltaTime)
//{
//    Super::Tick(DeltaTime);
//    if (GetNetMode() == NM_DedicatedServer) return;
//
//    // 위젯이 보일 때만 회전 로직 계산 (성능 최적화)
//    if (InteractionWidget && InteractionWidget->IsVisible())
//    {
//        // 1. 로컬 플레이어의 카메라 매니저 가져오기
//        APlayerController* PC = GetWorld()->GetFirstPlayerController();
//
//        if (PC && PC->PlayerCameraManager)
//        {
//            // 2. 카메라의 위치 정보 가져오기
//            FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
//            FVector WidgetLocation = InteractionWidget->GetComponentLocation();
//
//            // 3. 카메라를 향하는 방향 계산
//            FVector Direction = CameraLocation - WidgetLocation;
//            FRotator TargetRot = Direction.Rotation();
//
//
//            FRotator NewRotation = FRotator(0.f, TargetRot.Yaw, 0.f);
//            InteractionWidget->SetWorldRotation(NewRotation);
//        }
//    }
//}

void AInteractionActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void AInteractionActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}