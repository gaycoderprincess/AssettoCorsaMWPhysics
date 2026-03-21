#include <windows.h>
#include <format>
#include <codecvt>
#include <filesystem>
#include "toml++/toml.hpp"
#include "nya_commonhooklib.h"

#include "nya_commonmath.h"

#include "nya_commonaudio.cpp"
#include "nya_commontimer.cpp"

#include "ac.h"

void OnPluginStartup();

bool bRevLimiter = true;
bool bSpeedbreakerEnabled = true;
bool bNitrousEnabled = true;
bool bCSPHacks = false;
float fUpgradeLevel = 1.0;
float fTireOffset = 0.1;

#include "util.h"
#include "inputs.h"

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

EngineRacer* pMWEngine;
SuspensionRacerMW* pMWSuspension;
void ACCarPrePhysics(Car* pThis, float dT) {
	pThis->pollControls(dT);

	auto lastLights = pThis->controlsProvider->getAction(DriverActions::eHeadlightsSwitch);
	if (pThis->controlsProvider && lastLights && !pThis->lastLigthSwitchState) {
		pThis->lightsOn = !pThis->lightsOn;
	}
	pThis->lastLigthSwitchState = lastLights;

	if (pThis->blackFlagged && !pThis->isInPits()) {
		// todo
	}

	// todo penalty code here

	pThis->finalSteerAngleSignal = DEG2RAD(pMWSuspension->mSteering.Previous);
	pThis->body->getVelocity(&pThis->lastVelocity);
}

void ACCarPostPhysics(Car* pThis, float dT) {
	// vanilla components
	pThis->colliderManager.step(dT);
	pThis->setupManager.step(dT);
	if (!pThis->physicsGUID) {
		pThis->telemetry.step(dT);
		pThis->driftMode.step(dT);
		pThis->performanceMeter.step(dT);
		pThis->lapInvalidator.step(dT);
		pThis->penaltyManager.step(dT);
	}
	pThis->splineLocator.step(dT);
	pThis->transponder.step(dT);
	pThis->fuelLapEvaluator.step(dT);

	pThis->updateColliderStatus(dT);
	if (!pThis->physicsGUID) {
		pThis->stepJumpStart(dT);
	}
}

void DoShifting(Car* pCar) {
	auto iinput = GetPlayerInterface(pCar)->Find<IInput>();

	if (pCar->controls.requestedGearIndex != -1) {
		auto nextGear = pCar->controls.requestedGearIndex;
		if (nextGear != pMWEngine->GetGear()) {
			if (iinput->IsAutomaticShift()) {
				pMWEngine->SportShift((GearID)nextGear);
			}
			else {
				pMWEngine->Shift((GearID)nextGear);
			}
		}
		return;
	}

	static bool bLastUp = false;
	auto currentUp = pCar->controls.gearUp;
	if (currentUp && !bLastUp && pMWEngine->GetGear() != pMWEngine->GetTopGear()) {
		auto nextGear = pMWEngine->GetGear()+1;
		if (iinput->IsAutomaticShift()) {
			if (nextGear == G_NEUTRAL) nextGear = G_FIRST;
			if (nextGear == G_FIRST) {
				pMWEngine->Shift((GearID)nextGear);
			}
			else {
				pMWEngine->SportShift((GearID)nextGear);
			}
		}
		else {
			pMWEngine->Shift((GearID)nextGear);
		}
	}
	bLastUp = currentUp;

	static bool bLastDown = false;
	auto currentDown = pCar->controls.gearDn;
	if (currentDown && !bLastDown && pMWEngine->GetGear() != G_REVERSE) {
		auto nextGear = pMWEngine->GetGear()-1;
		if (iinput->IsAutomaticShift()) {
			if (nextGear == G_NEUTRAL) nextGear = G_REVERSE;
			if (nextGear == G_REVERSE) {
				pMWEngine->Shift((GearID)nextGear);
			}
			else {
				pMWEngine->SportShift((GearID)nextGear);
			}
		}
		else {
			pMWEngine->Shift((GearID)nextGear);
		}
	}
	bLastDown = currentDown;
}

