
#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "TimerManager.h"

class MOBILEPLATFORMER_API FTouchInputSimulator
{
public:
	FTouchInputSimulator();
	~FTouchInputSimulator();

	void StartRandomSwipeSimulation();
	void StopSimulation();
	void ExecuteRandomSwipe();

private:
	FTimerHandle SwipeTimerHandle;
	UWorld* World;
	FVector2D ScreenSize;

	void GenerateRandomSwipeParams(FVector2D& StartPos, FVector2D& EndPos, float& Duration) const;
	void SimulateSwipe(const FVector2D& StartPos, const FVector2D& EndPos, float Duration) const;
	static float GetRandomDelay();
};