#include "my_plugin.hpp"

class RazorEditArea {
  public:
    static std::vector<RazorEditArea> GetRazorEditAreas();
    static std::vector<MediaItem*> GetMediaItemsCrossingRazorEditAreas(
        bool includeEnclosedItems);

    MediaTrack* GetMediaTrack();
    double GetAreaStart();
    double GetAreaEnd();
    std::string GetEnvelopeGUID();

  private:
    RazorEditArea();

    MediaTrack* m_track;
    double m_areaStart;
    double m_areaEnd;
    std::string m_envelopeGUID;
};

RazorEditArea::RazorEditArea()
    : m_track(nullptr)
    , m_areaStart(-1.0)
    , m_areaEnd(-1.0)
    , m_envelopeGUID("")
{
}

std::vector<RazorEditArea> RazorEditArea::GetRazorEditAreas()
{
    std::vector<RazorEditArea> razorEditAreas;
    if (atof(GetAppVersion()) < 6.24) // Razor edit area API was added in v6.24
        return razorEditAreas;

    char* razorEditAreasStr;
    for (int i = 1; i <= GetNumTracks(); i++) {
        // API returns space-separated triples of start time, end time, and
        // envelope GUID string
        razorEditAreasStr = reinterpret_cast<char*>(GetSetMediaTrackInfo(
            CSurf_TrackFromID(i, false),
            "P_RAZOREDITS",
            nullptr));
        if (razorEditAreasStr && razorEditAreasStr[0]) {
            RazorEditArea rea;
            // parse razorEditAreasStr
            char* token = strtok(razorEditAreasStr, " "); // start time
            while (token != nullptr) {
                rea.m_track = (CSurf_TrackFromID(i, false));
                rea.m_areaStart = (atof(token));

                token = strtok(nullptr, " "); // end time
                rea.m_areaEnd = (atof(token));

                token = strtok(nullptr, " "); // envelope GUID string
                if (token[1] == '{')
                    rea.m_envelopeGUID = token;
                else
                    rea.m_envelopeGUID = "";

                razorEditAreas.push_back(rea);
                token = strtok(nullptr, " "); // next triple
            }
        }
    }
    return razorEditAreas;
}

std::vector<MediaItem*> RazorEditArea::GetMediaItemsCrossingRazorEditAreas(
    bool includeEnclosedItems)
{
    std::vector<MediaItem*> itemsCrossingRazorEditAreas;

    std::vector<RazorEditArea> reas = RazorEditArea::GetRazorEditAreas();
    for (size_t i = 0; i < reas.size(); i++) {
        if (reas[i].m_envelopeGUID ==
            "") // only check razor edit areas which aren't envelopes
        {
            MediaTrack* track = reas[i].m_track;
            int itemCount = CountTrackMediaItems(track);
            for (int j = 0; j < itemCount; j++) {
                MediaItem* item = GetTrackMediaItem(track, j);
                double itemPos = GetMediaItemInfo_Value(item, "D_POSITION");
                double itemEnd =
                    itemPos + GetMediaItemInfo_Value(item, "D_LENGTH");

                // check if item is in razor edit area bounds
                if (includeEnclosedItems == false &&
                    reas[i].m_areaStart <= itemPos &&
                    reas[i].m_areaEnd >= itemEnd) // rea encloses item
                    continue;
                if ((itemEnd > reas[i].m_areaStart &&
                     itemEnd <= reas[i].m_areaEnd) // item end within rea
                    || (itemPos >= reas[i].m_areaStart &&
                        itemPos < reas[i].m_areaEnd) // item start within rea
                    || (itemPos <= reas[i].m_areaStart &&
                        itemEnd >= reas[i].m_areaEnd)) // item encloses rae
                    itemsCrossingRazorEditAreas.push_back(item);
            }
        }
    }
    return itemsCrossingRazorEditAreas;
}

MediaTrack* RazorEditArea::GetMediaTrack()
{
    return m_track;
}

double RazorEditArea::GetAreaStart()
{
    return m_areaStart;
}

double RazorEditArea::GetAreaEnd()
{
    return m_areaEnd;
}

std::string RazorEditArea::GetEnvelopeGUID()
{
    return m_envelopeGUID;
}
