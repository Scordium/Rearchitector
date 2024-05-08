﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "Actions/ToolSetRotateAction.h"

void UToolSetRotateAction::PerformAction_Implementation()
{
	RotationUndoCache.Empty();
	for (const FArchitectorToolTarget& Target : Targets) RotationUndoCache.Add(Target, Target.Target->GetActorQuat());
	
	auto RCO = GetRCO();

	FArchitectorTransformData TransformData;
	TransformData.Rotate = FArchitectorRotateTransform(Value, true);

	RCO->ApplyTransformDataIndependent(Targets, TransformData);	
}

void UToolSetRotateAction::UndoAction_Implementation()
{
	
}