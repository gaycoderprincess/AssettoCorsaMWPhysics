EngineRacer::EngineRacer(Car* car) {
	ENGINERACER_FUNCTION_LOG("Create");

	pCar = car;

	GetOwner()->Add<IEngine>(this);
	GetOwner()->Add<ITransmission>(this);
	//GetOwner()->Add<IInductable>(this);
	GetOwner()->Add<ITiptronic>(this);
	GetOwner()->Add<IRaceEngine>(this);
	GetOwner()->Add<IEngineDamage>(this);

	mDriveTorque = 0.0f;
	mDriveTorqueAtEngine = 0.0f;
	mGear = G_NEUTRAL;
	mGearShiftTimer = 0.0f;
	mThrottle = 0.0f;
	mSpool = 0.0f;
	mPSI = 0.0f;
	mInductionBoost = 0.0f;
	mAngularVelocity = 0.0f;
	mAngularAcceleration = 0.0f;
	mTransmissionVelocity = 0.0f;
	mNOSCapacity = 1.0f;
	mNOSBoost = 1.0f;
	mNOSEngaged = 0.0f;
	mClutchRPMDiff = 0.0f;
	mEngineBraking = false;
	mSportShifting = 0.0f;
	mIInput = nullptr;
	mSuspension = nullptr;

	mMWInfo = new MWCarTuning;
	GetLerpedCarTuning(*mMWInfo, GetVehicle()->GetVehicleName());

	mRPM = 0.0f;
	mShiftStatus = SHIFT_STATUS_NONE;
	mShiftPotential = SHIFT_POTENTIAL_NONE;
	mPeakTorque = 0.0f;
	mPeakTorqueRPM = 0.0f;
	mClutch = Clutch();
	mBlown = false;
	mSabotage = 0.0f;

	GetOwner()->QueryInterface(&mIInput);
	GetOwner()->QueryInterface(&mSuspension);
	GetOwner()->QueryInterface(&mCheater);

	if (mMWInfo->NOS_CAPACITY > 0.0f) {
		mNOSCapacity = 1.0f;
	} else {
		mNOSCapacity = 0.0f;
	}

	Reset();

	WriteLog("EngineRacer::Create finished");
}

EngineRacer::~EngineRacer() {
	ENGINERACER_FUNCTION_LOG("dtor");

	GetOwner()->Remove<IEngine>(this);
	GetOwner()->Remove<ITransmission>(this);
	//GetOwner()->Remove<IInductable>(this);
	GetOwner()->Remove<ITiptronic>(this);
	GetOwner()->Remove<IRaceEngine>(this);
	GetOwner()->Remove<IEngineDamage>(this);

	delete mMWInfo;

	WriteLog("EngineRacer::dtor finished");
}

float EngineRacer::GetHorsePower() {
	float engine_torque = GetEngineTorque(mRPM);
	return NM2HP(engine_torque * mThrottle, mRPM);
}

void EngineRacer::OnBehaviorChange() {
	ENGINERACER_FUNCTION_LOG("OnBehaviorChange");
	GetOwner()->QueryInterface(&mIInput);
	GetOwner()->QueryInterface(&mCheater);
	GetOwner()->QueryInterface(&mSuspension);
}

void EngineRacer::Sabotage(float time) {
	if ((mSabotage <= 0.0f) && (time > FLOAT_EPSILON) && !IsBlown()) {
		mSabotage = Sim::GetTime() + time;
	}
}

bool EngineRacer::Blow() {
	if (!mBlown) {
		mBlown = true;
		mSabotage = 0.0f;
		return true;
	}
	return false;
}

void EngineRacer::Reset() {
	ENGINERACER_FUNCTION_LOG("Reset");

	mDriveTorque = 0.0f;
	mDriveTorqueAtEngine = 0.0f;
	mAngularVelocity = RPM2RPS(mMWInfo->IDLE);
	mAngularAcceleration = 0.0f;
	mRPM = mMWInfo->IDLE;
	mTransmissionVelocity = 0.0f;
	mClutch.Reset();
	mGearShiftTimer = 0.0f;
	mSpool = 0.0f;
	mPSI = 0.0f;
	mInductionBoost = 0.0f;
	mShiftStatus = SHIFT_STATUS_NONE;
	mShiftPotential = SHIFT_POTENTIAL_NONE;
	mGear = G_FIRST;
	mNOSEngaged = 0.0f;
	mClutchRPMDiff = 0.0f;
	mThrottle = 0.0f;
	mNOSBoost = 1.0f;
	mSportShifting = 0.0f;

	CalcShiftPoints();
}

float EngineRacer::GetEngineTorque(float rpm) {
	float ftlbs = Physics::Info::Torque(mMWInfo, rpm);
	float result = FTLB2NM(ftlbs);
	result *= 1.0f + mInductionBoost;

	if (GetVehicle()->GetDriverClass() != DRIVER_REMOTE) {
		result *= mNOSBoost;
	}

	return result;
}

