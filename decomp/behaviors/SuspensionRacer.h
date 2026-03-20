// Credits: Brawltendo
class SuspensionRacerMW : public ChassisMW {
  public:
	class Tire : public WheelMW {
	  public:
		Tire(float radius, int index, MWCarTuning* mwSpecs);
		void BeginFrame(float max_slip, float grip_boost, float traction_boost, float drag_reduction);
		void EndFrame(float dT);
		float ComputeLateralForce(float load, float slip_angle);
		float GetPilotFactor(const float speed);
		void CheckForBrakeLock(float ground_force);
		void CheckSign();
		void UpdateFree(float dT);
		float UpdateLoaded(float lat_vel, float fwd_vel, float body_speed, float load, float dT);

		bool IsOnGround() {
			return mCompression > 0.0f;
		}

		bool IsSlipping() {
			return mTraction >= 1.0f;
		}

		void SetBrake(float brake) {
			mBrake = brake;
		}

		void SetEBrake(float ebrake) {
			mEBrake = ebrake;
		}

		float GetEBrake() {
			return mEBrake;
		}

		float GetTraction() {
			return mTraction;
		}

		float GetRoadSpeed() {
			return mRoadSpeed;
		}

		float GetLoad() {
			return mLoad;
		}

		float GetRadius() {
			return mRadius;
		}

		float GetAngularVelocity() {
			return mAV;
		}

		void SetAngularVelocity(float w) {
			mAV = w;
		}

		float GetLateralSpeed() {
			return mLateralSpeed;
		}

		float GetCurrentSlip() {
			return mSlip;
		}

		float GetToleratedSlip() {
			return mMaxSlip;
		}

		void SetLateralBoost(float f) {
			mLateralBoost = f;
		}

		void SetBottomOutTime(float time) {
			mBottomOutTime = time;
		}

		void ScaleTractionBoost(float scale) {
			mTractionBoost *= scale;
		}

		void SetDriftFriction(float scale) {
			mDriftFriction = scale;
		}

		void ApplyDriveTorque(float torque) {
			if (!mBrakeLocked) {
				mDriveTorque += torque;
			}
		}

		void ApplyBrakeTorque(float torque) {
			if (!mBrakeLocked) {
				mBrakeTorque += torque;
			}
		}

		float GetTotalTorque() {
			return mDriveTorque + mBrakeTorque;
		}

		float GetDriveTorque() {
			return mDriveTorque;
		}

		float GetLongitudeForce() {
			return mLongitudeForce;
		}

		bool IsBrakeLocked() {
			return mBrakeLocked;
		}

		bool IsSteeringWheel() {
			return mWheelIndex < 2;
		}

		void SetTractionCircle(const UMath::Vector2 &circle) {
			mTractionCircle = circle;
		}

		float GetSlipAngle() {
			return mSlipAngle;
		}

		void MatchSpeed(float speed) {
			mAV = speed / mRadius;
			mRoadSpeed = speed;
			mTraction = 1.0f;
			mBrakeLocked = false;
			mSlip = 0.0f;
			mAngularAcc = 0.0f;
			mLastTorque = 0.0f;
			mBrake = 0.0f;
			mEBrake = 0.0f;
			mLateralSpeed = 0.0f;
		}

		const float mRadius;
		float mBrake;
		float mEBrake;
		float mAV;
		float mLoad;
		float mLateralForce;
		float mLongitudeForce;
		float mDriveTorque;
		float mBrakeTorque;
		float mLateralBoost;
		float mTractionBoost;
		float mSlip;
		float mLastTorque;
		const int mWheelIndex;
		float mRoadSpeed;
		const MWCarTuning *mMWSpecs;
		float mAngularAcc;
		const int mAxleIndex;
		float mTraction;
		float mBottomOutTime;
		float mSlipAngle;
		UMath::Vector2 mTractionCircle;
		float mMaxSlip;
		float mGripBoost;
		float mDriftFriction;
		float mLateralSpeed;
		bool mBrakeLocked;

		enum LastRotationSign { WAS_POSITIVE, WAS_ZERO, WAS_NEGATIVE } mLastSign;

		float mDragReduction;
	};

