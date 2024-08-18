#include "includes.h"
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Present oPresent;
HWND window = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11RenderTargetView* mainRenderTargetView;


bool freezeValue = false;
int* targetAddress = (int*)0x0D34DAE4;
int originalValue = 120;  
bool showmenu;
bool drawWatermark = true;
int fov = 90;
float aspectRatio = 16.0f / 9.0f;  
float nearPlane = 0.1f;            
float farPlane = 1000.0f;
ID3D11DeviceContext* pContext = nullptr;
ID3D11Buffer* pConstantBuffer = nullptr;
using namespace DirectX;

void DrawWatermark() {
	if (!drawWatermark)
		return;
	
	float current_fps = ImGui::GetIO().Framerate;

	
	time_t now = time(NULL);
	struct tm timeinfo;
	localtime_s(&timeinfo, &now);
	char time_str[10];
	strftime(time_str, sizeof(time_str), "%H:%M:%S", &timeinfo);

	
	char watermark_text[256];
	sprintf_s(watermark_text, sizeof(watermark_text), "pivoware [beta] | FPS: %.1f | %s", current_fps, time_str); 

	
	ImVec4 bg_color = ImVec4(0.1f, 0.1f, 0.1f, 0.6f);
	ImVec4 line_color = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
	ImVec4 text_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

	
	ImVec2 text_size = ImGui::CalcTextSize(watermark_text);
	ImVec2 window_size = ImVec2(text_size.x + 20, text_size.y + 20);

	
	ImGui::SetNextWindowSize(window_size);
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground;

	
	ImGui::Begin("##Watermark", nullptr, window_flags);

	
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 p = ImGui::GetWindowPos();
	draw_list->AddRectFilled(p, ImVec2(p.x + window_size.x, p.y + window_size.y), ImGui::GetColorU32(bg_color), 8.0f);

	
	float line_height = 4.0f;
	draw_list->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + window_size.x, p.y + line_height), ImGui::GetColorU32(line_color));

	
	ImGui::SetCursorPos(ImVec2(10, (window_size.y - text_size.y) * 0.5f));
	ImGui::TextColored(text_color, watermark_text);

	
	ImGui::End();
}



void InitImGui()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(pDevice, pContext);
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	

	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

bool init = false;

HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (!init)
	{
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)& pDevice)))
		{

			pDevice->GetImmediateContext(&pContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			window = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)& pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
			pBackBuffer->Release();
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
			D3D11_BUFFER_DESC cbDesc; 
			ZeroMemory(&cbDesc, sizeof(cbDesc)); 
			cbDesc.Usage = D3D11_USAGE_DYNAMIC; 
			cbDesc.ByteWidth = sizeof(XMMATRIX); 
			cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; 
			cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			pDevice->CreateBuffer(&cbDesc, nullptr, &pConstantBuffer);
			
			InitImGui();
			init = true; 
		}

		else
			return oPresent(pSwapChain, SyncInterval, Flags);
	}
	
	if (GetAsyncKeyState(VK_INSERT) & 1) {
		showmenu = !showmenu;
	}
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::RenderToasts();
	if (!showmenu) {
		ImGui::Begin("ImGui Window");
		ImGui::Checkbox("Watermark", &drawWatermark);
		if (ImGui::Checkbox("Unlock FPS", &freezeValue)) {
			if (freezeValue) {
				//originalValue = *targetAddress;
				//*targetAddress = 999;  
				ImGui::AddToast(ImGui::ToastType::Info, 3000, "eta hueta ne rabotaet");
			}
			else {
				
			}
		}
		if (ImGui::Button("Notify")) {
			ImGui::AddToast(ImGui::ToastType::Info, 3000, "hui pizda123");
		}

		if (ImGui::SliderInt("Fov", &fov, 1, 180)) {
			// Преобразуем FOV из градусов в радианы
			float fovRadians = XMConvertToRadians(static_cast<float>(fov));

			// Создаем новую матрицу проекции с заданным FOV
			XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(fovRadians, aspectRatio, nearPlane, farPlane);

			// Обновляем константный буфер с новой матрицей проекции
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			HRESULT hr = pContext->Map(pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			if (SUCCEEDED(hr)) {
				memcpy(mappedResource.pData, &projectionMatrix, sizeof(projectionMatrix));
				pContext->Unmap(pConstantBuffer, 0);
			}

			// Устанавливаем буфер в вершинный шейдер (на слот 0)
			pContext->VSSetConstantBuffers(0, 1, &pConstantBuffer);
		}

		ImGui::End();

	}
	if (freezeValue) {
		*targetAddress = 999;
	}

	if (drawWatermark) {
		DrawWatermark();
	}
	ImGui::Render();
	
	pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	return oPresent(pSwapChain, SyncInterval, Flags);
}

DWORD WINAPI MainThread(LPVOID lpReserved)
{
	bool init_hook = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
		{
			kiero::bind(8, (void**)& oPresent, hkPresent);
			init_hook = true;
		}
	} while (!init_hook);
	return TRUE;
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hMod);
		CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
		break;
	case DLL_PROCESS_DETACH:
		kiero::shutdown();
		break;
	}
	return TRUE;
}