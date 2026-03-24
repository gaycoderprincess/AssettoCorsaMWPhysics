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
bool bSpeedbreakerEnabled = false;
bool bNitrousEnabled = true;
bool bCSPHacks = false;
float fUpgradeLevel = 1.0;
float fTireOffset = 0.1;
float fSteeringWheelLock = 360.0;

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

std::vector<EngineRacer*> aEngines;
std::vector<SuspensionRacerMW*> aSuspensions;
EngineRacer* GetCarMWEngine(Car* pCar) {
	for (auto& engine : aEngines) {
		if (engine->pCar == pCar) return engine;
	}
	return nullptr;
}

SuspensionRacerMW* GetCarMWSuspension(Car* pCar) {
	for (auto& susp : aSuspensions) {
		if (susp->pCar == pCar) return susp;
	}
	return nullptr;
}

void ACCarPrePhysics(Car* pThis, float dT) {
	if (bCSPHacks) return;

	pThis->pollControls(dT);

	auto lastLights = pThis->controlsProvider->getAction(DriverActions::eHeadlightsSwitch);
	if (pThis->controlsProvider && lastLights && !pThis->lastLigthSwitchState) {
		pThis->lightsOn = !pThis->lightsOn;
	}
	pThis->lastLigthSwitchState = lastLights;

	if (pThis->blackFlagged && !pThis->isInPits()) {
		// todo
	}

	pThis->drivetrain.acEngine.fuelPressure = 1.0;

	// todo penalty code here
	pThis->penaltyTime = 0.0;
	pThis->penaltyTimeAccumulator = 0.0;

	pThis->finalSteerAngleSignal = DEG2RAD(GetCarMWSuspension(pThis)->mSteering.Previous);

	UMath::Vector3 worldVel;
	pThis->body->getVelocity(&worldVel);

	auto absAcc = (worldVel - pThis->lastVelocity) * (1.0 / dT) * 0.10197838;
	pThis->body->worldToLocalNormal(&pThis->accG, &absAcc);

	pThis->lastVelocity = worldVel;
}

void ACCarPostPhysics(Car* pThis, float dT) {
	if (bCSPHacks) {
		pThis->setupManager.checkRules = false;
		pThis->setupManager.waitTime = 0.0;
		pThis->setupManager.setupState = CarSetupState::Legal;
		return;
	}

	// vanilla components
	pThis->colliderManager.step(dT);
	//pThis->setupManager.step(dT); // crash in PhysicsCore::reseatDistanceJointLocal
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
	auto transmission = GetPlayerInterface(pCar)->Find<ITransmission>();
	auto tiptronic = GetPlayerInterface(pCar)->Find<ITiptronic>();
	if (!GetPlayerInterface(pCar)->Find<IHumanAI>()) return; // ai uses automatic shift

	if (pCar->controls.requestedGearIndex != -1) {
		auto nextGear = pCar->controls.requestedGearIndex;
		if (nextGear != transmission->GetGear()) {
			if (iinput->IsAutomaticShift()) {
				tiptronic->SportShift((GearID)nextGear);
			}
			else {
				transmission->Shift((GearID)nextGear);
			}
		}
		return;
	}

	static bool bLastUp = false;
	auto currentUp = pCar->controls.gearUp;
	if (currentUp && !bLastUp && transmission->GetGear() != transmission->GetTopGear()) {
		auto nextGear = transmission->GetGear()+1;
		if (iinput->IsAutomaticShift()) {
			if (nextGear == G_NEUTRAL) nextGear = G_FIRST;
			if (nextGear == G_FIRST) {
				transmission->Shift((GearID)nextGear);
			}
			else {
				tiptronic->SportShift((GearID)nextGear);
			}
		}
		else {
			transmission->Shift((GearID)nextGear);
		}
	}
	bLastUp = currentUp;

	static bool bLastDown = false;
	auto currentDown = pCar->controls.gearDn;
	if (currentDown && !bLastDown && transmission->GetGear() != G_REVERSE) {
		auto nextGear = transmission->GetGear()-1;
		if (iinput->IsAutomaticShift()) {
			if (nextGear == G_NEUTRAL) nextGear = G_REVERSE;
			if (nextGear == G_REVERSE) {
				transmission->Shift((GearID)nextGear);
			}
			else {
				tiptronic->SportShift((GearID)nextGear);
			}
		}
		else {
			transmission->Shift((GearID)nextGear);
		}
	}
	bLastDown = currentDown;
}

