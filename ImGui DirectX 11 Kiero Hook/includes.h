#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3d9.h>
#include <tchar.h>
#include <string>
#include <ctime>
#include <DirectXMath.h>
#include <chrono>
#include "kiero/kiero.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_internal.h"
#include <fstream>
#include <ctime>
#include <iomanip>
#include <psapi.h>
#include <sstream>
#include <shellapi.h>
#include <mutex>
#include "json.hpp"
#include "MinHook.h"
#include <windows.h>
#include <iostream>
#include "detours.h"
#include <wininet.h>
#include "scriptsystem.h"




using json = nlohmann::json;
typedef HRESULT(__stdcall* Present) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef uintptr_t PTR;
typedef BOOL(WINAPI* HttpSendRequestW_t)(HINTERNET, LPCWSTR, DWORD, LPVOID, DWORD);
HttpSendRequestW_t originalHttpSendRequestW;