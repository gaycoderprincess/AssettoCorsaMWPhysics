class IInput : public IMWInterface {
public:
	IInput(Car* car) : pCar(car) {}

	static inline const char* _IIDName = "IInput";

	Car* pCar;

	virtual float GetControlGas() { return pCar->fGasPedal; }
	virtual float GetControlBrake() { return pCar->fBrakePedal; }
	virtual float GetControlHandBrake() { return pCar->fHandbrake; }
	virtual float GetControlSteering() {
		if (pCar->pPlayer->nPlayerType == PLAYERTYPE_AI || pGameFlow->nRaceState > RACE_STATE_RACING) return pCar->fSteerAngle;

		auto ply = pCar->pPlayer;
		if (ply->nIsUsingKeyboard) {
			float steeringInput = 0;
			if (ply->nSteeringKeyboardLeft) steeringInput = -1;
			if (ply->nSteeringKeyboardRight) steeringInput += 1;
			return steeringInput;
		}
		else {
			auto xinput = (XInputController*)ply->pController;
			return xinput->fLeftStickX;
		}
	}
	virtual bool GetControlNOS() { return pCar->fNitroButton > 0.0; }
	virtual bool IsAutomaticShift() { return true; }
};