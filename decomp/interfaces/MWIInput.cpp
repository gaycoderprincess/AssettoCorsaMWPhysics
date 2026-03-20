class IInput : public IMWInterface {
public:
	IInput(Car* car) : pCar(car) {}

	static inline const char* _IIDName = "IInput";

	Car* pCar;

	virtual float GetControlGas() { return pCar->controls.gas; }
	virtual float GetControlBrake() { return pCar->controls.brake; }
	virtual float GetControlHandBrake() { return pCar->controls.handBrake; }
	virtual float GetControlSteering() { return -pCar->controls.steer; }
	virtual bool GetControlNOS() {
		// todo!
		//return pCar->fNitroButton > 0.0;
		return false;
	}
	virtual bool IsAutomaticShift() { return true; }
};