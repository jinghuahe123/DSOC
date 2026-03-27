#include "EventLogger.h"

EventLogger::EventLogger(const std::string& data_file_name) {
    dataFileName = data_file_name;

    // error checking: make sure data file exists for reading files later
    std::ifstream dataCheck(dataFileName); // opens file for reading
    if (!dataCheck.good()) { // if data file doesn't exist
        dataCheck.close();
        std::ofstream dataFile(dataFileName); // opens file for writing
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

    // loop through each event, and turn them into json objects
    for (const auto& event : events) { 
        std::string key = std::to_string(event.index); // a number classifier that is assigned to each event
        data[key] = event; // to_json is called here
    }

    // write the json objects to the file
    std::ofstream file(dataFileName);
    file << data.dump(4);

}

// reads events from data file
std::vector<EventLogger::CalendarEvent> EventLogger::readEvents() {
    std::ifstream file(dataFileName); // opens file for reading
    json data;
    file >> data; // put the file contents into the JSON object

    std::vector<CalendarEvent> events;

    // transforms JSON into a vector of CalendarEvents
    // [key, value] destructures a json block (e.g. {1, <json_object}) into an index 'key' and stored data 'values'
    for (auto& [key, value] : data.items()) { 
        // DEPRECATED to favour from_json rather than manual assignment
        /*
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
        */

        // loop through the json objects and turn them into CalendarEvent structs
        CalendarEvent event = value.get<CalendarEvent>(); // from_json called here
        event.index = std::stoi(key); // string to integer
        events.push_back(event);
    }

    return events;
}