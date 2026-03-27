#pragma once

#include <iostream>


#ifdef _WIN32

#include <comdef.h>
#include <atlbase.h>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>

// import office - necessarily required?
#import "libid:2DF8D04C-5BFA-101B-BDE5-00AA0044DE52" \
    version("2.8") \
    rename_namespace("Office") \
    rename("RGB", "OfficeRGB") \
    rename("DocumentProperties", "OfficeDocumentProperties")

// import outlook
#import "libid:00062FFF-0000-0000-C000-000000000046" \
    version("16.0") \
    rename_namespace("Outlook") \
    rename("CopyFile", "OutlookCopyFile") \
    rename("PlaySound", "OutlookPlaySound") \
    rename("DocumentProperties", "OutlookDocumentProperties") \
    rename("GetOrganizer", "GetOrganizerString") \
    rename("OnlineMeetingProvider", "OnlineMeetingProviderProp")

#include <windows.h>

#endif

using namespace Outlook;



class MCalendar {
public:
	MCalendar();
    ~MCalendar();

    struct CalendarEvent {
        std::string title;
        int startHour;
        int startMinute;
        int endHour;
        int endMinute;
        std::string description;
    };

    static SYSTEMTIME getDate();

    _ItemsPtr getCalendarItems();
    std::vector<CalendarEvent> getTodaysEvents(_ItemsPtr& pItems);
	
private:
  
    _ApplicationPtr pOutlook;
    _NameSpacePtr pNamespace;
    bool initialized;
    
    // Helper function to convert _bstr_t to std::string
	std::string BstrToString(const _bstr_t& bstr);

    bool initialise();
};