	// Methods
	SuspensionRacerMW(Car* car);
	~SuspensionRacerMW();
	void CreateTires();
	void OnTaskSimulate(float dT);
	void DoDrifting(const State &state);
	void TuneWheelParams(State &state);
	void DoWheelForces(State &state);
	float CalculateMaxSteering(State &state, SteeringType steer_type);
	float CalculateSteeringSpeed(State &state);
	void DoWallSteer(State &state);
	void DoDriveForces(State &state);
	float DoHumanSteering(State &state);
	float DoAISteering(State &state);
	void DoSteering(State &state, UMath::Vector3 &right, UMath::Vector3 &left);
	void DoAerobatics(State &state);
	float CalcYawControlLimit(float speed);

	// Overrides
	void OnBehaviorChange();
	Meters GetRideHeight(unsigned int idx);
	Radians GetWheelAngularVelocity(int index);
	void MatchSpeed(float speed);
	UMath::Vector3* GetWheelCenterPos(UMath::Vector3* result, unsigned int i);
	void Reset();

	float GetWheelTraction(unsigned int index) {
		return mTires[index]->GetTraction();
	}
	float GetWheelRadius(unsigned int index) {
		return mTires[index]->GetRadius();
	}
	float GetWheelSlip(unsigned int idx) {
		return mTires[idx]->GetCurrentSlip();
	}
	float GetToleratedSlip(unsigned int idx) {
		return mTires[idx]->GetToleratedSlip();
	}
	float GetWheelSkid(unsigned int idx) {
		return mTires[idx]->GetLateralSpeed();
	}
	float GetWheelLoad(unsigned int i) {
		return mTires[i]->GetLoad();
	}
	void SetWheelAngularVelocity(int wheel, float w) {
		mTires[wheel]->SetAngularVelocity(w);
	}
	unsigned int GetNumWheels() {
		return 4;
	}
	const UMath::Vector3 *GetWheelPos(unsigned int i) {
		return &mTires[i]->GetPosition();
	}
	const UMath::Vector3 *GetWheelLocalPos(unsigned int i) {
		return &mTires[i]->GetLocalArm();
	}
	float GetWheelRoadHeight(unsigned int i) {
		return mTires[i]->GetNormal().w;
	}
	float GetCompression(unsigned int i) {
		return mTires[i]->GetCompression();
	}
	const UMath::Vector4 *GetWheelRoadNormal(unsigned int i) {
		return &mTires[i]->GetNormal();
	}
	bool IsWheelOnGround(unsigned int i) {
		return mTires[i]->IsOnGround();
	}
	int GetWheelRoadSurface(unsigned int i) {
		return mTires[i]->GetSurface();
	}
	const UMath::Vector3 *GetWheelVelocity(unsigned int i) {
		return &mTires[i]->GetVelocity();
	}
	int GetNumWheelsOnGround() {
		return mNumWheelsOnGround;
	}
	float GetWheelSteer(unsigned int wheel) {
		return wheel < 2 ? RAD2ANGLE(mSteering.Wheels[wheel]) : 0.0f;
	}
	float GetMaxSteering() {
		return DEG2ANGLE(mSteering.Maximum);
	}
	float GetWheelSlipAngle(unsigned int idx) {
		return mTires[idx]->GetSlipAngle();
	}

	// UC IChassis
	float GetWheelTorque(int idx) {
		return mTires[idx]->GetTotalTorque();
	}
	float GetWheelTorqueRatio(int idx) {
		auto tire = mTires[idx];
		auto v70 = tire->mGripBoost;
		if (v70 <= 1.0) v70 = 1.0;
		float v63 = 1.0; // todo Curve::GetValueLinear((v19->mVehicleInfo->mLayoutPtr + 48 * v19->mAxleIndex + 0x1F0), (v21 * 2.23699)) Mu0[AxleIndex]
		auto v69 = std::sqrt(((tire->mLongitudeForce * tire->mLongitudeForce) + (tire->mLateralForce * tire->mLateralForce)));
		auto v72 = (v69 / (((((tire->mBrake + 1.0) * tire->mTractionBoost) * tire->mLoad) * v70) * v63));
		return UMath::Clamp(v72, -3.0f, 3.0f);
	}
	float GetWheelBrakeTorque(int idx) {
		return mTires[idx]->mBrakeTorque;
	}
	float GetWheelLateralForce(int idx) {
		return mTires[idx]->mLateralForce;
	}
	void ForceCompression(int idx, float f) {
		mTires[idx]->SetCompression(f);
	}

