#include <windows.h>
#include <format>
#include <codecvt>
#include <filesystem>
#include "toml++/toml.hpp"
#include "nya_commonhooklib.h"

#include "nya_commonmath.h"

#include "ac.h"

void OnPluginStartup();
#include "util.h"
#include "inputs.h"

bool bRevLimiter = true;
bool bSpeedbreakerEnabled = true;
float fUpgradeLevel = 1.0;
double fGlobalDeltaTime = 1.0 / 60.0;

#include "decomp/ConversionUtil.hpp"
#include "decomp/UMathExtras.h"
#include "decomp/HelperTypes.h"

#include "MWCarTuning.h"
#include "decomp/interfaces/MWInterface.h"
#include "decomp/interfaces/MWIChassis.h"
#include "decomp/interfaces/MWIRaceEngine.h"
#include "decomp/interfaces/MWITiptronic.h"
#include "decomp/interfaces/MWIEngineDamage.h"
//#include "decomp/interfaces/MWIInductable.cpp"
#include "decomp/interfaces/MWITransmission.h"
#include "decomp/interfaces/MWIEngine.h"
#include "decomp/interfaces/MWIVehicle.cpp"
#include "decomp/interfaces/MWICollisionBody.cpp"
#include "decomp/interfaces/MWIRigidBody.cpp"
#include "decomp/interfaces/MWIPlayer.cpp"
#include "decomp/interfaces/MWIInput.cpp"
#include "decomp/interfaces/MWISpikeable.cpp"
#include "decomp/interfaces/MWICheater.cpp"
#include "decomp/interfaces/MWIHumanAI.cpp"

#include "decomp/behaviors/MWWheel.h"
#include "decomp/behaviors/MWChassisBase.h"
#include "decomp/behaviors/SuspensionRacer.h"
#include "decomp/behaviors/EngineRacer.h"
#include "decomp/behaviors/MWWheel.cpp"
#include "decomp/behaviors/MWChassisBase.cpp"
#include "decomp/behaviors/SuspensionRacer.cpp"
#include "decomp/behaviors/EngineRacer.cpp"

#include "nya_commontimer.cpp"

/*
dT 0.06667
oom 0.00138
mass 727.00000
force -161.71716 87390.30469 -488.61813
src -0.00893 -4.47674 -0.01546
add -0.01483 8.01378 -0.04481
total_force -161.72 87390.30 -488.62
total_torque 9032.70 84.98 -5453.57
*/

EngineRacer* pMWEngine;
SuspensionRacerMW* pMWSuspension;
CNyaTimer gRealTimer;
void __fastcall MWCarUpdate(Car* pThis, float dT) {
	if (pThis != pMyPlugin->car) return;

	pThis->ksPhysics->core->id->dWorldSetGravity(0.0, -9.81, 0.0);

	gRealTimer.Process();

	pThis->isControlsLocked = false;
	pThis->pollControls(dT);

	SimSystem::fSimTime += dT;
	fGlobalDeltaTime = dT;

	pMWEngine->OnTaskSimulate(dT);
	pMWSuspension->OnTaskSimulate(dT);

	if (pThis->controls.gas > 0.0f) {
		NyaHookLib::Patch<uint8_t>(NyaHookLib::mEXEBase + 0xD0440, 0xC3); // disable car reset
	}

	//for (int i = 0; i < pThis->suspensions.size(); i++) {
	//	pThis->suspensions[i]->step(dT);
	//}

	for (int i = 0; i < 4; i++) {
		auto mwTire = pMWSuspension->mTires[GetMWWheelID(i)];
		auto tire = &pThis->tyres[i];
		UMath::Matrix4 carMatrix;
		tire->car->body->getWorldMatrix(&carMatrix, 0.0);

		UMath::Matrix4 steerAngle;
		steerAngle.SetIdentity();
		steerAngle.Rotate(NyaVec3(0, 0, ANGLE2RAD(-pMWSuspension->GetWheelSteer(i))));
		tire->worldRotation = carMatrix * steerAngle;

		tire->localWheelRotation.Rotate(NyaVec3(-pMWSuspension->GetWheelAngularVelocity(i) * dT, 0, 0));

		tire->worldRotation = carMatrix * steerAngle * tire->localWheelRotation;

		//tire->worldPosition = *pMWSuspension->GetWheelPos(i);
		tire->contactPoint = tire->unmodifiedContactPoint = mwTire->mWorldPos.fHitPosition;
		tire->contactNormal = UMath::Vector4To3(mwTire->mNormal);
		tire->status.angularVelocity = mwTire->GetAngularVelocity();
		tire->status.distToGround = pMWSuspension->GetWheelRoadHeight(i);
		tire->status.load = mwTire->GetLoad();
		tire->status.isLocked = mwTire->IsBrakeLocked();
		tire->status.slipAngleRAD = mwTire->GetSlipAngle();
		tire->slidingVelocityX = mwTire->GetLateralSpeed();
		tire->slidingVelocityY = mwTire->GetRoadSpeed() * (1.0 - mwTire->GetTraction());
		tire->totalSlideVelocity = tire->slidingVelocityX + tire->slidingVelocityY;
		tire->roadVelocityX = mwTire->GetLateralSpeed();
		tire->roadVelocityY = mwTire->GetRoadSpeed();

		if (pThis->rigidAxle) {
			pThis->rigidAxle->stop(0.0);
		}
	}

	// todo tire states, rpm, gear!

	RefreshInputs();
}