GearID EngineRacer::GuessGear(float speed) {
	if (speed < 0.0f) {
		return G_REVERSE;
	}

	GearID result = G_FIRST;
	for (int i = G_FIRST; i < GetTopGear(); ++i) {
		GearID this_gear = (GearID)i;
		GearID next_gear = (GearID)(i + 1);

		float shift_at = GetShiftPoint(this_gear, next_gear);
		float this_rpm = GuessRPM(speed, this_gear);
		if (this_rpm <= shift_at) {
			break;
		}
		result = next_gear;
	}

	return result;
}

float EngineRacer::GuessRPM(float speed, GearID atgear) {
	float wheelrear = Physics::Info::WheelDiameter(mMWInfo, false) * 0.5f;
	float wheelfront = Physics::Info::WheelDiameter(mMWInfo, true) * 0.5f;
	float avg_wheel_radius = (wheelrear + wheelfront) * 0.5f;

	if (avg_wheel_radius <= 0.0f) {
		return mMWInfo->IDLE;
	}

	float differential_w = UMath::Abs(speed) / avg_wheel_radius;
	float max_w = RPM2RPS(mMWInfo->RED_LINE);
	float min_w = RPM2RPS(mMWInfo->IDLE);
	float rear_end = GetFinalGear();
	float total_gear_ratio = GetFinalGear() * GetGearRatio(atgear);
	float av = RPS2RPM(min_w + differential_w * total_gear_ratio * (max_w - min_w) / max_w);
	av = UMath::Clamp(av, mMWInfo->IDLE, mMWInfo->RED_LINE);
	return av;
}

void EngineRacer::MatchSpeed(float speed) {
	Reset();
	mAngularAcceleration = 0.0f;
	mSportShifting = 0.0f;
	mGearShiftTimer = 0.0f;
	mGear = GuessGear(speed);
	mRPM = GuessRPM(speed, (GearID)mGear);
	mAngularVelocity = RPM2RPS(mRPM);
	mTransmissionVelocity = mAngularVelocity;
	mClutch.Engage(0.0f);
}

// Credits: Brawltendo
float EngineRacer::GetBrakingTorque(float engine_torque, float rpm) {
	float torque = engine_torque;
	unsigned int numpts = mMWInfo->ENGINE_BRAKING.size();
	if (numpts > 1) {
		float rpm_min = mMWInfo->IDLE;
		float rpm_max = mMWInfo->MAX_RPM;
		float ratio;
		unsigned int index =
				UTIL_InterprolateIndex(numpts - 1, UMath::Clamp(rpm, mMWInfo->IDLE, mMWInfo->RED_LINE), rpm_min, rpm_max, ratio);

		float base = mMWInfo->ENGINE_BRAKING[index];
		unsigned int secondIndex = index + 1;
		float step = mMWInfo->ENGINE_BRAKING[(int)UMath::Min(numpts - 1, secondIndex)];
		float load_pct = base + (step - base) * ratio;
		return -torque * UMath::Clamp(load_pct, 0.f, 1.f);
	} else {
		return -torque * mMWInfo->ENGINE_BRAKING[0];
	}
}

void EngineRacer::CalcShiftPoints() {
	bool shift_points_calced = Physics::Info::ShiftPoints(mMWInfo, mShiftUpRPM, mShiftDownRPM, 10);

	mPeakTorque = Physics::Info::MaxInductedTorque(mMWInfo, mPeakTorqueRPM, nullptr);
	mPeakTorque = FTLB2NM(mPeakTorque);
	mMaxHP = Physics::Info::MaxInductedPower(mMWInfo, nullptr);
}

// Credits: Brawltendo
void EngineRacer::AutoShift(float dT) {
	if (mGear == G_REVERSE || mGearShiftTimer > 0.0f || GetVehicle()->IsStaging() || mSportShifting > 0.0f)
		return;

	// skip neutral when using auto transmission
	if (mGear == G_NEUTRAL) {
		mGear = G_FIRST;
		return;
	}

	switch (mShiftPotential) {
		case SHIFT_POTENTIAL_DOWN: {
			int next_gear = mGear - 1;
			if (next_gear > G_FIRST) {
				float current_rpm = RPS2RPM(mTransmissionVelocity);
				float rpm = current_rpm * GetRatioChange(next_gear, mGear);
				for (; next_gear > G_FIRST && FindShiftPotential((GearID)next_gear, rpm, rpm) == SHIFT_POTENTIAL_DOWN;) {
					rpm = current_rpm * GetRatioChange(--next_gear, mGear);
				}
			}
			DoGearChange((GearID)next_gear, true);
			break;
		}
		case SHIFT_POTENTIAL_NONE:
			break;
		case SHIFT_POTENTIAL_UP:
		case SHIFT_POTENTIAL_PERFECT:
		case SHIFT_POTENTIAL_MISS: {
			int have_traction = 1;
			for (int i = 0; i < 4; ++i) {
				have_traction &= mSuspension->IsWheelOnGround(i) && mSuspension->GetWheelSlip(i) < 4.f;
			}
			if (have_traction) {
				DoGearChange((GearID)(mGear + 1), true);
			}
			break;
		}

		default:
			break;
	}
}

