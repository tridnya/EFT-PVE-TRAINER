#include "tarkov_map_webview.h"

#include <atomic>
#include <filesystem>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <wrl.h>
#include <WebView2.h>

#include "../../util/logger.h"

using Microsoft::WRL::Callback;
using Microsoft::WRL::ComPtr;

struct TarkovMapWebView::Impl : public std::enable_shared_from_this<TarkovMapWebView::Impl>
{
    struct ViewRequest
    {
        HWND parent_window = nullptr;
        RECT bounds{};
        std::string url{};
        bool visible = false;
    };

    static constexpr UINT apply_message = WM_APP + 0x641;
    static constexpr UINT close_message = WM_APP + 0x642;

    std::atomic_bool running = false;
    std::atomic_bool apply_queued = false;
    std::atomic_bool visible_cache = false;
    std::atomic_bool force_refresh = false;
    std::atomic<DWORD> worker_thread_id = 0;
    std::thread worker_thread{};

    mutable std::mutex request_mutex{};
    ViewRequest request{};

    mutable std::mutex status_mutex{};
    std::string status = "idle";

    HWND host_window = nullptr;
    HWND host_parent_window = nullptr;
    RECT host_bounds{};
    RECT webview_bounds{};
    bool creating = false;
    bool com_initialized = false;
    bool owns_com = false;
    std::string current_url{};
    std::string pending_url{};

    ComPtr<ICoreWebView2Environment> environment{};
    ComPtr<ICoreWebView2Controller> controller{};
    ComPtr<ICoreWebView2> webview{};
    EventRegistrationToken navigation_completed_token{};

    ~Impl()
    {
        shutdown();
    }

    void set_status(const std::string& next_status)
    {
        std::lock_guard lock(status_mutex);
        status = next_status;
    }

    [[nodiscard]] std::string get_status() const
    {
        std::lock_guard lock(status_mutex);
        return status;
    }

    [[nodiscard]] static const wchar_t* host_class_name()
    {
        return L"VeilTarkovMapWebViewHost";
    }

    [[nodiscard]] static std::wstring widen(const std::string& value)
    {
        if (value.empty())
        {
            return {};
        }

        const int size = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, nullptr, 0);
        if (size <= 0)
        {
            return {};
        }