void SwitchToMWPhysics() {
	if (pMWEngine || pMWSuspension) return;

	auto ply = pMyPlugin->car;
	//for (int i = 0; i < 4; i++) {
	//	ply->tyres[i].hub = new ACMWSuspension();
	//}
	//for (int i = 0; i < ply->suspensions.size(); i++) {
	//	ply->suspensions[i] = new ACMWSuspension();
	//}

	aPlayerInterfaces[0].aInterfaces.clear();
	aPlayerInterfaces[0].pCar = ply;
	aPlayerInterfaces[0].Add(new IVehicle(ply));
	aPlayerInterfaces[0].Add(new IRigidBodyMW(ply));
	aPlayerInterfaces[0].Add(new ICollisionBody(ply));
	aPlayerInterfaces[0].Add(new IInput(ply));
	aPlayerInterfaces[0].Add(new IPlayer(ply));
	aPlayerInterfaces[0].Add(new IHumanAI());

	auto engine = new EngineRacer(ply);
	auto susp = new SuspensionRacerMW(ply);
	engine->OnBehaviorChange();
	susp->OnBehaviorChange();
	pMWEngine = engine;
	pMWSuspension = susp;
}

UMath::Matrix4* MWSuspensionGetMatrix(ISuspension* susp, UMath::Matrix4* result) {
	// todo add deltatime * velocity (0.003) this is one frame behind
	auto car = pMyPlugin->car;
	for (int i = 0; i < 4; i++) {
		if (car->tyres[i].hub == susp) {
			car->body->getWorldMatrix(result, 0.0);
			pMWSuspension->GetWheelCenterPos(&result->p, GetMWWheelID(i));
			UMath::Vector3 velocity;
			car->body->getVelocity(&velocity);
			result->p += velocity * 0.003;
			return result;
		}
	}
	result->SetIdentity();
	return result;
}

UMath::Vector3* MWSuspensionGetPointVelocity(ISuspension* susp, UMath::Vector3* result, const UMath::Vector3* p) {
	*result = {0,0,0};
	return result;
}

void MWSuspensionAddLocalForceAndTorque(ISuspension* susp, const UMath::Vector3* force, const UMath::Vector3* torque, const UMath::Vector3* driveTorque) {}