void SwitchToMWPhysics(Car* ply) {
	if (GetCarMWEngine(ply) || GetCarMWSuspension(ply)) return;

	int playerId = -1;
	for (auto& data : aPlayerInterfaces) {
		if (data.pCar == nullptr) {
			playerId = &data - &aPlayerInterfaces[0];
			break;
		}
	}

	WriteLog(std::format("Assigning car {:X} to player ID {}", (uintptr_t)ply, playerId));

	if (playerId < 0) return;

	aPlayerInterfaces[playerId].aInterfaces.clear();
	aPlayerInterfaces[playerId].pCar = ply;
	aPlayerInterfaces[playerId].Add(new IVehicle(ply));
	aPlayerInterfaces[playerId].Add(new IRigidBodyMW(ply));
	aPlayerInterfaces[playerId].Add(new ICollisionBody(ply));
	if (playerId == 0) {
		aPlayerInterfaces[playerId].Add(new IInput(ply));
		aPlayerInterfaces[playerId].Add(new IPlayer(ply));
		aPlayerInterfaces[playerId].Add(new IHumanAI());
	}
	else {
		aPlayerInterfaces[playerId].Add(new IInputAI(ply));
	}

	auto engine = new EngineRacer(ply);
	auto susp = new SuspensionRacerMW(ply);
	engine->OnBehaviorChange();
	susp->OnBehaviorChange();
	aEngines.push_back(engine);
	aSuspensions.push_back(susp);

	//ply->mass = susp->mMWAttributes->MASS;
	//ply->bodyInertia = {susp->mMWAttributes->TENSOR_SCALE[0],susp->mMWAttributes->TENSOR_SCALE[1],susp->mMWAttributes->TENSOR_SCALE[2]};
	//ply->body->setMassBox(ply->mass, ply->bodyInertia.x, ply->bodyInertia.y, ply->bodyInertia.z);

	WriteLog(std::format("MW max RPM {}", engine->GetMaxRPM()));
	WriteLog(std::format("AC max RPM {}", ply->drivetrain.acEngine.defaultEngineLimiter));
}

float fOverrideTimescale = 1.0;
void DoGameBreaker(float real_time_delta, IPlayer* player) {
	player->DoGameBreaker(real_time_delta * fOverrideTimescale, real_time_delta);

	float speed = 1.0;
	float target = 1.0;
	if (player->InGameBreaker()) {
		target = 0.25;
		speed = 2.0;
	}
	else {
		target = 1.0;
		speed = 0.5;
	}

	if (target == fOverrideTimescale) return;

	auto modify = speed * real_time_delta;
	if (fOverrideTimescale < target) {
		fOverrideTimescale += modify;
		if (fOverrideTimescale > target) fOverrideTimescale = target;
	}
	else if (fOverrideTimescale > target) {
		fOverrideTimescale -= modify;
		if (fOverrideTimescale > target) fOverrideTimescale = target;
	}
}

void SpeedbreakerLoop() {
	if (!bSpeedbreakerEnabled) return;

	static CNyaTimer gRealTimer;
	gRealTimer.Process();

	if (auto pPlayer = GetPlayerInterface(pMyPlugin->car)->GetPlayer()) {
		if (IsKeyJustPressed('X') || IsPadKeyJustPressed(NYA_PAD_KEY_X, -1)) {
			pPlayer->ToggleGameBreaker();
		}
		DoGameBreaker(gRealTimer.fDeltaTime, pPlayer);
	}
}

