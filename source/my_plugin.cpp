#define _CRT_SECURE_NO_WARNINGS
#include "my_plugin.hpp"
#include <reaper_plugin_functions.h>
#include "razor.hpp"
#include "window.hpp"
auto COMMAND = "TBO_MIDI_PREVIEW";
auto ACTION = "Tools by Onaya: Preview MIDI Notes";
bool preview = true;
static WNDPROC g_ReaperTrackWndProc = NULL;

static int translateAccel(MSG* msg, accelerator_register_t* ctx)
{
    if (msg->message == WM_KEYDOWN) {
        char buf[256];
        sprintf_s(
            buf,
            "HWND: %p wParam: %p lParam: %p\n",
            msg->hwnd,
            (void*)msg->wParam,
            (void*)msg->lParam);
        ShowConsoleMsg(buf);
        return 1;
    }
    return 0;
}

static accelerator_register_t g_ar = {translateAccel, TRUE, NULL};

double GetItemHeaderHeight(MediaItem* item)
{
    int limit = 4;
    MediaTrack* track = GetMediaItem_Track(item);
    double track_height = GetMediaTrackInfo_Value(track, "I_TCPH");
    double height = GetMediaItemInfo_Value(item, "I_LASTH");
    double header_height =
        track_height - height > limit && track_height - height || 0;
    if (header_height > 0) {
        header_height = header_height - limit;
    }
    header_height += 16;
    return header_height;
}
LRESULT CALLBACK
TrackWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static POINT p;
    GetCursorPos(&p);
    std::vector<RazorEditArea> reas = RazorEditArea::GetRazorEditAreas();
    //if (uMsg == WM_LBUTTONDOWN) {
    //    MediaItem* item = GetItemFromPoint(p.x, p.y, true, NULL);
    //    ScreenToClient(hwnd, &p);
    //    if (item) {
    //        MediaTrack* track = GetMediaItem_Track(item);
    //        double track_y = GetMediaTrackInfo_Value(track, "I_TCPY") +
    //        GetItemHeaderHeight(item);
    //        // IF LBUTTON IS NOT DOWN ON LABEL'S ITEM
    //        if (p.y > track_y) {
    //            SetMouseModifier("MM_CTX_ITEM", 0, "62");
    //            // IF CURSOR IS NOT ON ENVELOPE CLEAR RAZOR AREA
    //            if (GetCursorContext() != 2) { 
    //                Main_OnCommand(42406, 0); // Razor edit: Clear all areas
    //            }
    //        }
    //        else {
    //            SetMouseModifier("MM_CTX_ITEM", 0, "13");
    //        }
    //    }
    //    else {
    //        if (reas.size() > 0) {
    //            // IF CURSOR IS NOT ON ENVELOPE CLEAR RAZOR AREA
    //            if (GetCursorContext() != 2) {
    //                Main_OnCommand(42406, 0); // Razor edit: Clear all areas
    //            }
    //        }
    //    }
    //}
    //if (uMsg == WM_LBUTTONUP) {
    //    std::vector<MediaItem*> items =
    //        reas[0].GetMediaItemsCrossingRazorEditAreas(true);
    //    for (MediaItem* item : items) {
    //        SetMediaItemSelected(item, true);
    //        UpdateArrange();
    //    }

    //}
    if (uMsg == WM_MOUSEWHEEL && GetAsyncKeyState(VK_LCONTROL)) {
        int val = (short)HIWORD(wParam);
        double division;
        int ret = GetSetProjectGrid(0, false, &division, 0, 0);
        if (val > 0) {
            SetProjectGrid(0, division / 2);
        }
        else if (val < 0) {
            SetProjectGrid(0, division * 2);
        }
    }
    return g_ReaperTrackWndProc(hwnd, uMsg, wParam, lParam);
}
double TimeToBeat(double time)
{
    return TimeMap2_timeToBeats(0, time, NULL, NULL, NULL, NULL);
}

