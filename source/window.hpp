
static HWND SearchChildren(
    const char* name,
    HWND hwnd,
    HWND startHwnd = NULL,
    bool windowHasNoChildren = false)
{
    {
        HWND returnHwnd = startHwnd;
        while (true) {
            returnHwnd = FindWindowEx(hwnd, returnHwnd, NULL, name);
            if (!returnHwnd || !windowHasNoChildren ||
                !GetWindow(returnHwnd, GW_CHILD))
                return returnHwnd;
        }
    }
}
static void AllocPreparedString(const char* name, char** destination)
{
    *destination = const_cast<char*>(name);
}

static HWND SearchFloatingDockers(
    const char* name,
    const char* dockerName,
    bool windowHasNoChildren = false)
{
    HWND main_hwnd = GetMainHwnd();
    HWND docker = FindWindowEx(NULL, NULL, NULL, dockerName);
    while (docker) {
        if (GetParent(docker) == main_hwnd) {
            HWND insideDocker = FindWindowEx(docker, NULL, NULL, "REAPER_dock");
            while (insideDocker) {
                if (HWND w = SearchChildren(
                        name,
                        insideDocker,
                        NULL,
                        windowHasNoChildren))
                    return w;
                insideDocker =
                    FindWindowEx(docker, insideDocker, NULL, "REAPER_dock");
            }
        }
        docker = FindWindowEx(NULL, docker, NULL, dockerName);
    }
    return NULL;
}

static HWND FindInFloatingDockers(
    const char* name,
    bool windowHasNoChildren = false)
{
#ifdef _WIN32
    HWND hwnd = SearchFloatingDockers(name, NULL, windowHasNoChildren);
#else
    HWND hwnd = SearchFloatingDockers(
        name,
        __localizeFunc("Docker", "docker", 0),
        windowHasNoChildren);
    if (!hwnd) {
        WDL_FastString dockerName;
        dockerName.AppendFormatted(
            256,
            "%s%s",
            name,
            __localizeFunc(" (docked)", "docker", 0));
        hwnd =
            SearchFloatingDockers(name, dockerName.Get(), windowHasNoChildren);
    }
    if (!hwnd)
        hwnd = SearchFloatingDockers(
            name,
            __localizeFunc("Toolbar Docker", "docker", 0),
            windowHasNoChildren);
#endif

    return hwnd;
}

static HWND FindInReaperDockers(const char* name)
{
    HWND main_hwnd = GetMainHwnd();
    HWND docker = FindWindowEx(main_hwnd, NULL, NULL, "REAPER_dock");
    while (docker) {
        if (HWND hwnd = SearchChildren(name, docker))
            return hwnd;
        docker = FindWindowEx(main_hwnd, docker, NULL, "REAPER_dock");
    }
    return NULL;
}
static HWND FindFloating(
    const char* name,
    bool checkForNoCaption = false,
    bool windowHasNoChildren = false)
{
    HWND main_hwnd = GetMainHwnd();
    HWND hwnd = SearchChildren(name, NULL);
    while (hwnd) {
        if (GetParent(hwnd) == main_hwnd) {
            if ((!checkForNoCaption ||
                 !(GetWindowLongPtr(hwnd, GWL_STYLE) & WS_CAPTION)) &&
                (!windowHasNoChildren || !GetWindow(hwnd, GW_CHILD)))
                return hwnd;
        }
        hwnd = SearchChildren(name, NULL, hwnd);
    }
    return NULL;
}

static HWND FindInReaper(const char* name)
{
    HWND main_hwnd = GetMainHwnd();
    return SearchChildren(name, main_hwnd);
}

static HWND FindReaperWndByPreparedString(const char* name)
{
    if (name) {
        HWND hwnd = FindInReaperDockers(name);
        if (!hwnd)
            hwnd = FindFloating(name);
        if (!hwnd)
            hwnd = FindInFloatingDockers(name);
        if (!hwnd)
            hwnd = FindInReaper(name);
        return hwnd;
    }
    return NULL;
}

HWND GetMediaExplorerWnd()
{
    static char* s_name = NULL;
    if (!s_name)
        AllocPreparedString("Media Explorer", &s_name);
    return FindReaperWndByPreparedString(s_name);
}

HWND GetVirtualKeyboardWnd()
{
    static char* s_name = NULL;
    if (!s_name)
        AllocPreparedString("Virtual MIDI keyboard", &s_name);
    return FindReaperWndByPreparedString(s_name);
}

HWND GetTrackWnd()
{
    static HWND s_hwnd = nullptr;
    if (!s_hwnd)
        s_hwnd = GetDlgItem(GetMainHwnd(), 1000);
    return s_hwnd;
}

HWND GetPianoView()
{
    HWND MIDIEditor = MIDIEditor_GetActive();
    if (MIDIEditor_GetMode(MIDIEditor) != -1)
        return GetDlgItem(MIDIEditor, 1000);
    else
        return nullptr;
}