std::mutex mCarUpdateMutex;
void __fastcall MWCarUpdate(Car* pCar, float dT) {
	if (pMyPlugin->sim->physicsAvatar->isPaused) return;

	if (!bCSPHacks) {
		pMyPlugin->sim->physicsAvatar->engine.core->id->dWorldSetGravity(0.0, -9.8128, 0.0); // rigidbodyspecs, exact car gravity in MW
	}

	ACCarPrePhysics(pCar, dT);

	if (pCar == pMyPlugin->car) {
		SimSystem::fSimTime += dT;
		SpeedbreakerLoop();
	}
	fGlobalDeltaTime = dT;

	EngineRacer* pEngine = GetCarMWEngine(pCar);
	SuspensionRacerMW* pSuspension = GetCarMWSuspension(pCar);
	if (!pEngine || !pSuspension) return;

	mCarUpdateMutex.lock();

	pCar->steerLock = fSteeringWheelLock;

	auto humanai = GetPlayerInterface(pCar)->Find<IHumanAI>();
	if (humanai) {
		DoShifting(pCar);
	}

	auto ivehicle = GetPlayerInterface(pCar)->Find<IVehicle>();

	// hack to fix start positions
	if (ivehicle->IsStaging() && pCar->hasGridPosition && !pCar->isInPits()) {
		UMath::Vector3 p;
		pCar->body->getPosition(&p, 0.0);
		p.x = pCar->gridPosition.x;
		p.z = pCar->gridPosition.z;
		pCar->body->setPosition(&p);

		UMath::Vector3 vel;
		pCar->body->getVelocity(&vel);
		vel.x = 0.0;
		vel.z = 0.0;
		pCar->body->setVelocity(&vel);
	}

	ivehicle->OnTaskSimulate(dT);
	pEngine->OnTaskSimulate(dT);
	pSuspension->OnTaskSimulate(dT);

	GetPlayerInterface(pCar)->Find<IRigidBodyMW>()->DoDrag();

	for (int i = 0; i < 4; i++) {
		int mwTireId = GetMWWheelID(i);
		auto mwTire = pSuspension->mTires[mwTireId];
		auto tire = &pCar->tyres[i];
		UMath::Matrix4 carMatrix;
		tire->car->body->getWorldMatrix(&carMatrix, 0.0);

		UMath::Matrix4 steerAngle;
		steerAngle.SetIdentity();
		steerAngle.Rotate(NyaVec3(0, 0, ANGLE2RAD(-pSuspension->GetWheelSteer(mwTireId))));
		tire->worldRotation = carMatrix * steerAngle;

		tire->localWheelRotation.Rotate(NyaVec3(-pSuspension->GetWheelAngularVelocity(mwTireId) * dT, 0, 0));

		tire->worldRotation = carMatrix * steerAngle * tire->localWheelRotation;
		tire->worldRotation.p = {0,0,0};

		pSuspension->GetWheelCenterPos(&tire->worldPosition, mwTireId);
		tire->contactPoint = tire->unmodifiedContactPoint = mwTire->mWorldPos.fHitPosition;
		tire->contactNormal = UMath::Vector4To3(mwTire->mNormal);
		tire->status.angularVelocity = mwTire->GetAngularVelocity();
		tire->status.distToGround = pSuspension->GetWheelRoadHeight(mwTireId);
		tire->status.load = mwTire->GetLoad();
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
		tire->driven = pSuspension->IsDriveWheel(mwTireId);
		tire->status.normalizedSlideX = tire->slidingVelocityX / tire->totalSlideVelocity;
		tire->status.normalizedSlideY = tire->slidingVelocityY / tire->totalSlideVelocity;

		// wth is this?
		if (bCSPHacks) {
			tire->status.Fx = 0.0;
			tire->status.Fy = 0.0;
			tire->status.Mz = 0.0;
			tire->status.Dx = 0.0;
			tire->status.Dy = 0.0;
			tire->status.D = 0.0;
		}
	}

	pCar->onTyresStepCompleted();

	if (pCar->rigidAxle) {
		pCar->rigidAxle->release();
		pCar->rigidAxle = nullptr;
	}

	if (pCar->fuelTankBody) {
		pCar->fuelTankBody->release();
		pCar->fuelTankBody = nullptr;
	}

	auto iinput = GetPlayerInterface(pCar)->Find<IInput>();
	pCar->controls.gas = iinput->GetControlGas();
	pCar->controls.brake = iinput->GetControlBrake();
	pCar->controls.handBrake = iinput->GetControlHandBrake();

	// todo calculate
	pCar->brakeSystem.frontBias = pCar->brakeSystem.biasOverride = 0.5;

	pCar->drivetrain.currentGear = pEngine->GetGear();
	pCar->drivetrain.isGearGrinding = pEngine->IsGearChanging();
	pCar->drivetrain.acEngine.status.turboBoost = pEngine->mInductionBoost; // todo is this correct?
	pCar->drivetrain.acEngine.lastInput.gasInput = iinput->GetControlGas();
	pCar->drivetrain.acEngine.lastInput.carSpeed = GetPlayerInterface(pCar)->Find<IVehicle>()->GetAbsoluteSpeed();

	auto rpm = pEngine->GetRPM();
	rpm /= pEngine->GetMaxRPM();
	rpm *= pCar->drivetrain.acEngine.data.limiter;

	pCar->drivetrain.acEngine.lastInput.rpm = rpm;
	pCar->drivetrain.engine.oldVelocity = pCar->drivetrain.engine.velocity;
	pCar->drivetrain.engine.velocity = (rpm * 6.28318029705) / 60.0;

	// reusing the fuel gauge as nitrous
	if (bNitrousEnabled) {
		pCar->fuel = pEngine->GetNOSCapacity() * pCar->maxFuel;

		// getting below 1/8th of the bar causes a fuel warning popup
		//pCar->fuel *= 0.875;
		//pCar->fuel += 0.125 * pCar->maxFuel;
		pCar->fuel = std::max(pCar->fuel, 0.01);
	}
	else {
		auto turboConsumption = pEngine->mInductionBoost + 1.0;
		pCar->fuel -= rpm * dT * pCar->drivetrain.acEngine.gasUsage * turboConsumption * pCar->fuelConsumptionK * 0.001 * pCar->ksPhysics->fuelConsumptionRate;
	}

	if (humanai) {
		static bool bNOSLast = false;
		if (bNOSLast != pEngine->IsNOSEngaged()) {
			static auto audio = NyaAudio::LoadFile("plugins/nitro_on.wav");
			if (audio && pEngine->IsNOSEngaged()) {
				NyaAudio::Stop(audio);
				NyaAudio::SkipTo(audio, 0, false);
				NyaAudio::SetVolume(audio, 1.0);
				NyaAudio::Play(audio);
			}
		}
		static auto audio = NyaAudio::LoadFile("plugins/nitro_loop.wav");
		if (audio) {
			if (pEngine->IsNOSEngaged()) {
				if (NyaAudio::IsFinishedPlaying(audio)) {
					NyaAudio::SkipTo(audio, 0, false);
					NyaAudio::SetVolume(audio, 1.0);
					NyaAudio::Play(audio);
				}
			}
			else {
				NyaAudio::Stop(audio);
			}
		}
		bNOSLast = pEngine->IsNOSEngaged();

		RefreshInputs();
	}

	ACCarPostPhysics(pCar, dT);

	mCarUpdateMutex.unlock();
}

