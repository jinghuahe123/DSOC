#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <stdio.h>
#include <unordered_set>
#include <mutex>
#include <atomic>

#include "MCalendar.h"
#include "ParameterHandler.h"
#include "EventLogger.h"
#include "NotificationHandler.h"
// #include "UIWindow.h"


// #include <MddBootstrap.h>


/*
    Format for device specificty:
    SENDTO: <device name> \r\n<Content of reminder>


    TODO:
        change every instance of push_back to emplace_back?
        better return carriage detection for getTarget
        better sendto: detection
        fix copy of data inefficiency
        remove SYSTEMTIME dependency entirely? - incompatible with nlohmann/json (pure integer hour/minute)
*/

// allowing a certain keyword to automatically put a notification somewhere?
// categorisation keywords / analyse description to optimise for certain function

// set up user settable categories similar to outlook calendar categorise
// set up broker to allow outlook calendar categorisation fields to automatically put a notificaion to a certain device?


// main struct for storing each calendar event
struct CalendarEvent {
    std::string title;
    int startHour;
    int startMinute;
    int endHour;
    int endMinute;
    std::string description;
    std::string targetDevice;
    bool notified = false;

    // used in comparing if event is new or old
    bool operator==(const auto& events) const {
        return title == events.title &&
            startHour == events.startHour &&
            startMinute == events.startMinute &&
            endHour == events.endHour &&
            endMinute == events.endMinute &&
            description == events.description &&
            targetDevice == events.targetDevice;
    }
};

struct CalendarEventHash { // dont hash index, order of events may change
    std::size_t operator()(const CalendarEvent& event) const {
        std::size_t hTitle = std::hash<std::string>{}(event.title);
        std::size_t hStartHour = std::hash<int>{}(event.startHour);
        std::size_t hStartMinute = std::hash<int>{}(event.startMinute);
        std::size_t hEndHour = std::hash<int>{}(event.endHour);
        std::size_t hEndMinute = std::hash<int>{}(event.endMinute);
        std::size_t hDescription = std::hash<std::string>{}(event.description);
        std::size_t hTargetDevice = std::hash<std::string>{}(event.targetDevice);

        return hTitle ^ (hStartHour << 1) ^ (hStartMinute << 2) ^ (hEndHour << 3) ^ (hEndMinute << 4) ^ (hDescription << 5) ^ (hTargetDevice << 6);
    }
};

// careful when using not to pass newEvents and oldEvents backwards
// compares and returns what is different in new events from old
void compareCalendarEvents(const std::vector<CalendarEvent>& newEvents, const std::vector<CalendarEvent>& oldEvents, std::vector<CalendarEvent>& duplicateEvents, std::vector<CalendarEvent>& uniqueNewEvents) {
    std::unordered_set<CalendarEvent, CalendarEventHash> vecOldEventsSet(oldEvents.begin(), oldEvents.end());

    for (const auto& event : newEvents) {
        if (vecOldEventsSet.find(event) != vecOldEventsSet.end()) {
            duplicateEvents.push_back(event);
        }
        else {
            uniqueNewEvents.push_back(event);
        }
    }
}

// helper for getting events from MCalendar into local struct
std::vector<CalendarEvent> getEvents(MCalendar& object) {
    _ItemsPtr pItems = object.getCalendarItems();
    std::vector<CalendarEvent> events; // = myCalendar.getTodaysEvents(pItems);
    std::vector<MCalendar::CalendarEvent> details = object.getTodaysEvents(pItems);
    for (size_t i = 0; i < details.size(); i++) {       // FIX: more efficient copying needed
        CalendarEvent event;
        event.title = details[i].title;
        event.startHour = details[i].startHour;
        event.startMinute = details[i].startMinute;
        event.endHour = details[i].endHour;
        event.endMinute = details[i].endMinute;
        event.description = details[i].description;
        //event.notified = details[i].notified;

        events.push_back(event);
    }

    return events;
} 

// returns target computer (if set) from event description
std::string getTarget(std::string description) {
    std::string target;

    // carriage return detection - see format for device specifity
    for (char c : description) {
        if (c == '\r' || c == '\n') break; // check for either carriage return or feed line/newline character
        target.push_back(c);

    }

    // detect if target has been set by user
    if (target.length() > 7 && (target.substr(0, 7) == "SENDTO:" || target.substr(0, 7) == "sendto:" || target.substr(0, 7) == "Sendto:")) { // figure out better way for case insensitivity (try changing all to uppercase before compare)
        if (target.length() > 8 && target[7] == ' ') return target.substr(8);
        return target.substr(7);
    }
    else {
        return "No target selected or wrong format.";
    }

    return "";
}

