class ChassisMW : public IChassis {
public:
	struct State {
		UMath::Matrix4 matrix;
		UMath::Vector3 local_vel;
		float gas_input;
		UMath::Vector3 linear_vel;
		float brake_input;
		UMath::Vector3 angular_vel;
		float ground_effect;
		UMath::Vector3 cog;
		float ebrake_input;
		UMath::Vector3 dimension;
		Angle steer_input;
		UMath::Vector3 local_angular_vel;
		Angle slipangle;
		UMath::Vector3 inertia;
		float mass;
		UMath::Vector3 world_cog;
		float speed;
		float time;
		int flags;
		short driver_style;
		short driver_class;
		short gear;
		short blown_tires;
		float nos_boost;
		float shift_boost;

		enum Flags { IS_STAGING = 1, IS_DESTROYED };

		const UMath::Vector3 &GetRightVector() const {
			return *(UMath::Vector3*)&matrix.x;
		}
		const UMath::Vector3 &GetUpVector() const {
			return *(UMath::Vector3*)&matrix.y;
		}
		const UMath::Vector3 &GetForwardVector() const {
			return *(UMath::Vector3*)&matrix.z;
		}
		const UMath::Vector3 &GetPosition() const {
			return *(UMath::Vector3*)&matrix.p;
		}
	};

	enum SleepState {
		SS_LATERAL = 2,
		SS_ALL = 1,
		SS_NONE = 0,
	};

	void Create(Car* car);
	void Destroy();

	Mps ComputeMaxSlip(const State &state);
	void DoTireHeat(const State &state);
	float ComputeLateralGripScale(const State &state);
	float ComputeTractionScale(const State &state);
	SleepState DoSleep(const State &state);
	void ComputeAckerman(const float steering, const State &state, UMath::Vector4 *left, UMath::Vector4 *right);
	void SetCOG(float extra_bias, float extra_ride);
	void ComputeState(float dT, State &state);
	void DoAerodynamics(const State &state, float drag_pct, float aero_pct, float aero_front_z, float aero_rear_z,
						const Physics::Tunings *tunings);
	void DoJumpStabilizer(const State &state);

	Meters GuessCompression(unsigned int id, float downforce);
	void OnBehaviorChange();
	float GetRenderMotion();
	Meters GetRideHeight(unsigned int idx);
	float CalculateUndersteerFactor();
	float CalculateOversteerFactor();
	void OnTaskSimulate(float dT);

	ICollisionBody *mRBComplex;
	IRigidBody *mRB;
	IInput *mInput;
	IEngine *mEngine;
	ITransmission *mTransmission;
	//IDragTransmission *mDragTransmission;
	IEngineDamage *mEngineDamage;
	ISpikeable *mSpikeDamage;
	Car* pCar;
	MWCarTuning* mMWAttributes;
	float mJumpTime;
	float mJumpAlititude;
	float mTireHeat;

	auto GetOwner() {
		return GetPlayerInterface(pCar);
	}

	IVehicle* GetVehicle() {
		return GetOwner()->Find<IVehicle>();
	}

	int nLastRaceState;

	const char* mChassisType = "Chassis";
};