//#define FUNCTION_LOG(name) WriteLog(std::format("{} called from {:X}", name, (uintptr_t)__builtin_return_address(0)));
#define WHEEL_FUNCTION_LOG(name) WriteLog(std::format("Wheel::{} called from {:X}", name, (uintptr_t)__builtin_return_address(0)));
#define CHASSIS_FUNCTION_LOG(name) WriteLog(std::format("Chassis::{} called from {:X}", name, (uintptr_t)__builtin_return_address(0)));
#define SUSPENSIONSIMPLE_FUNCTION_LOG(name) WriteLog(std::format("SuspensionSimple::{} called from {:X}", name, (uintptr_t)__builtin_return_address(0)));
#define SUSPENSIONRACER_FUNCTION_LOG(name) WriteLog(std::format("SuspensionRacer::{} called from {:X}", name, (uintptr_t)__builtin_return_address(0)));
#define ENGINERACER_FUNCTION_LOG(name) WriteLog(std::format("EngineRacer::{} called from {:X}", name, (uintptr_t)__builtin_return_address(0)));
//#define ICHASSIS_FUNCTION_LOG(name) WriteLog(std::format("IChassis::{} called from {:X}", name, (uintptr_t)__builtin_return_address(0)))
//#define ITIPTRONIC_FUNCTION_LOG(name) WriteLog(std::format("ITiptronic::{} called from {:X}", name, (uintptr_t)__builtin_return_address(0)))
//#define IRACEENGINE_FUNCTION_LOG(name) WriteLog(std::format("IRaceEngine::{} called from {:X}", name, (uintptr_t)__builtin_return_address(0)))
//#define IENGINEDAMAGE_FUNCTION_LOG(name) WriteLog(std::format("IEngineDamage::{} called from {:X}", name, (uintptr_t)__builtin_return_address(0)))
//#define IINDUCTABLE_FUNCTION_LOG(name) WriteLog(std::format("IInductable::{} called from {:X}", name, (uintptr_t)__builtin_return_address(0)))
//#define ITRANSMISSION_FUNCTION_LOG(name) WriteLog(std::format("ITransmission::{} called from {:X}", name, (uintptr_t)__builtin_return_address(0)))
//#define IENGINE_FUNCTION_LOG(name) WriteLog(std::format("IEngine::{} called from {:X}", name, (uintptr_t)__builtin_return_address(0)))
#define ICHASSIS_FUNCTION_LOG(name) {}
#define ITIPTRONIC_FUNCTION_LOG(name) {}
#define IRACEENGINE_FUNCTION_LOG(name) {}
#define IENGINEDAMAGE_FUNCTION_LOG(name) {}
#define IINDUCTABLE_FUNCTION_LOG(name) {}
#define ITRANSMISSION_FUNCTION_LOG(name) {}
#define IENGINE_FUNCTION_LOG(name) {}

void WriteLog(const std::string& str) {
	static auto file = std::ofstream("plugins/AssettoCorsaMWPhysics_gcp.log");

	file << str;
	file << "\n";
	file.flush();
}

auto GetStringWide(const std::string& string) {
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(string);
}

auto GetStringNarrow(const std::wstring& string) {
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.to_bytes(string);
}

ACPlugin* pMyPlugin = nullptr;
void __fastcall MWCarUpdate(Car* pThis, float dT);

extern "C" __declspec(dllexport) bool __fastcall acpGetName(wchar_t* out) { wcscpy_s(out, 256, L"AssettoCorsaMWPhysics"); return true; }
extern "C" __declspec(dllexport) bool __fastcall acpShutdown() { return true; }
extern "C" __declspec(dllexport) bool __fastcall acpOnGui(void*) { return false; }
extern "C" __declspec(dllexport) bool __fastcall acpGetControls(void*) { return false; }
extern "C" __declspec(dllexport) bool __fastcall acpUpdate(ACCarState*, float dT) { return true; }
extern "C" __declspec(dllexport) bool __fastcall acpInit(ACPlugin* plugin) {
	pMyPlugin = plugin;
	WriteLog(std::format("acpInit {:X}", (uintptr_t)plugin));
	WriteLog(std::format("carAvatar {:X}", (uintptr_t)plugin->carAvatar));
	WriteLog(std::format("car {:X}", (uintptr_t)plugin->car));
	WriteLog(std::format("sim {:X}", (uintptr_t)plugin->sim));
	WriteLog(std::format("car unixName {}", GetStringNarrow(plugin->car->unixName.c_str())));
	OnPluginStartup();
	return true;
}

// todo this isn't exactly the best way
auto GetTrack() {
	return pMyPlugin->car->splineLocator.track;
}

namespace SimSystem {
	double fSimTime = 0;

	double GetTime() {
		return fSimTime;
	}
}

int GetMWWheelID(int acWheel) {
	// mw has inverted tire positions?
	switch (acWheel) {
		case 0:
			return 1;
		case 1:
			return 0;
		case 2:
			return 3;
		case 3:
			return 2;
	}
}

uint64_t GetSupportedCSPBaseAddress() {
	auto pluginBase = (uintptr_t)GetModuleHandleW((std::filesystem::current_path().wstring() + L"/dwrite.dll").c_str());
	if (pluginBase && *(uint64_t*)(pluginBase + 0xC32B50) == 0x6C894810245C8948) return pluginBase;
	return 0;
}

bool IsAnyCSPInstalled() {
	auto dwritePresent = (uintptr_t)GetModuleHandleW((std::filesystem::current_path().wstring() + L"/dwrite.dll").c_str()) != 0;
	if (!dwritePresent) return false;
	return *(uint8_t*)(NyaHookLib::mEXEBase + 0x105480) != 0x48; // GhostCar::getGhostCarOpacity
}

bool IsSupportedCSPInstalled() {
	return IsAnyCSPInstalled() && GetSupportedCSPBaseAddress() != 0;
}

bool IsUnsupportedCSPInstalled() {
	return IsAnyCSPInstalled() && GetSupportedCSPBaseAddress() == 0;
}

bool IsPlayerCar(Car* car) {
	return car == pMyPlugin->car;
}

float GetCarUpgradeLevel(Car* car) {
	return fUpgradeLevel;
}

bool GetCarJunkman(Car* car) {
	return bUpgradeJunkman;
}