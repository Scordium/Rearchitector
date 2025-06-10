#include "ArchitectorRCO.h"

#include "FGPlayerState.h"

void UArchitectorRCO::ApplyModifyDataToAll_Implementation(const TArray<FArchitectorToolTarget>& Targets, const FActorTransformModifyData& TransformData)
{
	Multicast_ApplyModifyDataToAll(Targets, TransformData);
}

void UArchitectorRCO::Multicast_ApplyModifyDataToAll_Implementation(const TArray<FArchitectorToolTarget>& Targets, const FActorTransformModifyData& TransformData)
{
	for (const FArchitectorToolTarget& Target : Targets) PerformActionOnTarget(Target, TransformData);
}

void UArchitectorRCO::ApplyModifyDataOnEach_Implementation(const TArray<FTargetModifyData>& Data)
{
	Multicast_ApplyModifyDataOnEach_Implementation(Data);
}

void UArchitectorRCO::Multicast_ApplyModifyDataOnEach_Implementation(const TArray<FTargetModifyData>& Datas)
{
	for (const FTargetModifyData& Data : Datas) PerformActionOnTarget(Data.Target, Data.TransformData);
}

void UArchitectorRCO::ApplyTransformOnEach_Implementation(const TArray<FTargetTransformData>& Data)
{
	Multicast_ApplyTransformOnEach_Implementation(Data);
}

void UArchitectorRCO::Multicast_ApplyTransformOnEach_Implementation(const TArray<FTargetTransformData>& Data)
{
	for (const FTargetTransformData& TargetTransformData : Data)
	{
		if(!TargetTransformData.Target.IsValid()) continue;
		
		const auto& ActorT = TargetTransformData.ActorTransform;
		const auto& AbstT = TargetTransformData.AbstractTransform;
		const auto& Target = TargetTransformData.Target;

		Target.SetTargetTransform(ActorT);
	}
}

void UArchitectorRCO::DismantleAndRefund_Implementation(AFGPlayerState* Player, const TArray<FArchitectorToolTarget>& Targets)
{
	TArray<FInventoryStack> Refund;
	const bool NoBuildCost = Player->GetPlayerRules().NoBuildCost;

	for (const FArchitectorToolTarget& Target : Targets)
		Refund.Append(Target.Dismantle(NoBuildCost));

	Cast<AFGCharacterPlayer>(Player->GetPawn())->GetInventory()->AddStacks(Refund);
}

void UArchitectorRCO::PerformActionOnTarget(const FArchitectorToolTarget& Target, const FActorTransformModifyData& TransformData)
{
	if(!Target.IsValid()) return;
	Target.MakeMovable();

	auto Transform = Target.GetTargetTransform();

	//Move
	if(TransformData.Move.IsUsed)
	{
		//Transform actor
		auto& PositionValue = TransformData.Move.Value;
			
		if(TransformData.Move.UseAsSetAction) Transform.SetLocation(PositionValue);
		else Transform.AddToTranslation(PositionValue);
			
		Target.SetTargetTransform(Transform);
	}

	//Rotate
	if(TransformData.Rotate.IsUsed)
	{
		//Transform actor
		auto NewActorRotation = TransformData.Rotate.UseAsSetAction
		? TransformData.Rotate.Value
		: TransformData.Rotate.Value * Transform.GetRotation();

		Transform.SetRotation(NewActorRotation);
	}

	//Scale
	if(TransformData.Scale.IsUsed)
	{
		//Transform actor
		auto NewActorScale = TransformData.Scale.UseAsSetAction
		? TransformData.Scale.Value
		: TransformData.Scale.Value + Transform.GetScale3D();

		Transform.SetScale3D(NewActorScale);
	}

	Target.SetTargetTransform(Transform);
}
