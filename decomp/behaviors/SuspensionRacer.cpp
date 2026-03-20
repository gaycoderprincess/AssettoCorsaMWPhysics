SuspensionRacerMW::SuspensionRacerMW(Car* car) {
	SUSPENSIONRACER_FUNCTION_LOG("Create");

	ChassisMW::Create(car);

	mChassisType = "SuspensionRacer";

	GetOwner()->QueryInterface(&mHumanAI);

	mGameBreaker = 0.0f;
	mNumWheelsOnGround = 0;
	mLastGroundCollision = 0.0f;
	mDrift = Drift();
	mBurnOut = Burnout();
	mSteering = Steering();

	for (int i = 0; i < 4; ++i) {
		mTires[i] = NULL;
	}
	CreateTires();
}

SuspensionRacerMW::~SuspensionRacerMW() {
	SUSPENSIONRACER_FUNCTION_LOG("Destroy");

	for (int i = 0; i < 4; ++i) {
		WriteLog("delete mTires[i]");
		delete mTires[i];
	}

	WriteLog("SuspensionRacerMW::Destroy finished");

	ChassisMW::Destroy();
}

const float ZeroDegreeTable[6] = {0.0f};
float TwoDegreeTable[] = {0.0f, 1.2f, 2.3f, 3.0f, 3.0f, 2.8f};
float FourDegreeTable[] = {0.0f, 1.7f, 3.2f, 4.3f, 5.1f, 5.2f};
float SixDegreeTable[] = {0.0f, 1.8f, 3.5f, 4.9f, 5.8f, 6.1f};
float EightDegreeTable[] = {0.0f, 1.83f, 3.6f, 5.0f, 5.96f, 6.4f};
float TenDegreeTable[] = {0.0f, 1.86f, 3.7f, 5.1f, 6.13f, 6.7f};
float TwelveDegreeTable[] = {0.0f, 1.9f, 3.8f, 5.2f, 6.3f, 7.1f};

Table ZeroDegree = Table(ZeroDegreeTable, 6, 0.0f, 10.0f);
Table TwoDegree = Table(TwoDegreeTable, 6, 0.0f, 10.0f);
Table FourDegree = Table(FourDegreeTable, 6, 0.0f, 10.0f);
Table SixDegree = Table(SixDegreeTable, 6, 0.0f, 10.0f);
Table EightDegree = Table(EightDegreeTable, 6, 0.0f, 10.0f);
Table TenDegree = Table(TenDegreeTable, 6, 0.0f, 10.0f);
Table TwelveDegree = Table(TwelveDegreeTable, 6, 0.0f, 10.0f);

Table *LoadSensitivityTable[] = {&ZeroDegree, &TwoDegree, &FourDegree, &SixDegree, &EightDegree, &TenDegree, &TwelveDegree};
static const float NewCorneringScale = 1000.0f;
static const float LoadFactor = 0.8f;
static const float GripFactor = 2.5f;

static const float PostCollisionSteerReductionDuration = 1.0f;
UMath::Vector2 PostCollisionSteerReductionData[] = {UMath::Vector2(0.0f, 0.2f), UMath::Vector2(0.2f, 0.5f), UMath::Vector2(0.5f, 0.7f), UMath::Vector2(0.7f, 1.0f)};
Graph PostCollisionSteerReductionTable(PostCollisionSteerReductionData, 4);
static const float Tweak_CollisionImpulseSteerMax = 40.0f;
static const float Tweak_CollisionImpulseSteerMin = 10.0f;
static const float Tweak_WallSteerClosingSpeed = 56.25f;
static const float Tweak_WallSteerBodySpeed = 6.25f;

float BrakeSteeringRangeMultiplier = 1.45f;
float CounterSteerOn = MPH2MPS(30.f);
float CounterSteerOff;
float MAX_STEERING = 45.0f;

float SteeringRangeData[] = {40.0f, 20.0f, 10.0f, 5.5f, 4.5f, 3.25f, 2.9f, 2.9f, 2.9f, 2.9f};
float SteeringSpeedData[] = {1.0f, 1.0f, 1.0f, 0.56f, 0.5f, 0.35f, 0.3f, 0.3f, 0.3f, 0.3f};
float SteeringWheelRangeData[] = {45.0f, 15.0f, 11.0f, 8.0f, 7.0f, 7.0f, 7.0f, 7.0f, 7.0f, 7.0f};
float SteeringInputSpeedData[] = {1.0f, 1.05f, 1.1f, 1.5f, 2.2f, 3.1f};
float SteeringInputData[] = {1.0f, 1.05f, 1.1f, 1.2f, 1.3f, 1.4f};

// only the first steering remap table is actually used, the rest are HP2/UG1/UG2 leftovers

static float JoystickInputToSteerRemap1[] = {-1.0f,  -0.712f, -0.453f, -0.303f, -0.216f, -0.148f, -0.116f, -0.08f, -0.061f, -0.034f, 0.0f,
											 0.034f, 0.061f,  0.08f,   0.116f,  0.148f,  0.216f,  0.303f,  0.453f, 0.712f,  1.0f};
static float JoystickInputToSteerRemap2[] = {-1.0f,  -0.736f, -0.542f, -0.4f, -0.292f, -0.214f, -0.16f, -0.123f, -0.078f, -0.036f, 0.0f,
											 0.036f, 0.078f,  0.123f,  0.16f, 0.214f,  0.292f,  0.4f,   0.542f,  0.736f,  1.0f};
static float JoystickInputToSteerRemap3[] = {-1.0f,  -0.8f,  -0.615f, -0.483f, -0.388f, -0.288f, -0.22f, -0.161f, -0.111f, -0.057f, 0.0f,
											 0.057f, 0.111f, 0.161f,  0.22f,   0.288f,  0.388f,  0.483f, 0.615f,  0.8f,	1.0f};
static float JoystickInputToSteerRemapDrift[] = {-1.0f, -1.0f,  -0.688f, -0.492f, -0.319f, -0.228f, -0.16f, -0.123f, -0.085f, -0.05f, 0.0f,
												 0.05f, 0.085f, 0.123f,  0.16f,   0.228f,  0.319f,  0.492f, 0.688f,  1.0f,	1.0f};
static const int STEER_REMAP_DEAD = 0;
static const int STEER_REMAP_MEDIUM = 1;
static const int STEER_REMAP_TWITCHY = 2;
static const int STEER_REMAP_DRIFT = 3;
static const int SteerInputRemapping = STEER_REMAP_MEDIUM;
static const int SteerInputRemappingDrift = STEER_REMAP_DRIFT;
static Table SteerInputRemapTables[] = {Table(JoystickInputToSteerRemap1, 21, -1.0f, 1.0f), Table(JoystickInputToSteerRemap2, 21, -1.0f, 1.0f),
										Table(JoystickInputToSteerRemap3, 21, -1.0f, 1.0f), Table(JoystickInputToSteerRemapDrift, 21, -1.0f, 1.0f)};

float SteeringRangeCoeffData[] = {1.0f, 1.0f, 1.1f, 1.2f, 1.25f, 1.35f};
Table SteeringRangeTable = Table(SteeringRangeData, 10, 0.0f, 160.0f);
Table SteeringWheelRangeTable = Table(SteeringWheelRangeData, 10, 0.0f, 160.0f);
Table SteeringRangeCoeffTable = Table(SteeringRangeCoeffData, 6, 0.0f, 1.0f);
Table SteeringSpeedTable = Table(SteeringSpeedData, 10, 0.0f, 160.0f);
Table SteeringInputSpeedCoeffTable = Table(SteeringInputSpeedData, 6, 0.0f, 10.0f);
Table SteeringInputCoeffTable = Table(SteeringInputData, 6, 0.0f, 1.0f);

static const float HardTurnSteeringThreshold = 0.5f;
static const float HardTurnTightenSpeed = 0.0f;
static const float Tweak_GameBreakerMaxSteer = 60.0f;
static const float Tweak_TuningSteering_Ratio = 0.2f;

SuspensionRacerMW::Tire::Tire(float radius, int index, MWCarTuning *mwSpecs)
	: WheelMW(1), mRadius(UMath::Max(radius, 0.1f)), mWheelIndex(index), mAxleIndex(index >> 1), mMWSpecs(mwSpecs), mBrake(0.0f),
	  mEBrake(0.0f), mAV(0.0f), mLoad(0.0f), mLateralForce(0.0f), mLongitudeForce(0.0f), mDriveTorque(0.0f), mBrakeTorque(0.0f), mLateralBoost(1.0f),
	  mTractionBoost(1.0f), mSlip(0.0f), mLastTorque(0.0f), mRoadSpeed(0.0f), mAngularAcc(0.0f), mTraction(1.0f), mBottomOutTime(0.0f),
	  mSlipAngle(0.0f), mTractionCircle({1.0f, 1.0f}), mMaxSlip(0.5f), mGripBoost(1.0f), mDriftFriction(1.0f), mLateralSpeed(0.0f),
	  mBrakeLocked(false), mLastSign(SuspensionRacerMW::Tire::WAS_ZERO), mDragReduction(0.0f) {}