static const bool Tweak_CoastShifting = true;
static const float Tweak_CoastingPercent = 0.65f;

// Credits: Brawltendo
ShiftPotential EngineRacer::FindShiftPotential(GearID gear, float rpm, float rpmFromGround) {
	if (gear <= G_NEUTRAL)
		return SHIFT_POTENTIAL_NONE;

	float shift_up_rpm = GetShiftUpRPM(gear);
	float shift_down_rpm = GetShiftDownRPM(gear);
	float lower_gear_ratio = GetGearRatio(gear - 1);

	// is able to shift down
	if (gear != G_FIRST && lower_gear_ratio > 0.0f) {
		float lower_gear_shift_up_rpm = ((GetShiftUpRPM(gear - 1) * GetGearRatio(gear)) / lower_gear_ratio) - 200.0f;

		if (Tweak_CoastShifting) {
			// lower downshift RPM when coasting
			float off_throttle_rpm = UMath::Lerp(mMWInfo->IDLE, shift_down_rpm, Tweak_CoastingPercent);
			shift_down_rpm = UMath::Lerp(off_throttle_rpm, shift_down_rpm, mThrottle);
			shift_down_rpm = UMath::Min(shift_down_rpm, lower_gear_shift_up_rpm);
		}
	}

	if (rpm >= shift_up_rpm && gear < GetTopGear()) {
		return SHIFT_POTENTIAL_UP;
	}
	if (rpm <= shift_down_rpm && gear > G_FIRST) {
		return SHIFT_POTENTIAL_DOWN;
	}
	return SHIFT_POTENTIAL_NONE;
}

ShiftPotential EngineRacer::UpdateShiftPotential(GearID gear, float rpm, float rpmFromGround) {
	return FindShiftPotential(gear, rpm, rpmFromGround);
}

bool EngineRacer::SportShift(GearID gear) {
	if (gear == mGear || gear <= G_NEUTRAL || IsGearChanging())
		return false;

	ShiftPotential potential = GetShiftPotential();
	if (gear > mGear && potential == SHIFT_POTENTIAL_DOWN)
		return false;

	if (gear < mGear && potential == SHIFT_POTENTIAL_UP)
		return false;

	if (DoGearChange(gear, false)) {
		mSportShifting = 1.25f;
		return true;
	}

	return false;
}

// Credits: Brawltendo
ShiftStatus EngineRacer::OnGearChange(GearID gear) {
	ENGINERACER_FUNCTION_LOG("OnGearChange");

	if (gear >= GetNumGearRatios())
		return SHIFT_STATUS_NONE;
	// new gear can't be the same as the old one
	if (gear != mGear && gear >= G_REVERSE) {
		if (gear < mGear) {
			mGearShiftTimer = GetShiftDelay(gear, false) * 0.25f;
		} else {
			mGearShiftTimer = GetShiftDelay(gear, true);
		}
		mClutch.mShiftingUp = gear > mGear;
		mGear = gear;
		mClutch.Disengage();
		return SHIFT_STATUS_NORMAL;
	}

	return SHIFT_STATUS_NONE;
}

// Credits: Brawltendo
bool EngineRacer::DoGearChange(GearID gear, bool automatic) {
	if (GetVehicle()->IsStaging() && gear == G_NEUTRAL) gear = G_FIRST; // hack for prostreet to start staging in first gear

	if (gear > GetTopGear()) {
		return false;
	}
	if (gear < G_REVERSE) {
		return false;
	}

	GearID previous = (GearID)mGear;
	ShiftStatus status = OnGearChange(gear);
	if (status != SHIFT_STATUS_NONE) {
		mShiftStatus = status;
		mShiftPotential = SHIFT_POTENTIAL_NONE;
		return true;
	}

	return false;
}

// Credits: Brawltendo
float EngineRacer::GetDifferentialAngularVelocity(bool locked) {
	float into_gearbox = 0.0f;
	bool in_reverse = GetGear() == G_REVERSE;

	if (FrontWheelDrive()) {
		float w_vel = (mSuspension->GetWheelAngularVelocity(0) + mSuspension->GetWheelAngularVelocity(1)) * 0.5f;
		if (!locked) {
			if (UMath::Abs(w_vel) > 0.0f)
				into_gearbox = w_vel;
		} else {
			into_gearbox = in_reverse ? UMath::Min(w_vel, 0.0f) : UMath::Max(w_vel, 0.0f);
		}
	}

	if (RearWheelDrive()) {
		float w_vel = (mSuspension->GetWheelAngularVelocity(2) + mSuspension->GetWheelAngularVelocity(3)) * 0.5f;
		if (!locked) {
			if (UMath::Abs(w_vel) > UMath::Abs(into_gearbox))
				into_gearbox = w_vel;
		} else {
			into_gearbox = in_reverse ? UMath::Min(w_vel, into_gearbox) : UMath::Max(w_vel, into_gearbox);
		}
	}

	return into_gearbox;
}

