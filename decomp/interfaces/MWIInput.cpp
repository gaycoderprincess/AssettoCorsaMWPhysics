class IInput : public IMWInterface {
public:
	IInput(Car* car) : pCar(car) {}

	static inline const char* _IIDName = "IInput";

	Car* pCar;

private:
	virtual bool ShouldUseOriginalControls() {
		if (!strcmp(pCar->controlsProvider->getName(), "Kunos Simulazioni Keyboard controls provider")) return false;
		if (!strcmp(pCar->controlsProvider->getName(), "Kunos Simulazioni Joypad controls provider")) return false;
		return true;
	}

	KeyboardCarControl* GetKeyboardController() {
		if (!strcmp(pCar->controlsProvider->getName(), "Kunos Simulazioni Keyboard controls provider")) return (KeyboardCarControl*)pCar->controlsProvider;
		return nullptr;
	}

	float GetOriginalControlGas() {
		return pCar->controls.gas;
	}
	float GetOriginalControlBrake() {
		return pCar->controls.brake;
	}
	float GetOriginalControlHandBrake() {
		return pCar->controls.handBrake;
	}
	float GetOriginalControlSteering() {
		return -pCar->controls.steer;
	}

public:
	virtual bool AreControlsBlocked() {
		return pCar->isControlsLocked || pCar->lockControlsTime > pCar->ksPhysics->physicsTime;
	}
	virtual float GetControlGas() {
		if (AreControlsBlocked()) return 0.0;
		if (ShouldUseOriginalControls()) return GetOriginalControlGas();
		auto keyboard = GetKeyboardController();
		if (IsKeyPressed(keyboard ? keyboard->keyGas : VK_UP)) return 1.0;
		return GetPadKeyState(NYA_PAD_KEY_RT, -1) / 255.0;
	}
	virtual float GetControlBrake() {
		if (AreControlsBlocked()) return 0.0;
		if (ShouldUseOriginalControls()) return GetOriginalControlBrake();
		auto keyboard = GetKeyboardController();
		if (IsKeyPressed(keyboard ? keyboard->keyBrake : VK_DOWN)) return 1.0;
		return GetPadKeyState(NYA_PAD_KEY_LT, -1) / 255.0;
	}
	virtual float GetControlHandBrake() {
		if (AreControlsBlocked()) return 1.0;
		if (ShouldUseOriginalControls()) return GetOriginalControlHandBrake();
		auto keyboard = GetKeyboardController();
		if (IsKeyPressed(keyboard ? keyboard->keyHandbrake : VK_SPACE) || IsPadKeyPressed(NYA_PAD_KEY_A, -1)) return 1.0;
		return 0.0;
	}
	virtual float GetControlSteering() {
		if (AreControlsBlocked()) return 0.0;
		if (ShouldUseOriginalControls()) return GetOriginalControlSteering();

		int keyL = VK_LEFT;
		int keyR = VK_RIGHT;
		if (auto keyboard = GetKeyboardController()) {
			keyL = keyboard->keyLeft;
			keyR = keyboard->keyRight;
		}

		float f = 0;
		if (IsKeyPressed(keyL)) f += 1.0;
		if (IsKeyPressed(keyR)) f -= 1.0;
		f -= GetPadKeyState(NYA_PAD_KEY_LSTICK_X, -1) / 32768.0;
		return f;
	}
	virtual bool GetControlNOS() {
		if (!bNitrousEnabled) return false;
		if (AreControlsBlocked()) return false;
		return IsKeyPressed(VK_MENU) || IsPadKeyPressed(NYA_PAD_KEY_B, -1);
	}
	virtual bool IsAutomaticShift() { return pCar->autoShift.isActive; }
};

class IInputAI : public IInput {
public:
	IInputAI(Car* car) : IInput(car) {}

private:
	virtual bool ShouldUseOriginalControls() {
		return true;
	}
public:
	// the ai will pull the handbrake on a race restart for SOME REASON
	virtual float GetControlHandBrake() {
		return 0.0;
	}
	virtual bool GetControlNOS() {
		if (!bNitrousEnabled) return false;
		if (AreControlsBlocked()) return false;
		return GetControlGas() >= 0.99; // automatically nos when full throttle
	}
	virtual bool IsAutomaticShift() { return true; } // ai uses automatic shift
};