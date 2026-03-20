#include <windows.h>
#include <format>
#include <codecvt>
#include <filesystem>
#include "toml++/toml.hpp"
#include "nya_commonhooklib.h"

#include "nya_commonmath.h"

#include "ac.h"

void OnPluginStartup();
#include "util.h"

bool bRevLimiter = true;
bool bSpeedbreakerEnabled = true;
float fUpgradeLevel = 1.0;
double fGlobalDeltaTime = 1.0 / 60.0;

#include "decomp/ConversionUtil.hpp"
#include "decomp/UMathExtras.h"
#include "decomp/HelperTypes.h"

#include "MWCarTuning.h"
#include "decomp/interfaces/MWInterface.h"
#include "decomp/interfaces/MWIChassis.h"
#include "decomp/interfaces/MWIRaceEngine.h"
#include "decomp/interfaces/MWITiptronic.h"
#include "decomp/interfaces/MWIEngineDamage.h"
//#include "decomp/interfaces/MWIInductable.cpp"
#include "decomp/interfaces/MWITransmission.h"
#include "decomp/interfaces/MWIEngine.h"
#include "decomp/interfaces/MWIVehicle.cpp"
#include "decomp/interfaces/MWIPlayer.cpp"
#include "decomp/interfaces/MWICollisionBody.cpp"
#include "decomp/interfaces/MWIRigidBody.cpp"
#include "decomp/interfaces/MWIInput.cpp"
#include "decomp/interfaces/MWISpikeable.cpp"
#include "decomp/interfaces/MWICheater.cpp"
#include "decomp/interfaces/MWIHumanAI.cpp"

#include "decomp/behaviors/MWWheel.h"
#include "decomp/behaviors/MWChassisBase.h"
#include "decomp/behaviors/SuspensionRacer.h"
#include "decomp/behaviors/EngineRacer.h"
#include "decomp/behaviors/MWWheel.cpp"
#include "decomp/behaviors/MWChassisBase.cpp"
#include "decomp/behaviors/SuspensionRacer.cpp"
#include "decomp/behaviors/EngineRacer.cpp"

void MWCarUpdate(Car* pThis, float dT) {
	SimSystem::fSimTime += dT;
	fGlobalDeltaTime = dT;
}

void OnPluginStartup() {
	auto car = pMyPlugin->car;
	NyaHookLib::PatchRelative(NyaHookLib::JMP, NyaHookLib::mEXEBase + 0x275DA0, &MWCarUpdate);
}

BOOL WINAPI DllMain(HINSTANCE, DWORD fdwReason, LPVOID) {
	switch(fdwReason) {
		case DLL_PROCESS_ATTACH: {
			if (NyaHookLib::GetEntryPoint() != 0x15AE310) {
				MessageBoxA(nullptr, "Unsupported game version! Make sure you're using the latest x64 Steam release (.exe size of 22890776 bytes)", "nya?!~", MB_ICONERROR);
				return TRUE;
			}

			//MessageBoxA(nullptr, std::format("Base address {:X}", NyaHookLib::mEXEBase).c_str(), "nya?!~", 0);
		} break;
		default:
			break;
	}
	return TRUE;
}