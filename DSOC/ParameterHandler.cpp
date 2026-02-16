#include "ParameterHandler.h"

ParameterHandler::ParameterHandler(const std::string& file_name, const std::string& data_file_name) {
	fileName = file_name;
    dataFileName = data_file_name;

    if (!fileExists()) {
        std::ofstream file;

        file.open(fileName, std::ios::app);

        file << std::setw(4) << params;
        file.close();
        std::cerr << "Please input program parameters into .json file." << std::endl;
        exit(21);
    }

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

// returns if file exists
bool ParameterHandler::fileExists() {
    std::ifstream checkFile(fileName);
    bool fileExists = checkFile.good();
    checkFile.close();

    return fileExists;
}

// gets and returns program settings
ParameterHandler::ParameterData ParameterHandler::getData() {
    std::ifstream file(fileName);
    ParameterData data;

    try {
        file >> loaded_params;
        file.close();

        data.currentDevice = loaded_params["parameters"]["device_name"];
        data.enableAutomaticOptimisations = loaded_params["parameters"]["enable_automatic_optimisations"] != 0;
        data.updateTime = loaded_params["parameters"]["update_time"];
        data.reminderTime = loaded_params["parameters"]["reminder_time"];

    } catch (const json::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        std::exit(23);
    }

    return data;
}

// writes events to data file
void ParameterHandler::writeEvents(std::vector<CalendarEvent>& events) {
    json data;

    for (const auto& event : events) { // is auto typesafe for all use cases of this function?
        data[std::to_string(event.index)] = event;
    }

    std::ofstream file(dataFileName);
    file << data.dump(4);

}

// reads events from data file
std::vector<ParameterHandler::CalendarEvent> ParameterHandler::readEvents() {
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
