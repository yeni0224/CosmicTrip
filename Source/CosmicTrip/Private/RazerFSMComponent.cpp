// Fill out your copyright notice in the Description page of Project Settings.


#include "RazerFSMComponent.h"
#include "Boss.h"
#include "BulletActor.h"
#include "CloseAttackEnemy.h"
#include "CloseAttackEnemyFSM.h"
#include "CosmicPlayer.h"
#include "RazerRobot.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
URazerFSMComponent::URazerFSMComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void URazerFSMComponent::BeginPlay()
{
	Super::BeginPlay();

	me = Cast<ARazerRobot>(GetOwner());
	
	player = Cast<ACosmicPlayer>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));

	boss = Cast<ABoss>(UGameplayStatics::GetActorOfClass(GetWorld(), ABoss::StaticClass()));

	bullet = Cast<ABulletActor>(UGameplayStatics::GetActorOfClass(GetWorld(), ABulletActor::StaticClass()));

	SetRazerState(ERazerState::IDLE);

	me->GetCharacterMovement()->MaxFlySpeed = 300.0f;
}


// Called every frame
void URazerFSMComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	switch (razerState)
	{
	case ERazerState::IDLE:
		TickIdle();
		break;
	case ERazerState::MOVE:
		TickMove();
		break;
	case ERazerState::PATROL:
		TickPatrol();
		break;
	case ERazerState::ATTACK:
		TickAttack();
		break;
	case ERazerState::DAMAGE:
		TickDamage();
		break;
	case ERazerState::DIE:
		TickDie();
		break;
	}
}

void URazerFSMComponent::TickIdle()
{
	if (player !=nullptr)
	{
		SetRazerState(ERazerState::MOVE);
	}
}

void URazerFSMComponent::TickMove()
{
	FVector start = me->GetActorLocation();

	FVector target = player->GetActorLocation() + FVector(0, 0, 150.0f);

	FRotator rot = UKismetMathLibrary::FindLookAtRotation(start, target);

	FVector targetloc = UKismetMathLibrary::Conv_RotatorToVector(FRotator(0, rot.Yaw, 0));

	me->AddMovementInput(targetloc);

	float dist = me->GetDistanceTo(player);

	if (dist < 400.0f)
	{
		me->GetCharacterMovement()->MaxFlySpeed = 0;
		SetRazerState(ERazerState::PATROL);
	}
}

void URazerFSMComponent::TickPatrol()
{
	/*FVector newLocation = FVector(player->GetActorLocation().X, player->GetActorLocation().Y, 1500.0f);
	
	angleAxis+= multiplier;

	if(angleAxis >=360.0f)
	{
		angleAxis = 0;
	}

	FVector rotateValue = dimensions.RotateAngleAxis(angleAxis, axisVector);

	newLocation.X += rotateValue.X;
	newLocation.Y += rotateValue.Y;
	newLocation.Z += rotateValue.Z;

	->MoveToLocation(newLocation);*/

	OnOverlap();

	if (bOverlapEnemy != false && IsValid(enemis[0]))
	{
		me->GetCharacterMovement()->MaxFlySpeed = 300.0f;
		SetRazerState(ERazerState::ATTACK);
	}
	else
	{
		enemis.Reset();
		me->GetCharacterMovement()->MaxFlySpeed = 300.0f;
		SetRazerState(ERazerState::IDLE);
	}
}

void URazerFSMComponent::TickAttack()
{
	FVector target = enemis[0]->GetActorLocation() + FVector(0, 0, 150.0f);

	FVector start = me->GetActorLocation();

	FRotator lookRot = UKismetMathLibrary::FindLookAtRotation(start, target);

	FVector targetloc = UKismetMathLibrary::Conv_RotatorToVector(FRotator(0, lookRot.Yaw, 0));

	me->AddMovementInput(targetloc);

	float dist = me->GetDistanceTo(enemis[0]);

	if (dist < 600.0f)
	{
		SetRazerState(ERazerState::DAMAGE);
	}
}

void URazerFSMComponent::TickDamage()
{
	me->GetCharacterMovement()->MaxFlySpeed = 0;

	FVector lookDist = enemis[0]->GetActorLocation() - me->GetActorLocation();

	FRotator lookRot = FRotationMatrix::MakeFromX(lookDist).Rotator();

	me->SetActorRotation(FMath::Lerp(me->GetActorRotation(), lookRot, 0.1f));

	curTime += GetWorld()->GetDeltaSeconds();
	
	if (curTime > reTime || bCheckFire != true)
	{
		me->StartFire();
		curTime = 0;
		bCheckFire = true;
	}

	if (enemis[0] == nullptr || !IsValid(enemis[0]))
	{
		enemis.Reset();
		me->SetActorRotation(FRotator::ZeroRotator);
		me->GetCharacterMovement()->MaxFlySpeed = 300;
		SetRazerState(ERazerState::IDLE);
	}
}

void URazerFSMComponent::TickDie()
{

}

void URazerFSMComponent::OnDamage(int32 damage)
{

}

void URazerFSMComponent::SetRazerState(ERazerState next)
{
	razerState = next;
}

void URazerFSMComponent::OnOverlap()
{
	TArray<FOverlapResult> oversInfo;
	FVector loc = me->GetActorLocation();
	FQuat rot = me->GetActorRotation().Quaternion();
	FCollisionQueryParams params;
	params.AddIgnoredActor(me);
	params.AddIgnoredActor(player);
	params.AddIgnoredActor(boss);
	params.AddIgnoredActor(bullet);
	
	bOverlapEnemy = GetWorld()->OverlapMultiByChannel(oversInfo, loc, rot, ECC_Visibility, FCollisionShape::MakeSphere(1000), params);

	for (FOverlapResult overInfo : oversInfo)
	{
		auto enemy = Cast<ACloseAttackEnemy>(overInfo.GetActor());

		enemis.Add(enemy);
	}
	
	//UE_LOG(LogTemp, Warning, TEXT("%s"), *enemis[0]->GetName());

	//for (int i = 0; i<oversInfo.Num(); i++)
	//{
	//	auto enemy = Cast<ACloseAttackEnemy>(oversInfo[i].GetActor());

	//	enemis.Add(enemy);
	//}
}