void SuspensionRacerMW::Tire::BeginFrame(float max_slip, float grip_boost, float traction_boost, float drag_reduction) {
	mMaxSlip = max_slip;
	mDriveTorque = 0.0f;
	mBrakeTorque = 0.0f;
	SetForce({});
	mLateralForce = 0.0f;
	mLongitudeForce = 0.0f;
	mTractionCircle = {1.0f, 1.0f};
	mTractionBoost = traction_boost;
	mGripBoost = grip_boost;
	mDriftFriction = 1.0f;
	mDragReduction = drag_reduction;
}

void SuspensionRacerMW::Tire::EndFrame(float dT) {}

// Credits: Brawltendo
Newtons SuspensionRacerMW::Tire::ComputeLateralForce(float load, float slip_angle) {
	float angle = ANGLE2DEG(slip_angle);
	float norm_angle = angle * 0.5f;
	int slip_angle_table = (int)norm_angle;
	load *= 0.001f; // convert to kN
	float extra = norm_angle - slip_angle_table;
	load *= LoadFactor;

	if (slip_angle_table > 5) {
		return (mMWSpecs->GRIP_SCALE.At(mAxleIndex) * NewCorneringScale) * mGripBoost * GripFactor * LoadSensitivityTable[6]->GetValue(load);
	} else {
		float low = LoadSensitivityTable[slip_angle_table]->GetValue(load);
		float high = LoadSensitivityTable[slip_angle_table + 1]->GetValue(load);
		return (mMWSpecs->GRIP_SCALE.At(mAxleIndex) * NewCorneringScale) * mGripBoost * GripFactor * (extra * (high - low) + low);
	}
}

// Credits: Brawltendo
float SuspensionRacerMW::Tire::GetPilotFactor(const float speed) {
	float PilotFactor = 0.85f;

	if (mBrakeLocked) {
		return 1.0f;
	}
	if (mAV < 0.0f) {
		return 1.0f;
	}
	if (IsSteeringWheel()) {
		return 1.0f;
	}

	float speed_factor = (speed - MPH2MPS(30.0f)) / MPH2MPS(20.0f);
	float val = UMath::Clamp(speed_factor, 0.0f, 1.0f);
	return val * (1.0f - PilotFactor) + PilotFactor;
}

float BrakingTorque = 4.0f;
float EBrakingTorque = 10.0f;

// Credits: Brawltendo
void SuspensionRacerMW::Tire::CheckForBrakeLock(float ground_force) {
	const float brake_spec = mMWSpecs->BRAKE_LOCK.At(mAxleIndex) * FTLB2NM(mMWSpecs->BRAKES.At(mAxleIndex)) * BrakingTorque;
	const float ebrake_spec = FTLB2NM(mMWSpecs->EBRAKE) * EBrakingTorque;
	static float StaticToDynamicBrakeForceRatio = 1.2f;
	static float BrakeLockAngularVelocityFactor = 100.0f;

	float bt = mBrake * brake_spec;
	float ebt = mEBrake * ebrake_spec;
	float available_torque = (bt + ebt) * StaticToDynamicBrakeForceRatio;

	if (available_torque > ground_force * GetRadius() + UMath::Abs(mAV) * BrakeLockAngularVelocityFactor) {
		if (available_torque > 1.0f) {
			mBrakeLocked = true;
		} else {
			mBrakeLocked = false;
		}
		mAV = 0.0f;
	} else {
		mBrakeLocked = false;
	}
}

// Credits: Brawltendo
void SuspensionRacerMW::Tire::CheckSign() {
	if (mLastSign == WAS_POSITIVE) {
		if (mAV < 0.0f) {
			mAV = 0.0f;
		}
	} else if (mLastSign == WAS_NEGATIVE && mAV > 0.0f) {
		mAV = 0.0f;
	}

	if (mAV > FLOAT_EPSILON) {
		mLastSign = WAS_POSITIVE;
	} else if (mAV < -FLOAT_EPSILON) {
		mLastSign = WAS_NEGATIVE;
	} else {
		mLastSign = WAS_ZERO;
	}
}

float WheelMomentOfInertia = 10.0f;
static const float kOneMPH = 0.44703f;

// Credits: Brawltendo
// Updates forces for an unloaded/airborne tire
void SuspensionRacerMW::Tire::UpdateFree(float dT) {
	mLoad = 0.0f;
	mSlip = 0.0f;
	mTraction = 0.0f;
	mSlipAngle = 0.0f;
	CheckForBrakeLock(0.0f);

	if (mBrakeLocked) {
		mAngularAcc = 0.0f;
		mAV = 0.0f;
	} else {
		const float brake_spec = FTLB2NM(mMWSpecs->BRAKES.At(mAxleIndex)) * BrakingTorque;
		const float ebrake_spec = FTLB2NM(mMWSpecs->EBRAKE) * EBrakingTorque;
		float bt = mBrake * brake_spec;
		float ebt = mEBrake * ebrake_spec;
		ApplyBrakeTorque(mAV > 0.0f ? -bt : bt);
		ApplyBrakeTorque(mAV > 0.0f ? -ebt : ebt);

		mAngularAcc = GetTotalTorque() / WheelMomentOfInertia;
		mAV += mAngularAcc * dT;
	}
	CheckSign();
	mLateralForce = 0.0f;
	mLongitudeForce = 0.0f;
}

float RollingFriction = 2.0f;
static const float TireForceEllipseRatio = 1.5f;
static const float InvTireForceEllipseRatio = 1.0 / TireForceEllipseRatio;

