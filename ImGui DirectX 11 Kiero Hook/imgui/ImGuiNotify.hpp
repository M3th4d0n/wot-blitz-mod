#ifndef IMGUI_NOTIFY_HPP
#define IMGUI_NOTIFY_HPP

#include "imgui.h"
#include <vector>
#include <string>
#include <chrono>
#include <functional>

namespace ImGui
{
    enum class ToastType
    {
        None,
        Success,
        Warning,
        Error,
        Info
    };

    struct Toast
    {
        ToastType type;
        std::string message;
        int duration_ms;
        std::chrono::steady_clock::time_point start_time;

        Toast(ToastType t, int d, const std::string& msg)
            : type(t), message(msg), duration_ms(d), start_time(std::chrono::steady_clock::now()) {}

        bool isExpired() const
        {
            return std::chrono::steady_clock::now() - start_time > std::chrono::milliseconds(duration_ms);
        }
    };

    static std::vector<Toast> notifications;

    inline ImVec4 GetToastColor(ToastType type)
    {
        switch (type)
        {
        case ToastType::Success: return ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green
        case ToastType::Warning: return ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
        case ToastType::Error: return ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red
        case ToastType::Info: return ImVec4(0.0f, 0.0f, 1.0f, 1.0f); // Blue
        default: return ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White
        }
    }

    inline void AddToast(ToastType type, int duration_ms, const std::string& message)
    {
        notifications.emplace_back(type, duration_ms, message);
    }

    inline void RenderToasts()
    {
        const float padding = 10.0f;
        const float toastHeight = 50.0f;
        const float toastWidth = 300.0f;
        const float fadeDuration = 0.5f; // Duration for fade-in/out

        ImVec2 screenSize = ImGui::GetIO().DisplaySize;
        float yOffset = padding;

        for (auto it = notifications.begin(); it != notifications.end(); )
        {
            if (it->isExpired())
            {
                it = notifications.erase(it);
                continue;
            }

            float elapsedTime = std::chrono::duration<float>(std::chrono::steady_clock::now() - it->start_time).count();
            float alpha = std::min(1.0f, elapsedTime / fadeDuration);
            float fadeOutAlpha = std::max(0.0f, 1.0f - ((elapsedTime - (it->duration_ms / 1000.0f)) / fadeDuration));
            float finalAlpha = std::min(alpha, fadeOutAlpha);

            ImGui::SetNextWindowPos(ImVec2(screenSize.x - toastWidth - padding, screenSize.y - yOffset - toastHeight - padding), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(toastWidth, toastHeight), ImGuiCond_Always);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, finalAlpha * 0.8f)); // Semi-transparent background
            ImGui::PushStyleColor(ImGuiCol_Text, GetToastColor(it->type));

            ImGui::Begin("Toast", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("%s", it->message.c_str());
            ImGui::End();

            ImGui::PopStyleColor(2);

            yOffset += toastHeight + padding;

            ++it;
        }
    }
}

#endif // IMGUI_NOTIFY_HPP
