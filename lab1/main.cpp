#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include <Meter.hpp>
#include <json.hpp> 

#include <windows.h>
#include <d3dcompiler.h>
#include <d3d11.h>
#include <dxgi.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

using json = nlohmann::json;


//IMeter::Date parseDate(const std::string& dateString) {
//    IMeter::Date date;
//    std::stringstream ss(dateString);
//    std::string yearStr, monthStr, dayStr;
//
//    std::getline(ss, yearStr, '-');
//    std::getline(ss, monthStr, '-');
//    std::getline(ss, dayStr, '-');
//
//    date.year = std::stoi(yearStr);
//    date.month = std::stoi(monthStr);
//    date.day = std::stoi(dayStr);
//
//    return date;
//}
//
//int main() {
//    std::vector<IMeter*> meters;
//
//    std::ifstream file("meters.json");
//
//    if (!file.is_open()) {
//        return 1;
//    }
//
//    try {
//        json j;
//        file >> j;
//
//        for (const auto& element : j) {
//            std::string resourceType = element["resourceType"];
//            std::string dateStr = element["date"];
//            double value = element["value"];
//
//            IMeter::Date date = parseDate(dateStr);
//
//            if (resourceType == "Electricity") {
//                double volatge = element["voltage"];
//                double current = element["current"];
//                meters.push_back(new CElectricMeter(date, value, volatge, current));
//            } else if (resourceType == "Water") {
//                std::string waterTypeStr = element.value("type", "unknown");
//                CWaterMeter::WaterType waterType;
//                if (waterTypeStr == "cold") {
//                    waterType = CWaterMeter::WaterType::Cold;
//                } else if (waterTypeStr == "warm") {
//                    waterType = CWaterMeter::WaterType::Warm;
//                } else {
//                    waterType = CWaterMeter::WaterType::Unknown;
//                }
//                meters.push_back(new CWaterMeter(date, value, waterType));
//            }
//            else if (resourceType == "Gas") {
//                std::string gasType = element["gasType"];
//                double pressure = element["pressure"];
//                meters.push_back(new CGasMeter(date, value, gasType, pressure));
//            }
//            else {
//                std::cerr << "Unkown resource type: " << resourceType << std::endl;
//            }
//        }
//
//        for (IMeter* meter : meters) {
//            std::cout << meter->to_string() << std::endl;
//        }
//
//        for (IMeter* meter : meters) {
//            delete meter;
//        }
//        meters.clear();
//
//    } catch (const json::exception& e) {
//        std::cerr << "Error while parsing JSON: " << e.what() << std::endl;
//        return 1;
//    }
//
//    return 0;
//}

static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

IMeter::Date parseDate(const std::string& dateString) {
    IMeter::Date date;
    std::stringstream ss(dateString);
    std::string yearStr, monthStr, dayStr;

    std::getline(ss, yearStr, '-');
    std::getline(ss, monthStr, '-');
    std::getline(ss, dayStr, '-');

    date.year = std::stoi(yearStr);
    date.month = std::stoi(monthStr);
    date.day = std::stoi(dayStr);

    return date;
}