// Credits: Brawltendo
float EngineRacer::GetDriveWheelSlippage() {
	float retval = 0.0f;
	int drivewheels = 0;
	if (RearWheelDrive()) {
		drivewheels += 2;
		retval += mSuspension->GetWheelSlip(TIRE_RR) + mSuspension->GetWheelSlip(TIRE_RL);
	}
	if (FrontWheelDrive()) {
		drivewheels += 2;
		retval += mSuspension->GetWheelSlip(TIRE_FL) + mSuspension->GetWheelSlip(TIRE_FR);
	}

	return retval / drivewheels;
}

// Credits: Brawltendo
void EngineRacer::SetDifferentialAngularVelocity(float w) {
	float current = GetDifferentialAngularVelocity(0);
	float diff = w - current;
	float speed = MPS2MPH(GetVehicle()->GetAbsoluteSpeed());
	int lockdiff = speed < 40.0f;
	if (RearWheelDrive()) {
		if (!mSuspension->IsWheelOnGround(2) && !mSuspension->IsWheelOnGround(3))
			lockdiff = 1;

		float w1 = mSuspension->GetWheelAngularVelocity(2);
		float w2 = mSuspension->GetWheelAngularVelocity(3);
		if (lockdiff)
			w2 = w1 = (w1 + w2) * 0.5f;

		mSuspension->SetWheelAngularVelocity(2, w1 + diff);
		mSuspension->SetWheelAngularVelocity(3, w2 + diff);
	}

	lockdiff = speed < 40.0f;
	if (FrontWheelDrive()) {
		if (!mSuspension->IsWheelOnGround(0) && !mSuspension->IsWheelOnGround(1))
			lockdiff = 1;

		float w1 = mSuspension->GetWheelAngularVelocity(0);
		float w2 = mSuspension->GetWheelAngularVelocity(1);
		if (lockdiff)
			w2 = w1 = (w1 + w2) * 0.5f;

		mSuspension->SetWheelAngularVelocity(0, w1 + diff);
		mSuspension->SetWheelAngularVelocity(1, w2 + diff);
	}
}

// Credits: Brawltendo
float EngineRacer::CalcSpeedometer(float rpm, unsigned int gear) {
	const Physics::Tunings *tunings = GetVehicleTunings();
	return Physics::Info::Speedometer(mMWInfo, rpm, (GearID)gear, tunings);
}

// Credits: Brawltendo
float EngineRacer::GetMaxSpeedometer() {
	unsigned int num_ratios = GetNumGearRatios();
	if (num_ratios > 0) {
		float limiter = MPH2MPS(mMWInfo->SPEED_LIMITER[0]);
		float max_speedometer = CalcSpeedometer(mMWInfo->RED_LINE, num_ratios - 1);
		if (limiter > 0.0f) {
			return UMath::Min(max_speedometer, limiter);
		} else {
			return max_speedometer;
		}
	} else {
		return 0.0f;
	}
}

// Credits: Brawltendo
float EngineRacer::GetSpeedometer() {
	return CalcSpeedometer(RPS2RPM(mTransmissionVelocity), mGear);
}

// Credits: Brawltendo
void EngineRacer::LimitFreeWheels(float w) {
	unsigned int numwheels = mSuspension->GetNumWheels();
	for (unsigned int i = 0; i < numwheels; ++i) {
		if (!mSuspension->IsWheelOnGround(i)) {
			if (i < 2) {
				if (!FrontWheelDrive())
					continue;
			} else if (!RearWheelDrive())
				continue;

			float ww = mSuspension->GetWheelAngularVelocity(i);
			if (ww * w < 0.0f) {
				ww = 0.0f;
			} else if (ww > 0.0f) {
				ww = UMath::Min(ww, w);
			} else if (ww < 0.0f) {
				ww = UMath::Max(ww, w);
			}

			mSuspension->SetWheelAngularVelocity(i, ww);
		}
	}
}

float SmoothRPMDecel[] = {2.5f, 15.0f};

// Credits: Brawltendo
float Engine_SmoothRPM(bool is_shifting, GearID gear, float dT, float old_rpm, float new_rpm, float engine_inertia) {
	bool fast_shifting = is_shifting && gear > G_FIRST || gear == G_NEUTRAL;
	// this ternary is dumb but that's what makes it match
	float max_rpm_decel = -SmoothRPMDecel[fast_shifting ? 1 : 0];
	float rpm = new_rpm;
	float max_decel = max_rpm_decel * 1000.0f / engine_inertia;
	if (dT > 0.0f && (rpm - old_rpm) / dT < max_decel) {
		float newrpm = max_decel * dT + old_rpm;

		if (newrpm < rpm) {
			newrpm = rpm;
		}
		rpm = newrpm;
	}

	return rpm * 0.55f + old_rpm * 0.45f;
}