// Credits: Brawltendo
float SuspensionRacerMW::Tire::UpdateLoaded(float lat_vel, float fwd_vel, float body_speed, float load, float dT) {
	const float brake_spec = FTLB2NM(mMWSpecs->BRAKES.At(mAxleIndex)) * BrakingTorque;
	const float ebrake_spec = FTLB2NM(mMWSpecs->EBRAKE) * EBrakingTorque;
	const float dynamicgrip_spec = mMWSpecs->DYNAMIC_GRIP.At(mAxleIndex);
	const float staticgrip_spec = mMWSpecs->STATIC_GRIP.At(mAxleIndex);
	// free rolling wheel
	if (mLoad <= 0.0f && !mBrakeLocked) {
		mAV = fwd_vel / mRadius;
	}

	float fwd_acc = (fwd_vel - mRoadSpeed) / dT;

	mRoadSpeed = fwd_vel;
	mLoad = UMath::Max(load, 0.0f);
	mLateralSpeed = lat_vel;

	float bt = mBrake * brake_spec;
	float ebt = mEBrake * ebrake_spec;
	float abs_fwd = UMath::Abs(fwd_vel);
	if (abs_fwd < 1.0f) {
		// when car is nearly stopped, apply brake torque using forward velocity and wheel load
		bt = -mBrake * load * fwd_vel / mRadius;
		ebt = -mEBrake * load * fwd_vel / mRadius;

		ApplyDriveTorque(-GetDriveTorque() * mEBrake);
		ApplyBrakeTorque(bt);
		ApplyBrakeTorque(ebt);
	} else {
		ApplyBrakeTorque(mAV > 0.0f ? -bt : bt);
		ApplyBrakeTorque(mAV > 0.0f ? -ebt : ebt);
	}

	mSlipAngle = UMath::Atan2a(lat_vel, abs_fwd);
	float groundfriction = 0.0f;
	float slip_speed = mAV * mRadius - fwd_vel;
	float dynamicfriction = 1.0f;
	mSlip = slip_speed;
	float skid_speed = UMath::Sqrt(slip_speed * slip_speed + lat_vel * lat_vel);
	float pilot_factor = GetPilotFactor(body_speed);
	if (skid_speed > FLOAT_EPSILON && (lat_vel != 0.0f || fwd_vel != 0.0f)) {
		dynamicfriction = dynamicgrip_spec * mTractionBoost;
		dynamicfriction *= pilot_factor;
		groundfriction = mLoad * dynamicfriction / skid_speed;
		float slipgroundfriction = mLoad * dynamicfriction / UMath::Sqrt(fwd_vel * fwd_vel + lat_vel * lat_vel);
		CheckForBrakeLock(abs_fwd * slipgroundfriction);
	}

	if (mTraction < 1.0f || mBrakeLocked) {
		mLongitudeForce = groundfriction;
		mLongitudeForce *= slip_speed;
		mLateralForce = -groundfriction * lat_vel;

		if (body_speed < kOneMPH && dynamicfriction > 0.1f) {
			mLateralForce /= dynamicfriction;
			mLongitudeForce /= dynamicfriction;
		}
		mLongitudeForce = UMath::Limit(mLongitudeForce, GetTotalTorque() / mRadius);
	} else {
		mBrakeLocked = false;
		mLongitudeForce = GetTotalTorque() / mRadius;
		float slip_ang = mSlipAngle;
		mLateralForce = ComputeLateralForce(mLoad, UMath::Abs(mSlipAngle));
		if (lat_vel > 0.0f) {
			mLateralForce = -mLateralForce;
		}
	}

	mLateralForce *= mLateralBoost;
	if (mTraction >= 1.0f && !mBrakeLocked) {
		float acc_diff = mAngularAcc * mRadius - fwd_acc;
		mLongitudeForce += acc_diff * WheelMomentOfInertia / mRadius;
	}

	bool use_ellipse = false;
	if (GetTotalTorque() * fwd_vel > 0.0f && !mBrakeLocked) {
		use_ellipse = true;
		mLongitudeForce *= TireForceEllipseRatio;
	}

	mLateralForce *= mTractionCircle.x;
	mLongitudeForce *= mTractionCircle.y;

	float len_force = UMath::Sqrt(mLateralForce * mLateralForce + mLongitudeForce * mLongitudeForce);
	float max_force = mLoad * staticgrip_spec * mTractionBoost * mDriftFriction;

	max_force *= pilot_factor;

	mTraction = 1.0f;
	float max_slip = mMaxSlip;

	if (len_force > max_force && len_force > 0.001f) {
		float ratio = max_force / len_force;
		mTraction = ratio;
		mLateralForce *= ratio;
		mLongitudeForce *= ratio;
		max_slip = (ratio * ratio) * max_slip;
	} else if (use_ellipse) {
		mLongitudeForce *= InvTireForceEllipseRatio;
	}

	if (UMath::Abs(slip_speed) > max_slip) {
		mTraction *= max_slip / UMath::Abs(slip_speed);
	}

	// factor surface friction into the tire force
	auto surface = GetSimSurface(mSurface);
	if (surface) {
		mLateralForce *= surface->LATERAL_GRIP;
		mLongitudeForce *= surface->DRIVE_GRIP;
	}

	if (fwd_vel > 1.0f) {
		mLongitudeForce -= UMath::Sina(mSlipAngle) * mLateralForce * mDragReduction / mMWSpecs->GRIP_SCALE.At(mAxleIndex);
	} else {
		mLateralForce *= UMath::Min(UMath::Abs(lat_vel), 1.0f);
	}

	if (mBrakeLocked) {
		mAngularAcc = 0.0f;

	} else {
		if (mTraction < 1.0f) {
			float torque = (GetTotalTorque() - mLongitudeForce * mRadius + mLastTorque) * 0.5f;
			mLastTorque = torque;
			float rolling_resistance = RollingFriction;
			if (surface) {
				rolling_resistance *= surface->ROLLING_RESISTANCE;
			}
			float effective_torque = torque - mAV * rolling_resistance;
			mAngularAcc = (effective_torque / WheelMomentOfInertia) - (mTraction * mSlip) / (mRadius * dT);
		}

		mAngularAcc = UMath::Lerp(mAngularAcc, fwd_acc / mRadius, mTraction);
	}

	mAV += mAngularAcc * dT;
	CheckSign();
	return mLateralForce;
}

void SuspensionRacerMW::CreateTires() {
	for (int i = 0; i < 4; ++i) {
		delete mTires[i];
		bool is_front = IsFront(i);
		float diameter = Physics::Info::WheelDiameter(mMWAttributes, is_front);
		mTires[i] = new Tire(diameter * 0.5f, i, mMWAttributes);
	}
	UMath::Vector3 dimension;
	mRB->GetDimension(&dimension);

	float wheelbase = mMWAttributes->WHEEL_BASE;
	float axle_width_f = mMWAttributes->TRACK_WIDTH.At(0) - mMWAttributes->SECTION_WIDTH.At(0) * 0.001f;
	float axle_width_r = mMWAttributes->TRACK_WIDTH.At(1) - mMWAttributes->SECTION_WIDTH.At(1) * 0.001f;
	float front_axle = mMWAttributes->FRONT_AXLE;

	//float fWheelY = -dimension.y;
	float fWheelY = -0.05;

	for (int i = 0; i < 4; i++) {
		UMath::Vector3 v = pCar->aTires[i].GetMatrix()->p;
		v.y = fWheelY;
		GetWheel(i).SetLocalArm(v);
	}

	/*
	UMath::Vector3 fl(-axle_width_f * 0.5f, fWheelY, front_axle);
	UMath::Vector3 fr(axle_width_f * 0.5f, fWheelY, front_axle);
	UMath::Vector3 rl(-axle_width_r * 0.5f, fWheelY, front_axle - wheelbase);
	UMath::Vector3 rr(axle_width_r * 0.5f, fWheelY, front_axle - wheelbase);

	GetWheel(0).SetLocalArm(fl);
	GetWheel(1).SetLocalArm(fr);
	GetWheel(2).SetLocalArm(rl);
	GetWheel(3).SetLocalArm(rr);*/
}

void SuspensionRacerMW::OnBehaviorChange() {
	ChassisMW::OnBehaviorChange();
	GetOwner()->QueryInterface(&mHumanAI);
}

Meters SuspensionRacerMW::GetRideHeight(unsigned int idx) {
	float ride = ChassisMW::GetRideHeight(idx);
	const Physics::Tunings *tunings = GetVehicleMWTunings(GetVehicle());
	if (tunings) {
		ride += INCH2METERS(tunings->Value[Physics::Tunings::RIDEHEIGHT]);
	}
	return ride;
}

Radians SuspensionRacerMW::GetWheelAngularVelocity(int index) {
	SuspensionRacerMW::Tire *tire = mTires[index];
	if (tire->IsBrakeLocked()) {
		return 0.0f;
	}
	if (!tire->IsOnGround() || !tire->IsSlipping()) {
		return tire->GetAngularVelocity();
	}
	return tire->GetRoadSpeed() / tire->GetRadius();
}

void SuspensionRacerMW::DoAerobatics(State &state) {
	DoJumpStabilizer(state);
}

static const float Tweak_SteerDragReduction = 0.15f;
static const float Tweak_GameBreakerSteerDragReduction = 0.15f;
static const float Tweak_GameBreakerExtraGs = -2.0f;
static const float TweakGameBreakerRampOutPhysicsTime = 1.0f / 3.0f;
static const float Tweak_DragAeroMult = 1.5f;

void SuspensionRacerMW::OnTaskSimulate(float dT) {
	if (!mRBComplex || !mRB) {
		return;
	}

	if (nLastRaceState > pGameFlow->nRaceState) {
		Reset();
	}
	nLastRaceState = pGameFlow->nRaceState;

	float ride_extra = 0.0f;
	const Physics::Tunings *tunings = GetVehicleMWTunings(GetVehicle());
	if (tunings) {
		ride_extra = tunings->Value[Physics::Tunings::RIDEHEIGHT];
	}
	SetCOG(0.0, ride_extra);

	State state;
	ComputeState(dT, state);
	LastChassisState = state;

	mSteering.CollisionTimer = UMath::Max(mSteering.CollisionTimer - state.time, 0.0f);
	mGameBreaker = 0.0f;

	IPlayer *player = GetOwner()->GetPlayer();
	if (player && player->InGameBreaker()) {
		mGameBreaker = 1.0f;
	} else if (mGameBreaker > 0.0f) {
		mGameBreaker -= state.time * TweakGameBreakerRampOutPhysicsTime;
		mGameBreaker = UMath::Max(mGameBreaker, 0.0f);
	}
	if (mGameBreaker > 0.0f) {
		UMath::Vector3 extra_df;
		UMath::Scale(state.GetUpVector(), Tweak_GameBreakerExtraGs * mGameBreaker * state.mass * 9.81f, extra_df);
		mRB->ResolveForce(&extra_df);
	}

	float max_slip = ComputeMaxSlip(state);
	float grip_scale = ComputeLateralGripScale(state);
	float traction_scale = ComputeTractionScale(state);
	float steerdrag_reduction = UMath::Lerp(Tweak_SteerDragReduction, 1.0f, mGameBreaker);
	if ((state.flags & 1) == 0) {
		float launch = GetVehicle()->GetPerfectLaunch();
		if (launch > 0.0f) {
			traction_scale += launch * 0.25f;
		}
	}

	for (unsigned int i = 0; i < 4; ++i) {
		mTires[i]->BeginFrame(max_slip, grip_scale, traction_scale, steerdrag_reduction);
	}

	float drag_pct = 1.0f - mGameBreaker * 0.75f;
	float aero_pct = 1.0f;
	if (state.driver_style == STYLE_DRAG) {
		aero_pct = Tweak_DragAeroMult;
	}
	DoAerodynamics(state, drag_pct, aero_pct, GetWheel(0).GetLocalArm().z, GetWheel(2).GetLocalArm().z, tunings);
	DoDriveForces(state);
	DoWheelForces(state);

	for (unsigned int i = 0; i < 4; ++i) {
		mTires[i]->UpdateTime(dT);
	}

	mSteering.WallNoseTurn = 0.0f;
	mSteering.WallSideTurn = 0.0f;

	for (unsigned int i = 0; i < 4; ++i) {
		mTires[i]->EndFrame(dT);
	}

	DoTireHeat(state);
	DoAerobatics(state);
	//DoSleep(state); // this is broken in flatout
	ChassisMW::OnTaskSimulate(dT);
}

