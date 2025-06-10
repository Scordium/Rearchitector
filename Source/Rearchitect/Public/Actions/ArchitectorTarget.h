#pragma once
#include "AbstractInstanceManager.h"
#include "FGOutlineComponent.h"
#include "Buildables/FGBuildable.h"

#include "ArchitectorTarget.generated.h"

USTRUCT(Blueprintable, BlueprintType)
struct FArchitectorToolTarget
{
	GENERATED_BODY()

public:

	FArchitectorToolTarget(){}
	FArchitectorToolTarget(const FHitResult& HitResult)
	{
		HitResultActor = HitResult.GetActor();
		HitComponent = HitResult.GetComponent();
		HitItem = HitResult.Item;
		
		if(auto AbstractInstance = Cast<AAbstractInstanceManager>(HitResultActor))
		{
			FInstanceHandle InstanceHandle;
			AbstractInstance->ResolveHit(HitResult, InstanceHandle);
			Target = InstanceHandle.GetOwner<AActor>();
			IsAbstract = true;
		}
		else Target = Cast<AFGBuildable>(HitResult.GetActor());
	}

	void HideOutline() const
	{
		if (!Target) return;
		
		if (IsAbstract) UFGBlueprintFunctionLibrary::HideOutline(HitComponent);
		else UFGOutlineComponent::Get(Target->GetWorld())->HideOutline(Target);
	}

	void ShowOutline(EOutlineColor Color) const
	{
		if (!Target) return;
		
		if (IsAbstract) UFGBlueprintFunctionLibrary::ShowOutline(HitComponent, Color);
		else UFGOutlineComponent::Get(Target->GetWorld())->ShowOutline(Target, Color);
	}

	bool IsValid() const { return HitComponent || Target; }

	void MakeMovable() const
	{
		if (!Target) return;
		
		if(auto Root = Target->GetRootComponent())
			Root->SetMobility(EComponentMobility::Movable);
		
		if(IsAbstract)
			GenerateInstanceHandle().GetInstanceComponent()->SetMobility(EComponentMobility::Movable);
	}

	FTransform GetTargetTransform() const
	{
		if (!Target) return FTransform::Identity;
		
		return IsAbstract ? HitComponent->GetComponentTransform() : Target->GetActorTransform();
	}

	void SetTargetTransform(const FTransform& NewTransform) const
	{
		if (!Target) return;
		
		if (IsAbstract)
		{
			auto InstanceHandle = GenerateInstanceHandle();

			FTransform AbstractTransform;
			InstanceHandle.GetInstanceComponent()->GetInstanceTransform(InstanceHandle.GetHandleID(), AbstractTransform, true);
			
			auto LocationDelta = AbstractTransform.GetLocation() - Target->GetActorLocation();
			auto RotationDelta = AbstractTransform.GetRotation() * Target->GetActorQuat();
			auto ScaleDelta = AbstractTransform.GetScale3D() - Target->GetActorScale3D();

			AbstractTransform = FTransform(RotationDelta, LocationDelta, ScaleDelta);

			HitComponent->SetWorldTransform(NewTransform);
			InstanceHandle.UpdateTransform(NewTransform + AbstractTransform);
		}

		Target->SetActorTransform(NewTransform);
		Target->CachedActorTransform = NewTransform;
	}

	TArray<FInventoryStack> Dismantle(bool NoBuildCost) const
	{
		TArray<FInventoryStack> Refund;
		if(auto Buildable = Cast<AFGBuildable>(Target))
		{
			IFGDismantleInterface::Execute_GetDismantleRefund(Buildable, Refund, NoBuildCost);
			IFGDismantleInterface::Execute_Dismantle(Buildable);
		}

		return Refund;
	}

	FBox GetBounds() const
	{
		if (!Target) return FBox();
		return IsAbstract ? HitComponent->Bounds.GetBox() : Target->GetComponentsBoundingBox(true, true);
	}

	void AddToRaycastIgnored(FCollisionQueryParams& QueryParams) const
	{
		if (!Target) return;
		
		if (IsAbstract) QueryParams.AddIgnoredComponent(HitComponent);
		else QueryParams.AddIgnoredActor(Target);
	}

	FInstanceHandle GenerateInstanceHandle() const
	{
		FHitResult Hit = FHitResult(HitResultActor, HitComponent, FVector::ZeroVector, FVector::ZeroVector);
		Hit.Item = HitItem;

		FInstanceHandle Out;
		auto AbstractInstance = Cast<AAbstractInstanceManager>(HitResultActor);
		AbstractInstance->ResolveHit(Hit, Out);

		return Out;
	}

	FORCEINLINE friend uint32 GetTypeHash(const FArchitectorToolTarget& Thing)
	{
		uint32 Hash = FCrc::MemCrc32(&Thing, sizeof(FArchitectorToolTarget));
		return Hash;
	}

	bool Equals(const FArchitectorToolTarget& Other) const { return *this == Other; }
	bool operator==(const FArchitectorToolTarget& Other) const { return Target == Other.Target; }
	bool operator==(AActor* Actor) const { return Target == Actor; }

	static bool IsValidTarget(const FHitResult& HitResult)
	{
		auto Actor = HitResult.GetActor();
		if(!Actor) return false;
		
		bool IsValidClass = (Actor->IsA<AFGBuildable>() && !Actor->HasAnyFlags(RF_WasLoaded)) || HitResult.GetActor()->IsA<AAbstractInstanceManager>();
		
		return IsValidClass;
	}

private:

	UPROPERTY() AActor* Target = nullptr;
	UPROPERTY() UPrimitiveComponent* HitComponent = nullptr;
	UPROPERTY() bool IsAbstract = false;
	UPROPERTY() AActor* HitResultActor = nullptr;
	UPROPERTY() int32 HitItem = 0;
	
};