UMath::Matrix4* MWSuspensionGetMatrix(Car* car, ISuspension* susp, UMath::Matrix4* result) {
	auto mwSusp = GetCarMWSuspension(car);
	if (!mwSusp) {
		WriteLog("MWSuspensionGetMatrix: failed to find SuspensionRacer!");
		result->SetIdentity();
		return result;
	}
	for (int i = 0; i < 4; i++) {
		if (car->tyres[i].hub == susp) {
			car->body->getWorldMatrix(result, 0.0);
			mwSusp->GetWheelCenterPos(&result->p, GetMWWheelID(i));
			UMath::Vector3 velocity;
			car->body->getVelocity(&velocity);
			result->p += velocity * 0.003;
			return result;
		}
	}
	WriteLog("MWSuspensionGetMatrix: failed to find tire!");
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
	return MWSuspensionGetMatrix(susp->car, susp, result);
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
	return MWSuspensionGetMatrix(susp->car, susp, result);
}

UMath::Matrix4* MWSuspensionAxleGetMatrix_DeleteBody(SuspensionAxle* susp, UMath::Matrix4* result) {
	if (susp->axle) {
		susp->axle = nullptr;
		WriteLog("ISuspension type: SuspensionAxle");
	}
	return MWSuspensionGetMatrix(susp->car, susp, result);
}

UMath::Matrix4* MWSuspensionMLGetMatrix_DeleteBody(SuspensionML* susp, UMath::Matrix4* result) {
	if (susp->hub) {
		susp->hub->release();
		susp->hub = nullptr;
		WriteLog("ISuspension type: SuspensionML");
	}
	return MWSuspensionGetMatrix(susp->car, susp, result);
}

// fixes brakes with keyboard controls, fixes ai brakes
float GetOptimalBrakeHooked() {
	return 1.0;
}

void MWTimeUpdate(PhysicsEngine* pThis, float dt, double currentTime, double gt) {
	pThis->step(dt * fOverrideTimescale, currentTime, gt);
}