// Credits: Brawltendo
void EngineRacer::DoECU() {
	if (GetGear() <= G_NEUTRAL) {
		return;
	}
	// the speed at which the limiter starts to kick in
	float limiter = MPH2MPS(mMWInfo->SPEED_LIMITER[0]);
	if (limiter > 0.0f) {
		// the speed for the limiter to take full effect
		float cutoff = MPH2MPS(mMWInfo->SPEED_LIMITER[1]);
		if (cutoff > 0.0f) {
			float speedometer = GetSpeedometer();
			if (speedometer > limiter) {
				float limiter_range = speedometer - limiter;
				mThrottle *= (1.0f - UMath::Clamp(limiter_range / cutoff, 0.f, 1.0f));
			}
		}
	}
}

bool Tweak_InfiniteNOS = false;
static const float Tweak_MinSpeedForNosMPH = 10.0f;
static const float Tweak_MaxNOSRechargeCheat = 2.0f;
static const float Tweak_MaxNOSDischargeCheat = 0.5f;

// Credits: Brawltendo
float EngineRacer::DoNos(const Physics::Tunings *tunings, float dT, bool engaged) {
	if (!HasNOS())
		return 1.0f;

	float speed_mph = MPS2MPH(GetVehicle()->GetAbsoluteSpeed());
	float recharge_rate = 0.0f;
	IPlayer *player = GetOwner()->GetPlayer();

	if (!player || player->CanRechargeNOS()) {
		float min_speed = mMWInfo->RECHARGE_MIN_SPEED;
		float max_speed = mMWInfo->RECHARGE_MAX_SPEED;
		if (speed_mph >= min_speed && mGear >= G_FIRST) {
			float t = UMath::Ramp(speed_mph, min_speed, max_speed);
			recharge_rate = UMath::Lerp(mMWInfo->RECHARGE_MIN, mMWInfo->RECHARGE_MAX, t);
		}
	}

	if (mGear < G_FIRST || mThrottle <= 0.0f || this->IsBlown())
		engaged = false;
	if (speed_mph < Tweak_MinSpeedForNosMPH && !IsNOSEngaged() || speed_mph < Tweak_MinSpeedForNosMPH * 0.5f && IsNOSEngaged())
		engaged = false;

	float nos_discharge = Physics::Info::NosCapacity(mMWInfo, tunings);
	float nos_torque_scale = 1.0f;
	if (nos_discharge > 0.0f) {
		float nos_disengage = mMWInfo->NOS_DISENGAGE;
		if (engaged && mNOSCapacity > 0.0f) {
			float discharge = dT / nos_discharge;
			// don't deplete nitrous
			if (Tweak_InfiniteNOS || GetVehicle()->GetDriverClass() == DRIVER_REMOTE)
				discharge = 0.0f;
			// GetCatchupCheat returns 0.0 for human racers, but AI racers get hax
			discharge *= UMath::Lerp(1.0f, Tweak_MaxNOSDischargeCheat, GetCatchupCheat());
			mNOSCapacity -= discharge;
			nos_torque_scale = Physics::Info::NosBoost(mMWInfo, tunings);
			mNOSEngaged = 1.0f;
			mNOSCapacity = UMath::Max(mNOSCapacity, 0.0f);
		} else if (mNOSEngaged > 0.0f && nos_disengage > 0.0f) {
			// nitrous can't start recharging until the disengage timer runs out
			mNOSEngaged -= dT / nos_disengage;
			mNOSEngaged = UMath::Max(mNOSEngaged, 0.0f);
		} else if (mNOSCapacity < 1.0f && recharge_rate > 0.0f) {
			float recharge = dT / recharge_rate;
			// GetCatchupCheat returns 0.0 for human racers, but AI racers get hax
			recharge *= UMath::Lerp(1.0f, Tweak_MaxNOSRechargeCheat, GetCatchupCheat());
			mNOSCapacity += recharge;
			mNOSCapacity = UMath::Min(mNOSCapacity, 1.0f);
			mNOSEngaged = 0.0f;
		} else {
			mNOSEngaged = 0.0f;
		}

	} else {
		mNOSCapacity = 0.0f;
		mNOSEngaged = 0.0f;
	}
	return nos_torque_scale;
}