// Çàãðóçêà äàííûõ èç JSON
std::vector<IMeter*> loadMetersFromJSON() {
    std::vector<IMeter*> meters;

    std::ifstream file("meters.json");
    if (!file.is_open()) {
        std::cerr << "Íå óäàëîñü îòêðûòü ôàéë meters.json" << std::endl;
        return meters;
    }

    try {
        json j;
        file >> j;

        for (const auto& element : j) {
            std::string resourceType = element["resourceType"];
            std::string dateStr = element["date"];
            double value = element["value"];

            IMeter::Date date = parseDate(dateStr);

            if (resourceType == "Electricity") {
                double voltage = element["voltage"];
                double current = element["current"];
                meters.push_back(new CElectricMeter(date, value, voltage, current));
            }
            else if (resourceType == "Water") {
                std::string waterTypeStr = element.value("type", "unknown");
                CWaterMeter::WaterType waterType;
                if (waterTypeStr == "cold") {
                    waterType = CWaterMeter::WaterType::Cold;
                }
                else if (waterTypeStr == "warm") {
                    waterType = CWaterMeter::WaterType::Warm;
                }
                else {
                    waterType = CWaterMeter::WaterType::Unknown;
                }
                meters.push_back(new CWaterMeter(date, value, waterType));
            }
            else if (resourceType == "Gas") {
                std::string gasType = element["gasType"];
                double pressure = element["pressure"];
                meters.push_back(new CGasMeter(date, value, gasType, pressure));
            }
            else {
                std::cerr << "Íåèçâåñòíûé òèï ðåñóðñà: " << resourceType << std::endl;
            }
        }

    }
    catch (const json::exception& e) {
        std::cerr << "Îøèáêà ïðè ïàðñèíãå JSON: " << e.what() << std::endl;
    }

    return meters;
}

// Î÷èñòêà ïàìÿòè
void cleanupMeters(std::vector<IMeter*>& meters) {
    for (auto* meter : meters) {
        delete meter;
    }
    meters.clear();
}

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chains
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//-  Win32 message handler 
// - You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true,  do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// - Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}


//int main() {
//    std::vector<IMeter*> meters = loadMetersFromJSON();
//
//    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
//    ::RegisterClassExW(&wc);
//    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX11 Example", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);
//
//    // Initialize Direct3D
//    if (!CreateDeviceD3D(hwnd))
//    {
//        CleanupDeviceD3D();
//        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
//        return 1;
//    }
//
//    // Show the window
//    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
//    ::UpdateWindow(hwnd);
//
//    // Setup Dear ImGui context
//    IMGUI_CHECKVERSION();
//    ImGui::CreateContext();
//    ImGuiIO& io = ImGui::GetIO(); (void)io;
//    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
//    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
//
//    // Setup Dear ImGui style
//    ImGui::StyleColorsDark();
//    //ImGui::StyleColorsLight();
//
//    // Setup Platform/Renderer backends
//    ImGui_ImplWin32_Init(hwnd);
//    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
//
//    int itemToDelete = -1;
//
//    // Ãëàâíûé öèêë
//    MSG msg;
//    ZeroMemory(&msg, sizeof(msg));
//    while (msg.message != WM_QUIT) {
//        if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
//            TranslateMessage(&msg);
//            DispatchMessage(&msg);
//            continue;
//        }
//
//        ImGui_ImplDX11_NewFrame();
//        ImGui_ImplWin32_NewFrame();
//        ImGui::NewFrame();
//
//        ImGui::Begin("Lab2");
//
//        if (ImGui::BeginTabBar("MainTabBar")) {
//            if (ImGui::BeginTabItem("Table")) {
//                if (ImGui::BeginTable("Meters", 4)) {
//                    ImGui::TableSetupColumn("Type");
//                    ImGui::TableSetupColumn("Date");
//                    ImGui::TableSetupColumn("Value");
//                    ImGui::TableSetupColumn("Action");
//                    ImGui::TableHeadersRow();
//
//                    for (size_t i = 0; i < meters.size(); ++i) {
//                        ImGui::TableNextRow();
//                        ImGui::TableSetColumnIndex(0);
//                        ImGui::Text("%s", meters[i]->get_resource_type().c_str());
//                        //ImGui::Text("%s","type");
//
//                        ImGui::TableSetColumnIndex(1);
//                        IMeter::Date date = meters[i]->get_verification_date();
//                        ImGui::Text("%04d-%02d-%02d", date.year, date.month, date.day);
//                        //ImGui::Text("%04d-%02d-%02d", 2025, 10, 23);
//
//                        ImGui::TableSetColumnIndex(2);
//                        ImGui::Text("%.2f", meters[i]->getValue());
//                        //ImGui::Text("%.2f", 123.4586);
//
//                        ImGui::TableSetColumnIndex(3);
//                        if (ImGui::Button(("Delete##" + std::to_string(i)).c_str())) {
//                            itemToDelete = static_cast<int>(i);
//                        }
//                    }
//
//                    ImGui::EndTable();
//                }
//                ImGui::EndTabItem();
//            }
//            ImGui::EndTabBar();
//        }
//
//        ImGui::End();
//
//        // Îáðàáîòêà óäàëåíèÿ
//        if (itemToDelete != -1 && itemToDelete < meters.size()) {
//            delete meters[itemToDelete];
//            meters.erase(meters.begin() + itemToDelete);
//            itemToDelete = -1;
//        }
//
//        ImGui::Render();
//
//        const float clear_color_with_alpha[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
//        ID3D11DeviceContext* ctx = g_pd3dDeviceContext;
//        ID3D11RenderTargetView* rtv = g_mainRenderTargetView;
//        ctx->OMSetRenderTargets(1, &rtv, nullptr);
//        ctx->ClearRenderTargetView(rtv, clear_color_with_alpha);
//        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
//
//        g_pSwapChain->Present(1, 0); // Present with vsync
//    }
//
//    // Î÷èñòêà
//    ImGui_ImplDX11_Shutdown();
//    ImGui_ImplWin32_Shutdown();
//    ImGui::DestroyContext();
//
//    cleanupMeters(meters);
//    CleanupDeviceD3D();
//    DestroyWindow(hwnd);
//    UnregisterClassW(wc.lpszClassName, wc.hInstance);
//
//    return 0;
//}

