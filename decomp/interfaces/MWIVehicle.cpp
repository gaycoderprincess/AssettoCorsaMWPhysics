class IVehicle : public IMWInterface {
public:
	IVehicle(Car* car) : pCar(car) {}

	static inline const char* _IIDName = "IVehicle";

	Car* pCar;

	virtual const char* GetVehicleName() {
		static std::string str;
		auto carId = GetCarDataID_slow(pCar->pPlayer->nCarId);
		if (carId >= 200 && carId < 300) carId -= 200; // fo2 cars
		str = std::format("car_{}", carId);
		return str.c_str();
	}
	virtual int GetDriverStyle() { return STYLE_RACING; }
	virtual int GetDriverClass() { return DRIVER_HUMAN; }
	virtual float GetSpeedometer() { return mCOMObject->Find<ITransmission>()->GetSpeedometer(); }
	virtual float GetSpeed() { // this is so weird and useless
		UMath::Vector3 vel = *pCar->GetVelocity();
		ConvertWorldToLocal(pCar, vel, false);
		return vel.z >= 0.0 ? GetAbsoluteSpeed() : -GetAbsoluteSpeed();
	}
	virtual float GetAbsoluteSpeed() { return pCar->GetVelocity()->length(); }
	virtual bool IsStaging() { return pGameFlow->nRaceState <= RACE_STATE_COUNTDOWN; }
	virtual float GetPerfectLaunch() { return mPerfectLaunch.Time > 0.0 ? mPerfectLaunch.Amount : 0.0; }
	virtual bool IsDestroyed() { return pCar->nIsWrecked; }

	struct LaunchState {
		float Time = 0;
		float Amount = 0;
	} mPerfectLaunch;

	virtual float CalculatePerfectLaunch() {
		auto engine = mCOMObject->Find<IEngine>();
		if (!engine) return 0.0;
		auto raceEngine = mCOMObject->Find<IRaceEngine>();
		if (!raceEngine) return 0.0;

		float range = 0.0;
		auto peak_rpm = raceEngine->GetPerfectLaunchRange(&range);
		if (range > 0.0 && peak_rpm > 0.0) {
			auto dist = engine->GetRPM() - peak_rpm;
			if (dist < range && dist > 0.0) {
				return ((dist / range) + 1.0) * 0.5;
			}
		}
		return 0.0;
	}

	virtual bool IsInPerfectLaunchRange() {
		return CalculatePerfectLaunch() > 0.0;
	}

	virtual void DoStaging() {
		mPerfectLaunch.Time = 0.0;
		mPerfectLaunch.Amount = CalculatePerfectLaunch();
	}

	virtual void Launch() {
		auto engine = mCOMObject->Find<IEngine>();
		if (!engine) return;

		if (mPerfectLaunch.Time > 0.0) return;

		if (GetDriverClass() != DRIVER_HUMAN) {
			mPerfectLaunch.Amount = 0.0;
			mPerfectLaunch.Time = 0.0;
		}
		else if (mPerfectLaunch.Amount > 0.0) {
			mPerfectLaunch.Time = 4.5;
		}
	}

	virtual void OnTaskSimulate(float dT) {
		static bool bLastStaging = false;
		if (bLastStaging && !IsStaging()) {
			Launch();
		}
		bLastStaging = IsStaging();

		if (IsStaging()) {
			DoStaging();
		}
		else if (mPerfectLaunch.Time > 0.0) {
			if (auto transmission = mCOMObject->Find<ITransmission>()) {
				if (!transmission->IsGearChanging()) {
					mPerfectLaunch.Time = mPerfectLaunch.Time - dT;
					if (mPerfectLaunch.Time <= 0.0) {
						mPerfectLaunch.Amount = 0.0;
						mPerfectLaunch.Time = 0.0;
					}
				}
				if (GetSpeed() > MPH2MPS(60)) {
					mPerfectLaunch.Time = 0.0;
					mPerfectLaunch.Amount = 0.0;
				}
			}
		}
	}
};