// Credits: Brawltendo
void EngineRacer::DoInduction(const Physics::Tunings *tunings, float dT) {
	Physics::Info::eInductionType type = Physics::Info::InductionType(mMWInfo);
	if (type == Physics::Info::INDUCTION_NONE) {
		mSpool = 0.0f;
		mInductionBoost = 0.0f;
		mPSI = 0.0f;
		return;
	}

	float desired_spool = UMath::Ramp(mThrottle, 0.0f, 0.5f);
	float rpm = RPS2RPM(mAngularVelocity);

	if (IsGearChanging())
		desired_spool = 0.0f;
	// turbocharger can't start spooling up until the engine rpm is >= the boost threshold
	if (type == Physics::Info::INDUCTION_TURBO_CHARGER && rpm < Physics::Info::InductionRPM(mMWInfo, tunings)) {
		desired_spool = 0.0f;
	}

	if (mSpool > desired_spool) {
		float spool_time = mMWInfo->SPOOL_TIME_DOWN;
		if (spool_time > FLOAT_EPSILON) {
			mSpool -= dT / spool_time;
			mSpool = UMath::Max(mSpool, desired_spool);
		} else {
			mSpool = desired_spool;
		}
	} else if (mSpool < desired_spool) {
		float spool_time = mMWInfo->SPOOL_TIME_UP;
		if (spool_time > FLOAT_EPSILON) {
			mSpool += dT / spool_time;
			mSpool = UMath::Min(mSpool, desired_spool);
		} else {
			mSpool = desired_spool;
		}
	}

	float target_psi;
	mSpool = UMath::Clamp(mSpool, 0.0f, 1.0f);
	mInductionBoost = Physics::Info::InductionBoost(mMWInfo, rpm, mSpool, tunings, &target_psi);
	if (mPSI > target_psi) {
		mPSI = UMath::Max(mPSI - dT * 20.0f, target_psi);
	} else if (mPSI < target_psi) {
		mPSI = UMath::Min(mPSI + dT * 20.0f, target_psi);
	}
}

// Credits: Brawltendo
float EngineRacer::DoThrottle(float dT) {
	if (IsBlown() || !mIInput) {
		// cut the throttle when the engine is blown
		return 0.0f;
	}
	return mIInput->GetControlGas();
}

// Credits: Brawltendo
void EngineRacer::DoShifting(float dT) {
	if (mIInput && mIInput->IsAutomaticShift()) {
		AutoShift(dT);
	}

	if (mGearShiftTimer > 0.0f) {
		mGearShiftTimer -= dT;
		if (mGearShiftTimer <= 0.0f) {
			mGearShiftTimer = 0.0f;
		}
	}

	if (mSportShifting > 0.0f && mShiftPotential) {
		if (mIInput) {
			float gas = mIInput->GetControlGas();
			mSportShifting = UMath::Max(mSportShifting - dT * (2.0f - gas), 0.0f);
		} else {
			mSportShifting = 0.0f;
		}
	}
}

static const bool Tweak_EnableTorqueConverter = true;
static const float Tweak_ClutchEngageTime = 0.25f;
static const float Tweak_1stGearClutchEngageTime = 0.05f;
static const float Tweak_CheaterTorqueBoost = 0.5f;
float ClutchStiffness = 20.0f;
GraphEntry<float> ClutchPlayData[] = {{-10.f, 1.f}, {-7.5f, 0.96f}, {-3.5f, 0.925f}, {-0.3f, 0.875f}, {-0.05f, 0.f}};
tGraph<float> ClutchPlayTable(ClutchPlayData, 5);
static const bool Tweak_PrintShiftPotentials = false;
static const float Tweak_IdleClutchRPM = 800.0f;
static const bool Tweak_PrintClutch = false;
static const float ClutchReductionFactor = 0.0f;
static const int ClutchPlayEnabled = true;
static const bool PrintClutchPlayDebugInfo = false;
static const float Tweak_SeizeRPM = 2000.0f;
float ClutchLimiter = 300.0f;
static const float Tweak_EngineDamageFrequency = 12.0f;
static const float Tweak_EngineDamageAmplitude = 0.5f;
static const float Tweak_EngineCounterClutch = 0.0f;

