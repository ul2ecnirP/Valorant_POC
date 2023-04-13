#pragma once
#include <stdint.h>
#include "offsets.h"
#include "preprocess.h"
#include "matrix.hpp"
#include "game/bonesid.h"//REDACTED


template<class T>

class TArrayDrink {
public:
	TArrayDrink() {
		Data = NULL;
		Count = 0;
		Max = 0;
	};
	T operator[](uint64_t i) const {
		return driver->RPMPTR(((uintptr_t)Data) + i * sizeof(T));
	};

	T* Data;
	unsigned int Count;
	unsigned int Max;
};




uintptr_t ULevel(uintptr_t World) {
	return driver->RPMPTR(World + offsets::persistent_level);
};
uintptr_t GameInstance(uintptr_t GameWorld) {
	return driver->RPM<uintptr_t>(GameWorld + offsets::game_instance);
};
uintptr_t ULocalPlayer(uintptr_t UGameInstance) {
	uintptr_t ULocalPlayerArray = driver->RPM<uintptr_t>(UGameInstance + offsets::localplayers_array);
	return driver->RPM<uintptr_t>(ULocalPlayerArray);
};
uintptr_t APlayerController(uintptr_t ULocalPlayer) {
	return driver->RPMPTR(ULocalPlayer + offsets::player_controller);
};
uintptr_t APawn(uintptr_t APlayerController) {
	return driver->RPMPTR(APlayerController + offsets::apawn);
};
float Health(uintptr_t APawn) {
	uintptr_t DamageHandler = driver->RPMPTR(APawn + offsets::damage_handler);
	return driver->RPM<float>(DamageHandler + offsets::health);
};


uintptr_t CameraManager(uintptr_t APlayerController){
	return driver->RPMPTR(APlayerController + offsets::camera_manager);
}
float GetPlayerfov(uintptr_t camera_controller){
	return driver->RPM<float>(camera_controller + offsets::camera_fov);
}
Vector3 GetCameraLocation(uintptr_t camera_controller){
	return driver->RPM<Vector3>(camera_controller + offsets::camera_position);
}
Vector3 GetCameraRotation(uintptr_t camera_controller){
	return driver->RPM<Vector3>(camera_controller + offsets::camera_rotation);
}


Vector3 RelativeLocation(uintptr_t APawn) {
	uintptr_t RootComponent = driver->RPM<uintptr_t>(APawn + offsets::root_component);
	return driver->RPM<Vector3>(RootComponent + offsets::root_position);
};
int GetActorCount(uintptr_t persistantlevel){
	return driver->RPM<int>(persistantlevel + offsets::actors_count);
}
int UniqueID(uintptr_t APawn){
	return driver->RPM<int>(APawn + offsets::unique_id);
};
TArrayDrink<uintptr_t> AActorArray(uintptr_t ULevel) {
	return driver->RPM<TArrayDrink<uintptr_t>>(ULevel + offsets::actor_array);
};
uintptr_t AHUD(uintptr_t APlayerController) {
	return driver->RPMPTR(APlayerController + offsets::camera_manager);
};
uintptr_t USkeletalMeshComponent(uintptr_t Pawn) {
	return driver->RPMPTR(Pawn + offsets::mesh);
};
int GetBoneCount(uintptr_t Mesh){
	return driver->RPM<int>(Mesh+ offsets::bone_count);
}


Vector3 GetEntityBone(DWORD_PTR mesh, int id)
{
	DWORD_PTR array = driver->RPM<uintptr_t>(mesh + offsets::bone_array);
	if (array == 0)
		array = driver->RPM<uintptr_t>(mesh + offsets::BoneArrayCache);

	FTransform bone = driver->RPM<FTransform>(array + (id * 30));

	FTransform ComponentToWorld = driver->RPM<FTransform>(mesh + offsets::component_to_world);
	D3DMATRIX Matrix;

	Matrix = MatrixMultiplication(bone.ToMatrixWithScale(), ComponentToWorld.ToMatrixWithScale());
	Vector3 result;
	result.x = Matrix._41;
	result.y = Matrix._42;
	result.z = Matrix._43;
	return result;
}




