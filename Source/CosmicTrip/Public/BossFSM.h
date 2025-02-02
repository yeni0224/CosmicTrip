// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BossFSM.generated.h"

UENUM(BlueprintType)
enum class EBossState : uint8
{
	IDLE,
	MOVE,
	ATTACK,
	NEXTATTACK,
	DAMAGE,
	DIE,
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COSMICTRIP_API UBossFSM : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBossFSM();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//PlayAnimMontage로 해보자
	UPROPERTY()
	class ABoss* me;
	UPROPERTY()
	class UBossAnim* bossAnim;
	UPROPERTY()
	class ACosmicPlayer* mainTarget;
	UPROPERTY()
	class AAIController* ai;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Boss")
	EBossState state;

	//시간체크
	float currentTime = 0;
	//Damage 진행되는 동안
	float damageDelayTime = 2;
	//Die 진행되는 동안

	//Boss HP 계산하기
	float hp;
	float maxHP = 100;

	//Boss가 Player를 감지하는 거리 범위
	UPROPERTY(EditDefaultsOnly, Category = "Range")
	float attackRange = 300;

	//플레이어와 Boss 사이의 거리
	UPROPERTY(EditDefaultsOnly, Category = "Range")
	float mainTargetDist;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BossSettings")
	float walkSpeed = 350.f;

	void IdleState();
	void MoveState();
	void AttackState();
	void NextAttackState();
	void DamageState();
	void DieState();

	UFUNCTION()
	void OnDamageProcess(float attack);



};
