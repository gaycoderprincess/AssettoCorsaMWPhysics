#include <windows.h>
#include <format>
#include <codecvt>
#include "toml++/toml.hpp"
#include "nya_commonhooklib.h"

#include "nya_commonmath.h"

#include "ac.h"

void WriteLog(const std::string& str) {
	static auto file = std::ofstream("AssettoCorsaMWPhysics_gcp.log");

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

void MWCarUpdate(Car* pThis, float dT) {

}

extern "C" __declspec(dllexport) bool __fastcall acpGetName(wchar_t* out) { wcscpy_s(out, 256, L"AssettoCorsaMWPhysics"); return true; }
extern "C" __declspec(dllexport) bool __fastcall acpShutdown() { return true; }
extern "C" __declspec(dllexport) bool __fastcall acpOnGui(void*) { return false; }
extern "C" __declspec(dllexport) bool __fastcall acpGetControls(void*) { return false; }

extern "C" __declspec(dllexport) bool __fastcall acpInit(ACPlugin* plugin) {
	pMyPlugin = plugin;
	WriteLog(std::format("acpInit {:X}", (uintptr_t)pMyPlugin));
	WriteLog(std::format("carAvatar {:X}", (uintptr_t)pMyPlugin->carAvatar));
	WriteLog(std::format("car {:X}", (uintptr_t)pMyPlugin->car));
	WriteLog(std::format("sim {:X}", (uintptr_t)pMyPlugin->sim));
	WriteLog(std::format("car configName {}", GetStringNarrow(pMyPlugin->car->configName.c_str())));
	WriteLog(std::format("car unixName {}", GetStringNarrow(pMyPlugin->car->unixName.c_str())));
	WriteLog(std::format("car screenName {}", GetStringNarrow(pMyPlugin->car->screenName.c_str())));
	NyaHookLib::PatchRelative(NyaHookLib::JMP, NyaHookLib::mEXEBase + 0x275DA0, &MWCarUpdate);
	return true;
}

extern "C" __declspec(dllexport) bool __fastcall acpUpdate(void*, float dT) {
	auto car = pMyPlugin->car;


	return true;
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