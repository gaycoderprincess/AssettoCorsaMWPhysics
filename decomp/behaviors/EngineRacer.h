enum eTireIdx {
	TIRE_FL,
	TIRE_FR,
	TIRE_MAX_FRONT = 2,
	TIRE_RR = 2,
	TIRE_RL,
	TIRE_MAX,
};

inline unsigned int UTIL_InterprolateIndex(unsigned int last_index, float value, float limit_min, float limit_max, float &ratio) {
	float value_range = limit_max - limit_min;
	float value_offset = value - limit_min;
	ratio = last_index * value_offset / value_range;
	unsigned int index1 = static_cast<unsigned int>(ratio);
	ratio -= index1;
	return index1;
}

namespace Physics {
	namespace Info {
		float AerodynamicDownforce(MWCarTuning* pThis, const float speed) {
			return speed * 2 * pThis->AERO_COEFFICIENT * 1000.0f;
		}

		enum eInductionType {
			INDUCTION_NONE = 0,
			INDUCTION_TURBO_CHARGER = 1,
			INDUCTION_SUPER_CHARGER = 2,
		};

		// Credits: Brawltendo
		float EngineInertia(const MWCarTuning* mw, const bool loaded) {
			float scale;
			if (loaded)
				scale = 1.f;
			else
				scale = 0.35f;
			return scale * (mw->FLYWHEEL_MASS * 0.025f + 0.25f);
		}

		// Credits: Brawltendo
		eInductionType InductionType(const MWCarTuning* mw) {
			if (mw->HIGH_BOOST > 0.0f || mw->LOW_BOOST > 0.0f) {
				// turbochargers don't produce significant boost until above the boost threshold (the lowest engine RPM at which it will spool up)
				// meanwhile superchargers apply boost proportionally to the engine RPM, so this param isn't needed there
				if (mw->SPOOL > 0.0f) {
					return INDUCTION_TURBO_CHARGER;
				} else {
					return INDUCTION_SUPER_CHARGER;
				}
			} else {
				return INDUCTION_NONE;
			}
		}

		float Torque(const MWCarTuning* mw, float rpm) {
			float rpm_min = mw->IDLE;
			float rpm_max = mw->MAX_RPM;
			rpm = UMath::Clamp(rpm, mw->IDLE, mw->RED_LINE);
			unsigned int numpts = mw->TORQUE.size();
			if (numpts > 1) {
				float ratio;
				unsigned int index = UTIL_InterprolateIndex(numpts - 1, rpm, rpm_min, rpm_max, ratio);
				float power = mw->TORQUE[index];
				unsigned int secondIndex = UMath::Min(numpts - 1, index + 1);
				return UMath::Lerp(power, mw->TORQUE[secondIndex], ratio);
			}

			return 0.0f;
		}

		float WheelDiameter(const MWCarTuning* tires, bool front) {
			int axle = front ? 0 : 1;
			float diameter = INCH2METERS(tires->RIM_SIZE.At(axle));
			return diameter + tires->SECTION_WIDTH.At(axle) * 0.001f * 2.0f * (tires->ASPECT_RATIO.At(axle) * 0.01f);
		}

		// Credits: Brawltendo
		float NosBoost(const MWCarTuning* mw, const Tunings *tunings) {
			float torque_scale = 1.0f;
			float boost = mw->TORQUE_BOOST;
			if (tunings) {
				boost += boost * tunings->Value[Physics::Tunings::NOS] * 0.25f;
			}
			return boost + torque_scale;
		}

		// Credits: Brawltendo
		float NosCapacity(const MWCarTuning* mw, const Tunings *tunings) {
			float capacity = mw->NOS_CAPACITY;
			if (tunings) {
				capacity -= capacity * tunings->Value[Physics::Tunings::NOS] * 0.25f;
			}
			return capacity;
		}

