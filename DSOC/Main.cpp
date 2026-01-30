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

// #include "NotificationHandler.h"

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

        events.push_back(event);
    }

    return events;
} 

// helper for formatting local events into log file format
std::vector<ParameterHandler::CalendarEvent> formatEventsToLogfile(std::vector<CalendarEvent> events) {
    std::vector<ParameterHandler::CalendarEvent> logEvents;

    for (CalendarEvent event : events) {
        ParameterHandler::CalendarEvent logEvent;
        logEvent.title = event.title;
        logEvent.startHour = event.startHour;
        logEvent.startMinute = event.startMinute;
        logEvent.endHour = event.endHour;
        logEvent.endMinute = event.endMinute;
        logEvent.targetDevice = event.targetDevice;
        logEvent.description = event.description;

        logEvents.push_back(logEvent);
    }
    for (int i = 0; i < logEvents.size(); i++) {
        logEvents[i].index = i;
    }
    
    return logEvents;
}

// helper for formatting log file events into local events format
std::vector<CalendarEvent> formatLogFileToEvents(std::vector<ParameterHandler::CalendarEvent> events) {
    std::vector<CalendarEvent> logEvents;

    for (ParameterHandler::CalendarEvent event : events) {
        CalendarEvent logEvent;
        logEvent.title = event.title;
        logEvent.startHour = event.startHour;
        logEvent.startMinute = event.startMinute;
        logEvent.endHour = event.endHour;
        logEvent.endMinute = event.endMinute;
        logEvent.targetDevice = event.targetDevice;
        logEvent.description = event.description;

        logEvents.push_back(logEvent);
    }

    return logEvents;
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

// debugging use
// displays to console details of a vector of events
void displayEvents(std::vector<CalendarEvent> events) { 
    SYSTEMTIME today = MCalendar::getDate();

    std::cout << "Today's date is " << today.wYear << "/" 
        << std::setw(2) << std::setfill('0') << today.wMonth << "/" 
        << std::setw(2) << std::setfill('0') << today.wDay 
        << std::endl << std::endl;
    // std::cout << std::endl;

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

// thread locks and parameters
std::mutex checkEventsMutex;
comparedEvents latestEvents; // global variable for struct of new and old events to access between threads
std::atomic<bool> checkEventsRunning = true;


comparedEvents checkEvents(MCalendar& myCalendar, ParameterHandler& myParams, ParameterHandler::ParameterData& data) {
    // get calendar events and parse for relevant events for the current device
    std::vector<CalendarEvent> events = getEvents(myCalendar);
    // displayEvents(events);
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
    std::vector<CalendarEvent> previousEvents = formatLogFileToEvents(myParams.readEvents());
    // save new events to data file
    std::vector<ParameterHandler::CalendarEvent> logEvents = formatEventsToLogfile(relevantEvents);
    myParams.writeEvents(logEvents);

    // create struct for easy return
    comparedEvents comparedevents;

    // parse for newly added events since last update
    // std::vector<CalendarEvent> duplicateEvents;
    // std::vector<CalendarEvent> uniqueNewEvents;
    compareCalendarEvents(relevantEvents, previousEvents, comparedevents.duplicateEvents, comparedevents.uniqueNewEvents);

    return comparedevents;
}

void checkEventsThread(ParameterHandler &myParams, ParameterHandler::ParameterData &data) {
    // initialise calendar in thread
    // rquires initilisation of instance and also object separately - outlook instance is thread-specific
    CoInitialize(NULL);
    MCalendar myCalendar;

    using namespace std::chrono;

    auto lastRun = steady_clock::now() - seconds(data.updateTime); // force update on furst run

    while (checkEventsRunning) {
        auto now = steady_clock::now();

        if (now - lastRun >= seconds(data.updateTime)) { // if more time has passed than json file parameter specifies

            comparedEvents result = checkEvents(myCalendar, myParams, data);

            { // new context for mutex lock
                std::lock_guard<std::mutex> lock(checkEventsMutex);
                latestEvents = result;
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



int main() {
    // set program basic parameters
    const std::string file_name = "DSOC-config.json"; // user config file filename
    const std::string data_file_name = "DSOC-data.data"; // program events data filename
    // const std::wstring appID = L"DeviceSpecificMicrosoftCalendar";

    // initialise objets
    // MCalendar myCalendar;
    ParameterHandler myParams(file_name, data_file_name);

    // get program settings from log file
    ParameterHandler::ParameterData data = myParams.getData();

    // start the thread that constantly checks for new events
    std::thread worker(checkEventsThread,
        std::ref(myParams),
        std::ref(data));

    // requried so first display of events is not null
    std::cout << "Waiting for Outlook Calendar to start..." << std::endl << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2)); 

    // std::cout << data.currentDevice << std::endl;

    while(1) {
        { // new context specifically for mutex lock
            std::lock_guard<std::mutex> lock(checkEventsMutex); // lock the context for safe data accessing

            std::cout << "Duplicate Events: " << std::endl;
            displayEvents(latestEvents.duplicateEvents);
            std::cout << std::endl << std::endl << "Newly Added Events" << std::endl;
            displayEvents(latestEvents.uniqueNewEvents);
            std::cout << std::endl << std::endl << std::endl << std::endl;
        } // end mutex lock context

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    // stop all threads before exiting
    checkEventsRunning = false;
    worker.join();

    

    return 0;
}