// helper for formatting local events into log file format
std::vector<EventLogger::CalendarEvent> formatEventsToLogfile(std::vector<CalendarEvent> events) {
    std::vector<EventLogger::CalendarEvent> logEvents;

    for (CalendarEvent event : events) {
        EventLogger::CalendarEvent logEvent;
        logEvent.title = event.title;
        logEvent.startHour = event.startHour;
        logEvent.startMinute = event.startMinute;
        logEvent.endHour = event.endHour;
        logEvent.endMinute = event.endMinute;
        logEvent.targetDevice = event.targetDevice;
        logEvent.description = event.description;
        logEvent.notified = event.notified;

        logEvents.push_back(logEvent);
    }
    for (int i = 0; i < logEvents.size(); i++) {
        logEvents[i].index = i;
    }
    
    return logEvents;
}

// helper for formatting log file events into local events format
std::vector<CalendarEvent> formatLogFileToEvents(std::vector<EventLogger::CalendarEvent> events) {
    std::vector<CalendarEvent> logEvents;

    for (EventLogger::CalendarEvent event : events) {
        CalendarEvent logEvent;
        logEvent.title = event.title;
        logEvent.startHour = event.startHour;
        logEvent.startMinute = event.startMinute;
        logEvent.endHour = event.endHour;
        logEvent.endMinute = event.endMinute;
        logEvent.targetDevice = event.targetDevice;
        logEvent.description = event.description;
        logEvent.notified = event.notified;

        logEvents.push_back(logEvent);
    }

    return logEvents;
}

// debugging use
// displays to console details of a vector of events
void displayEvents(std::vector<CalendarEvent> events) { 
    SYSTEMTIME today = MCalendar::getDate();

    std::cout << "Today's date is " << today.wYear << "/"
        << std::setw(2) << std::setfill('0') << today.wMonth << "/"
        << std::setw(2) << std::setfill('0') << today.wDay
        << std::endl; 

    if (!events.empty()) {
        std::cout << "Today's Calendar:" << std::endl;
        std::cout << "Found " << events.size() << " events." << std::endl << std::endl;

        for (size_t i = 0; i < events.size(); i++) {
            const CalendarEvent& event = events[i];

            std::cout << "Title: " << event.title << std::endl;
            std::cout << "Time: "
                << std::setw(2) << std::setfill('0') << event.startHour << ":"
                << std::setw(2) << std::setfill('0') << event.startMinute << " - "
                << std::setw(2) << std::setfill('0') << event.endHour << ":"
                << std::setw(2) << std::setfill('0') << event.endMinute << std::endl;

            if (!event.description.empty()) {
                std::cout << "Target: " << event.targetDevice << std::endl;
                std::cout << "Description: " << event.description << std::endl;
            }
            std::cout << "----------------------------------------" << std::endl;
        }
    }
    else {
        std::cout << "No events today." << std::endl;
    }
}





// struct of new and old events
struct comparedEvents {
    std::vector<CalendarEvent> duplicateEvents;
    std::vector<CalendarEvent> uniqueNewEvents;
};

comparedEvents latestEvents; // global variable for struct of new and old events to access between threads
std::mutex latestEventsMutex;

std::atomic<bool> checkEventsRunning = true;
void checkEventsThread(EventLogger &myEvents, NotificationHandler &notifications, ParameterHandler::ParameterData &data) {
    // initialise calendar in thread
    // rquires initilisation of instance and also object separately - outlook instance is thread-specific
    CoInitialize(NULL);
    MCalendar myCalendar;

    using namespace std::chrono;

    auto lastRun = steady_clock::now() - seconds(data.updateTime); // force update on furst run

    while (checkEventsRunning) {
        auto now = steady_clock::now();

        if (now - lastRun >= seconds(data.updateTime)) { // if more time has passed than json file parameter specifies

            // get calendar events and parse for relevant events for the current device
            std::vector<CalendarEvent> events = getEvents(myCalendar);
            std::vector<CalendarEvent> relevantEvents;
            if (!events.empty()) {
                for (CalendarEvent& event : events) {
                    event.targetDevice = getTarget(event.description);

                    if (event.targetDevice == data.currentDevice) { // does it require checking date as well as time, or does following logfile write handle that already?
                        relevantEvents.push_back(event);
                    }
                }
            }

            // read old events from data file
            std::vector<CalendarEvent> previousEvents = formatLogFileToEvents(myEvents.readEvents());

            // merge notified flag into new events so that it persists every cycle rather than being overwritten
            // CHANGE THIS TO USE UNORDRERED SET? HOW MUCH MORE EFFICIENT?
            for (auto& newEvent : relevantEvents) {
                for (const auto& oldEvent : previousEvents) {
                    if (newEvent == oldEvent) {
                        newEvent.notified = oldEvent.notified;
                        break;
                    }
                }
            }

            // save new events to data file
            std::vector<EventLogger::CalendarEvent> logEvents = formatEventsToLogfile(relevantEvents);
            myEvents.writeEvents(logEvents);

            // create struct for easy return
            comparedEvents comparedevents;

            // parse for newly added events since last update
            compareCalendarEvents(relevantEvents, previousEvents, comparedevents.duplicateEvents, comparedevents.uniqueNewEvents);


            for (CalendarEvent& event : comparedevents.uniqueNewEvents) {
                // change string to wstring conversion to support chinese characters if time
                if (!event.notified) {
                    notifications.sendNotification(std::wstring(event.title.begin(), event.title.end()), std::wstring(event.description.begin(), event.description.end()));
                    // std::cout << "NEW EVENT NOTIFYING" << std::endl;
                    // more logic to display notification as an upcoming event rather than event about to happen
                }
            }

            { // new context for mutex lock
                std::lock_guard<std::mutex> lock(latestEventsMutex);
                latestEvents = comparedevents; // result;
            } // end mutex lock

            lastRun = now;
        }

        // slight delay for stability
        // how many ms should this be? - req. balance speed and cpu usage
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

    }

    // deinitialise the outlook instance
    CoUninitialize();
    
}




