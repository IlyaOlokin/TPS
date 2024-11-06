// Fill out your copyright notice in the Description page of Project Settings.


#include "BonePair.h"

BonePair::BonePair(FName InBone1, FName InBone2)
	: Bone1(InBone1), Bone2(InBone2)
{
}

FName BonePair::GetBoneName() const
{
	return BoneName;
}

void BonePair::AddChildBone(BonePair* LinkedBone)
{
	ChildBones.Add(LinkedBone);
}

const TArray<BonePair*>& BonePair::GetChildBones() const
{
	return ChildBones;
}
