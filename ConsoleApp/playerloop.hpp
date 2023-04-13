#pragma once

#include "readfunc.hpp"
#include "preprocess.h"
#include "global.hpp"

#include <stdint.h>
#include <stdio.h>
#include <windows.h>

void playerloop(){
    uintptr_t ginstance = GameInstance(uworld_global);
    uintptr_t persistent_level = ULevel(uworld_global);

    uintptr_t ULocalPlayerPtr = ULocalPlayer(ginstance);
    uintptr_t APlayerControllerPtr = APlayerController(ULocalPlayerPtr);
    uintptr_t APawnPtr = APawn(APlayerControllerPtr);
    
    uintptr_t AplayerCameraManager;
    int LocaluniqueID = UniqueID(APawnPtr);
    unsigned short currentplayer = 0;
    auto MyHUD = AHUD(APlayerControllerPtr);
    uintptr_t SkeletalMesh;
    int BoneCount;
    HWND valowindow = get_process_wnd(pid);
    while(1){
        GetSWindowsSize(valowindow);
        
        if(MyHUD != 0){
            auto PlayerArray = AActorArray(persistent_level);
        for (uint32_t i = 0; i < PlayerArray.Count; ++i){
            auto Pawns = PlayerArray[i];
            if (Pawns != APawnPtr){
                if (LocaluniqueID == UniqueID(Pawns)){
                    PArray[currentplayer].is_in_game = 1;
                    PArray[currentplayer].vie = Health(Pawns);
                    PArray[currentplayer].pos = RelativeLocation(Pawns);
                    SkeletalMesh = USkeletalMeshComponent(Pawns);
                    BoneCount = GetBoneCount(SkeletalMesh);
                    
                    switch (BoneCount)
                    {
                    case 104:
                        PArray[currentplayer].bones = GetPlayerSkeletton(SkeletalMesh,1);
                        break;
                    case 103:
                        PArray[currentplayer].bones = GetPlayerSkeletton(SkeletalMesh,3);
                        break;
                    default:
                        PArray[currentplayer].bones = GetPlayerSkeletton(SkeletalMesh,2);
                        
                        break;
                    }

                    PArray[currentplayer].bones.Head = GetEntityBone(SkeletalMesh, 8);
                    printf("BOT HEADPOS: %f %f %f\n", PArray[currentplayer].bones.Head.x, PArray[currentplayer].bones.Head.y,PArray[currentplayer].bones.Head.z);
                    //POC: can get headpos but can get anything else
                    currentplayer++;
                    
                }
            }
        }
        }
        nbplayers = currentplayer;
        //LocalPlayer.pos = RelativeLocation(APawnPtr); //for loop
        //LocalPlayer.vie = Health(APawnPtr);


        AplayerCameraManager = CameraManager(APlayerControllerPtr);

        PlayerViewInfo.FOV = GetPlayerfov(AplayerCameraManager);
        PlayerViewInfo.Location = GetCameraLocation(AplayerCameraManager);
        PlayerViewInfo.Rotation = GetCameraRotation(AplayerCameraManager);  

        //printf("\n>>> %f (%f/%f/%f) (%f/%f/%f)\n", PlayerViewInfo.FOV, PlayerViewInfo.Location.x, PlayerViewInfo.Location.y, PlayerViewInfo.Location.z, PlayerViewInfo.Rotation.x, PlayerViewInfo.Rotation.y, PlayerViewInfo.Rotation.z);
        //playermesh = USkeletalMeshComponent(APawnPtr);
        //Vector3 headpos = GetEntityBone(playermesh, 8);
        //printf("PLAYER HEADPOS: %f %f %f", headpos.x, headpos.y,headpos.z);
        
        currentplayer = 0;
        Sleep(2);
    }
}