void ReplaceSuspensionVTable(uintptr_t getHubWorldMatrix_addr) {
	NyaHookLib::Patch(NyaHookLib::mEXEBase + getHubWorldMatrix_addr, &MWSuspensionGetMatrix); // getHubWorldMatrix
	NyaHookLib::Patch(NyaHookLib::mEXEBase + getHubWorldMatrix_addr+8, &MWSuspensionGetPointVelocity); // getPointVelocity
	NyaHookLib::Patch(NyaHookLib::mEXEBase + getHubWorldMatrix_addr+0x18, &MWSuspensionGetPointVelocity); // getHubAngularVelocity
	NyaHookLib::Patch(NyaHookLib::mEXEBase + getHubWorldMatrix_addr+0xA0, &MWSuspensionGetPointVelocity); // getVelocity
	NyaHookLib::Patch(NyaHookLib::mEXEBase + getHubWorldMatrix_addr+0xC0, &MWSuspensionAddLocalForceAndTorque); // addLocalForceAndTorque
}

UMath::Matrix4* MWSuspensionGetMatrix_DeleteBody(Suspension* susp, UMath::Matrix4* result) {
	if (susp->hub) {
		susp->hub->release();
		susp->hub = nullptr;
	}
	return MWSuspensionGetMatrix(susp, result);
}

UMath::Matrix4* MWSuspensionStrutGetMatrix_DeleteBody(SuspensionStrut* susp, UMath::Matrix4* result) {
	if (susp->hub) {
		susp->hub->release();
		susp->hub = nullptr;
	}
	return MWSuspensionGetMatrix(susp, result);
}

UMath::Matrix4* MWSuspensionMLGetMatrix_DeleteBody(SuspensionML* susp, UMath::Matrix4* result) {
	if (susp->hub) {
		susp->hub->release();
		susp->hub = nullptr;
	}
	return MWSuspensionGetMatrix(susp, result);
}

void OnPluginStartup() {
	SwitchToMWPhysics();

	NyaHookLib::PatchRelative(NyaHookLib::JMP, NyaHookLib::mEXEBase + 0x275DA0, &MWCarUpdate);
	ReplaceSuspensionVTable(0x4FF878);
	ReplaceSuspensionVTable(0x4FFC88);
	ReplaceSuspensionVTable(0x4FFE98);
	ReplaceSuspensionVTable(0x5001A8);
	NyaHookLib::Patch(0x4FF878, &MWSuspensionGetMatrix_DeleteBody); // Suspension
	NyaHookLib::Patch(0x4FFC88, &MWSuspensionStrutGetMatrix_DeleteBody); // SuspensionStrut
	//NyaHookLib::Patch(0x4FFE98, &MWSuspensionGetMatrix_DeleteBody); // SuspensionAxle
	NyaHookLib::Patch(0x5001A8, &MWSuspensionMLGetMatrix_DeleteBody); // SuspensionML

	// remove suspension attach calls
	//NyaHookLib::Patch<uint8_t>(NyaHookLib::mEXEBase + 0x2C4100, 0xC3);
	//NyaHookLib::Patch<uint8_t>(NyaHookLib::mEXEBase + 0x2C0FB0, 0xC3);
}

BOOL WINAPI DllMain(HINSTANCE, DWORD fdwReason, LPVOID) {
	switch(fdwReason) {
		case DLL_PROCESS_ATTACH: {
			if (NyaHookLib::GetEntryPoint() != 0x15AE310) {
				MessageBoxA(nullptr, "Unsupported game version! Make sure you're using the latest x64 Steam release (.exe size of 22890776 bytes)", "nya?!~", MB_ICONERROR);
				return TRUE;
			}

			if (std::filesystem::exists("AssettoCorsaMWPhysics_gcp.toml")) {
				auto config = toml::parse_file("AssettoCorsaMWPhysics_gcp.toml");
				bSpeedbreakerEnabled = config["speedbreaker"].value_or(bSpeedbreakerEnabled);
				bRevLimiter = config["rev_limiter"].value_or(bRevLimiter);
				fUpgradeLevel = config["upgrade_level"].value_or(fUpgradeLevel);
			}

			WriteLog("Mod initialized");

			//MessageBoxA(nullptr, std::format("Base address {:X}", NyaHookLib::mEXEBase).c_str(), "nya?!~", 0);
		} break;
		default:
			break;
	}
	return TRUE;
}