int nNOSState = 0;
void EngineRacer::OnTaskSimulate(float dT) {
	IInput *iinput = mIInput;
	if (iinput == NULL || mSuspension == NULL) {
		return;
	}

	if (nLastRaceState > pGameFlow->nRaceState) {
		ChargeNOS(1.0);
		if (auto ply = GetOwner()->GetPlayer()) {
			ply->ResetGameBreaker(true);
		}
		Reset();
	}
	nLastRaceState = pGameFlow->nRaceState;

	if (mSuspension->GetNumWheels() != 4) {
		return;
	}

	const Physics::Tunings *tunings = GetVehicleTunings();
	bool is_staging = GetVehicle()->IsStaging();
	mThrottle = DoThrottle(dT);
	mNOSBoost = DoNos(tunings, dT, iinput->GetControlNOS());
	DoECU();
	DoInduction(tunings, dT);
	DoShifting(dT);

	if (GetVehicle()->GetDriverClass() == DRIVER_HUMAN) {
		nNOSState = iinput->GetControlNOS();
	}

	float max_rpm = UseRevLimiter() ? mMWInfo->RED_LINE : mMWInfo->MAX_RPM;
	float max_w = RPM2RPS(max_rpm);
	bool was_engaged = mClutch.GetState() == Clutch::ENGAGED;
	float min_w = RPM2RPS(mMWInfo->IDLE);
	float engine_inertia = Physics::Info::EngineInertia(mMWInfo, mGear != G_NEUTRAL);
	float axle_w = GetDifferentialAngularVelocity(false);
	float differential_w = GetDifferentialAngularVelocity(true);
	int num_wheels_onground = mSuspension->GetNumWheelsOnGround();
	float wheel_ratio = std::max(0.25f, num_wheels_onground * 0.25f);
	float clutch_ratio = mClutch.Update(dT);
	float gear_direction = mGear == G_REVERSE ? -1.0f : 1.0f;
	const float gear_ratio = GetGearRatio(mGear);
	float total_gear_ratio = GetGearRatio(mGear) * GetFinalGear() * gear_direction;
	float rpm = RPS2RPM(mAngularVelocity);

	float torque_converter = mMWInfo->TORQUE_CONVERTER;
	if (Tweak_EnableTorqueConverter && torque_converter > 0.0f) {
		float converter_ratio = torque_converter * mThrottle * (1.0f - UMath::Ramp(rpm, mMWInfo->IDLE, mPeakTorqueRPM));
		if (IsGearChanging()) {
			converter_ratio *= clutch_ratio;
		}
		total_gear_ratio *= 1.0f + converter_ratio;
	}

	if (total_gear_ratio == 0.0f && mGear != G_NEUTRAL) {
		return;
	}

	if (mGear == G_REVERSE || mGear == G_FIRST) {
		float idle_limit = Tweak_IdleClutchRPM;
		if (mGear == G_REVERSE) {
			idle_limit *= 3.0f;
		}

		float idle_w = RPM2RPS(idle_limit) + min_w;
		if (mAngularVelocity > idle_w || mTransmissionVelocity > idle_w || mThrottle >= 0.1f) {
			mClutch.Engage(Tweak_1stGearClutchEngageTime);
		} else {
			mClutch.Disengage();
		}
	} else if (mGear == G_NEUTRAL) {
		mClutch.Disengage();
	} else {
		mClutch.Engage(Tweak_ClutchEngageTime);
	}

	float engine_torque = GetEngineTorque(rpm);
	float braking_torque = GetBrakingTorque(engine_torque, rpm);
	if (!is_staging) {
		float perfect_launch = GetVehicle()->GetPerfectLaunch();
		if (perfect_launch > 0.0f && mThrottle > 0.0f) {
			// force the engine to operate at peak torque during a perfect launch
			mThrottle = 1.0f;
			engine_torque = mPeakTorque * mNOSBoost;
			braking_torque = 0.0f;
		}
	}

	float old_clutchv = mTransmissionVelocity;
	mTransmissionVelocity = min_w + differential_w * total_gear_ratio * (max_w - min_w) / max_w;
	float trans_acceleration = (mTransmissionVelocity - old_clutchv) / dT;
	float overrev_torque = 0.0f;
	if (mGear != G_NEUTRAL && mClutch.GetState() == Clutch::ENGAGED && braking_torque * axle_w * gear_direction > overrev_torque) {
		braking_torque = -braking_torque;
	}

	float total_engine_torque = engine_torque * mThrottle + braking_torque * (1.0f - mThrottle);
	float drive_torque = 0.0f;
	float road_torque = overrev_torque;
	mEngineBraking = total_engine_torque < 0.0f;

	if (mGear != G_NEUTRAL) {
		switch (mClutch.GetState()) {
			case Clutch::ENGAGED: {
				mClutchRPMDiff = 0.0f;
				drive_torque = total_engine_torque;
				road_torque -= total_engine_torque * wheel_ratio;
				break;
			}
			case Clutch::ENGAGING: {
				float diff = mAngularVelocity - mTransmissionVelocity;
				if (diff > ClutchLimiter) {
					diff = ClutchLimiter;
				} else if (diff < -ClutchLimiter) {
					diff = -ClutchLimiter;
				}

				float stiffness = ClutchStiffness;
				float rpmdiff = RPS2RPM(mTransmissionVelocity - mAngularVelocity);
				float clutchingtorque;
				if (mClutchRPMDiff != 0.0f && rpmdiff * mClutchRPMDiff < 0.0f && std::abs(rpmdiff) > Tweak_SeizeRPM &&
					std::abs(mClutchRPMDiff) > Tweak_SeizeRPM) {
					stiffness *= 0.5f;
				}

				mClutchRPMDiff = rpmdiff;
				clutchingtorque = diff * stiffness * clutch_ratio;
				drive_torque += clutchingtorque;
				road_torque -= clutchingtorque * wheel_ratio;
				break;
			}
			case Clutch::DISENGAGED: {
				mClutchRPMDiff = 0.0f;
				break;
			}
			default:
				break;
		}
	}

	if (mGear != G_NEUTRAL) {
		if (mClutch.GetState() == Clutch::ENGAGED) {
			float ae = (total_engine_torque + road_torque) / engine_inertia;
			float diff = ae - trans_acceleration;
			float wheel_response = 0.1f;
			float response = 1.0f / engine_inertia;
			float response1;
			float torquesplit = 1.0f - mMWInfo->TORQUE_SPLIT;
			if (FrontWheelDrive()) {
				float front_gear = UMath::Abs(total_gear_ratio);
				response += (1.0f - torquesplit) * wheel_response * front_gear * front_gear * 0.5f;
			}
			if (RearWheelDrive()) {
				float rear_gear = UMath::Abs(total_gear_ratio);
				response += torquesplit * wheel_response * rear_gear * rear_gear * 0.5f;
			}

			response1 = diff / response;
			drive_torque += UMath::Min(response1, 0.0f);
			road_torque -= response1 * wheel_ratio;
		}
	}

	if (mGear != G_NEUTRAL) {
		if (mClutch.GetState() == Clutch::ENGAGED && num_wheels_onground > 0) {
			float slip = GetDriveWheelSlippage();
			float delta = mTransmissionVelocity - mAngularVelocity;
			if (mThrottle > 0.2f && slip * gear_direction > 0.1f && mGear <= G_FIRST && delta < 0.0f) {
				mTransmissionVelocity = mAngularVelocity;
				SetDifferentialAngularVelocity(mAngularVelocity / total_gear_ratio);
			} else {
				float max_torque = std::abs(total_engine_torque);
				float counter_torque = -max_torque;
				road_torque += bClamp(delta * ClutchStiffness, counter_torque, max_torque);
			}
		}

		if (mGear == G_FIRST || mGear == G_REVERSE) {
			float etorque = total_engine_torque;
			float rtorque = road_torque;
			float torque_diff = total_engine_torque + road_torque;
			if (rtorque < etorque && torque_diff < 0.f) {
				float clutch_play_coeff = ClutchPlayTable.GetValue(torque_diff * 1000.f);
				float clutch_torque = clutch_play_coeff * torque_diff;
				road_torque += clutch_torque * mMWInfo->CLUTCH_SLIP;
			}
		}
	}

	// used for when the player's car breaks down in the prologue
	if (mSabotage > 0.f) {
		float count_down = mSabotage - Sim::GetTime();
		if (count_down <= 0.f) {
			mSabotage = 0.f;
			Blow();
		} else {
			// oscillate engine angular vel while the engine breaks down
			mAngularVelocity += Tweak_EngineDamageAmplitude * UMath::Sina(count_down * Tweak_EngineDamageFrequency) * mAngularVelocity;
		}
	}

	if (mGear != G_NEUTRAL && mThrottle > 0.0f && mGear <= G_FIRST && mClutch.GetState() == Clutch::ENGAGED &&
		road_torque * total_engine_torque < 0.0f) {
		float clutch_slip = mMWInfo->CLUTCH_SLIP * mThrottle;
		float power_ratio = 1.0f - mThrottle * UMath::Ramp(rpm, mMWInfo->IDLE, mPeakTorqueRPM);
		clutch_slip *= power_ratio;
		float allowed_road_torque = 1.0f - clutch_slip;
		road_torque *= allowed_road_torque * allowed_road_torque;
	}

	mAngularAcceleration = (total_engine_torque + road_torque) / engine_inertia;
	mAngularVelocity += mAngularAcceleration * dT;
	mAngularVelocity = UMath::Clamp(mAngularVelocity, min_w, max_w);

	if (total_gear_ratio != 0.f) {
		LimitFreeWheels(max_w / total_gear_ratio);
	}

	if (mTransmissionVelocity > max_w && total_gear_ratio != 0.f) {
		if (drive_torque * total_gear_ratio > 0.f)
			drive_torque = 0.f;
		mTransmissionVelocity = max_w;
		SetDifferentialAngularVelocity(max_w / total_gear_ratio);
	}

	// apply catch up torque for AI racers (rubberbanding)
	if (drive_torque > 0.f) {
		drive_torque *= GetCatchupCheat() * Tweak_CheaterTorqueBoost + 1.0f;
	}

	mDriveTorque = drive_torque * total_gear_ratio * GetGearEfficiency(mGear);
	mDriveTorqueAtEngine = drive_torque;
	mRPM = Engine_SmoothRPM(IsShiftingGear() || mClutch.GetState() == Clutch::DISENGAGED, GetGear(), dT, mRPM, RPS2RPM(mAngularVelocity),
							engine_inertia);

	if (mClutch.GetState() || GetVehicle()->IsStaging()) {
		mShiftPotential = SHIFT_POTENTIAL_NONE;
	} else {
		mShiftPotential = UpdateShiftPotential((GearID)mGear, RPS2RPM(mTransmissionVelocity), RPS2RPM(mTransmissionVelocity));
	}
}

// Credits: Brawltendo
float EngineRacer::GetShiftPoint(GearID from_gear, GearID to_gear) {
	if (from_gear <= G_REVERSE) {
		return 0.0f;
	}
	if (to_gear <= G_NEUTRAL) {
		return 0.0f;
	}
	if (to_gear > from_gear) {
		return mShiftUpRPM[from_gear];
	}
	if (to_gear < from_gear) {
		return mShiftDownRPM[from_gear];
	}

	return 0.0f;
}