		// Credits: Brawltendo
		Mps Speedometer(const MWCarTuning* mw, Rpm rpm, GearID gear, const Tunings *tunings) {
			float speed = 0.0f;
			float gear_ratio = mw->GEAR_RATIO[gear] * mw->FINAL_GEAR;
			float power_range = mw->RED_LINE - mw->IDLE;
			gear_ratio = UMath::Abs(gear_ratio);
			if (gear_ratio > 0.0f && power_range > 0.0f) {
				float wheelrear = WheelDiameter(mw, false) * 0.5f;
				float wheelfront = WheelDiameter(mw, true) * 0.5f;
				float avg_wheel_radius = (wheelrear + wheelfront) * 0.5f;
				float clutch_rpm = (rpm - mw->IDLE) / gear_ratio / power_range * mw->RED_LINE;
				speed = RPM2RPS(clutch_rpm) * avg_wheel_radius;
			}

			float limiter = MPH2MPS(mw->SPEED_LIMITER[0]);
			if (limiter > 0.0f) {
				speed = UMath::Min(speed, limiter);
			}

			return speed;
		}

		// Credits: Brawltendo
		float InductionRPM(const MWCarTuning* mw, const Tunings *tunings) {
			float spool = mw->SPOOL;

			// tune the (normalized) RPM at which forced induction kicks in
			if (tunings && spool > 0.0f) {
				float range;
				float value = tunings->Value[Physics::Tunings::INDUCTION];
				if (value < 0.0f) {
					range = spool * 0.25f;
				} else {
					range = (1.0f - spool) * 0.25f;
				}
				spool += range * value;
			}

			// return the unnormalized RPM
			return spool * (mw->RED_LINE - mw->IDLE) + mw->IDLE;
		}

		// Credits: Brawltendo
		float InductionBoost(const MWCarTuning* mw, float rpm, float spool, const Tunings *tunings, float *psi) {
			if (psi) {
				*psi = 0.0f;
			}

			spool = UMath::Clamp(spool, 0.0f, 1.0f);
			float rpm_min = mw->IDLE;
			float rpm_max = mw->RED_LINE;
			float induction_boost = 0.f;
			float spool_rpm = InductionRPM(mw, tunings);
			float high_boost = mw->HIGH_BOOST;
			float low_boost = mw->LOW_BOOST;
			float drag = mw->VACUUM;

			if (high_boost > 0.0f || low_boost > 0.0f) {
				// tuning slider adjusts the induction boost bias
				// -tuning produces more low end boost, while +tuning produces more high end boost
				if (tunings) {
					float value = tunings->Value[Physics::Tunings::INDUCTION];
					low_boost -= low_boost * value * 0.25f;
					high_boost += high_boost * value * 0.25f;
				}

				if (rpm >= spool_rpm) {
					float induction_ratio = UMath::Ramp(rpm, spool_rpm, rpm_max);
					induction_boost = induction_ratio * high_boost + (1.0f - induction_ratio) * low_boost;
					if (psi) {
						*psi = spool * mw->PSI * UMath::Ramp(induction_boost, 0.0f, UMath::Max(high_boost, low_boost));
					}
				} else if (drag < 0.0f) {
					// apply vacuum effect when not in boost
					float drag_ratio = UMath::Ramp(rpm, rpm_min, spool_rpm);
					induction_boost = drag_ratio * drag;
					if (psi) {
						*psi = drag_ratio * -mw->PSI * UMath::Ramp(-induction_boost, 0.0f, UMath::Max(high_boost, low_boost));
					}
				}
			}

			return induction_boost * spool;
		}

