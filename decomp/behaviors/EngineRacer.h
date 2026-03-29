class EngineRacer : public IEngine, public IEngineDamage, public IRaceEngine, public ITiptronic, public ITransmission {
  public:
	struct Clutch {
	  public:
		enum State { ENGAGED, ENGAGING, DISENGAGED };

		Clutch() {
			mState = ENGAGED;
			mTime = 0.0f;
			mEngageTime = 0.0f;
			mShiftingUp = false;
		}

		void Disengage() {
			if (mState == ENGAGED) {
				mState = DISENGAGED;
			}
		}

		void Engage(float time) {
			if (mState == DISENGAGED) {
				if (time > 0.0f) {
					mState = ENGAGING;
				} else {
					mState = ENGAGED;
				}
				mTime = time;
				mEngageTime = time;
			}
		}

		void Reset() {
			mState = ENGAGED;
			mTime = 0.0f;
			mShiftingUp = false;
		}

		float Update(float dT) {
			if (mTime > 0.0f) {
				mTime -= dT;
				if (mTime <= 0.0f && mState == ENGAGING) {
					mState = ENGAGED;
				}
			}

			switch (mState) {
				case DISENGAGED:
					return 0.25f;
				case ENGAGING:
					return 1.0f - UMath::Ramp(mTime, 0.0f, mEngageTime) * 0.75f;
				case ENGAGED:
					return 1.0f;
				default:
					return 1.0f;
			}
		}

		State GetState() {
			return mState;
		}

		State mState;
		float mTime;
		float mEngageTime;
		bool mShiftingUp;
	};

	auto GetOwner() {
		return GetPlayerInterface(pCar);
	}

	IVehicle* GetVehicle() {
		return GetOwner()->Find<IVehicle>();
	}

	EngineRacer(Car* car);
	GearID GuessGear(float speed);
	float GuessRPM(float speed, GearID atgear);
	ShiftPotential FindShiftPotential(GearID gear, float rpm, float rpmFromGround);
	float GetDifferentialAngularVelocity(bool locked);
	float GetDriveWheelSlippage();
	void SetDifferentialAngularVelocity(float w);
	float CalcSpeedometer(float rpm, unsigned int gear);
	void LimitFreeWheels(float w);
	float GetBrakingTorque(float engine_torque, float rpm);
	void CalcShiftPoints();
	bool DoGearChange(GearID gear, bool automatic);
	void AutoShift(float dT);

	Physics::Tunings* GetVehicleTunings() {
		return GetVehicleMWTunings(GetVehicle());
	}

	~EngineRacer();

	// IEngine
	void MatchSpeed(float speed);
	float GetHorsePower();

	// Behavior
	void Reset();
	void OnTaskSimulate(float dT);
	void OnBehaviorChange();

	// ITransmission
	float GetSpeedometer();
	float GetMaxSpeedometer();
	float GetShiftPoint(GearID from_gear, GearID to_gear);

	// ITiptronic
	bool SportShift(GearID gear);

	// IEngineDamage
	void Sabotage(float time);
	bool Blow();

	// IRaceEngine
	// Credits: Brawltendo
	float GetPerfectLaunchRange(float *range) {
		// perfect launch only applies to first gear
		if (mGear != G_FIRST) {
			*range = 0.0f;
			return 0.0f;
		} else {
			*range = (mMWInfo->RED_LINE - mMWInfo->IDLE) * 0.25f;
			float upper_limit = mMWInfo->RED_LINE + 500.0f;
			return UMath::Min(mPeakTorqueRPM + *range, upper_limit) - *range;
		}
	}

	// IEngine
	float GetMaxHorsePower() {
		return mMaxHP;
	}
	Hp GetMinHorsePower() {
		return FTLB2HP(Physics::Info::Torque(mMWInfo, mMWInfo->IDLE) * mMWInfo->IDLE, 1.0f);
	}
	float GetRPM() {
		return mRPM;
	}
	float GetMaxRPM() {
		return mMWInfo->MAX_RPM;
	}
	float GetPeakTorqueRPM() {
		return mPeakTorqueRPM;
	}
	float GetRedline() {
		return mMWInfo->RED_LINE;
	}
	float GetMinRPM() {
		return mMWInfo->IDLE;
	}
	float GetNOSCapacity() {
		return mNOSCapacity;
	}
	float GetNOSBoost() {
		return mNOSBoost;
	}
	bool IsNOSEngaged() {
		return mNOSEngaged >= 1.0f;
	}
	bool HasNOS() {
		return mMWInfo->NOS_CAPACITY > 0.0f && mMWInfo->TORQUE_BOOST > 0.0f;
	}
	float GetNOSFlowRate() {
		return mMWInfo->FLOW_RATE;
	}

	void ChargeNOS(float charge) {
		if (HasNOS()) {
			mNOSCapacity = UMath::Clamp(mNOSCapacity + charge, 0.0f, 1.0f);
		}
	}

	bool IsEngineBraking() {
		return mEngineBraking;
	}
	bool IsShiftingGear() {
		return mGearShiftTimer > 0.0f;
	}
	bool IsReversing() {
		return mGear == G_REVERSE;
	}

	// IInductable
	Physics::Info::eInductionType InductionType() {
		return Physics::Info::InductionType(mMWInfo);
	}
	float GetInductionPSI() {
		return mPSI;
	}
	float InductionSpool() {
		return mSpool;
	}
	float GetMaxInductionPSI() {
		return mMWInfo->PSI;
	}

	// IEngineDamage
	bool IsBlown() {
		return mBlown;
	}
	void Repair() {
		mSabotage = 0.0f;
		mBlown = false;
	}
	bool IsSabotaged() {
		return mSabotage > 0.0f;
	}

	// ITransmission
	float GetDriveTorque() {
		return mDriveTorque;
	}
	GearID GetTopGear() {
		return (GearID)(GetNumGearRatios() - 1);
	}
	GearID GetGear() {
		return (GearID)mGear;
	}
	bool IsGearChanging() {
		return mGearShiftTimer > 0.0f;
	}

	bool Shift(GearID gear) {
		return DoGearChange(gear, false);
	}
	ShiftStatus GetShiftStatus() {
		return mShiftStatus;
	}
	ShiftPotential GetShiftPotential() {
		return mShiftPotential;
	}

	ShiftStatus OnGearChange(GearID gear);
	bool UseRevLimiter() {
		return bRevLimiter;
	}
	void DoECU();
	float DoThrottle(float dT);
	void DoInduction(const Physics::Tunings *tunings, float dT);
	float DoNos(const Physics::Tunings *tunings, float dT, bool engaged);
	void DoShifting(float dT);
	ShiftPotential UpdateShiftPotential(GearID gear, float rpm, float rpmFromGround);
	float GetEngineTorque(float rpm);

	// Inlines
	unsigned int GetNumGearRatios() {
		return mMWInfo->GEAR_RATIO.size();
	}

	float GetGearRatio(unsigned int idx) {
		return mMWInfo->GEAR_RATIO[idx];
	}

	float GetGearEfficiency(unsigned int idx) {
		return mMWInfo->GEAR_EFFICIENCY[idx];
	}

	float GetFinalGear() {
		return mMWInfo->FINAL_GEAR;
	}

	float GetRatioChange(unsigned int from, unsigned int to) {
		float ratio1 = mMWInfo->GEAR_RATIO[from];
		float ratio2 = mMWInfo->GEAR_RATIO[to];

		if (ratio1 > 0.0f && ratio2 > FLOAT_EPSILON) {
			return ratio1 / ratio2;
		} else {
			return 0.0f;
		}
	}

	float GetShiftDelay(unsigned int gear, bool shiftUp) {
		return mMWInfo->SHIFT_SPEED * GetGearRatio(gear);
	}

	bool RearWheelDrive() {
		return mMWInfo->TORQUE_SPLIT < 1.0f;
	}

	bool FrontWheelDrive() {
		return mMWInfo->TORQUE_SPLIT > 0.0f;
	}

	float GetShiftUpRPM(int gear) {
		return mShiftUpRPM[gear];
	}

	float GetShiftDownRPM(int gear) {
		return mShiftDownRPM[gear];
	}

	float GetCatchupCheat() {
		if (mCheater) {
			return mCheater->GetCatchupCheat();
		}
		return 0.0;
	}

	float mDriveTorque;
	float mDriveTorqueAtEngine;
	int mGear;
	float mGearShiftTimer;
	float mThrottle;
	float mSpool;
	float mPSI;
	float mInductionBoost;
	float mShiftUpRPM[10];
	float mShiftDownRPM[10];
	float mAngularVelocity;
	float mAngularAcceleration;
	float mTransmissionVelocity;
	float mNOSCapacity;
	float mNOSBoost;
	float mNOSEngaged;
	float mClutchRPMDiff;
	bool mEngineBraking;
	float mSportShifting;
	IInput *mIInput;
	IChassis *mSuspension;
	ICheater *mCheater;
	MWCarTuning* mMWInfo;
	Car* pCar;
	float mRPM;
	ShiftStatus mShiftStatus;
	ShiftPotential mShiftPotential;
	float mPeakTorque;
	float mPeakTorqueRPM;
	float mMaxHP;
	Clutch mClutch;
	bool mBlown;
	float mSabotage;

	int nLastRaceState;
};