// our "main function" in this example
void MainFunctionOfMyPlugin()
{
    HWND MIDIEditor = MIDIEditor_GetActive();
    int preview_midi_notes_toggle =
        GetToggleCommandStateEx(MIDI_EDITOR_SECTION, 40041);
    int media_item_lane_toggle =
        GetToggleCommandStateEx(MIDI_EDITOR_SECTION, 40819);
    int docked_midi_editor_toggle =
        GetToggleCommandStateEx(MIDI_EDITOR_SECTION, 40018);
    int playstate = GetPlayState();
    // Select all items include in razor edit area //
    //std::vector<MediaItem*> items = RazorEditArea::GetMediaItemsCrossingRazorEditAreas(true);
    //for (MediaItem* item : items) {
    //        SetMediaItemSelected(item, true);
    //        UpdateArrange();
    //}
    //double zoom = GetHZoomLevel();
    //double zoom_beats = TimeToBeat(zoom);
    //char buf[256];
    //sprintf(buf, "zoom beats: %f \n", round(zoom_beats));
    //ShowConsoleMsg(buf);
    if (MIDIEditor && (GetAsyncKeyState(VK_RBUTTON) & 1)) {
        MIDIEditor_OnCommand(MIDIEditor, 40214); // Edit: Unselect all
    }
    if (preview) {
        if (MIDIEditor && !playstate && !preview_midi_notes_toggle) {
            MIDIEditor_OnCommand(
                MIDIEditor,
                40041); // Options: Preview notes when inserting or editing
        }
        else if (MIDIEditor && playstate && preview_midi_notes_toggle) {
            MIDIEditor_OnCommand(
                MIDIEditor,
                40041); // Options: Preview notes when inserting or editing
        }
    }

    if (media_item_lane_toggle) {
        MIDIEditor_OnCommand(
            MIDIEditor,
            40819); // Contents: Show/hide media item lane
    }
    if (!docked_midi_editor_toggle) {
        MIDIEditor_OnCommand(
            MIDIEditor,
            40018); // Options: Toggle window docking
    }
    int crossfade = GetToggleCommandState(40041);
    int VKB = GetToggleCommandState(40637);
    if (crossfade) {
        Main_OnCommand(40041, 0);
    }
    if (!VKB) {
        Main_OnCommand(40637, 0);
    }

    return;
}

int commandID {0};
bool state {false};
bool defer {true};

custom_action_register_t action = {0, COMMAND, ACTION, NULL};

int ToggleActionCallback(int command)
{
    if (command != commandID) {
        return -1;
    }
    else if (state) {
        return 1;
    }
    return 0;
}

bool OnAction(
    KbdSectionInfo* sec,
    int command,
    int val,
    int valhw,
    int relmode,
    HWND hwnd)
{

    (void)sec;
    (void)val;
    (void)valhw;
    (void)relmode;
    (void)hwnd;

    if (command != commandID) {
        return false;
    }

    if (defer) {

        state = !state;

        if (state) {

            plugin_register("timer", (void*)MainFunctionOfMyPlugin);
        }
        else {

            plugin_register("-timer", (void*)MainFunctionOfMyPlugin);
        }
    }
    else {

        MainFunctionOfMyPlugin();
    }

    return true;
}

// static void importExtensionAPI()
// {
// 	plugin_register("-timer", (void*)importExtensionAPI);

// 	// import functions exposed by third-party extensions
// 	BR_GetMouseCursorContext =
// (decltype(BR_GetMouseCursorContext))plugin_getapi("BR_GetMouseCursorContext");
// }

void MenuHook(const char* menuidstr, HMENU menu, int flag)
{
    if (strcmp(menuidstr, "Main extensions") || flag != 0)
        return;

    if (!menu) {
        menu = CreatePopupMenu();
    }

    int pos = GetMenuItemCount(menu);

    MENUITEMINFO mii;
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_TYPE | MIIM_ID;
    mii.fType = MFT_STRING;

    mii.dwTypeData = (char*)ACTION;

    mii.wID = commandID;

    InsertMenuItem(menu, pos++, true, &mii);
    return;
}

void RegisterMyPlugin()
{

    commandID = plugin_register("custom_action", &action);
    if (defer) {
        plugin_register("toggleaction", (void*)ToggleActionCallback);
    }
    plugin_register("hookcommand2", (void*)OnAction);
    plugin_register("hookcustommenu", (void*)MenuHook);
    // plugin_register("accelerator",&g_ar);
    // plugin_register("timer", (void*)importExtensionAPI);
    //HWND hTrackView = GetTrackWnd();
    //if (hTrackView)
    //    g_ReaperTrackWndProc = (WNDPROC)
    //        SetWindowLongPtr(hTrackView, GWLP_WNDPROC, (LONG_PTR)TrackWndProc);
    AddExtensionsMainMenu();
}

void UnregisterMyPlugin()
{
    plugin_register("-custom_action", &action);
    plugin_register("-toggleaction", (void*)ToggleActionCallback);
    plugin_register("-hookcommand2", (void*)OnAction);
    plugin_register("-hookcustommenu", (void*)MenuHook);
    // plugin_register("-accelerator",&g_ar);
    return;
}
