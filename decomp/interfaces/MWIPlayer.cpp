float Tweak_GameBreakerRechargeTime = 25.0f;
float Tweak_GameBreakerRechargeSpeed = 100.0f;

class IPlayer : public IMWInterface {
public:
	IPlayer(Car* car) : pCar(car) {
		ResetGameBreaker(true);
	}

	static inline const char* _IIDName = "IPlayer";

	Car* pCar;

	bool mInGameBreaker = false;
	float mGameBreakerCharge = 1.0;

	virtual bool CanDoGameBreaker() {
		IVehicle *ivehicle;
		if (pGameFlow->nRaceState != RACE_STATE_RACING || !mCOMObject->QueryInterface(&ivehicle)) {
			return false;
		}
		if (pCar->nIsRagdolled) {
			return false;
		}
		float speed_mph = MPS2MPH(ivehicle->GetSpeedometer());
		if (speed_mph < 30.0f) {
			return false;
		}
		if (!ivehicle->IsStaging()) {
			return true;
		}
		return false;
	}
	virtual bool ToggleGameBreaker() {
		if (!CanDoGameBreaker() && !mInGameBreaker) {
			return false;
		}
		if (mInGameBreaker) {
			SetGameBreaker(false);
		} else if (mGameBreakerCharge > 0.0f) {
			SetGameBreaker(true);
		} else {
			return false;
		}
		return true;
	}
	virtual void SetGameBreaker(bool on) {
		if (on != mInGameBreaker) {
			mInGameBreaker = on;
		}
	}
	virtual bool CanRechargeNOS() { return mInGameBreaker == 0; }
	virtual void ResetGameBreaker(bool full) {
		fOverrideTimescale = 1.0;
		mGameBreakerCharge = full ? 1.0f : 0.0f;
		SetGameBreaker(false);
	}
	virtual bool InGameBreaker() { return mInGameBreaker; }
	virtual void ChargeGameBreaker(float f) {}
	virtual void DoGameBreaker(float dT, float dT_real) {
		if (!CanDoGameBreaker()) {
			SetGameBreaker(false);
			return;
		}
		IVehicle *ivehicle;
		if (!mCOMObject->QueryInterface(&ivehicle)) {
			SetGameBreaker(false);
			return;
		}

		float speed_mph = MPS2MPH(ivehicle->GetSpeedometer());
		if (mInGameBreaker) {
			mGameBreakerCharge = mGameBreakerCharge - dT_real * 0.1f;
			mGameBreakerCharge = UMath::Max(mGameBreakerCharge, 0.0f);
		} else {
			if (speed_mph > Tweak_GameBreakerRechargeSpeed) {
				mGameBreakerCharge = mGameBreakerCharge + dT / Tweak_GameBreakerRechargeTime;
				mGameBreakerCharge = UMath::Min(mGameBreakerCharge, 1.0f);
			}
		}
		if (mGameBreakerCharge <= 0.0f) {
			SetGameBreaker(false);
		}
	}
};