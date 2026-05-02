// Fill out your copyright notice in the Description page of Project Settings.


#include "Base/Actor/VL_InteractActorBase.h"

#include "Player/MainCharacterBase.h"

// Sets default values
AVL_InteractActorBase::AVL_InteractActorBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	InteractionBox->SetupAttachment(Mesh);

	InteractionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	InteractionBox->OnComponentBeginOverlap.AddDynamic(this, &AVL_InteractActorBase::OnBoxBeginOverlap);
	InteractionBox->OnComponentEndOverlap.AddDynamic(this, &AVL_InteractActorBase::OnBoxEndOverlap);
}

// Called when the game starts or when spawned
void AVL_InteractActorBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AVL_InteractActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AVL_InteractActorBase::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMainCharacterBase* Player = Cast<AMainCharacterBase>(OtherActor);
	
	if (!Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("BeginOverlap: Cast to MainCharacterBase Failed"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("BeginOverlap: %s set as CurrentInteractActor"), *GetName());
	Player->SetCurrentInteractorActor(this);
}

void AVL_InteractActorBase::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AMainCharacterBase* Player = Cast<AMainCharacterBase>(OtherActor);
	if (!Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("EndOverlap: Cast to MainCharacterBase Failed"));
		return;
	}

	if (Player->GetCurrentInteractActor() == this)
	{
		UE_LOG(LogTemp, Warning, TEXT("EndOverlap: %s cleared from CurrentInteractActor"), *GetName());
		Player->SetCurrentInteractorActor(nullptr);
	}
}

void AVL_InteractActorBase::OnInteracted_Implementation(AMainCharacterBase* InteractCharacter)
{
	if (bIsInteracted)
	{
		return;
	}

	bIsInteracted = true;
}