void SuspensionRacerMW::MatchSpeed(float speed) {
	for (int i = 0; i < 4; ++i) {
		mTires[i]->MatchSpeed(speed);
	}
	mBurnOut.Reset();
	mDrift.Reset();
}

UMath::Vector3* SuspensionRacerMW::GetWheelCenterPos(UMath::Vector3* result, unsigned int i) {
	*result = mTires[i]->GetPosition();
	if (!mRB) {
		return result;
	} else {
		UMath::Vector3 tmp;
		mRB->GetUpVector(&tmp);
		UMath::ScaleAdd(tmp, GetWheelRadius(i), *result, *result);
		return result;
	}
}

void SuspensionRacerMW::Reset() {
	SUSPENSIONRACER_FUNCTION_LOG("Reset");

	UMath::Vector3 vUp;
	mRB->GetUpVector(&vUp);

	unsigned int numonground = 0;
	this->mGameBreaker = 0.0f;
	for (int i = 0; i < GetNumWheels(); ++i) {
		auto &wheel = GetWheel(i);
		wheel.Reset();
		if (wheel.InitPosition(mRBComplex, mRB, wheel.GetRadius())) {
			float upness = UMath::Clamp(UMath::Dot(UMath::Vector4To3(wheel.GetNormal()), vUp), 0.0f, 1.0f);
			float newCompression = wheel.GetNormal().w + GetRideHeight(i) * upness;
			if (newCompression < 0.0f) {
				newCompression = 0.0f;
			}
			wheel.SetCompression(newCompression);
			if (newCompression > 0.0f) {
				numonground++;
			}
		}
	}
	this->mNumWheelsOnGround = numonground;
	mSteering.Reset();
	mBurnOut.Reset();
	mDrift.Reset();
}

// Credits: Brawltendo
float SuspensionRacerMW::DoHumanSteering(State &state) {
	float steer_input = state.steer_input;
	float steer = mSteering.Previous;

	if (steer >= 180.0f) {
		steer -= 360.0f;
	}

	float steering_coeff = mMWAttributes->STEERING;
	SteeringType steer_type = kGamePad;

	//IPlayer *player = GetOwner()->GetPlayer();
	//if (player) {
	//	ISteeringWheel *device = player->GetSteeringDevice();
	//
	//	if (device && device->IsConnected()) {
	//		steer_type = device->GetSteeringType();
	//	}
	//}

	float max_steering;
	float newsteer = steer_input * CalculateMaxSteering(state, steer_type) * steering_coeff;
	newsteer = bClamp(newsteer, -45.0f, 45.0f);

	if (steer_type == kGamePad) {
		int steer_remapping = SteerInputRemapping;
		steer_input = SteerInputRemapTables[steer_remapping].GetValue(steer_input);
		float steering_speed = (CalculateSteeringSpeed(state) * steering_coeff) * state.time;
		float max_diff = steer + steering_speed;
		newsteer = bClamp(newsteer, steer - steering_speed, max_diff);
		// this is absolutely pointless but it's part of the steering calculation for whatever reason
		if (std::abs(newsteer) < 0.0f) {
			newsteer = 0.0f;
		}
	}
	mSteering.LastInput = steer_input;
	mSteering.Previous = newsteer;

	// speedbreaker increases the current steering angle beyond the normal maximum
	// this change is instant, so the visual steering angle while using speedbreaker doesn't accurately represent this
	// instead it interpolates to this value so it looks nicer
	if (mGameBreaker > 0.0f) {
		newsteer = UMath::Lerp(newsteer, state.steer_input * Tweak_GameBreakerMaxSteer, mGameBreaker);
	}

	mSteering.InputAverage.Record(mSteering.LastInput, Sim::GetTime());
	return DEG2ANGLE(newsteer);
}

// Credits: Brawltendo
float SuspensionRacerMW::CalculateMaxSteering(State &state, SteeringType steer_type) {
	const float steer_input = state.steer_input;

	// max possible steering output scales with the car's forward speed
	float max_steering = SteeringRangeTable.GetValue(state.local_vel.z);
	// there are 2 racing wheel input types, one scales with speed and the other doesn't
	if (steer_type == kWheelSpeedSensitive) {
		max_steering = SteeringWheelRangeTable.GetValue(state.local_vel.z);
	} else if (steer_type == kWheelSpeedInsensitive) {
		return MAX_STEERING;
	}

	float tbcoeff = 1.0f - (state.gas_input + 1.0f - (state.brake_input + state.ebrake_input) * 0.5f) * 0.5f;
	max_steering *= BrakeSteeringRangeMultiplier * tbcoeff * SteeringSpeedTable.GetValue(state.local_vel.z) + 1.0f;
	max_steering *= SteeringRangeCoeffTable.GetValue(std::abs(mSteering.InputAverage.GetValue()));

	const Physics::Tunings *tunings = GetVehicleMWTunings(GetVehicle());
	if (tunings) {
		max_steering *= tunings->Value[Physics::Tunings::STEERING] * Tweak_TuningSteering_Ratio + 1.0f;
	}

	float collision_coeff;
	// reduce steering range after collisions
	if (mSteering.CollisionTimer > 0.0f) {
		float secs = PostCollisionSteerReductionDuration - mSteering.CollisionTimer;
		if (secs < PostCollisionSteerReductionDuration && secs > 0.0f) {
			float speed_coeff = std::min(1.0f, state.local_vel.z / (MPH2MPS(170.0f) * 0.7f));
			speed_coeff = 1.0f - speed_coeff;
			collision_coeff = PostCollisionSteerReductionTable.GetValue(secs);
			max_steering *= speed_coeff * (1.0f - collision_coeff) + collision_coeff;
		}
	}

	float yaw_left = ANGLE2DEG(mTires[2]->GetSlipAngle());
	float yaw_right = ANGLE2DEG(mTires[3]->GetSlipAngle());
	// clamp the max steering range to [rear slip, MAX_STEERING] when countersteering
	if (steer_input > 0.0f && yaw_right > 0.0f) {
		max_steering = UMath::Max(max_steering, yaw_right);
		max_steering = UMath::Min(max_steering, MAX_STEERING);
	} else if (steer_input < 0.0f && yaw_left < 0.0f) {
		max_steering = UMath::Max(max_steering, -yaw_left);
		max_steering = UMath::Min(max_steering, MAX_STEERING);
	} else if (std::abs(mSteering.InputAverage.GetValue()) >= HardTurnSteeringThreshold) {
		max_steering = std::max(max_steering, mSteering.LastMaximum - state.time * HardTurnTightenSpeed);
	}

	max_steering = std::min(max_steering, MAX_STEERING);
	mSteering.LastMaximum = max_steering;
	return max_steering;
}