void CarResetHooked(Car* pCar) {
	pCar->reset();
	if (auto engine = GetCarMWEngine(pCar)) {
		engine->ChargeNOS(1.0);
		if (auto ply = engine->GetOwner()->GetPlayer()) {
			ply->ResetGameBreaker(true);
		}
		engine->Reset();
	}
	if (auto susp = GetCarMWSuspension(pCar)) {
		susp->Reset();
	}
}

void MWCarUpdateCSP(DRS* pThis, float dt) {
	for (int i = 0; i < pMyPlugin->sim->cars.size(); i++) {
		auto car = pMyPlugin->sim->cars[i]->physics;
		if (&car->drs == pThis) {
			MWCarUpdate(car, dt);
			break;
		}
	}
}

void OnPluginStartup() {
	if (std::filesystem::exists("plugins/AssettoCorsaMWPhysics_gcp.toml")) {
		auto config = toml::parse_file("plugins/AssettoCorsaMWPhysics_gcp.toml");
		bSpeedbreakerEnabled = config["speedbreaker"].value_or(bSpeedbreakerEnabled);
		bNitrousEnabled = config["nitrous"].value_or(bNitrousEnabled);
		bRevLimiter = config["rev_limiter"].value_or(bRevLimiter);
		fUpgradeLevel = config["upgrade_level"].value_or(fUpgradeLevel);
		fTireOffset = config["tire_y_offset"].value_or(fTireOffset);
		fSteeringWheelLock = config["steering_wheel_lock"].value_or(fSteeringWheelLock);
		bCSPHacks = config["csp_compatibility_hack"].value_or(bCSPHacks);
	}

	if (pMyPlugin->sim->client || bCSPHacks) {
		bSpeedbreakerEnabled = false;
	}

	SwitchToMWPhysics(pMyPlugin->car);
	for (int i = 0; i < pMyPlugin->sim->cars.size(); i++) {
		auto car = pMyPlugin->sim->cars[i]->physics;
		if (car == pMyPlugin->car) continue;
		SwitchToMWPhysics(car);
	}

	if (bSpeedbreakerEnabled) {
		NyaHookLib::PatchRelative(NyaHookLib::CALL, NyaHookLib::mEXEBase + 0x1232DF, &MWTimeUpdate);
		NyaHookLib::PatchRelative(NyaHookLib::CALL, NyaHookLib::mEXEBase + 0x1236BF, &MWTimeUpdate);
	}

	if (!bCSPHacks) {
		NyaHookLib::PatchRelative(NyaHookLib::JMP, NyaHookLib::mEXEBase + 0x275DA0, &MWCarUpdate);
	}
	ReplaceSuspensionVTable(0x4FF878);
	ReplaceSuspensionVTable(0x4FFC88);
	ReplaceSuspensionVTable(0x4FFE98);
	ReplaceSuspensionVTable(0x5001A8);
	NyaHookLib::Patch(NyaHookLib::mEXEBase + 0x4FF878, &MWSuspensionGetMatrix_DeleteBody); // Suspension
	NyaHookLib::Patch(NyaHookLib::mEXEBase + 0x4FFC88, &MWSuspensionStrutGetMatrix_DeleteBody); // SuspensionStrut
	NyaHookLib::Patch(NyaHookLib::mEXEBase + 0x4FFE98, &MWSuspensionAxleGetMatrix_DeleteBody); // SuspensionAxle
	NyaHookLib::Patch(NyaHookLib::mEXEBase + 0x5001A8, &MWSuspensionMLGetMatrix_DeleteBody); // SuspensionML
	NyaHookLib::PatchRelative(NyaHookLib::JMP, NyaHookLib::mEXEBase + 0x27C320, &GetOptimalBrakeHooked);

	NyaHookLib::PatchRelative(NyaHookLib::CALL, NyaHookLib::mEXEBase + 0x26FE7D, &CarResetHooked);

	// remove fuelTankBody references
	NyaHookLib::Fill(NyaHookLib::mEXEBase + 0x270188, 0x90, 0x27019D - 0x270188); // Car::forceRotation
	NyaHookLib::Fill(NyaHookLib::mEXEBase + 0x2701F1, 0x90, 0x270206 - 0x2701F1); // Car::forceRotation

	NyaHookLib::Fill(NyaHookLib::mEXEBase + 0x26FEAC, 0x90, 0x26FEB6 - 0x26FEAC); // Car::forcePosition
	NyaHookLib::Fill(NyaHookLib::mEXEBase + 0x26FECF, 0x90, 0x26FEDF - 0x26FECF); // Car::forcePosition
	NyaHookLib::Fill(NyaHookLib::mEXEBase + 0x26FFE6, 0x90, 0x26FFFB - 0x26FFE6); // Car::forcePosition

	NyaHookLib::Fill(NyaHookLib::mEXEBase + 0x276D79, 0x90, 0x276DD5 - 0x276D79); // Car::updateBodyMass

	if (bCSPHacks) {
		NyaHookLib::Patch<uint8_t>(NyaHookLib::mEXEBase + 0x276AE0, 0xC3); // disable Car::updateAirPressure
		NyaHookLib::Patch<uint8_t>(NyaHookLib::mEXEBase + 0x2B9590, 0xC3); // disable Autoclutch::step
		NyaHookLib::Patch<uint8_t>(NyaHookLib::mEXEBase + 0x2769F0, 0xC3); // disable Car::stepThermalObjects
		//NyaHookLib::Patch<uint8_t>(NyaHookLib::mEXEBase + 0x2764D0, 0xC3); // disable Car::stepComponents
		NyaHookLib::Patch<uint8_t>(NyaHookLib::mEXEBase + 0x28E640, 0xC3); // disable BrakeSystem::step
		NyaHookLib::Patch<uint8_t>(NyaHookLib::mEXEBase + 0x2BB460, 0xC3); // disable EDL::step
		NyaHookLib::Patch<uint8_t>(NyaHookLib::mEXEBase + 0x283800, 0xC3); // disable Tyre::step
		NyaHookLib::Patch<uint8_t>(NyaHookLib::mEXEBase + 0x2B3960, 0xC3); // disable HeaveSpring::step
		NyaHookLib::Patch<uint8_t>(NyaHookLib::mEXEBase + 0x2B4E60, 0xC3); // disable DRS::step
		NyaHookLib::Patch<uint8_t>(NyaHookLib::mEXEBase + 0x2B7150, 0xC3); // disable AeroMap::step
		NyaHookLib::Patch<uint8_t>(NyaHookLib::mEXEBase + 0x2B7E10, 0xC3); // disable Kers::step
		NyaHookLib::Patch<uint8_t>(NyaHookLib::mEXEBase + 0x2930E0, 0xC3); // disable ERS::step
		NyaHookLib::Patch<uint8_t>(NyaHookLib::mEXEBase + 0x2B81B0, 0xC3); // disable SteeringSystem::step
		NyaHookLib::Patch<uint8_t>(NyaHookLib::mEXEBase + 0x2B9EF0, 0xC3); // disable AutoBlip::step
		NyaHookLib::Patch<uint8_t>(NyaHookLib::mEXEBase + 0x2BA7F0, 0xC3); // disable AutoShifter::step
		NyaHookLib::Patch<uint8_t>(NyaHookLib::mEXEBase + 0x2BAB50, 0xC3); // disable GearChanger::step
		NyaHookLib::Patch<uint8_t>(NyaHookLib::mEXEBase + 0x26B130, 0xC3); // disable Drivetrain::step
		NyaHookLib::Patch<uint8_t>(NyaHookLib::mEXEBase + 0x2BB640, 0xC3); // disable AntirollBar::step
		NyaHookLib::Patch<uint8_t>(NyaHookLib::mEXEBase + 0x28F610, 0xC3); // disable ABS::step
		NyaHookLib::Patch<uint8_t>(NyaHookLib::mEXEBase + 0x290200, 0xC3); // disable TractionControl::step
		NyaHookLib::Patch<uint8_t>(NyaHookLib::mEXEBase + 0x2BB910, 0xC3); // disable SpeedLimiter::step
		NyaHookLib::Patch<uint8_t>(NyaHookLib::mEXEBase + 0x28D090, 0xC3); // disable SetupManager::step
		NyaHookLib::Patch<uint8_t>(NyaHookLib::mEXEBase + 0x2BFA50, 0xC3); // disable StabilityControl::step

		NyaHookLib::PatchRelative(NyaHookLib::JMP, NyaHookLib::mEXEBase + 0x2B4E60, &MWCarUpdateCSP); // DRS::step
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