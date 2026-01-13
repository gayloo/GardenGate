#include "Utils.hpp"
#include <filesystem>
#include <shlobj.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <process.h>

namespace fs = std::filesystem;

namespace Utils {
	namespace Registry {
		std::string GetGamePathFromRegistry(bool isGW2) {
			HKEY hKey = NULL;
			LONG result = 0;
			char pathBuffer[MAX_PATH] = { 0 };
			DWORD bufferSize = MAX_PATH;

			std::string regPath;
			if (isGW2) {
				regPath = "SOFTWARE\\WOW6432Node\\PopCap\\Plants vs Zombies GW2";
				result = RegOpenKeyExA(HKEY_LOCAL_MACHINE, regPath.c_str(), 0, KEY_READ, &hKey);
				if (result != ERROR_SUCCESS) {
					regPath = "SOFTWARE\\PopCap\\Plants vs Zombies GW2";
					result = RegOpenKeyExA(HKEY_LOCAL_MACHINE, regPath.c_str(), 0, KEY_READ, &hKey);
				}
			}
			else {
				regPath = "SOFTWARE\\WOW6432Node\\PopCap\\Plants vs Zombies Garden Warfare";
				result = RegOpenKeyExA(HKEY_LOCAL_MACHINE, regPath.c_str(), 0, KEY_READ, &hKey);
				if (result != ERROR_SUCCESS) {
					regPath = "SOFTWARE\\PopCap\\Plants vs Zombies Garden Warfare";
					result = RegOpenKeyExA(HKEY_LOCAL_MACHINE, regPath.c_str(), 0, KEY_READ, &hKey);
				}
			}

			if (result == ERROR_SUCCESS) {
				result = RegQueryValueExA(hKey, "Install Dir", NULL, NULL, (LPBYTE)pathBuffer, &bufferSize);
				if (result != ERROR_SUCCESS) {
					result = RegQueryValueExA(hKey, "Path", NULL, NULL, (LPBYTE)pathBuffer, &bufferSize);
				}
				RegCloseKey(hKey);
				return std::string(pathBuffer);
			}
			return "";
		}
	}

	namespace Dialog {
		std::string BrowseForExe(HWND hwnd, bool isGW2) {
			char pathBuffer[MAX_PATH] = "";
			OPENFILENAMEA ofn = {};
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hwnd;
			ofn.lpstrFilter = isGW2 ?
				"GW2 Executable\0GW2.Main_Win64_Retail.exe;*.exe\0All Files\0*.*\0" :
				"GW1 Executable\0PVZ.Main_Win64_Retail.exe;*.exe\0All Files\0*.*\0";
			ofn.lpstrFile = pathBuffer;
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

			if (GetOpenFileNameA(&ofn)) return pathBuffer;
			return "";
		}
	}

	namespace UI {
		void CenteredInput(const char* label, std::string& value,
			float centerOffset, float fieldWidth,
			ImGuiInputTextFlags flags) {
			ImGui::SetCursorPosX(centerOffset);
			ImGui::TextUnformatted(label);
			ImGui::SetCursorPosX(centerOffset);
			ImGui::SetNextItemWidth(fieldWidth);

			char buf[256];
			strncpy_s(buf, value.c_str(), sizeof(buf) - 1);
			buf[sizeof(buf) - 1] = '\0';

			if (ImGui::InputText(std::string("##").append(label).c_str(), buf, sizeof(buf), flags)) {
				value = buf;
				if (strcmp(label, "Username:") == 0 && value.length() > 15) value.resize(15);
			}
		}

		bool CenteredButton(const char* label, float width, float dpiScale) {
			float safeWidth = ImGui::GetContentRegionAvail().x;
			float offset = (safeWidth - width) / 2.0f;
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
			return ImGui::Button(label, ImVec2(width, 38.0f * dpiScale));
		}
	}

	namespace Process {

		bool InjectDLL(DWORD processId, const std::string& dllPath) {
			HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
				PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
				FALSE, processId);
			if (!hProcess) return false;

			char fullPath[MAX_PATH];
			if (!GetFullPathNameA(dllPath.c_str(), MAX_PATH, fullPath, nullptr)) {
				CloseHandle(hProcess);
				return false;
			}

			SIZE_T bytes = strlen(fullPath) + 1;
			LPVOID remoteMem = VirtualAllocEx(hProcess, nullptr, bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
			if (!remoteMem) {
				CloseHandle(hProcess);
				return false;
			}

			if (!WriteProcessMemory(hProcess, remoteMem, fullPath, bytes, nullptr)) {
				VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
				CloseHandle(hProcess);
				return false;
			}

			LPTHREAD_START_ROUTINE loadLibAddr =
				(LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
			if (!loadLibAddr) {
				VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
				CloseHandle(hProcess);
				return false;
			}

			HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0,
				loadLibAddr, remoteMem, 0, nullptr);
			if (!hThread) {
				VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
				CloseHandle(hProcess);
				return false;
			}

			WaitForSingleObject(hThread, INFINITE);

			CloseHandle(hThread);
			VirtualFreeEx(hProcess, remoteMem, 0, MEM_RELEASE);
			CloseHandle(hProcess);

			return true;
		}

		LaunchResult LaunchAndInject(const std::string& exePath,
			const std::string& args,
			bool injectDLL,
			const std::string& dllName)
		{
			LaunchResult lr;
			STARTUPINFOA si = { sizeof(si) };
			PROCESS_INFORMATION pi = { 0 };

			std::string cmdLineStr = "\"" + exePath + "\" " + args;
			std::vector<char> cmdLine(cmdLineStr.begin(), cmdLineStr.end());
			cmdLine.push_back('\0');

			BOOL ok = CreateProcessA(
				NULL, cmdLine.data(),
				NULL, NULL, FALSE, 0, NULL,
				fs::path(exePath).parent_path().string().c_str(),
				&si, &pi);

			if (!ok) {
				lr.error = "CreateProcess failed. Error: " + std::to_string(GetLastError());
				return lr;
			}

			lr.ok = true;
			lr.pi = pi;

			if (injectDLL) {
				Sleep(4000);

				fs::path dllPath = fs::path(exePath).parent_path() / dllName;
				if (!fs::exists(dllPath)) {
					lr.error = dllName + " not found next to the game exe.";
					lr.ok = false;
					return lr;
				}

				if (!InjectDLL(pi.dwProcessId, dllPath.string())) {
					lr.error = "DLL injection failed.";
					lr.ok = false;
					return lr;
				}
			}

			return lr;
		}
	}

	namespace FileUtil {
		bool copy(const std::string& src, const std::string& dst, std::string& err) {
			if (!::CopyFileA(src.c_str(), dst.c_str(), FALSE)) {
				err = "Failed to copy " + src + " to " + dst;
				return false;
			}
			return true;
		}

		bool move(const std::string& src, const std::string& dst, std::string& err) {
			if (!::MoveFileA(src.c_str(), dst.c_str())) {
				err = "Failed to move " + src + " to " + dst;
				return false;
			}
			return true;
		}

		bool move_replace(const std::string& src, const std::string& dst, std::string& err) {
			if (!::MoveFileExA(src.c_str(), dst.c_str(), MOVEFILE_REPLACE_EXISTING)) {
				err = "Failed to replace " + dst + " with " + src;
				return false;
			}
			return true;
		}

		void remove(const std::string& path) {
			::DeleteFileA(path.c_str());
		}
	}
}