// Credits: Brawltendo
float SuspensionRacerMW::CalculateSteeringSpeed(State &state) {
	// get a rough approximation of how fast the player is steering
	// this ends up creating a bit of a difference in how fast you can actually steer on a controller under normal circumstances
	// using a keyboard will always give you the fastest steering possible
	float steer_input_speed = (state.steer_input - mSteering.LastInput) / state.time;

	mSteering.InputSpeedCoeffAverage.Record(SteeringInputSpeedCoeffTable.GetValue(std::abs(steer_input_speed)), Sim::GetTime());

	// steering speed scales with vehicle forward speed
	float steer_speed = 180.0f;
	steer_speed *= (SteeringSpeedTable.GetValue(state.local_vel.z));
	steer_speed *= mSteering.InputSpeedCoeffAverage.GetValue();

	float steer_input = std::abs(mSteering.InputAverage.GetValue());
	return steer_speed * SteeringInputCoeffTable.GetValue(steer_input);
}

// Credits: Brawltendo
float SuspensionRacerMW::DoAISteering(State &state) {
	mSteering.Maximum = 45.0f;
	if (state.driver_style != STYLE_DRAG)
		mSteering.Maximum = mMWAttributes->STEERING * 45.0f;

	return DEG2ANGLE(mSteering.Maximum * state.steer_input);
}

// Credits: Brawltendo
void SuspensionRacerMW::DoSteering(State &state, UMath::Vector3 &right, UMath::Vector3 &left) {
	float truesteer;
	UMath::Vector4 r4;
	UMath::Vector4 l4;

	if (mHumanAI && mHumanAI->IsPlayerSteering()) {
		truesteer = DoHumanSteering(state);
	} else {
		truesteer = DoAISteering(state);
	}

	ComputeAckerman(truesteer, state, &l4, &r4);
	right = Vector4To3(r4);
	left = Vector4To3(l4);
	mSteering.Wheels[0] = l4.w;
	mSteering.Wheels[1] = r4.w;
	DoWallSteer(state);
}

GraphEntry<float> BurnoutFrictionData[] = {{0.0f, 1.0f}, {5.0f, 0.8f}, {9.0f, 0.9f}, {12.6f, 0.833f}, {17.1f, 0.72f}, {25.0f, 0.65f}};
tGraph<float> BurnoutFrictionTable(BurnoutFrictionData, 6);
float BurnOutCancelSlipValue = 0.5f;
float asd[] = {1.0f, 2.0f, 3.0f};
float BurnOutYawCancel = 0.5f;
float BurnOutAllowTime = 1.0f;
float BurnOutMaxSpeed = 20.0f;
float BurnOutFishTailTime = 2.0f;
int BurnOutFishTails = 6;
static const float Tweak_MinThrottleForBurnout = 1.0f;

// Credits: Brawltendo
void SuspensionRacerMW::Burnout::Update(float dT, float speedmph, float max_slip, int max_slip_wheel, float yaw) {
	// continue burnout/fishtailing state
	if (GetState()) {
		if (speedmph > 5.0f && UMath::Abs(ANGLE2RAD(yaw)) > BurnOutYawCancel) {
			Reset();
		} else if (max_slip < BurnOutCancelSlipValue) {
			IncBurnOutAllow(dT);
			if (mBurnOutAllow > BurnOutAllowTime)
				Reset();
		} else {
			ClearBurnOutAllow();
			DecBurnOutTime(dT);
			if (mBurnOutTime < 0.0f) {
				SetState(GetState() - 1);
				SetBurnOutTime(BurnOutFishTailTime);
			}
		}
	}
	// initialize burnout/fishtailing state
	else if (speedmph < BurnOutMaxSpeed) {
		const float friction_mult = 1.4f;
		// these conditions were split, there was probably some debug stuff here
		if (max_slip > 0.5f) {
			float burnout_coeff;
			BurnoutFrictionTable.GetValue(&burnout_coeff, max_slip);
			SetTraction(burnout_coeff / friction_mult);
			float friction_cut = 1.5f - burnout_coeff;
			// burnout state changes according to what side of the axle the wheel that's slipping the most is on
			SetState(BurnOutFishTails * friction_cut + (max_slip_wheel & 1));
			SetBurnOutTime(BurnOutFishTailTime);
			ClearBurnOutAllow();
		}
	}
}

// Calculates artificial steering for when the car is touching a wall
// Credits: Brawltendo
void SuspensionRacerMW::DoWallSteer(State &state) {
	float wall = mSteering.WallNoseTurn;
	// nose turn is applied when the car is perpendicular to the wall
	// allows the player to easily turn their car away from the wall after a head-on crash without reversing
	if (wall != 0.0f && mNumWheelsOnGround > 2 && state.gas_input > 0.0f) {
		float dW = state.steer_input * state.gas_input * 0.125f;
		if (wall * dW < 0.0f) {
			return;
		}

		dW *= UMath::Abs(wall);
		UMath::Vector3 chg = {};
		chg.y = dW;
		UMath::Rotate(chg, state.matrix, chg);
		UMath::Add(state.angular_vel, chg, chg);
		mRB->SetAngularVelocity(&chg);
	}

	wall = mSteering.WallSideTurn;
	// side turn is only applied when in reverse and if touching a wall parallel to the car
	// it helps the player move their car away from a wall when backing up
	if (wall * state.steer_input * state.gas_input > 0.0f && mNumWheelsOnGround > 2 && !state.gear) {
		float dW = -state.steer_input * state.gas_input * 0.125f;
		dW *= UMath::Abs(wall);

		UMath::Vector3 chg = {};
		chg.y = dW;
		UMath::Rotate(chg, state.matrix, chg);
		UMath::Add(state.angular_vel, chg, chg);
		mRB->SetAngularVelocity(&chg);
	}
}

static float LowSpeedSpeed = 0.0f;
static float HighSpeedSpeed = 30.0f;
static float MaxYawBonus = 0.35f;
static float LowSpeedYawBoost = 0.0f;
static float HighSpeedYawBoost = 1.0f;
static float YawEBrakeThreshold = 0.5f;
static float YawAngleThreshold = 20.0f;

// Credits: Brawltendo
float YawFrictionBoost(float yaw, float ebrake, float speed, float yawcontrol, float grade) {
	yaw = std::abs(yaw);
	float retval = 1.0f;
	retval += std::abs(grade);
	if (ebrake > YawEBrakeThreshold && yaw < DEG2RAD(YawAngleThreshold))
		return retval;

	float speed_factor = (speed - LowSpeedSpeed) / (HighSpeedSpeed - LowSpeedSpeed);
	float boost = LowSpeedYawBoost + (HighSpeedYawBoost - LowSpeedYawBoost) * speed_factor;
	float bonus = yaw * yawcontrol * boost;
	if (bonus > MaxYawBonus)
		bonus = MaxYawBonus;
	return retval + bonus;
}

// Credits: Brawltendo
float SuspensionRacerMW::CalcYawControlLimit(float speed) {
	if (mTransmission) {
		float maxspeed = mTransmission->GetMaxSpeedometer();
		if (maxspeed <= 0.0f) {
			return 0.0f;
		}
		float percent = UMath::Min(UMath::Abs(speed) / maxspeed, 1.0f);
		unsigned int numunits = mMWAttributes->YAW_CONTROL.size();
		if (numunits > 1) {
			float ratio = (numunits - 1) * percent;
			unsigned int index1 = static_cast<unsigned int>(ratio);
			ratio -= index1;
			unsigned int index2 = UMath::Min(numunits - 1, index1 + 1);
			float a = mMWAttributes->YAW_CONTROL[index1];
			float b = mMWAttributes->YAW_CONTROL[index2];
			return a + (b - a) * ratio;
		}
	}
	return mMWAttributes->YAW_CONTROL[0];
}

GraphEntry<float> DriftStabilizerData[] = {{0.0f, 0.0f},		{0.2617994f, 0.1f},  {0.52359879f, 0.45f}, {0.78539819f, 0.85f},
										   {1.0471976f, 0.95f}, {1.5533431f, 1.15f}, {1.5707964f, 0.0f}};
float DriftRearFrictionData[] = {1.1f, 0.95f, 0.87f, 0.77f, 0.67f, 0.6f, 0.51f, 0.43f, 0.37f, 0.34f};
tGraph<float> DriftStabilizerTable(DriftStabilizerData, 7);
Table DriftRearFrictionTable(DriftRearFrictionData, 10, 0.0f, 1.0f);
static const float DriftRotationalStabalizer = 4.0f;
static const float DriftYawAngularVelCoeff = 0.5f;
static const float Tweak_MinDriftSpeedMPH = 30.0f;
static const float Tweak_DriftSlipAngle = 12.0f;
static const float Tweak_DriftEnterSpeed = 30.0f;
static const float Tweak_DriftExitSpeed = 30.0f;
static const float Tweak_DriftCounterSteer = 4.0f;
static const float Tweak_GameBreakerDriftRechargePerSec = 0.5f;
static const float Tweak_GameBreakerDriftRechargeMinSpeed = 35.0f;
static const float Tweak_GameBreakerDriftRechargeYaw = 30.0f;
static const bool Tweak_AlwaysDrift = false;
static const bool DoDriftPhysics = true;

