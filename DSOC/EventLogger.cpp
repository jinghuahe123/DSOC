#include "EventLogger.h"

EventLogger::EventLogger(const std::string& data_file_name) {
    dataFileName = data_file_name;

    // error checking: make sure data file exists for reading files later
    std::ifstream dataCheck(dataFileName);
    if (!dataCheck.good()) {
        dataCheck.close();
        std::ofstream dataFile(dataFileName);
        dataFile << "{}";
        dataFile.close();
    }
    else {
        dataCheck.close();
    }
}

// writes events to data file
void EventLogger::writeEvents(std::vector<CalendarEvent>& events) {
    json data;

    for (const auto& event : events) { // is auto typesafe for all use cases of this function?
        data[std::to_string(event.index)] = event;
    }

    std::ofstream file(dataFileName);
    file << data.dump(4);

}

// reads events from data file
std::vector<EventLogger::CalendarEvent> EventLogger::readEvents() {
    std::ifstream file(dataFileName);
    json data;
    file >> data;

    std::vector<CalendarEvent> events;

    for (auto& [key, value] : data.items()) { // is auto typesafe for this?
        CalendarEvent event;
        event.index = std::stoi(key);
        event.title = value["title"];
        event.targetDevice = value["target"];
        event.startHour = value["startHour"];
        event.startMinute = value["startMinute"];
        event.endHour = value["endHour"];
        event.endMinute = value["endMinute"];
        event.description = value["description"];
        event.notified = value["notified"];

        events.push_back(event);
    }

    return events;
}