CNyaTimer gRealTimer;
void __fastcall MWCarUpdate(Car* pThis, float dT) {
	if (pThis != pMyPlugin->car) return;
	if (pMyPlugin->sim->physicsAvatar->isPaused) return;

	gRealTimer.Process();

	ACCarPrePhysics(pThis, dT);

	SimSystem::fSimTime += dT;
	fGlobalDeltaTime = dT;

	DoShifting(pThis);

	pMWEngine->OnTaskSimulate(dT);
	pMWSuspension->OnTaskSimulate(dT);

	for (int i = 0; i < 4; i++) {
		int mwTireId = GetMWWheelID(i);
		auto mwTire = pMWSuspension->mTires[mwTireId];
		auto tire = &pThis->tyres[i];
		UMath::Matrix4 carMatrix;
		tire->car->body->getWorldMatrix(&carMatrix, 0.0);

		UMath::Matrix4 steerAngle;
		steerAngle.SetIdentity();
		steerAngle.Rotate(NyaVec3(0, 0, ANGLE2RAD(-pMWSuspension->GetWheelSteer(mwTireId))));
		tire->worldRotation = carMatrix * steerAngle;

		tire->localWheelRotation.Rotate(NyaVec3(-pMWSuspension->GetWheelAngularVelocity(mwTireId) * dT, 0, 0));

		tire->worldRotation = carMatrix * steerAngle * tire->localWheelRotation;

		//tire->worldPosition = *pMWSuspension->GetWheelPos(i);
		tire->contactPoint = tire->unmodifiedContactPoint = mwTire->mWorldPos.fHitPosition;
		tire->contactNormal = UMath::Vector4To3(mwTire->mNormal);
		tire->status.angularVelocity = mwTire->GetAngularVelocity();
		tire->status.distToGround = pMWSuspension->GetWheelRoadHeight(mwTireId);
		tire->status.load = mwTire->GetLoad(); // todo this is 0.0-1.0
		tire->status.isLocked = mwTire->IsBrakeLocked();
		tire->status.slipAngleRAD = mwTire->GetSlipAngle();
		tire->status.slipRatio = 1.0 - mwTire->GetTraction();
		tire->status.ndSlip = 1.0 - mwTire->GetTraction();
		tire->slidingVelocityX = mwTire->GetLateralSpeed();
		tire->slidingVelocityY = mwTire->GetRoadSpeed() * tire->status.slipAngleRAD;
		tire->totalSlideVelocity = std::abs(tire->slidingVelocityX) + std::abs(tire->slidingVelocityY);
		tire->roadVelocityX = mwTire->GetLateralSpeed();
		tire->roadVelocityY = mwTire->GetRoadSpeed();
		tire->surfaceDef = mwTire->mWorldPos.fSurface;
		tire->driven = pMWSuspension->IsDriveWheel(mwTireId);
		tire->status.normalizedSlideX = tire->slidingVelocityX / tire->totalSlideVelocity;
		tire->status.normalizedSlideY = tire->slidingVelocityY / tire->totalSlideVelocity;
	}

	if (pThis->rigidAxle) {
		pThis->rigidAxle->release();
		pThis->rigidAxle = nullptr;
	}

	pThis->drivetrain.currentGear = pMWEngine->GetGear();
	pThis->drivetrain.isGearGrinding = pMWEngine->IsGearChanging();
	pThis->drivetrain.acEngine.status.turboBoost = pMWEngine->GetInductionPSI(); // todo is this correct?
	pThis->drivetrain.acEngine.lastInput.gasInput = GetPlayerInterface(pThis)->Find<IInput>()->GetControlGas();
	pThis->drivetrain.acEngine.lastInput.carSpeed = GetPlayerInterface(pThis)->Find<IVehicle>()->GetAbsoluteSpeed();

	auto rpm = pMWEngine->GetRPM();
	rpm /= pMWEngine->GetMaxRPM();
	rpm *= pThis->drivetrain.acEngine.data.limiter;

	pThis->drivetrain.acEngine.lastInput.rpm = rpm;
	pThis->drivetrain.engine.oldVelocity = pThis->drivetrain.engine.velocity;
	pThis->drivetrain.engine.velocity = (rpm * 6.28318029705) / 60.0;

	// reusing the fuel gauge as nitrous
	if (bNitrousEnabled) {
		pThis->fuel = pMWEngine->GetNOSCapacity() * pThis->maxFuel;

		// getting below 1/8th of the bar causes a fuel warning popup
		//pThis->fuel *= 0.875;
		//pThis->fuel += 0.125 * pThis->maxFuel;
		pThis->fuel = std::max(pThis->fuel, 0.01);
	}

	pThis->controls.gas = GetPlayerInterface(pThis)->Find<IInput>()->GetControlGas();
	pThis->controls.brake = GetPlayerInterface(pThis)->Find<IInput>()->GetControlBrake();
	pThis->controls.handBrake = GetPlayerInterface(pThis)->Find<IInput>()->GetControlHandBrake();

	static bool bNOSLast = false;
	if (bNOSLast != pMWEngine->IsNOSEngaged()) {
		static auto audio = NyaAudio::LoadFile("plugins/nitro_on.wav");
		if (audio && pMWEngine->IsNOSEngaged()) {
			NyaAudio::Stop(audio);
			NyaAudio::SkipTo(audio, 0, false);
			NyaAudio::SetVolume(audio, 1.0);
			NyaAudio::Play(audio);
		}
	}
	bNOSLast = pMWEngine->IsNOSEngaged();

	ACCarPostPhysics(pThis, dT);

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

	WriteLog(std::format("MW max RPM {}", pMWEngine->GetMaxRPM()));
	WriteLog(std::format("AC max RPM {}", ply->drivetrain.acEngine.data.limiter));
}