// Credits: Brawltendo
void SuspensionRacerMW::DoDrifting(const State &state) {
	if (mDrift.State && ((state.flags & 1) || state.driver_style == STYLE_DRAG)) {
		mDrift.Reset();
		return;
	}

	float drift_change = 0.0f;
	switch (mDrift.State) {
		case SuspensionRacerMW::Drift::D_IN:
		case SuspensionRacerMW::Drift::D_ENTER:
			// the drift value will increment by (dT * 8) when entering and holding a drift
			drift_change = 8.0f;
			break;
		case SuspensionRacerMW::Drift::D_OUT:
		case SuspensionRacerMW::Drift::D_EXIT:
			// the drift value will decrement by (dT * 2) when not drifting or exiting a drift
			drift_change = -2.0f;
			break;
		default:
			break;
	}

	mDrift.Value += drift_change * state.time;
	// clamp the drift value between 0 and 1
	if (mDrift.Value <= 0.0f) {
		mDrift.State = SuspensionRacerMW::Drift::D_OUT;
		mDrift.Value = 0.0f;
	} else if (mDrift.Value >= 1.0f) {
		mDrift.State = SuspensionRacerMW::Drift::D_IN;
		mDrift.Value = 1.0f;
	}

	if (mDrift.State > SuspensionRacerMW::Drift::D_ENTER) {
		float avg_steer = mSteering.InputAverage.GetValue();
		if ((state.local_angular_vel.y * state.slipangle) < 0.0f && UMath::Abs(state.slipangle) <= 0.25f &&
			state.speed > MPH2MPS(Tweak_MinDriftSpeedMPH) && (avg_steer * state.slipangle) <= 0.0f &&
			UMath::Abs(state.slipangle) > DEG2ANGLE(Tweak_DriftSlipAngle)) {
			mDrift.State = SuspensionRacerMW::Drift::D_IN;
		} else if (state.gas_input * state.steer_input * state.slipangle > DEG2ANGLE(Tweak_DriftSlipAngle) &&
				   state.speed > MPH2MPS(Tweak_MinDriftSpeedMPH)) {
			mDrift.State = SuspensionRacerMW::Drift::D_IN;
		} else if (state.gas_input * UMath::Abs(state.slipangle) > DEG2ANGLE(Tweak_DriftSlipAngle)) {
			mDrift.State = SuspensionRacerMW::Drift::D_ENTER;
		} else {
			mDrift.State = SuspensionRacerMW::Drift::D_EXIT;
		}
	} else if (state.speed > MPH2MPS(Tweak_DriftEnterSpeed) &&
			   (state.ebrake_input > 0.5f || UMath::Abs(state.slipangle) > DEG2ANGLE(Tweak_DriftSlipAngle))) {
		mDrift.State = SuspensionRacerMW::Drift::D_ENTER;
	}

	if (mDrift.Value <= 0.0f)
		return;
	{
		float yaw = ANGLE2RAD(state.slipangle);
		float ang_vel = state.local_angular_vel.y;

		// charge speedbreaker if not in use and drifting is detected
		if (mGameBreaker <= 0.0f && state.speed > MPH2MPS(Tweak_GameBreakerDriftRechargeMinSpeed) &&
			UMath::Abs(yaw) > DEG2RAD(Tweak_GameBreakerDriftRechargeYaw)) {
			IPlayer *player = GetOwner()->GetPlayer();
			if (player) {
				player->ChargeGameBreaker(state.time * Tweak_GameBreakerDriftRechargePerSec * mDrift.Value);
			}
		}

		// apply yaw damping torque
		if ((yaw * ang_vel) < 0.0f && mNumWheelsOnGround >= 2) {
			float yaw_coef = DriftStabilizerTable.GetValue(UMath::Abs(yaw)) * DriftRotationalStabalizer;
			UMath::Vector3 moment;

			UMath::Scale(state.GetUpVector(), (mDrift.Value * -ang_vel) * yaw_coef * state.inertia.y, moment);
			mRB->ResolveTorque(&moment);
		}

		// detect counter steering
		float countersteer = 0.0f;
		if (state.steer_input * yaw > 0.0f) {
			countersteer = UMath::Abs(state.steer_input);
		}

		float yawangularvel_coeff = DriftYawAngularVelCoeff;
		float driftcoeff = UMath::Abs(yaw) * yawangularvel_coeff + countersteer * Tweak_DriftCounterSteer + UMath::Abs(ang_vel) * yawangularvel_coeff;
		float driftmult_rear = DriftRearFrictionTable.GetValue(driftcoeff * mDrift.Value);
		mTires[2]->SetDriftFriction(driftmult_rear);
		mTires[3]->SetDriftFriction(driftmult_rear);
	}
}

float EBrakeYawControlMin = 0.5f;
float EBrakeYawControlOnSpeed = 1.0f;
float EBrakeYawControlOffSpeed = 20.0f;
float EBrake180Yaw = 0.3f;
float EBrake180Speed = 80.0f;
static const float Tweak_GameBreakerGripIncrease = 0.75f;
static const float Tweak_StagingTraction = 0.25f;
static const float Tweak_DragYawControl = 0.1f;
static const float Tweak_StabilityControl = 2.5f;
static const float Tweak_BlownTireEbrake = 0.0f;
static const float Tweak_BlownTireBrake = 1.0f;
static const float Tweak_BlownTireTraction = 0.3f;

// Credits: Brawltendo
void SuspensionRacerMW::TuneWheelParams(State &state) {
	float ebrake = state.ebrake_input;
	float t = state.time;
	float speedmph = MPS2MPH(state.local_vel.z);
	float car_yaw = ANGLE2RAD(state.slipangle);
	float yawcontrol = mSteering.YawControl;

	// engaging the handbrake decreases steering yaw control
	if (ebrake >= 0.5f) {
		yawcontrol -= EBrakeYawControlOffSpeed * t;
		if (yawcontrol < EBrakeYawControlMin) {
			yawcontrol = EBrakeYawControlMin;
		}
	} else {
		yawcontrol += EBrakeYawControlOnSpeed * t;
		if (yawcontrol > 1.0f) {
			yawcontrol = 1.0f;
		}
	}
	mSteering.YawControl = yawcontrol;

	float brake_biased[2] = {state.brake_input, state.brake_input};
	yawcontrol *= (1.0f - mDrift.Value); // pointless parentheses for matching purposes
	const Physics::Tunings *tunings = GetVehicleMWTunings(GetVehicle());
	if (tunings) {
		// brake tuning adjusts the brake bias
		brake_biased[0] += brake_biased[0] * tunings->Value[Physics::Tunings::BRAKES] * 0.5f;
		brake_biased[1] -= brake_biased[1] * tunings->Value[Physics::Tunings::BRAKES] * 0.5f;
	}
	float suspension_yaw_control_limit = CalcYawControlLimit(state.speed);
	//IPlayer *player = GetOwner()->GetPlayer();
	if (state.driver_style == STYLE_DRAG) {
		suspension_yaw_control_limit = 0.1f;
	}
	//else if (player) {
	//	PlayerSettings *settings = player->GetSettings();
	//	if (settings) {
	//		// increase yaw control limit when stability control is off (unused by normal means)
	//		if (!settings->Handling) {
	//			suspension_yaw_control_limit += 2.5f;
	//		}
	//	}
	//}

	float max_slip = 0.0f;
	int max_slip_wheel = 0;
	for (int i = 0; i < 4; ++i) {
		float lateral_boost = 1.0f;

		// at speeds below 10 mph, 5% of the current speed in mph is applied as the brake scale for driven wheels
		if (state.gas_input > 0.8f && state.brake_input > 0.5f && UMath::Abs(speedmph) < 10.0f && IsDriveWheel(i)) {
			mTires[i]->SetBrake(UMath::Abs(speedmph) * 0.05f);
		} else {
			mTires[i]->SetBrake(brake_biased[i >> 1]);
		}

		// handbrake only applies to the rear wheels
		if (IsRear(i)) {
			float b = ebrake;
			// increase handbrake multiplier when a hard handbrake turn is detected
			if (ebrake > 0.2f && car_yaw > EBrake180Yaw && speedmph < EBrake180Speed) {
				b += 0.5f;
			}
			mTires[i]->SetEBrake(b);
		} else {
			mTires[i]->SetEBrake(0.0f);
		}

		float friction_boost = 1.0f;
		// rear wheels get extra boost according to the yaw control
		if (IsRear(i)) {
			float grade = state.GetForwardVector().y;
			float boost = YawFrictionBoost(car_yaw, mTires[i]->GetEBrake(), state.speed, suspension_yaw_control_limit, grade) - 1.0f;
			friction_boost = yawcontrol * boost + 1.0f;
		}

		// speedbreaker increases front tire friction relative to the absolute steering input
		if (mGameBreaker > 0.0f && IsFront(i)) {
			float over_boost = mGameBreaker * UMath::Abs(state.steer_input) * 0.75f + 1.0f;
			lateral_boost = over_boost;
			friction_boost *= over_boost;
		}
		mTires[i]->ScaleTractionBoost(friction_boost);
		mTires[i]->SetLateralBoost(lateral_boost);

		if (tunings) {
			UMath::Vector2 circle;
			circle.x = tunings->Value[Physics::Tunings::HANDLING] * 0.2f + 1.0f;
			circle.y = 1.0f - tunings->Value[Physics::Tunings::HANDLING] * 0.2f;
			mTires[i]->SetTractionCircle(circle);
		}
		// traction is increased by perfect shifts in drag races and also by engaging the nitrous
		mTires[i]->ScaleTractionBoost(state.nos_boost * state.shift_boost);

		// popped tires are permanently braking and have reduced traction
		if ((1 << i) & state.blown_tires) {
			mTires[i]->SetEBrake(0.0f);
			mTires[i]->SetBrake(1.0f);
			mTires[i]->ScaleTractionBoost(0.3f);
		}

		// find the highest slip of all tires for the burnout/fishtailing state
		if (mTires[i]->GetCurrentSlip() > max_slip) {
			max_slip = mTires[i]->GetCurrentSlip();
			max_slip_wheel = i;
		}
	}

	// burnout state only applies when in first gear and the throttle is fully pressed outside of drag events
	if (state.driver_style != STYLE_DRAG && state.gear == G_FIRST && state.gas_input >= 1.0f) {
		mBurnOut.Update(state.time, MPS2MPH(state.local_vel.z), max_slip, max_slip_wheel, state.slipangle);
	} else {
		mBurnOut.Reset();
	}

	// lower traction for all wheels when staging
	if (state.flags & 1) {
		for (int i = 0; i < 4; ++i) {
			mTires[i]->ScaleTractionBoost(0.25f);
		}
	}

	DoDrifting(state);
}