BoneAtriculation GetPlayerSkeletton(uintptr_t mesh, short type){ //1 = male //2=femme //3=bot
	BoneAtriculation skl;
	if(type ==1){
		skl.RThigh1 = GetEntityBone(mesh, Bones::MaleRThigh1);
		skl.RThigh2 = GetEntityBone(mesh, Bones::MaleRThigh2);
		skl.RFoot = GetEntityBone(mesh, Bones::MaleRFoot);
		skl.LThigh1 = GetEntityBone(mesh, Bones::MaleLThigh1);
		skl.LThigh2 = GetEntityBone(mesh, Bones::MaleLThigh2);
		skl.LFoot = GetEntityBone(mesh, Bones::MaleLFoot);
		skl.Chest = GetEntityBone(mesh, Bones::MaleChest);
		skl.Stomach = GetEntityBone(mesh, Bones::MaleStomach);
		skl.Pelvis = GetEntityBone(mesh, Bones::MalePelvis);
		skl.LUpperarm = GetEntityBone(mesh, Bones::MaleLUpperarm);
		skl.LForearm1 = GetEntityBone(mesh, Bones::MaleLForearm1);
		skl.LForearm2 = GetEntityBone(mesh, Bones::MaleLForearm2);
		skl.RUpperarm = GetEntityBone(mesh, Bones::MaleRUpperarm);
		skl.RForearm1 = GetEntityBone(mesh, Bones::MaleRForearm1);
		skl.RForearm2 = GetEntityBone(mesh, Bones::MaleRForearm2);
		skl.Neck = GetEntityBone(mesh, Bones::MaleNeck);
		skl.Head = GetEntityBone(mesh, Bones::MaleHead);
	}
	else if(type == 2){
		skl.RThigh1 = GetEntityBone(mesh, Bones::FemaleRThigh1);
		skl.RThigh2 = GetEntityBone(mesh, Bones::FemaleRThigh2);
		skl.RFoot = GetEntityBone(mesh, Bones::FemaleRFoot);
		skl.LThigh1 = GetEntityBone(mesh, Bones::FemaleLThigh1);
		skl.LThigh2 = GetEntityBone(mesh, Bones::FemaleLThigh2);
		skl.LFoot = GetEntityBone(mesh, Bones::FemaleLFoot);
		skl.Chest = GetEntityBone(mesh, Bones::FemaleChest);
		skl.Stomach = GetEntityBone(mesh, Bones::FemaleStomach);
		skl.Pelvis = GetEntityBone(mesh, Bones::FemalePelvis);
		skl.LUpperarm = GetEntityBone(mesh, Bones::FemaleLUpperarm);
		skl.LForearm1 = GetEntityBone(mesh, Bones::FemaleLForearm1);
		skl.LForearm2 = GetEntityBone(mesh, Bones::FemaleLForearm2);
		skl.RUpperarm = GetEntityBone(mesh, Bones::FemaleRUpperarm);
		skl.RForearm1 = GetEntityBone(mesh, Bones::FemaleRForearm1);
		skl.RForearm2 = GetEntityBone(mesh, Bones::FemaleRForearm2);
		skl.Neck = GetEntityBone(mesh, Bones::FemaleNeck);
		skl.Head = GetEntityBone(mesh, Bones::FemaleHead);
	}
	else if(type == 3){
		skl.RThigh1 = GetEntityBone(mesh, Bones::DummyRThigh1);
		skl.RThigh2 = GetEntityBone(mesh, Bones::DummyRThigh2);
		skl.RFoot = GetEntityBone(mesh, Bones::DummyRFoot);
		skl.LThigh1 = GetEntityBone(mesh, Bones::DummyLThigh1);
		skl.LThigh2 = GetEntityBone(mesh, Bones::DummyLThigh2);
		skl.LFoot = GetEntityBone(mesh, Bones::DummyLFoot);
		skl.Chest = GetEntityBone(mesh, Bones::DummyChest);
		skl.Stomach = GetEntityBone(mesh, Bones::DummyStomach);
		skl.Pelvis = GetEntityBone(mesh, Bones::DummyPelvis);
		skl.LUpperarm = GetEntityBone(mesh, Bones::DummyLUpperarm);
		skl.LForearm1 = GetEntityBone(mesh, Bones::DummyLForearm1);
		skl.LForearm2 = GetEntityBone(mesh, Bones::DummyLForearm2);
		skl.RUpperarm = GetEntityBone(mesh, Bones::DummyRUpperarm);
		skl.RForearm1 = GetEntityBone(mesh, Bones::DummyRForearm1);
		skl.RForearm2 = GetEntityBone(mesh, Bones::DummyRForearm2);
		skl.Neck = GetEntityBone(mesh, Bones::DummyNeck);
		skl.Head = GetEntityBone(mesh, 8);
	}
	return skl;
}

