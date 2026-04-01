namespace ChloeMWPhysics {
	template<typename T>
	T GetFuncPtr(const char* funcName) {
		if (auto dll = GetModuleHandleA("AssettoCorsaMWPhysics_gcp.dll")) {
			return (T)GetProcAddress(dll, funcName);
		}
		return nullptr;
	}

	bool IsNOSEnabled(void* car) {
		static auto funcPtr = GetFuncPtr<bool(__fastcall*)(void*)>("ChloeMW_IsNOSEnabled");
		if (!funcPtr) return false;
		return funcPtr(car);
	}

	bool HasTurbo(void* car) {
		static auto funcPtr = GetFuncPtr<bool(__fastcall*)(void*)>("ChloeMW_HasTurbo");
		if (!funcPtr) return false;
		return funcPtr(car);
	}

	float GetInductionPSI(void* car) {
		static auto funcPtr = GetFuncPtr<float(__fastcall*)(void*)>("ChloeMW_GetInductionPSI");
		if (!funcPtr) return 0.0;
		return funcPtr(car);
	}

	float GetRPM(void* car) {
		static auto funcPtr = GetFuncPtr<float(__fastcall*)(void*)>("ChloeMW_GetRPM");
		if (!funcPtr) return 0.0;
		return funcPtr(car);
	}

	float GetMaxRPM(void* car) {
		static auto funcPtr = GetFuncPtr<float(__fastcall*)(void*)>("ChloeMW_GetMaxRPM");
		if (!funcPtr) return 0.0;
		return funcPtr(car);
	}

	float GetRedline(void* car) {
		static auto funcPtr = GetFuncPtr<float(__fastcall*)(void*)>("ChloeMW_GetRedline");
		if (!funcPtr) return 0.0;
		return funcPtr(car);
	}

	float GetGameBreakerCharge(void* car) {
		static auto funcPtr = GetFuncPtr<float(__fastcall*)(void*)>("ChloeMW_GetGameBreakerCharge");
		if (!funcPtr) return 0.0;
		return funcPtr(car);
	}

	bool InGameBreaker(void* car) {
		static auto funcPtr = GetFuncPtr<float(__fastcall*)(void*)>("ChloeMW_InGameBreaker");
		if (!funcPtr) return false;
		return funcPtr(car);
	}

	bool IsInPerfectLaunchRange(void* car) {
		static auto funcPtr = GetFuncPtr<bool(__fastcall*)(void*)>("ChloeMW_IsInPerfectLaunchRange");
		if (!funcPtr) return false;
		return funcPtr(car);
	}
}