// Credits: Brawltendo
void SuspensionRacerMW::Differential::CalcSplit(bool locked) {
	if (!has_traction[0] || !has_traction[1] || locked || (factor <= 0.0f)) {
		torque_split[0] = bias;
		torque_split[1] = 1.0f - bias;
		return;
	}
	float av_0 = angular_vel[0] * (1.0f - bias);
	float av_1 = angular_vel[1] * bias;
	float combined_av = UMath::Abs(av_0 + av_1);

	if (combined_av > FLOAT_EPSILON) {
		torque_split[0] = ((1.0f - factor) * bias) + ((factor * UMath::Abs(av_1)) / combined_av);
		torque_split[1] = ((1.0f - factor) * (1.0f - bias)) + ((factor * UMath::Abs(av_0)) / combined_av);
	} else {
		torque_split[0] = bias;
		torque_split[1] = 1.0f - bias;
	}

	torque_split[0] = UMath::Clamp(torque_split[0], 0.0f, 1.0f);
	torque_split[1] = UMath::Clamp(torque_split[1], 0.0f, 1.0f);
}

// Credits: Brawltendo
void SuspensionRacerMW::DoDriveForces(State &state) {
	if (!mTransmission) {
		return;
	}

	float drive_torque = mTransmission->GetDriveTorque();
	SuspensionRacerMW::Differential center_diff;
	if (drive_torque == 0.0f) {
		return;
	}

	center_diff.factor = mMWAttributes->DIFFERENTIAL[2];
	if (center_diff.factor > 0.0f) {
		center_diff.bias = mMWAttributes->TORQUE_SPLIT;
		center_diff.angular_vel[0] = mTires[0]->GetAngularVelocity() + mTires[1]->GetAngularVelocity();
		center_diff.angular_vel[1] = mTires[2]->GetAngularVelocity() + mTires[3]->GetAngularVelocity();
		center_diff.has_traction[0] = mTires[0]->IsOnGround() || mTires[1]->IsOnGround();
		center_diff.has_traction[1] = mTires[2]->IsOnGround() || mTires[3]->IsOnGround();
		center_diff.CalcSplit(false);
	} else {
		center_diff.torque_split[0] = mMWAttributes->TORQUE_SPLIT;
		center_diff.torque_split[1] = 1.0f - center_diff.torque_split[0];
	}

	for (unsigned int axle = 0; axle < 2; ++axle) {
		float axle_torque = drive_torque * center_diff.torque_split[axle];
		if (UMath::Abs(axle_torque) > FLOAT_EPSILON) {
			SuspensionRacerMW::Differential diff;
			diff.bias = 0.5f;

			float fwd_slip = 0.0f;
			float lat_slip = 0.0f;

			float traction_control[2] = {1.0f, 1.0f};
			float traction_boost[2] = {1.0f, 1.0f};
			diff.factor = mMWAttributes->DIFFERENTIAL[axle];

			for (unsigned int i = 0; i < 2; ++i) {
				unsigned int tire = axle * 2 + i;
				diff.angular_vel[i] = mTires[tire]->GetAngularVelocity();
				diff.has_traction[i] = mTires[tire]->IsOnGround();

				fwd_slip += center_diff.torque_split[axle] * mTires[tire]->GetCurrentSlip() * 0.5f;
				lat_slip += center_diff.torque_split[axle] * mTires[tire]->GetLateralSpeed() * 0.5f;
			}

			bool locked_diff = false;
			if ((mBurnOut.GetState() & 1) != 0) {
				traction_boost[1] = mBurnOut.GetTraction();
				diff.bias = mBurnOut.GetTraction() * 0.5f;
				locked_diff = true;
			} else if ((mBurnOut.GetState() & 2) != 0) {
				traction_boost[0] = mBurnOut.GetTraction();
				diff.bias = 1.0f - mBurnOut.GetTraction() * 0.5f;
				locked_diff = true;
			} else {
				float delta_lat_slip = lat_slip - state.local_vel.x;
				if (delta_lat_slip * state.steer_input > 0.0f && axle_torque * fwd_slip > 0.0f) {
					float delta_fwd_slip = fwd_slip - state.local_vel.z;
					float traction_control_limit = UMath::Ramp(delta_fwd_slip, 1.0f, 20.0f);

					if (traction_control_limit > 0.0f) {
						float traction_angle = UMath::Abs(state.steer_input * UMath::Atan2d(delta_lat_slip, UMath::Abs(delta_fwd_slip)));
						traction_control_limit *= UMath::Ramp(traction_angle, 1.0f, 16.0f);
						if (traction_control_limit > 0.0f) {
							if (delta_lat_slip > 0.0f) {
								traction_control[1] = 1.0f - traction_control_limit;
								traction_control[0] = 1.0f - traction_control_limit * 0.5f;
								traction_boost[0] = traction_control_limit + 1.0f;
							} else {
								traction_control[0] = 1.0f - traction_control_limit;
								traction_control[1] = 1.0f - traction_control_limit * 0.5f;
								traction_boost[1] = traction_control_limit + 1.0f;
							}
						}
					}
				}
			}
			diff.CalcSplit(locked_diff);

			for (unsigned int i = 0; i < 2; ++i) {
				unsigned int tire = axle * 2 + i;
				if (mTires[tire]->IsOnGround()) {
					mTires[tire]->ApplyDriveTorque(axle_torque * diff.torque_split[i] * traction_control[i]);
					mTires[tire]->ScaleTractionBoost(traction_boost[i]);
				}
			}
		}
	}
}

static const float Tweak_DragYawSpeed = 1.6f;