std::atomic<bool> upcomingEventRunning = true;
void notifyUpcomingEventThread(EventLogger &myEvents, NotificationHandler& notifications, ParameterHandler::ParameterData& data) {

    using namespace std::chrono;

    while (upcomingEventRunning) {
        auto now = system_clock::now();

        // create a copy in the format of std::tm
        // otherwise when intialising, std::tm gives garbage non-set values
        std::time_t now_copy = std::chrono::system_clock::to_time_t(now); 
        //std::tm today = *std::localtime(&now_copy);
        std::tm today;
        localtime_s(&today, &now_copy); // previous implementation was not thread_safe

        { // mutex lock for global latestEvents struct
            std::lock_guard<std::mutex> lock(latestEventsMutex);

            for (CalendarEvent& event : latestEvents.duplicateEvents) {
                // only update what is required from template 'today'
                std::tm event_time = today;
                event_time.tm_hour = event.startHour;
                event_time.tm_min = event.startMinute;
                event_time.tm_sec = 0; // another precaution to prevent accidental garbage values
                auto targetTime = system_clock::from_time_t(std::mktime(&event_time));

                if (!event.notified && targetTime - now <= seconds(data.reminderTime)) {
                    notifications.sendNotification(std::wstring(event.title.begin(), event.title.end()), std::wstring(event.description.begin(), event.description.end()));
                    event.notified = true;
                    std::vector<EventLogger::CalendarEvent> logEvents = formatEventsToLogfile(latestEvents.duplicateEvents);
                    myEvents.writeEvents(logEvents);
                    // std::cout << "OLD EVENTS NOTIFYING" << std::endl;
                }
            }
        } // end mutex lock

        std::this_thread::sleep_for(std::chrono::seconds(5));

    }
}




int main() {
#ifndef _WIN32
    std::cerr << "Does not support current operating system. Please run on Windows." << std::endl;
#endif

    // careful is blocking
    //UIWindow ui;
    //ui.Run();


    // set program basic parameters
    const std::string file_name = "DSOC-config.json"; // user config file filename
    const std::string data_file_name = "DSOC-data.data"; // program events data filename
    const std::wstring appID = L"DeviceSpecificMicrosoftCalendar"; // program id required for notifications
    const std::wstring exePath = NotificationHandler::getExecutablePath(); // program executable path to pass to notificationhandler
    const std::wstring shortcutPath = std::wstring(_wgetenv(L"APPDATA")) + L"\\Microsoft\\Windows\\Start Menu\\Programs\\" + std::filesystem::path(exePath).stem().wstring() + L".lnk"; // path to place link to program (req. for notifications)
    
    // initialise classes
    ParameterHandler myParams(file_name); // initialise the json parameter service
    ParameterHandler::ParameterData data = myParams.getData(); // get program settings from log file
    EventLogger myEvents(data_file_name); // initialise the json event logging service
    NotificationHandler notifications(appID, shortcutPath, exePath); // initialise the notification service



    // start the thread that constantly checks for new events
    std::thread checkEventsWorker(checkEventsThread,
        std::ref(myEvents),
        std::ref(notifications),
        std::ref(data)
    );

    // start the thread that will show events when they arrive
    std::thread notifyUpcomingEventWorker(notifyUpcomingEventThread,
        std::ref(myEvents),
        std::ref(notifications),
        std::ref(data)
    );



    // requried so first display of events is not null
    std::cout << "Waiting for Outlook Calendar to start..." << std::endl << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2)); 

    // std::cout << data.currentDevice << std::endl;



    while(1) {
        { // new context specifically for mutex lock
            std::lock_guard<std::mutex> lock(latestEventsMutex); // lock the context for safe data accessing

            // notifications.sendNotification(L"Test Notification", L"Test content of notification.");

            std::cout << "==============================================" << std::endl;
            std::cout << "Duplicate Events: " << std::endl;
            displayEvents(latestEvents.duplicateEvents);
            std::cout << std::endl << std::endl << "Newly Added Events" << std::endl;
            displayEvents(latestEvents.uniqueNewEvents);
            std::cout << "==============================================" << std::endl << std::endl << std::endl << std::endl;
        } // end mutex lock context

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }



    // stop all threads before exiting
    checkEventsRunning = false;
    upcomingEventRunning = false;
    checkEventsWorker.join();
    notifyUpcomingEventWorker.join();

    return 0;
}