	struct Drift {
		Drift() : State(D_OUT), Value(0.0f) {}

		enum eState { D_OUT, D_ENTER, D_IN, D_EXIT } State; // offset 0x0, size 0x4
		float Value;										// offset 0x4, size 0x4

		void Reset() {
			State = D_OUT;
			Value = 0.0f;
		}
	};

	struct Burnout {
		Burnout() : mState(0), mBurnOutTime(0.0f), mTraction(1.0f), mBurnOutAllow(0.0f) {}

		void Update(const float dT, const float speedmph, const float max_slip, const int max_slip_wheel, const float yaw);

		int GetState() {
			return mState;
		}

		float GetTraction() {
			return mTraction;
		}

		void Reset() {
			mState = 0;
			mBurnOutTime = 0.0f;
			mTraction = 1.0f;
			mBurnOutAllow = 0.0f;
		}

		void SetState(int s) {
			mState = s;
		}

		void SetBurnOutTime(float t) {
			mBurnOutTime = t;
		}

		void SetTraction(float t) {
			mTraction = t;
		}

		float GetBurnOutTime(float t) {
			return mBurnOutTime;
		}

		void DecBurnOutTime(float t) {
			mBurnOutTime -= t;
		}

		void ClearBurnOutAllow() {
			mBurnOutAllow = 0.0f;
		}

		void IncBurnOutAllow(float t) {
			mBurnOutAllow += t;
		}

	  private:
		int mState;		  // offset 0x0, size 0x4
		float mBurnOutTime;  // offset 0x4, size 0x4
		float mTraction;	 // offset 0x8, size 0x4
		float mBurnOutAllow; // offset 0xC, size 0x4
	};

	struct Steering {
		Steering() : InputAverage(0.55f, 60.0f), InputSpeedCoeffAverage(0.15f, 60.0f) {
			Reset();
		}

		float Previous;					   // offset 0x0, size 0x4
		float Wheels[2];					  // offset 0x4, size 0x8
		float Maximum;						// offset 0xC, size 0x4
		float LastMaximum;					// offset 0x10, size 0x4
		float LastInput;					  // offset 0x14, size 0x4
		AverageWindow InputAverage;		   // offset 0x18, size 0x38
		AverageWindow InputSpeedCoeffAverage; // offset 0x50, size 0x38
		float CollisionTimer;				 // offset 0x88, size 0x4
		float WallNoseTurn;				   // offset 0x8C, size 0x4
		float WallSideTurn;				   // offset 0x90, size 0x4
		float YawControl;					 // offset 0x94, size 0x4

		void Reset() {
			Previous = 0.0f;
			Wheels[1] = 0.0f;
			Wheels[0] = 0.0f;
			Maximum = 45.0f;
			LastMaximum = 45.0f;
			LastInput = 0.0f;
			InputAverage.Reset(0.0f);
			InputSpeedCoeffAverage.Reset(0.0f);
			CollisionTimer = 0.0f;
			WallNoseTurn = 0.0f;
			WallSideTurn = 0.0f;
			YawControl = 1.0f;
		}
	};

	struct Differential {
		void CalcSplit(bool locked);

		float angular_vel[2];
		int has_traction[2];
		float bias;
		float factor;
		float torque_split[2];
	};

	bool RearWheelDrive() {
		return mMWAttributes->TORQUE_SPLIT < 1.0f;
	}

	bool FrontWheelDrive() {
		return mMWAttributes->TORQUE_SPLIT > 0.0f;
	}

	bool IsDriveWheel(unsigned int i) {
		return (IsRear(i) && RearWheelDrive()) || (IsFront(i) && FrontWheelDrive());
	}

	Tire &GetWheel(unsigned int i) {
		return *mTires[i];
	}

	IHumanAI *mHumanAI;
	float mGameBreaker;
	unsigned int mNumWheelsOnGround;
	float mLastGroundCollision;
	Drift mDrift;
	Burnout mBurnOut;
	Steering mSteering;
	Tire *mTires[4];
};
ChassisMW::State LastChassisState = {};

float YawFrictionBoost(float yaw, float ebrake, float speed, float yawcontrol, float grade);