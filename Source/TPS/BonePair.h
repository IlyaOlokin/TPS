// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class TPS_API BonePair
{
public:
	FName Bone1;
	FName Bone2;
	
	BonePair(FName InBone1, FName InBone2);

	FName GetBoneName() const;
	void AddChildBone(BonePair* LinkedBone);
	
	const TArray<BonePair*>& GetChildBones() const;

private:
	FName BoneName;
	TArray<BonePair*> ChildBones;
};
