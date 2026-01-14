#include <iostream>
#include <iomanip>
#include <cstdlib>


#include "MCalendar.h"
#include "ParameterHandler.h"

/*
    Format for device specificty:
    SENDTO: <device name> \r\n<Content of reminder>

*/

/*
    todo:
        change every instance of push_back to emplace_back?
        better return carriage detection for getTarget
        better sendto: detection
        fix copy of data inefficiency
*/

// allowing a certain keyword to automatically put a notification somewhere?
// categorisation keywords / analyse description to optimise for certain function

// set up user settable categories similar to outlook calendar categorise
// set up broker to allow outlook calendar categorisation fields to automatically put a notificaion to a certain device?


struct CalendarEvent {
    std::string title;
    SYSTEMTIME start;
    SYSTEMTIME end;
    std::string description;
    std::string targetDevice;
};

// helper for getting events from class into local struct
std::vector<CalendarEvent> getEvents(MCalendar& object) {
    _ItemsPtr pItems = object.getCalendarItems();
    std::vector<CalendarEvent> events; // = myCalendar.getTodaysEvents(pItems);
    std::vector<MCalendar::CalendarEvent> details = object.getTodaysEvents(pItems);
    for (size_t i = 0; i < details.size(); i++) {       // FIX: more efficient copying needed
        CalendarEvent event;
        event.title = details[i].title;
        event.start = details[i].start;
        event.end = details[i].end;
        event.description = details[i].description;

        events.push_back(event);
    }

    return events;
} 

std::string getTarget(std::string description) {
    std::string target;

    // carriage return detection - see format for device specifity
    for (char c : description) {
        //std::cout << std::hex << (int)(unsigned char)i << std::endl;
        if (c == '\r' || c == '\n') break; 
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

    /*
    if (target.length() > 7 && (target.substr(0, 7) == "SENDTO:" || target.substr(0, 7) == "sendto:" || target.substr(0, 7) == "Sendto:")) {
        if (target[7] == ' ') {
            return target.substr(8);
        }
        return target.substr(7);
    }
    else if (target.length() > 8 && (target.substr(0, 8) == "SENDTO: " || target.substr(0, 7) == "sendto:" || target.substr(0, 7) == "Sendto:")) {
        return target.substr(8);
    }
    else {
        return "No target selected or wrong format.";
    }*/

    //std::cout << target << std::endl;

    return "";
}


void displayEvents(std::vector<CalendarEvent> events) {
    SYSTEMTIME today = MCalendar::getDate();

    /*
    if (!events.empty()) {
        for (size_t i = 0; i < events.size(); i++) {
            events[i].targetDevice = getTarget(events[i].description);
        }
    }
    */

    std::cout << "Today's date is " << today.wYear << "/" 
        << std::setw(2) << std::setfill('0') << today.wMonth << "/" 
        << std::setw(2) << std::setfill('0') << today.wDay 
        << std::endl << std::endl;
    // std::cout << std::endl;

    // std::cout << "Today's Calendar Events:" << std::endl;
    // std::cout << "Found " << events.size() << " events." << std::endl << std::endl;

    if (!events.empty()) {
        std::cout << "Today's Calendar:" << std::endl;
        std::cout << "Found " << events.size() << " events." << std::endl << std::endl;

        for (size_t i = 0; i < events.size(); i++) {
            const CalendarEvent& event = events[i];


            std::cout << "Title: " << event.title << std::endl;
            std::cout << "Time: "
                << std::setw(2) << std::setfill('0') << event.start.wHour << ":"
                << std::setw(2) << std::setfill('0') << event.start.wMinute << " - "
                << std::setw(2) << std::setfill('0') << event.end.wHour << ":"
                << std::setw(2) << std::setfill('0') << event.end.wMinute << std::endl;

            if (!event.description.empty()) {
                // std::cout << "Description: " << event.description << std::endl;
                std::cout << "Target: " << event.targetDevice << std::endl;
            }
            std::cout << "----------------------------------------" << std::endl;
        }
    }
    else {
        std::cout << "No events today." << std::endl;
    }

}

int main() {
    const std::string file_name = "DSOC-config.json";

    MCalendar myCalendar;
    ParameterHandler myParams(file_name);

    ParameterHandler::ParameterData data = myParams.getData();

    // std::cout << "Device Name: " << data.currentDevice << std::endl;
    // std::cout << "Enable Automatic Optimisations: " << data.enableAutomaticOptimisations << std::endl;



    std::vector<CalendarEvent> events = getEvents(myCalendar);
    if (!events.empty()) {
        for (CalendarEvent& event : events) {
            event.targetDevice = getTarget(event.description);
        }
    }

    std::vector<CalendarEvent> relevantEvents;
    for (CalendarEvent& event : events) {
        // const CalendarEvent& event = events[i];

        if (event.targetDevice == data.currentDevice) {
            relevantEvents.push_back(event);
        }
    }

    // std::cout << events[0].targetDevice << std::endl;

    displayEvents(events);
    std::cout << "==============================================================" << std::endl;
    displayEvents(relevantEvents);

 
    

    return 0;
}