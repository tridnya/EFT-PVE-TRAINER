#pragma once

#include <memory>
#include <string>
#include <Windows.h>

class TarkovMapWebView
{
public:
    TarkovMapWebView() = default;
    ~TarkovMapWebView();

    TarkovMapWebView(const TarkovMapWebView&) = delete;
    TarkovMapWebView& operator=(const TarkovMapWebView&) = delete;

    void show(HWND parent_window, const RECT& bounds, const std::string& remote_id, const std::string& map_key);
    void hide();
    void refresh();
    void shutdown();

    [[nodiscard]] bool is_visible() const;
    [[nodiscard]] std::string get_status() const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl{};
};
