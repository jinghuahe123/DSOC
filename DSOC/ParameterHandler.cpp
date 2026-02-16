#include "ParameterHandler.h"

ParameterHandler::ParameterHandler(const std::string& file_name) {
	fileName = file_name;

    if (!fileExists()) {
        std::ofstream file;

        file.open(fileName, std::ios::app);

        file << std::setw(4) << params;
        file.close();
        std::cerr << "Please input program parameters into .json file." << std::endl;
        exit(21);
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


