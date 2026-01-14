#include "ParameterHandler.h"

ParameterHandler::ParameterHandler(const std::string& file_name) {
	fileName = file_name;

    if (!fileExists()) {
        std::ofstream file;

        file.open(fileName, std::ios::app);

        file << std::setw(4) << data;
        file.close();
        std::cerr << "Please input program parameters into .json file." << std::endl;
        exit(21);
    }
}

bool ParameterHandler::fileExists() {
    std::ifstream checkFile(fileName);
    bool fileExists = checkFile.good();
    checkFile.close();

    return fileExists;
}

ParameterHandler::ParameterData ParameterHandler::getData() {
    std::ifstream file(fileName);
    ParameterData data;

    try {

    //file.open(fileName);

    file >> loaded_data;
    file.close();

    data.currentDevice = loaded_data["parameters"]["device_name"];
    data.enableAutomaticOptimisations = loaded_data["parameters"]["enable_automatic_optimisations"] != 0;
    // int optValue = loaded_data["parameters"]["enable_automatic_optimisations"];
    // data.enableAutomaticOptimisations = (optValue != 0);
    data.updateTime = loaded_data["parameters"]["update_time"];
    data.reminderTime = loaded_data["parameters"]["reminder_time"];

    

    /*
    if (!file.is_open()) {
        std::cerr << "Error opening configuration file (" << fileName << ")." << std::endl;
        std::exit(22);
    }
    else {

    }
    */

    }
    catch (const json::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        std::exit(23);
    }

    return data;
}