#include "MCalendar.h"

MCalendar::MCalendar() {
#ifndef _WIN32
    std::cerr << "Does not support current operating system. Please run on Windows." << std::endl; 
#endif
    // CoInitialize(NULL);
    if (!initialise()) {
        std::cerr << "Failed to initialize calendar." << std::endl;
        std::exit(10);
    }
}

MCalendar::~MCalendar() {
    pNamespace->Logoff();
    pNamespace.Release();
    pOutlook.Release();
    // CoUninitialize();
}

// convert _bstr_t to std::string
// basically COM's own string implementation 
// https://learn.microsoft.com/en-us/previous-versions/windows/desktop/automat/bstr 
std::string MCalendar::BstrToString(const _bstr_t& bstr) {
    if (bstr.length() == 0) return "";
    return std::string((char*)bstr);
}

SYSTEMTIME MCalendar::getDate() {
    SYSTEMTIME today;
    GetLocalTime(&today);

    return today;
}

bool MCalendar::initialise() {
    try {
        HRESULT instance = pOutlook.CreateInstance(__uuidof(Application)); // is native com, maybe change for .net implementation (easier to manage)?
        if (FAILED(instance)) {
            std::cerr << "Unable to access Outlook." << std::endl;
            return false;
        } // manual error checking can be deleted with .net (i think)

        pNamespace = pOutlook->GetNamespace(_bstr_t("MAPI")); // get MAPI (default) namespace (primary data storage of Outlook)
        pNamespace->Logon(
            _variant_t(),       // default username
            _variant_t(),       // and default password
            _variant_t(false),   // dont show dialog, use default account (will show setup wizard on first run)
            _variant_t(true)    // make new session
        );

        // if (pNamespace != NULL) std::cout << pNamespace << std::endl;

        initialized = true;
        return true;
    }
    catch (const _com_error& e) {
        std::cerr << "Initialization Error\n";
        std::wcerr << L"HRESULT: 0x" << std::hex << e.Error() << std::endl;
        if (e.ErrorMessage())
            std::wcerr << L"Message: " << e.ErrorMessage() << std::endl;
        return false;
    }
}

_ItemsPtr MCalendar::getCalendarItems() {
    if (!initialized) {
        std::cerr << "Calendar initialise not called yet - check production code." << std::endl;
        std::cerr << "Initialising automatically." << std::endl;
        initialise();
    }

    _ItemsPtr pItems; // FIX: req. for declaration outside of try block for valid return?

    try {
        MAPIFolderPtr pCalendarFolder = pNamespace->GetDefaultFolder(olFolderCalendar); // get calendar folder specifically
        pItems = pCalendarFolder->GetItems(); // then get individual items within calendar folder
        pItems->PutIncludeRecurrences(VARIANT_TRUE); // include recurring events
        pItems->Sort(
            _bstr_t("[Start]"), // sort chronologically
            _variant_t(false)   // direction: false=ascending(earliest to latest), true=descending(latest to earliest)
        ); 

        pCalendarFolder.Release();

    } catch (const _com_error& e) {
        std::cerr << "Error getting events\n";
        std::wcerr << L"HRESULT: 0x" << std::hex << e.Error() << std::endl; // reqires wcerr - wide string
        if (e.ErrorMessage())
            std::wcerr << L"Message: " << e.ErrorMessage() << std::endl;
    }

    

    return pItems; // better to make pItems global?
}

std::vector<MCalendar::CalendarEvent> MCalendar::getTodaysEvents(_ItemsPtr& pItems) {
    std::vector<CalendarEvent> todaysEvents; // vector that stores structs (CalendarEvent event) of events

    try {

        //_ItemsPtr pItems = getCalendarItems();

        SYSTEMTIME today = getDate();

        // use GetFirst/GetNext to iterate
        IDispatchPtr pIndividualItem = pItems->GetFirst(); // pIndividualItem stores single item from list of pItems (no specific type)

        // loops through each individual item until none are left
        while (pIndividualItem != NULL) {
            try {
                _AppointmentItemPtr pCalendarEvent = pIndividualItem; // converts item to a calendar event (appointment)

                if (pCalendarEvent != NULL) {
                    // DATE start = pCalendarEvent->GetStart(); // get start of event

                    // needs to convert to SYSTEMTIME format for easier checking
                    SYSTEMTIME start;
                    VariantTimeToSystemTime(pCalendarEvent->GetStart(), &start); // get rid of original start variable - less confusing

                    // if event is today - i.e. checks year, month and day matches
                    if (start.wYear == today.wYear && start.wMonth == today.wMonth && start.wDay == today.wDay) {
                        CalendarEvent event; // struct that stores properties of each individual event

                        event.title = BstrToString(pCalendarEvent->GetSubject());
                        event.startHour = start.wHour;
                        event.startMinute = start.wMinute;

                        // DATE end = pCalendarEvent->GetEnd(); // convert again to SYSTEMTIME
                        SYSTEMTIME end;
                        VariantTimeToSystemTime(pCalendarEvent->GetEnd(), &end); // get rid of original end variable
                        event.endHour = end.wHour;
                        event.endMinute = end.wMinute;

                        event.description = BstrToString(pCalendarEvent->GetBody());

                        todaysEvents.push_back(event); // append struct to back of vector
                    }

                    // if event is after today
                    else if (
                         start.wYear > today.wYear ||
                        (start.wYear == today.wYear && start.wMonth > today.wMonth) ||
                        (start.wYear == today.wYear && start.wMonth == today.wMonth && start.wDay > today.wDay)) {

                        pCalendarEvent.Release();
                        break;
                    }

                    pCalendarEvent.Release();
                }
            } catch (_com_error& itemError) {
                // skip inaccessible stuff
                // what kind of error should this throw?
                std::cerr << "Failed to get a calendar item: " << std::endl;
            }

            pIndividualItem = pItems->GetNext(); // loops by getting next item in list
        }

        pItems.Release();
        
    } catch (const _com_error& e) {
        std::cerr << "Error parsing today's events\n";
        std::wcerr << L"HRESULT: 0x" << std::hex << e.Error() << std::endl;
        if (e.ErrorMessage())
            std::wcerr << L"Message: " << e.ErrorMessage() << std::endl;
    }

    return todaysEvents;
}