struct Date {
    int year;
    int month;
    int day;
};

// Ïàðñèíã äàòû
Date parseDate1(const std::string& dateString) {
    Date date;
    std::stringstream ss(dateString);
    std::string yearStr, monthStr, dayStr;

    std::getline(ss, yearStr, '-');
    std::getline(ss, monthStr, '-');
    std::getline(ss, dayStr, '-');

    date.year = std::stoi(yearStr);
    date.month = std::stoi(monthStr);
    date.day = std::stoi(dayStr);

    return date;
}

bool parseDateFromString(const std::string& str, Date& outDate) {
    std::stringstream ss(str);
    std::string yearStr, monthStr, dayStr;

    if (!std::getline(ss, yearStr, '-') ||
        !std::getline(ss, monthStr, '-') ||
        !std::getline(ss, dayStr, '-')) {
        return false;
    }

    try {
        outDate.year = std::stoi(yearStr);
        outDate.month = std::stoi(monthStr);
        outDate.day = std::stoi(dayStr);
    }
    catch (...) {
        return false;
    }

    return true;
}

int main() {
    std::vector<IMeter*> meters = loadMetersFromJSON();

    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX11 Example", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    int itemToDelete = -1;

    // Ïåðåìåííûå äëÿ ôîðìû äîáàâëåíèÿ
    bool show_add_window = false;
    std::string input_date_str = "2023-10-01";
    double new_value = 0.0;
    std::string new_gas_type = "Co2";
    double new_pressure = 0.0;
    double new_voltage = 5.0;
    double new_current = 2.0;
    int selected_type = 0; // 0: Water, 1: Electricity, 2: Gas
    int water_type_index = 0; // 0: Cold, 1: Warm
    std::string input_error;

    // Ãëàâíûé öèêë
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Meterss");

        if (ImGui::BeginTabBar("MainTabBar")) {
            if (ImGui::BeginTabItem("Meters list")) {
                if (ImGui::BeginTable("Meters", 4)) {
                    ImGui::TableSetupColumn("Type");
                    ImGui::TableSetupColumn("Date");
                    ImGui::TableSetupColumn("Value");
                    ImGui::TableSetupColumn("Action");
                    ImGui::TableHeadersRow();

                    for (size_t i = 0; i < meters.size(); ++i) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%s", meters[i]->get_resource_type().c_str());

                        ImGui::TableSetColumnIndex(1);
                        IMeter::Date date = meters[i]->get_verification_date();
                        ImGui::Text("%04d-%02d-%02d", date.year, date.month, date.day);

                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("%.2f", meters[i]->get_value());

                        ImGui::TableSetColumnIndex(3);
                        if (ImGui::Button(("Delete##" + std::to_string(i)).c_str())) {
                            itemToDelete = static_cast<int>(i);
                        }
                    }

                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();

            if (ImGui::Button("Add")) {
                show_add_window = true;
            }
        }

        ImGui::End();

        // Îáðàáîòêà óäàëåíèÿ
        if (itemToDelete != -1 && itemToDelete < meters.size()) {
            delete meters[itemToDelete];
            meters.erase(meters.begin() + itemToDelete);
            itemToDelete = -1;
        }

        // Ìîäàëüíîå îêíî äîáàâëåíèÿ
        if (show_add_window) {
            ImGui::OpenPopup("Add object");
        }

        if (ImGui::BeginPopupModal("Add object", &show_add_window)) {
            const char* types[] = { "Water", "Electricity", "Gas" };
            ImGui::Combo("Type", &selected_type, types, IM_ARRAYSIZE(types));

            ImGui::InputText("Date (YYYY-MM-DD)", input_date_str.data(), 20);
            ImGui::InputDouble("Value", &new_value);

            if (selected_type == 0) {
                const char* water_types[] = { "Cold", "Warm" };
                ImGui::Combo("Water type", &water_type_index, water_types, IM_ARRAYSIZE(water_types));
            }
            else if (selected_type == 1) {
                ImGui::InputDouble("Voltage", &new_voltage);
                ImGui::InputDouble("I", &new_current);
            }
            else if (selected_type == 2) {
                ImGui::InputText("Gas type", new_gas_type.data(), 20);
                ImGui::InputDouble("Pressure", &new_pressure);
            }

            if (ImGui::Button("Add")) {
                Date parsedDate;
                if (!parseDateFromString(input_date_str, parsedDate)) {
                    input_error = "Îøèáêà: íåâåðíûé ôîðìàò äàòû";
                }
                else {
                    input_error.clear();

                    IMeter* newMeter = nullptr;
                    if (selected_type == 0) {
                        CWaterMeter::WaterType wt = (water_type_index == 0)
                            ? CWaterMeter::WaterType::Cold
                            : CWaterMeter::WaterType::Warm;
                        newMeter = new CWaterMeter({ parsedDate.year, parsedDate.month, parsedDate.day }, new_value, wt);
                    }
                    else if (selected_type == 1) {
                        newMeter = new CElectricMeter({ parsedDate.year, parsedDate.month, parsedDate.day }, new_value, new_voltage, new_current);
                    }
                    else if (selected_type == 2) {
                        newMeter = new CGasMeter({ parsedDate.year, parsedDate.month, parsedDate.day }, new_value, new_gas_type, new_pressure);
                    }

                    if (newMeter) {
                        meters.push_back(newMeter);
                        show_add_window = false;

                        // Ñáðîñ ïîëåé
                        input_date_str = "2023-10-01";
                        new_value = 0.0;
                        new_gas_type = "Co2";
                        new_pressure = 0.0;
                        new_voltage = 5.0;
                        new_current = 2.0;
                        water_type_index = 0;
                    }
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                show_add_window = false;
                input_error.clear();
            }

            if (!input_error.empty()) {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", input_error.c_str());
            }

            ImGui::EndPopup();
        }

        ImGui::Render();

        const float clear_color_with_alpha[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
        ID3D11DeviceContext* ctx = g_pd3dDeviceContext;
        ID3D11RenderTargetView* rtv = g_mainRenderTargetView;
        ctx->OMSetRenderTargets(1, &rtv, nullptr);
        ctx->ClearRenderTargetView(rtv, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
    }

    // Î÷èñòêà
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    cleanupMeters(meters);
    CleanupDeviceD3D();
    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}
