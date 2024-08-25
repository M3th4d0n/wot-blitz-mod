#include "includes.h"
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

using namespace DirectX;

Present oPresent;
HWND window = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = nullptr;
ID3D11RenderTargetView* mainRenderTargetView = nullptr;

bool freezeValue = false;
int* targetAddress = (int*)0x0D34DAE4;
int originalValue = 120;
bool showmenu = true;
bool drawWatermark = true;
bool fpsunlock = false;
bool developermode = false;
bool infowindow = false;
std::ofstream logFile("log.txt");
enum Tab {
    TAB_SETTINGS,
    TAB_ACTIONS,
    TAB_NONE,
    TAB_WINDOWS
};
uintptr_t GetBaseAddress() {
    return reinterpret_cast<uintptr_t>(GetModuleHandle(NULL));
}
int ReadMemoryInt(uintptr_t address) {
    return *reinterpret_cast<int*>(address);
}

Tab currentTab = TAB_SETTINGS;
std::string ReadFileContent(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return "Unable to open file";
    }

    std::ostringstream contentStream;
    contentStream << file.rdbuf();
    return contentStream.str();
}

ImVec4 GetTextColor(const std::string& line) {
    if (line.find("success") != std::string::npos) {
        return ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    }
    else if (line.find("error") != std::string::npos) {
        return ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    }
    else if (line.find("info") != std::string::npos) {
        return ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
    }
    else {
        return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
}
void RenderLogWindow() {
    static std::string logFilePath = "log.txt";
    static std::string logContent = "";
    static std::time_t lastModificationTime = 0;

    
    struct stat fileInfo;
    if (stat(logFilePath.c_str(), &fileInfo) == 0) {
        if (fileInfo.st_mtime != lastModificationTime) {
            lastModificationTime = fileInfo.st_mtime;
            logContent = ReadFileContent(logFilePath);
        }
    }

    ImGui::SetNextWindowSize(ImVec2(800, 600));
    ImGui::Begin("Logs", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    std::istringstream logStream(logContent);
    std::string line;
    while (std::getline(logStream, line)) {
        ImVec4 color = GetTextColor(line);
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextWrapped("%s", line.c_str());
        ImGui::PopStyleColor();
    }

    ImGui::End();
}
void RenderInfoWindow() {
    ImGui::SetNextWindowSize(ImVec2(300, 150));
    ImGui::Begin("Info", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    uintptr_t fpsoffset = 0x3947308;
    uintptr_t fpsaddress = GetBaseAddress() + fpsoffset;
    int fpsValue = ReadMemoryInt(fpsaddress);
    ImGui::Text("base addres: 0x%d ", GetBaseAddress()); //idk 
    ImGui::Text("fps addres: 0x%d ", fpsaddress); //idk 
    ImGui::Text("fps limit: %d", fpsValue);
    ImGui::End();
}
void Log(const std::string& level, const std::string& message) {
    std::time_t now = std::time(nullptr);
    std::tm timeinfo;
    localtime_s(&timeinfo, &now);

    logFile << "[" << std::put_time(&timeinfo, "%H:%M:%S") << "] "
        << level << ": " << message << std::endl;
    logFile.flush();
}
struct SearchResult {
    //signs
    std::string name;
    uintptr_t address;
    std::string signature;
    std::string value;
    std::string offset;
};

void ApplyCustomStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.21f, 0.22f, 0.54f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.40f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.28f, 0.28f, 0.28f, 0.67f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.61f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.08f, 0.50f, 0.72f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.35f, 0.58f, 0.86f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.41f, 0.68f, 1.00f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    style.WindowPadding = ImVec2(10, 10);
    style.WindowRounding = 5.0f;
    style.FramePadding = ImVec2(5, 5);
    style.FrameRounding = 4.0f;
    style.ItemSpacing = ImVec2(12, 8);
    style.ItemInnerSpacing = ImVec2(8, 6);
    style.IndentSpacing = 25.0f;
    style.ScrollbarSize = 15.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabMinSize = 5.0f;
    style.GrabRounding = 3.0f;
}

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
    sprintf_s(watermark_text, sizeof(watermark_text), "pivoware [beta] | FPS: %.1f | %s",
        current_fps, time_str);

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

void InitImGui() {
    Log("info", "Initializing ImGui context");
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;

    if (!ImGui_ImplWin32_Init(window)) {
        Log("error", "Failed to initialize ImGui_ImplWin32");
    }
    else {
        Log("success", "ImGui_ImplWin32 initialized");
    }

    if (!ImGui_ImplDX11_Init(pDevice, pContext)) {
        Log("error", "Failed to initialize ImGui_ImplDX11");
    }
    else {
        Log("success", "ImGui_ImplDX11 initialized");
    }

    ApplyCustomStyle();
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (showmenu && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) {
        return false;
        Log("info", "wndproc");
    }
    
    
    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}


HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
    if (!pDevice || !pContext) {
        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice))) {
            pDevice->GetImmediateContext(&pContext);
            DXGI_SWAP_CHAIN_DESC sd;
            pSwapChain->GetDesc(&sd);
            window = sd.OutputWindow;
            ID3D11Texture2D* pBackBuffer;
            pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
            pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
            pBackBuffer->Release();
            oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
            InitImGui();
            Log("info", "Initialization complete");
        }
    }

    if (GetAsyncKeyState(VK_INSERT) & 1) {
        showmenu = !showmenu;
        if (!showmenu) {
            developermode = false;
        }
    }
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (showmenu) {
        ImGui::SetNextWindowSize(ImVec2(690, 450));
        ImGui::Begin("ImGui Menu", &showmenu, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

        ImGui::Columns(2, NULL, false);

        ImGui::SetColumnWidth(0, 150);
        ImGui::Text("Sidebar");
        ImGui::Separator();

        if (ImGui::Button("Settings", ImVec2(-1, 0))) {
            currentTab = TAB_SETTINGS;
        }
        if (ImGui::Button("Actions", ImVec2(-1, 0))) {
            currentTab = TAB_ACTIONS;
        }
        if (ImGui::Button("Windows", ImVec2(-1, 0))) {
            currentTab = TAB_WINDOWS;
        }

        ImGui::NextColumn();

        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::GetWindowDrawList()->AddLine(
            ImVec2(pos.x - ImGui::GetStyle().ItemSpacing.x / 2, pos.y - ImGui::GetWindowSize().y),
            ImVec2(pos.x - ImGui::GetStyle().ItemSpacing.x / 2, pos.y + ImGui::GetWindowSize().y),
            ImGui::GetColorU32(ImGuiCol_Separator)
        );

        if (currentTab == TAB_SETTINGS) {
            ImGui::Text("Settings");
            ImGui::Checkbox("Show Watermark", &drawWatermark);
            ImGui::Checkbox("Unlock Fps", &fpsunlock);

            if (fpsunlock) {
                uintptr_t offset = 0x3947308;
                uintptr_t address = GetBaseAddress() + offset;
                int newValue = 1337;
                *reinterpret_cast<int*>(address) = newValue;
            }
            else {
                uintptr_t offset = 0x3947308;
                uintptr_t address = GetBaseAddress() + offset;
                int newValue = 60;
                *reinterpret_cast<int*>(address) = newValue;
            }

        }
        else if (currentTab == TAB_ACTIONS) {
            ImGui::Text("Actions");
            if (ImGui::Button("Open Log File")) {
                ShellExecute(0, "open", "log.txt", 0, 0, SW_SHOW);
            }
            if (ImGui::Button("Write Log")) {
                Log("error", "Button Action");
            }
            if (ImGui::Button("Reload styles ")) {
                ApplyCustomStyle();
            }
        }
        else if (currentTab == TAB_WINDOWS) {
            ImGui::Checkbox("Developer Mode", &developermode);
            ImGui::Checkbox("Info Window", &infowindow);
        }

        ImGui::Columns(1);
        ImGui::End();
    }
    if (drawWatermark) {
        DrawWatermark();
    }
    if (developermode) {
        RenderLogWindow();
    }
    if (infowindow) {
        RenderInfoWindow();
    }

    ImGui::Render();
    pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return oPresent(pSwapChain, SyncInterval, Flags);
}

DWORD WINAPI MainThread(LPVOID lpReserved) {
    Log("info", "MainThread started");

    bool init_hook = false;

    for (int attempt = 0; attempt < 5; ++attempt) {
        Log("info", "Attempt to initialize Kiero... ");
        if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success) {
            Log("success", "Kiero initialized successfully");

            if (kiero::bind(8, (void**)&oPresent, hkPresent) == kiero::Status::Success) {
                init_hook = true;
                Log("success", "Hooked present function successfully");
                break;
            }
            else {
                Log("error", "Failed to hook present function");
            }
        }
        else {
            Log("warning", "Kiero initialization failed, retrying...");
        }
    }

    if (!init_hook) {
        Log("error", "Failed to initialize Kiero after multiple attempts");
    }
    
    return TRUE;
}


BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved) {
    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        Log("info", "DLL attached to process");
        DisableThreadLibraryCalls(hMod);
        CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
        break;
    case DLL_PROCESS_DETACH:
        Log("info", "DLL detached from process");
        kiero::shutdown();
        if (mainRenderTargetView) mainRenderTargetView->Release();
        logFile.close();
        break;
    }

    return TRUE;
}
