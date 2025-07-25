#include "MobilePlatformer/TouchInputSimulator.h"

#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

// For input simulation
#include "InputCoreTypes.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformProcess.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRandomTouchSwipeTest, "TouchAutomation.RandomSwipeTest", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FRandomTouchSwipeTest::RunTest(const FString& Parameters)
{
    TSharedPtr<FTouchInputSimulator> SimulatorPtr = MakeShared<FTouchInputSimulator>();
    SimulatorPtr->StartRandomSwipeSimulation();

    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(30.0f));

    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([SimulatorPtr]()
    {
        SimulatorPtr->StopSimulation();
        return true;
    }));
    
    return true;
}

FTouchInputSimulator::FTouchInputSimulator()
{
    World = nullptr;
    ScreenSize = FVector2D(1080, 1920); // Default Android screen size
    
    // Get the current world
    if (GEngine && GEngine->GetWorldContexts().Num() > 0)
    {
        World = GEngine->GetWorldContexts()[0].World();
    }
    
    // Get actual screen size if available
    if (FSlateApplication::IsInitialized())
    {
        if (GEngine && GEngine->GameViewport)
        {
            GEngine->GameViewport->GetViewportSize(ScreenSize);
        }
    }
}

FTouchInputSimulator::~FTouchInputSimulator()
{
    StopSimulation();
}

void FTouchInputSimulator::StartRandomSwipeSimulation()
{
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("TouchInputSimulator: No valid world found"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("TouchInputSimulator: Starting random swipe simulation"));
    
    // Schedule the first swipe
    float InitialDelay = GetRandomDelay();
    
    // Use a lambda function instead of a direct member function pointer
    World->GetTimerManager().SetTimer(SwipeTimerHandle, 
        [this]() { this->ExecuteRandomSwipe(); }, 
        InitialDelay, false);
}

void FTouchInputSimulator::ExecuteRandomSwipe()
{
    FVector2D StartPos, EndPos;
    float Duration;
    
    GenerateRandomSwipeParams(StartPos, EndPos, Duration);
    SimulateSwipe(StartPos, EndPos, Duration);
    
    // Schedule the next swipe
    if (World)
    {
        float NextDelay = GetRandomDelay();
        
        // Use a lambda function instead of a direct member function pointer
        World->GetTimerManager().SetTimer(SwipeTimerHandle, 
            [this]() { this->ExecuteRandomSwipe(); }, 
            NextDelay, false);
    }
}

void FTouchInputSimulator::StopSimulation()
{
    if (World && SwipeTimerHandle.IsValid())
    {
        World->GetTimerManager().ClearTimer(SwipeTimerHandle);
        UE_LOG(LogTemp, Log, TEXT("TouchInputSimulator: Stopped simulation"));
    }
}

void FTouchInputSimulator::GenerateRandomSwipeParams(FVector2D& StartPos, FVector2D& EndPos, float& Duration)
{
    // Generate random start position (avoiding edges)
    float MarginX = ScreenSize.X * 0.1f;
    float MarginY = ScreenSize.Y * 0.1f;
    
    StartPos.X = FMath::FRandRange(MarginX, ScreenSize.X - MarginX);
    StartPos.Y = FMath::FRandRange(MarginY, ScreenSize.Y - MarginY);
    
    // Generate random swipe direction and distance
    float SwipeDistance = FMath::RandRange(100.0f, 300.0f);
    float SwipeAngle = FMath::RandRange(0.0f, 2.0f * PI);
    
    EndPos.X = StartPos.X + SwipeDistance * FMath::Cos(SwipeAngle);
    EndPos.Y = StartPos.Y + SwipeDistance * FMath::Sin(SwipeAngle);
    
    // Clamp end position to screen bounds
    EndPos.X = FMath::Clamp(EndPos.X, 0.0f, ScreenSize.X);
    EndPos.Y = FMath::Clamp(EndPos.Y, 0.0f, ScreenSize.Y);
    
    // Random duration between 0.2 and 0.8 seconds - FIX: Use FMath::FRandRange for float values
    Duration = FMath::FRandRange(0.2f, 0.8f);
    
    UE_LOG(LogTemp, Log, TEXT("TouchInputSimulator: Swipe from (%.1f, %.1f) to (%.1f, %.1f) over %.2f seconds"),
        StartPos.X, StartPos.Y, EndPos.X, EndPos.Y, Duration);
}

void FTouchInputSimulator::SimulateSwipe(const FVector2D& StartPos, const FVector2D& EndPos, float Duration)
{
    // Number of interpolation steps for smooth swipe
    const int32 Steps = 10;
    
    // Find the player controller to simulate touch input
    APlayerController* PlayerController = nullptr;
    if (World)
    {
        if (World->GetFirstPlayerController())
        {
            PlayerController = World->GetFirstPlayerController();
        }
    }
    
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("TouchInputSimulator: No PlayerController available"));
        return;
    }
    
    // Schedule touch events using timer
    if (World)
    {
        // Send initial touch down event
        FVector2D NormalizedStartPos;
        NormalizedStartPos.X = StartPos.X / ScreenSize.X;
        NormalizedStartPos.Y = StartPos.Y / ScreenSize.Y;
        
        // Using PlayerController directly for touch events with all required parameters
        PlayerController->InputTouch(0, ETouchType::Began, NormalizedStartPos, 1.0f, FDateTime::Now(), 0);
        
        // Create a weak pointer to the player controller for safe capture
        TWeakObjectPtr<APlayerController> WeakPlayerController = PlayerController;
        
        for (int32 i = 1; i <= Steps; i++)
        {
            float Alpha = static_cast<float>(i) / Steps;
            FVector2D CurrentPos = FMath::Lerp(StartPos, EndPos, Alpha);
            
            // Convert screen coordinates to normalized coordinates (0-1)
            FVector2D NormalizedPos;
            NormalizedPos.X = CurrentPos.X / ScreenSize.X;
            NormalizedPos.Y = CurrentPos.Y / ScreenSize.Y;
            
            float Delay = Duration * Alpha;
            
            // Use a capture to ensure the correct values are used in the timer callback
            FVector2D CapturedPos = NormalizedPos;
            bool bIsLastStep = (i == Steps);
            
            // Schedule touch event with weak pointer instead of raw pointer
            FTimerHandle TempHandle;
            World->GetTimerManager().SetTimer(TempHandle, [WeakPlayerController, CapturedPos, bIsLastStep]()
            {
                if (APlayerController* PC = WeakPlayerController.Get())
                {
                    // Send the appropriate touch event with all required parameters
                    if (bIsLastStep)
                    {
                        PC->InputTouch(0, ETouchType::Ended, CapturedPos, 1.0f, FDateTime::Now(), 0);
                    }
                    else
                    {
                        PC->InputTouch(0, ETouchType::Moved, CapturedPos, 1.0f, FDateTime::Now(), 0);
                    }
                }
            }, Delay, false);
        }
    }
}

float FTouchInputSimulator::GetRandomDelay()
{
    // Random delay between 1 and 3 seconds
    return FMath::RandRange(1.0f, 3.0f);
}