        std::wstring result(static_cast<std::size_t>(size - 1), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, result.data(), size);
        return result;
    }

    [[nodiscard]] static std::string make_url(const std::string& remote_id, const std::string& map_key)
    {
        const std::string safe_map = map_key.empty() ? "customs" : map_key;
        std::ostringstream url{};
        url << "https://tarkov.dev/map/" << safe_map << "?connection=" << remote_id;
        return url.str();
    }

    [[nodiscard]] static std::wstring user_data_folder()
    {
        wchar_t temp_path[MAX_PATH]{};
        const DWORD length = GetTempPathW(MAX_PATH, temp_path);
        std::filesystem::path folder = length > 0
            ? std::filesystem::path(temp_path)
            : std::filesystem::temp_directory_path();
        folder /= L"veil-webview2";
        std::error_code error{};
        std::filesystem::create_directories(folder, error);
        return folder.wstring();
    }

    [[nodiscard]] static std::wstring map_only_script()
    {
        return LR"JS(
(() => {
    const styleId = 'veil-map-only-style';
    function installStyle() {
        if (document.getElementById(styleId)) {
            return;
        }
        const style = document.createElement('style');
        style.id = styleId;
        style.textContent = `
            html, body, #root {
                width: 100vw !important;
                height: 100vh !important;
                min-height: 100vh !important;
                margin: 0 !important;
                padding: 0 !important;
                overflow: hidden !important;
                background: #050608 !important;
            }
            header, nav, footer, .CookieConsent, .cookie-consent, .site-header,
            .site-footer, .menu-wrapper, .navbar, .leaflet-control-remote,
            .leaflet-control-attribution, .remote-control-wrapper,
            [class*="Header"], [class*="Footer"] {
                display: none !important;
                visibility: hidden !important;
                pointer-events: none !important;
            }
            .display-wrapper, .map-page, .page-wrapper, .map-wrapper,
            #leaflet-map, .leaflet-container {
                position: fixed !important;
                inset: 0 !important;
                width: 100vw !important;
                height: 100vh !important;
                max-width: none !important;
                max-height: none !important;
                min-width: 100vw !important;
                min-height: 100vh !important;
                margin: 0 !important;
                padding: 0 !important;
                border: 0 !important;
            }
            .leaflet-top.leaflet-left,
            .leaflet-top.leaflet-right,
            .leaflet-bottom.leaflet-left,
            .leaflet-bottom.leaflet-right {
                opacity: 0 !important;
                pointer-events: none !important;
            }
        `;
        document.documentElement.appendChild(style);
    }

    function cleanup() {
        installStyle();
        for (const selector of ['header', 'nav', 'footer', '.CookieConsent', '.leaflet-control-remote']) {
            document.querySelectorAll(selector).forEach((node) => node.remove());
        }
        const map = document.getElementById('leaflet-map') || document.querySelector('.leaflet-container');
        if (map) {
            let node = map;
            while (node && node !== document.body && node !== document.documentElement) {
                node.style.position = 'fixed';
                node.style.inset = '0';
                node.style.width = '100vw';
                node.style.height = '100vh';
                node.style.maxWidth = 'none';
                node.style.maxHeight = 'none';
                node.style.margin = '0';
                node.style.padding = '0';
                node = node.parentElement;
            }
        }
        window.dispatchEvent(new Event('resize'));
    }

    cleanup();
    setInterval(cleanup, 1000);
})();
)JS";
    }

    static LRESULT CALLBACK host_window_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
    {
        switch (message)
        {
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;
        case WM_NCHITTEST:
            return HTTRANSPARENT;
        case WM_SETFOCUS:
        {
            HWND parent_window = GetParent(window);
            if (parent_window)
            {
                SetFocus(parent_window);
            }
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
        default:
            return DefWindowProcW(window, message, w_param, l_param);
        }
    }

    static BOOL CALLBACK disable_child_window_proc(HWND window, LPARAM parameter)
    {
        (void)parameter;
        EnableWindow(window, FALSE);
        const LONG_PTR style = GetWindowLongPtrW(window, GWL_EXSTYLE);
        SetWindowLongPtrW(window, GWL_EXSTYLE, style | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT);
        return TRUE;
    }

    bool register_host_class()
    {
        WNDCLASSEXW window_class{};
        window_class.cbSize = sizeof(window_class);
        window_class.style = CS_HREDRAW | CS_VREDRAW;
        window_class.lpfnWndProc = host_window_proc;
        window_class.hInstance = GetModuleHandleW(nullptr);
        window_class.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        window_class.lpszClassName = host_class_name();

        if (RegisterClassExW(&window_class))
        {
            return true;
        }

        return GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
    }

    [[nodiscard]] bool ensure_com()
    {
        if (com_initialized)
        {
            return true;
        }

        const HRESULT result = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        if (SUCCEEDED(result))
        {
            com_initialized = true;
            owns_com = result == S_OK || result == S_FALSE;
            return true;
        }

        set_status("COM init failed");
        return false;
    }

    void start_worker()
    {
        bool expected = false;
        if (!running.compare_exchange_strong(expected, true))
        {
            return;
        }

        set_status("starting webview");
        worker_thread = std::thread([self = shared_from_this()] { self->worker_loop(); });
    }

    void queue_apply()
    {
        const DWORD thread_id = worker_thread_id.load();
        if (thread_id == 0)
        {
            return;
        }

        if (apply_queued.exchange(true))
        {
            return;
        }

        if (!PostThreadMessageW(thread_id, apply_message, 0, 0))
        {
            apply_queued.store(false);
        }
    }

    void show(HWND next_parent_window, const RECT& next_bounds, const std::string& remote_id, const std::string& map_key)
    {
        ViewRequest next_request{};
        next_request.parent_window = next_parent_window;
        next_request.bounds = next_bounds;
        next_request.url = make_url(remote_id, map_key);
        next_request.visible = next_parent_window != nullptr;

        bool changed = false;
        {
            std::lock_guard lock(request_mutex);
            changed = !same_request(request, next_request);
            request = next_request;
            visible_cache.store(request.visible);
        }

        start_worker();
        if (changed)
        {
            queue_apply();
        }
    }

    void hide()
    {
        bool changed = false;
        {
            std::lock_guard lock(request_mutex);
            changed = request.visible;
            request.visible = false;
            visible_cache.store(false);
        }

        if (changed)
        {
            queue_apply();
        }
    }

    void refresh()
    {
        force_refresh.store(true);
        queue_apply();
    }

    void shutdown()
    {
        hide();
        running.store(false);

        const DWORD thread_id = worker_thread_id.load();
        if (thread_id != 0)
        {
            PostThreadMessageW(thread_id, close_message, 0, 0);
        }

        if (worker_thread.joinable())
        {
            if (GetCurrentThreadId() == thread_id)
            {
                worker_thread.detach();
            }
            else
            {
                worker_thread.join();
            }
        }

        worker_thread_id.store(0);
        apply_queued.store(false);
        visible_cache.store(false);
        force_refresh.store(false);
        set_status("idle");
    }

    [[nodiscard]] ViewRequest get_request_copy()
    {
        std::lock_guard lock(request_mutex);
        return request;
    }

    void worker_loop()
    {
        MSG message{};
        PeekMessageW(&message, nullptr, WM_USER, WM_USER, PM_NOREMOVE);
        worker_thread_id.store(GetCurrentThreadId());

        if (!ensure_com())
        {
            running.store(false);
        }

        apply_queued.store(false);
        apply_request();

        while (running.load() && GetMessageW(&message, nullptr, 0, 0) > 0)
        {
            if (message.message == apply_message)
            {
                apply_queued.store(false);
                apply_request();
                continue;
            }

            if (message.message == close_message)
            {
                break;
            }

            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        destroy_webview();
        if (owns_com)
        {
            CoUninitialize();
        }

        owns_com = false;
        com_initialized = false;
        worker_thread_id.store(0);
        apply_queued.store(false);
        running.store(false);
    }

    [[nodiscard]] bool ensure_host_window(HWND parent_window)
    {
        if (!parent_window || !IsWindow(parent_window))
        {
            set_status("game window missing");
            return false;
        }

        if (host_window && host_parent_window == parent_window)
        {
            return true;
        }

        destroy_webview();

        if (!register_host_class())
        {
            set_status("webview host class failed");
            return false;
        }

        host_parent_window = parent_window;
        host_window = CreateWindowExW(
            WS_EX_NOACTIVATE | WS_EX_TRANSPARENT,
            host_class_name(),
            L"",
            WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            0,
            0,
            1,
            1,
            parent_window,
            nullptr,
            GetModuleHandleW(nullptr),
            nullptr);

        if (!host_window)
        {
            host_parent_window = nullptr;
            set_status("webview host failed");
            logger::warn("[Map] WebView2 host window failed");
            return false;
        }

        ShowWindow(host_window, SW_HIDE);
        return true;
    }

    [[nodiscard]] static bool has_area(const RECT& bounds)
    {
        return bounds.right > bounds.left && bounds.bottom > bounds.top;
    }

    [[nodiscard]] static bool same_rect(const RECT& left, const RECT& right)
    {
        return left.left == right.left &&
            left.top == right.top &&
            left.right == right.right &&
            left.bottom == right.bottom;
    }

    [[nodiscard]] static bool same_request(const ViewRequest& left, const ViewRequest& right)
    {
        return left.parent_window == right.parent_window &&
            left.visible == right.visible &&
            left.url == right.url &&
            same_rect(left.bounds, right.bounds);
    }

    void apply_request()
    {
        if (!com_initialized)
        {
            return;
        }

        const ViewRequest next_request = get_request_copy();
        if (!next_request.visible || !has_area(next_request.bounds))
        {
            hide_on_worker();
            return;
        }

        if (!ensure_host_window(next_request.parent_window))
        {
            hide_on_worker();
            return;
        }

        host_bounds = next_request.bounds;
        const int width = host_bounds.right - host_bounds.left;
        const int height = host_bounds.bottom - host_bounds.top;
        webview_bounds = { 0, 0, width, height };
        pending_url = next_request.url;

        EnableWindow(host_window, TRUE);
        SetWindowPos(
            host_window,
            HWND_TOP,
            host_bounds.left,
            host_bounds.top,
            width,
            height,
            SWP_NOACTIVATE | SWP_SHOWWINDOW);
        ShowWindow(host_window, SW_SHOWNOACTIVATE);

        if (!webview)
        {
            create_async();
            return;
        }

        update_bounds();
        disable_webview_input();

        const bool should_refresh = force_refresh.exchange(false);
        if (should_refresh || pending_url != current_url)
        {
            navigate_pending();
        }
    }

    void hide_on_worker()
    {
        if (controller)
        {
            controller->put_IsVisible(FALSE);
        }

        if (host_window)
        {
            ShowWindow(host_window, SW_HIDE);
            EnableWindow(host_window, FALSE);
        }

        set_status(webview ? "hidden" : "idle");
    }

    void update_bounds()
    {
        if (!controller)
        {
            return;
        }

        controller->put_Bounds(webview_bounds);
        controller->put_IsVisible(TRUE);
    }

    void apply_settings()
    {
        if (!webview)
        {
            return;
        }

        ComPtr<ICoreWebView2Settings> settings{};
        if (SUCCEEDED(webview->get_Settings(&settings)) && settings)
        {
            settings->put_AreDefaultContextMenusEnabled(FALSE);
            settings->put_AreDevToolsEnabled(FALSE);
            settings->put_IsStatusBarEnabled(FALSE);
            settings->put_AreHostObjectsAllowed(FALSE);
        }
    }

    void install_script()
    {
        if (!webview)
        {
            return;
        }

        const std::wstring script = map_only_script();
        webview->AddScriptToExecuteOnDocumentCreated(script.c_str(), nullptr);
        webview->ExecuteScript(script.c_str(), nullptr);
    }

    void hook_navigation_completed()
    {
        if (!webview)
        {
            return;
        }

        std::weak_ptr<Impl> weak_self = weak_from_this();
        webview->add_NavigationCompleted(
            Callback<ICoreWebView2NavigationCompletedEventHandler>(
                [weak_self](ICoreWebView2*, ICoreWebView2NavigationCompletedEventArgs*) -> HRESULT
                {
                    const std::shared_ptr<Impl> self = weak_self.lock();
                    if (!self)
                    {
                        return S_OK;
                    }

                    self->install_script();
                    self->disable_webview_input();
                    self->set_status("loaded");
                    return S_OK;
                }).Get(),
            &navigation_completed_token);
    }

    void disable_webview_input()
    {
        if (!host_window)
        {
            return;
        }

        EnumChildWindows(host_window, disable_child_window_proc, 0);
        EnableWindow(host_window, FALSE);
        if (host_parent_window)
        {
            SetFocus(host_parent_window);
        }
    }

    void navigate_pending()
    {
        if (!webview || pending_url.empty())
        {
            return;
        }

        const std::wstring wide_url = widen(pending_url);
        if (wide_url.empty())
        {
            return;
        }

        set_status("loading");
        webview->Navigate(wide_url.c_str());
        current_url = pending_url;
        disable_webview_input();
    }

    void create_async()
    {
        if (creating || webview || !host_window)
        {
            return;
        }

        creating = true;
        set_status("creating webview");

        const std::wstring data_folder = user_data_folder();
        std::weak_ptr<Impl> weak_self = weak_from_this();
        const HRESULT result = CreateCoreWebView2EnvironmentWithOptions(
            nullptr,
            data_folder.c_str(),
            nullptr,
            Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
                [weak_self](HRESULT environment_result, ICoreWebView2Environment* next_environment) -> HRESULT
                {
                    const std::shared_ptr<Impl> self = weak_self.lock();
                    if (!self)
                    {
                        return S_OK;
                    }

                    if (FAILED(environment_result) || !next_environment)
                    {
                        self->creating = false;
                        self->set_status("webview runtime missing");
                        logger::warn("[Map] WebView2 environment failed");
                        return S_OK;
                    }

                    self->environment = next_environment;
                    self->environment->CreateCoreWebView2Controller(
                        self->host_window,
                        Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                            [weak_self](HRESULT controller_result, ICoreWebView2Controller* next_controller) -> HRESULT
                            {
                                const std::shared_ptr<Impl> self = weak_self.lock();
                                if (!self)
                                {
                                    return S_OK;
                                }

                                self->creating = false;
                                if (FAILED(controller_result) || !next_controller)
                                {
                                    self->set_status("webview controller failed");
                                    logger::warn("[Map] WebView2 controller failed");
                                    return S_OK;
                                }

                                self->controller = next_controller;
                                self->controller->get_CoreWebView2(&self->webview);
                                self->apply_settings();
                                self->install_script();
                                self->hook_navigation_completed();
                                self->apply_request();
                                return S_OK;
                            }).Get());

                    return S_OK;
                }).Get());

        if (FAILED(result))
        {
            creating = false;
            set_status("webview create failed");
        }
    }

    void destroy_webview()
    {
        if (webview && navigation_completed_token.value != 0)
        {
            webview->remove_NavigationCompleted(navigation_completed_token);
            navigation_completed_token = {};
        }

        webview.Reset();
        controller.Reset();
        environment.Reset();
        creating = false;
        current_url.clear();
        pending_url.clear();

        if (host_window)
        {
            DestroyWindow(host_window);
            host_window = nullptr;
        }
        host_parent_window = nullptr;
    }
};

TarkovMapWebView::~TarkovMapWebView()
{
    shutdown();
}

void TarkovMapWebView::show(
    HWND parent_window,
    const RECT& bounds,
    const std::string& remote_id,
    const std::string& map_key)
{
    if (!impl)
    {
        impl = std::make_shared<Impl>();
    }

    impl->show(parent_window, bounds, remote_id, map_key);
}

void TarkovMapWebView::hide()
{
    if (impl)
    {
        impl->hide();
    }
}

void TarkovMapWebView::refresh()
{
    if (impl)
    {
        impl->refresh();
    }
}

void TarkovMapWebView::shutdown()
{
    if (!impl)
    {
        return;
    }

    impl->shutdown();
    impl.reset();
}

bool TarkovMapWebView::is_visible() const
{
    return impl && impl->visible_cache.load();
}

std::string TarkovMapWebView::get_status() const
{
    return impl ? impl->get_status() : "idle";
}