// Credits: Brawltendo
void SuspensionRacerMW::DoWheelForces(State &state) {
	const float dT = state.time;

	UMath::Vector3 steerR;
	UMath::Vector3 steerL;
	DoSteering(state, steerR, steerL);
	TuneWheelParams(state);

	unsigned int wheelsOnGround = 0;
	float maxDelta = 0.0f;

	const UMath::Vector3 &vFwd = state.GetForwardVector();
	const UMath::Vector3 &vUp = state.GetUpVector();

	const float mass = state.mass;

	float ride_extra = 0.0f;
	const Physics::Tunings *tunings = GetVehicleMWTunings(GetVehicle());
	if (tunings) {
		ride_extra = tunings->Value[Physics::Tunings::RIDEHEIGHT];
	}

	float time = Sim::GetTime();
	float shock_specs[2];
	float spring_specs[2];
	float sway_specs[2];
	float travel_specs[2];
	float rideheight_specs[2];
	float shock_ext_specs[2];
	float shock_valving[2];
	float shock_digression[2];
	float progression[2];

	for (unsigned int i = 0; i < 2; ++i) {
		shock_specs[i] = LBIN2NM(mMWAttributes->SHOCK_STIFFNESS.At(i));
		shock_ext_specs[i] = LBIN2NM(mMWAttributes->SHOCK_EXT_STIFFNESS.At(i));
		shock_valving[i] = INCH2METERS(mMWAttributes->SHOCK_VALVING.At(i));
		shock_digression[i] = 1.0f - mMWAttributes->SHOCK_DIGRESSION.At(i);
		spring_specs[i] = LBIN2NM(mMWAttributes->SPRING_STIFFNESS.At(i));
		sway_specs[i] = LBIN2NM(mMWAttributes->SWAYBAR_STIFFNESS.At(i));
		travel_specs[i] = INCH2METERS(mMWAttributes->TRAVEL.At(i));
		rideheight_specs[i] = INCH2METERS(mMWAttributes->RIDE_HEIGHT.At(i) + ride_extra);
		progression[i] = mMWAttributes->SPRING_PROGRESSION.At(i);
	}

	float sway_stiffness[4];
	sway_stiffness[0] = (mTires[0]->GetCompression() - mTires[1]->GetCompression()) * sway_specs[0];
	sway_stiffness[1] = -sway_stiffness[0];
	sway_stiffness[2] = (mTires[2]->GetCompression() - mTires[3]->GetCompression()) * sway_specs[1];
	sway_stiffness[3] = -sway_stiffness[2];

	UMath::Vector4 steering_normals[4];
	steering_normals[0] = UMath::Vector4Make(steerL, 1.0f);
	steering_normals[1] = UMath::Vector4Make(steerR, 1.0f);
	steering_normals[2] = UMath::Vector4Make(vFwd, 1.0f);
	steering_normals[3] = UMath::Vector4Make(vFwd, 1.0f);

	bool resolve = false;
	for (unsigned int i = 0; i < 4; ++i) {
		int axle = i / 2;
		Tire &wheel = GetWheel(i);
		UMath::Vector3 wp = wheel.GetWorldArm();
		wheel.UpdatePosition(state.angular_vel, state.linear_vel, state.matrix, state.world_cog, state.time, wheel.GetRadius(), true, state.dimension.y * 2.0f);
		const UMath::Vector3 groundNormal(wheel.GetNormal());
		const UMath::Vector3 forwardNormal(steering_normals[i]);
		UMath::Vector3 lateralNormal;
		UMath::UnitCross(groundNormal, forwardNormal, lateralNormal);

		float penetration = wheel.GetNormal().w;
		// how angled the wheel is relative to the ground
		float upness = UMath::Clamp(UMath::Dot(groundNormal, vUp), 0.0f, 1.0f);
		const float oldCompression = wheel.GetCompression();
		float newCompression = rideheight_specs[axle] * upness + penetration;
		float max_compression = travel_specs[axle];
		if (wheel.GetCompression() == 0.0f) {
			maxDelta = UMath::Max(maxDelta, newCompression - max_compression);
		}
		newCompression = UMath::Max(newCompression, 0.0f);

		// handle the suspension bottoming out
		if (newCompression > max_compression) {
			float delta = newCompression - max_compression;
			maxDelta = UMath::Max(maxDelta, delta);
			// spring can't compress past the travel length
			newCompression = max_compression;
			wheel.SetBottomOutTime(time);
		}

		if (newCompression > 0.0f && upness > VehicleSystem::ENABLE_ROLL_STOPS_THRESHOLD) {
			++wheelsOnGround;

			const float diff = newCompression - wheel.GetCompression();
			float rise = diff / dT;

			float spring = (newCompression * spring_specs[axle]) * (newCompression * progression[axle] + 1.0f);
			if (shock_valving[axle] > FLOAT_EPSILON && shock_digression[axle] < 1.0f) {
				float abs_rise = UMath::Abs(rise);
				float valving = shock_valving[axle];
				if (abs_rise > valving) {
					float digression = valving * UMath::Pow(abs_rise / valving, shock_digression[axle]);
					rise = (rise > 0.0f) ? digression : -digression;
				}
			}

			float damp = rise > 0.0f ? rise * shock_specs[axle] : rise * shock_ext_specs[axle];

			if (damp > mMWAttributes->SHOCK_BLOWOUT * 9.81f * mass) {
				damp = 0.0f;
			}

			float springForce = UMath::Max(damp + spring + sway_stiffness[i], 0.0f);

			UMath::Vector3 verticalForce(vUp * springForce);

			UMath::Vector3 driveForce;
			UMath::Vector3 lateralForce;
			UMath::Vector3 c;
			UMath::Cross(forwardNormal, groundNormal, c);
			UMath::Cross(c, forwardNormal, c);

			float d2 = UMath::Dot(c, groundNormal) * 4.0f - 3.0f;
			float load = UMath::Max(d2, 0.3f) * springForce;
			const UMath::Vector3 &pointVelocity = wheel.GetVelocity();
			float xspeed = UMath::Dot(pointVelocity, lateralNormal);
			float zspeed = UMath::Dot(pointVelocity, forwardNormal);

			float traction_force = wheel.UpdateLoaded(xspeed, zspeed, state.speed, load, state.time);
			float max_traction = UMath::Abs(xspeed / dT * (0.25f * mass));
			lateralForce = lateralNormal * UMath::Clamp(traction_force, -max_traction, max_traction);

			UMath::Vector3 force;
			UMath::UnitCross(lateralNormal, groundNormal, driveForce);
			UMath::Scale(driveForce, wheel.GetLongitudeForce(), driveForce);
			UMath::Add(lateralForce, driveForce, force);
			UMath::Add(force, verticalForce, force);

			wheel.SetForce(force);
			resolve = true;
		} else {
			wheel.SetForce({});
			wheel.UpdateFree(dT);
		}

		if (newCompression == 0.0f) {
			wheel.IncAirTime(dT);
		} else {
			wheel.SetAirTime(0.0f);
		}
		wheel.SetCompression(newCompression);
	}

	if (resolve) {
		UMath::Vector3 total_torque = {};
		UMath::Vector3 total_force = {};

		for (unsigned int i = 0; i < GetNumWheels(); ++i) {
			Tire &wheel = GetWheel(i);
			UMath::Vector3 p(wheel.GetLocalArm());
			p.y += wheel.GetCompression();
			p.y -= rideheight_specs[i / 2u];
			const UMath::Vector3 &force = wheel.GetForce();
			UMath::RotateTranslate(p, state.matrix, p);
			wheel.SetPosition(p);

			UMath::Vector3 torque;
			UMath::Vector3 r;
			UMath::Sub(p, state.world_cog, r);
			UMath::Sub(r, state.GetPosition(), r);
			UMath::Cross(r, force, torque);
			UMath::Add(total_force, force, total_force);
			UMath::Add(total_torque, torque, total_torque);
		}

		float yaw = UMath::Dot(UMath::Vector4To3(state.matrix.y), total_torque);
		float counter_yaw = yaw * mMWAttributes->YAW_SPEED;
		if (state.driver_style == STYLE_DRAG) {
			counter_yaw *= Tweak_DragYawSpeed;
		}
		UMath::ScaleAdd(UMath::Vector4To3(state.matrix.y), counter_yaw - yaw, total_torque, total_torque);
		mRB->Resolve(&total_force, &total_torque);
	}

	if (maxDelta > 0.0f) {
		for (int i = 0; i < GetNumWheels(); ++i) {
			auto& wheel = GetWheel(i);
			wheel.SetY(wheel.GetPosition().y + maxDelta);
		}
		mRB->ModifyYPos(maxDelta);
	}

	if (wheelsOnGround != 0 && !mNumWheelsOnGround) {
		state.local_angular_vel.z *= 0.5f;
		state.local_angular_vel.y *= 0.5f;
		UMath::Rotate(state.local_angular_vel, state.matrix, state.angular_vel);
		mRB->SetAngularVelocity(&state.angular_vel);
	}
	mNumWheelsOnGround = wheelsOnGround;
}