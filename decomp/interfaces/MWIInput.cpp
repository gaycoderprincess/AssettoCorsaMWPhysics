class IInput : public IMWInterface {
public:
	IInput(Car* car) : pCar(car) {}

	static inline const char* _IIDName = "IInput";

	Car* pCar;

	virtual float GetControlGas() {
		if (IsKeyPressed(VK_UP)) return 1.0;
		return GetPadKeyState(NYA_PAD_KEY_RT, -1) / 255.0;
	}
	virtual float GetControlBrake() {
		if (IsKeyPressed(VK_DOWN)) return 1.0;
		return GetPadKeyState(NYA_PAD_KEY_LT, -1) / 255.0;
	}
	virtual float GetControlHandBrake() {
		if (IsKeyPressed(VK_SPACE) || IsPadKeyPressed(NYA_PAD_KEY_A, -1)) return 1.0;
		return 0.0;
	}
	virtual float GetControlSteering() {
		float f = 0;
		if (IsKeyPressed(VK_LEFT)) f += 1.0;
		if (IsKeyPressed(VK_RIGHT)) f -= 1.0;
		f -= GetPadKeyState(NYA_PAD_KEY_LSTICK_X, -1) / 32768.0;
		return f;
	}
	virtual bool GetControlNOS() {
		return false; // todo there's no nos display, this'd be confusing
		//return IsKeyPressed(VK_MENU) || IsPadKeyPressed(NYA_PAD_KEY_B, -1);
	}
	//virtual float GetControlGas() { return pCar->controls.gas; }
	//virtual float GetControlBrake() { return pCar->controls.brake; }
	//virtual float GetControlHandBrake() { return pCar->controls.handBrake; }
	//virtual float GetControlSteering() { return -pCar->controls.steer; }
	virtual bool IsAutomaticShift() { return true; }
};