		// Credits: Brawltendo
		bool ShiftPoints(const MWCarTuning* mw, float *shift_up, float *shift_down, unsigned int numpts) {
			for (int i = 0; i < numpts; ++i) {
				shift_up[i] = 0.0f;
				shift_down[i] = 0.0f;
			}

			unsigned int num_gear_ratios = mw->GEAR_RATIO.size();
			if (numpts < num_gear_ratios)
				return false;

			float redline = mw->RED_LINE;
			int topgear = num_gear_ratios - 1;
			int j;
			for (j = G_FIRST; j < topgear; ++j) {
				float g1 = mw->GEAR_RATIO[j];
				float g2 = mw->GEAR_RATIO[j + 1];
				float rpm = (redline + mw->IDLE) * 0.5f;
				float max = rpm;
				int flag = 0;

				if (rpm < redline) {
					// find the upshift RPM for this gear using predicted engine torque
					while (!flag) {
						// seems like the rpm and spool params are swapped in both instances
						// so either it's a mistake that was copy-pasted or it was a deliberate choice
						float currenttorque = Torque(mw, max) * (InductionBoost(mw, 1.0f, max, NULL, NULL) + 1.0f);
						float shiftuptorque;
						if (UMath::Abs(g1) > 0.00001f) {
							float ratio = g2 / g1;
							float next_rpm = ratio * max;
							shiftuptorque = Torque(mw, next_rpm) * (InductionBoost(mw, 1.0f, next_rpm, NULL, NULL) + 1.0f) * g2 / g1;
						} else {
							shiftuptorque = 0.0f;
						}

						// set the upshift RPM to the current max
						if (shiftuptorque > currenttorque)
							break;

						max += 50.0f;
						// set the upshift RPM to the redline RPM
						flag = !(max < redline);
					}
					if (!flag) {
						shift_up[j] = max;
					}
				} else {
					flag = 1;
				}
				if (flag) {
					shift_up[j] = redline - 100.0f;
				}

				// calculate downshift RPM for the next gear
				if (UMath::Abs(g1) > 0.00001f) {
					shift_down[j + 1] = (g2 / g1) * shift_up[j];
				} else {
					shift_down[j + 1] = 0.0f;
				}
			}

			shift_up[topgear] = mw->RED_LINE;
			return true;
		}

		float MaxInductedTorque(const MWCarTuning* mw, Rpm &atrpm, const Tunings *tunings) {
			if (mw->TORQUE.size() > 1)  {
				auto v9 = 0.0;
				atrpm = mw->IDLE;
				auto v12 = mw->IDLE;
				auto v13 = ((mw->MAX_RPM - mw->IDLE) / (mw->TORQUE.size() - 1));
				int v10 = 0;
				while (v10 < mw->TORQUE.size()) {
					auto v18 = (mw->TORQUE[v10] * (Physics::Info::InductionBoost(mw, v12, 1.0, tunings, nullptr) + 1.0));
					if (v18 > v9) {
						atrpm = v12;
						v9 = v18;
					}
					v12 = (v12 + v13);
					++v10;
				}
				auto v20 = mw->RED_LINE;
				if (atrpm < v20)
					v20 = atrpm;
				auto v21 = v20;
				if (mw->IDLE > v20)
					v21 = mw->IDLE;
				atrpm = v21;
				return v9;
			} else {
				atrpm = mw->IDLE;
			}
			return 0.0;
		}

		float MaxInductedPower(const MWCarTuning* mw, const Tunings* tunings) {
			if (mw->TORQUE.size() > 1)  {
				float v11 = 0.0;
				auto v12 = mw->IDLE;
				auto v15 = ((mw->MAX_RPM - mw->IDLE) / (mw->TORQUE.size() - 1));
				int v13 = 0;
				while (v13 < mw->TORQUE.size())  {
					auto v20 = (Physics::Info::InductionBoost(mw, v12, 1.0, tunings, nullptr) + 1.0);
					if ( (((mw->TORQUE[v13] * v20) * v12) * 0.00019040366) > v11 )
						v11 = (((mw->TORQUE[v13] * v20) * v12) * 0.00019040366);
					v12 = (v12 + v15);
					++v13;
				}
				return v11;
			}
			return 0.0;
		}
	}
}

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