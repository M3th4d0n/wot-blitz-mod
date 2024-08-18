#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3d9.h>
#include <tchar.h>
#include <string>
#include "kiero/kiero.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include "ctime"
#include <DirectXMath.h>
#include <chrono>
#include "imgui/ImGuiNotify.hpp"

typedef HRESULT(__stdcall* Present) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef uintptr_t PTR;