UMath::Matrix4* MWSuspensionGetMatrix(ISuspension* susp, UMath::Matrix4* result) {
	auto car = pMyPlugin->car;
	for (int i = 0; i < 4; i++) {
		if (car->tyres[i].hub == susp) {
			car->body->getWorldMatrix(result, 0.0);
			pMWSuspension->GetWheelCenterPos(&result->p, GetMWWheelID(i));
			UMath::Vector3 velocity;
			car->body->getVelocity(&velocity);
			result->p += velocity * 0.003; // this doesn't seem to work in csp
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

UMath::Vector3* MWSuspensionGetVelocity(ISuspension* susp, UMath::Vector3* result) {
	*result = {0,0,0};
	return result;
}

void MWSuspensionAddLocalForceAndTorque(ISuspension* susp, const UMath::Vector3* force, const UMath::Vector3* torque, const UMath::Vector3* driveTorque) {}
void MWSuspensionAttach(ISuspension* susp) {}
void MWSuspensionStop(ISuspension* susp) {}
void MWSuspensionStep(ISuspension* susp, float dt) {}
float MWSuspensionGetMass(ISuspension* susp) { return 1.0; }
void MWSuspensionAddForceAtPos(ISuspension* susp, const UMath::Vector3* force, const UMath::Vector3* pos, int64_t driven, bool addToSteerTorque) {}

void ReplaceSuspensionVTable(uintptr_t getHubWorldMatrix_addr) {
	NyaHookLib::Patch(NyaHookLib::mEXEBase + getHubWorldMatrix_addr, &MWSuspensionGetMatrix); // getHubWorldMatrix
	NyaHookLib::Patch(NyaHookLib::mEXEBase + getHubWorldMatrix_addr+0x8, &MWSuspensionGetPointVelocity); // getPointVelocity
	NyaHookLib::Patch(NyaHookLib::mEXEBase + getHubWorldMatrix_addr+0x10, &MWSuspensionAddForceAtPos); // addForceAtPos
	NyaHookLib::Patch(NyaHookLib::mEXEBase + getHubWorldMatrix_addr+0x18, &MWSuspensionGetVelocity); // addTorque
	NyaHookLib::Patch(NyaHookLib::mEXEBase + getHubWorldMatrix_addr+0x30, &MWSuspensionGetVelocity); // getHubAngularVelocity
	NyaHookLib::Patch(NyaHookLib::mEXEBase + getHubWorldMatrix_addr+0x38, &MWSuspensionAttach); // attach
	NyaHookLib::Patch(NyaHookLib::mEXEBase + getHubWorldMatrix_addr+0x88, &MWSuspensionGetMass); // getMass
	NyaHookLib::Patch(NyaHookLib::mEXEBase + getHubWorldMatrix_addr+0x90, &MWSuspensionStop); // stop
	NyaHookLib::Patch(NyaHookLib::mEXEBase + getHubWorldMatrix_addr+0x98, &MWSuspensionGetVelocity); // getVelocity
	NyaHookLib::Patch(NyaHookLib::mEXEBase + getHubWorldMatrix_addr+0xA8, &MWSuspensionStep); // step
	NyaHookLib::Patch(NyaHookLib::mEXEBase + getHubWorldMatrix_addr+0xB8, &MWSuspensionAddLocalForceAndTorque); // addLocalForceAndTorque
}

UMath::Matrix4* MWSuspensionGetMatrix_DeleteBody(Suspension* susp, UMath::Matrix4* result) {
	if (susp->hub) {
		susp->hub->release();
		susp->hub = nullptr;
		WriteLog("ISuspension type: Suspension");
	}
	return MWSuspensionGetMatrix(susp, result);
}

UMath::Matrix4* MWSuspensionStrutGetMatrix_DeleteBody(SuspensionStrut* susp, UMath::Matrix4* result) {
	if (susp->hub) {
		susp->hub->release();
		susp->hub = nullptr;
		WriteLog("ISuspension type: SuspensionStrut");
	}
	if (susp->strutBody) {
		susp->strutBody->release();
		susp->strutBody = nullptr;
	}
	return MWSuspensionGetMatrix(susp, result);
}

UMath::Matrix4* MWSuspensionMLGetMatrix_DeleteBody(SuspensionML* susp, UMath::Matrix4* result) {
	if (susp->hub) {
		susp->hub->release();
		susp->hub = nullptr;
		WriteLog("ISuspension type: SuspensionML");
	}
	return MWSuspensionGetMatrix(susp, result);
}

void OnPluginStartup() {
	if (std::filesystem::exists("plugins/AssettoCorsaMWPhysics_gcp.toml")) {
		auto config = toml::parse_file("plugins/AssettoCorsaMWPhysics_gcp.toml");
		//bSpeedbreakerEnabled = config["speedbreaker"].value_or(bSpeedbreakerEnabled);
		bNitrousEnabled = config["nitrous"].value_or(bNitrousEnabled);
		bRevLimiter = config["rev_limiter"].value_or(bRevLimiter);
		fUpgradeLevel = config["upgrade_level"].value_or(fUpgradeLevel);
		fTireOffset = config["tire_y_offset"].value_or(fTireOffset);
	}

	SwitchToMWPhysics();

	NyaHookLib::PatchRelative(NyaHookLib::JMP, NyaHookLib::mEXEBase + 0x275DA0, &MWCarUpdate);
	//NyaHookLib::PatchRelative(NyaHookLib::JMP, NyaHookLib::mEXEBase + 0x26308C, &MWCarUpdate);
	//NyaHookLib::PatchRelative(NyaHookLib::CALL, NyaHookLib::mEXEBase + 0x2648F9, &MWCarUpdate);
	//NyaHookLib::PatchRelative(NyaHookLib::CALL, NyaHookLib::mEXEBase + 0x264B0E, &MWCarUpdate);
	ReplaceSuspensionVTable(0x4FF878);
	ReplaceSuspensionVTable(0x4FFC88);
	ReplaceSuspensionVTable(0x4FFE98);
	ReplaceSuspensionVTable(0x5001A8);
	NyaHookLib::Patch(NyaHookLib::mEXEBase + 0x4FF878, &MWSuspensionGetMatrix_DeleteBody); // Suspension
	NyaHookLib::Patch(NyaHookLib::mEXEBase + 0x4FFC88, &MWSuspensionStrutGetMatrix_DeleteBody); // SuspensionStrut
	//NyaHookLib::Patch(NyaHookLib::mEXEBase + 0x4FFE98, &MWSuspensionAxleGetMatrix_DeleteBody); // SuspensionAxle
	NyaHookLib::Patch(NyaHookLib::mEXEBase + 0x5001A8, &MWSuspensionMLGetMatrix_DeleteBody); // SuspensionML

	if (bCSPHacks) {
		NyaHookLib::Patch<uint8_t>(NyaHookLib::mEXEBase + 0x2764D0, 0xC3); // disable Car::stepComponents
		NyaHookLib::Patch<uint8_t>(NyaHookLib::mEXEBase + 0x283800, 0xC3); // disable Tyre::step
	}

	NyaAudio::Init((HWND)pMyPlugin->sim->game->window.hWnd);

	WriteLog("Mod initialized");
}

BOOL WINAPI DllMain(HINSTANCE, DWORD fdwReason, LPVOID) {
	switch(fdwReason) {
		case DLL_PROCESS_ATTACH: {
			if (NyaHookLib::GetEntryPoint() != 0x15AE310) {
				MessageBoxA(nullptr, "Unsupported game version! Make sure you're using the latest x64 Steam release (.exe size of 22890776 bytes)", "nya?!~", MB_ICONERROR);
				return TRUE;
			}

			//MessageBoxA(nullptr, std::format("Base address {:X}", NyaHookLib::mEXEBase).c_str(), "nya?!~", 0);
		} break;
		default:
			break;
	}
	return TRUE;
}