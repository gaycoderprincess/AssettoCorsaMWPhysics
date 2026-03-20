class IInput : public IMWInterface {
public:
	IInput(Car* car) : pCar(car) {}

	static inline const char* _IIDName = "IInput";

	Car* pCar;

	virtual float GetControlGas() { return IsKeyPressed(VK_UP); }
	virtual float GetControlBrake() { return IsKeyPressed(VK_DOWN); }
	virtual float GetControlHandBrake() { return IsKeyPressed(VK_SPACE); }
	virtual float GetControlSteering() {
		float f = 0;
		if (IsKeyPressed(VK_LEFT)) f += 1.0;
		if (IsKeyPressed(VK_RIGHT)) f -= 1.0;
		return f;
	}
	virtual float GetControlNOS() {
		return IsKeyPressed(VK_MENU);
	}
	//virtual float GetControlGas() { return pCar->controls.gas; }
	//virtual float GetControlBrake() { return pCar->controls.brake; }
	//virtual float GetControlHandBrake() { return pCar->controls.handBrake; }
	//virtual float GetControlSteering() { return -pCar->controls.steer; }
	//virtual bool GetControlNOS() {
	//	// todo!
	//	//return pCar->fNitroButton > 0.0;
	//	return false;
	//}
	virtual bool IsAutomaticShift() { return true; }
};