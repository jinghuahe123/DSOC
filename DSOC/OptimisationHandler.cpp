#include "OptimisationHandler.h"


OptimisationHandler::OptimisationHandler(const std::string& optimisation_file_name) {
	optimisationFileName = optimisation_file_name;
}

// creates file if not existing already
void OptimisationHandler::init() {
    if (!fileExists()) {
        std::vector<OptimisationData> data;

        // write two pieces of test data as a template on first run for user to know how to input data
        data.emplace_back(0, "keyword1", "testDevice1");
        data.emplace_back(1, "keyword2", "testDevice2");
        writeOptimisationData(data);

        std::cerr << "Please input program optimisation data into .json file." << std::endl;
        MessageBoxA(NULL, "Please input program optimisation data into .json file.", "ERROR", MB_OK); // error box to tell user to imput optimsation data before running
        exit(27);
    }
}

// check if file exists
bool OptimisationHandler::fileExists() {
    std::ifstream checkFile(optimisationFileName);
    bool fileExists = checkFile.good();
    checkFile.close();

    return fileExists;
}

// for writing the initial data to a file
void OptimisationHandler::writeOptimisationData(std::vector<OptimisationData>& optimisationData) {
    json data;
    
    // will only ever be passed two pieces of data, vector is not very efficient?
    for (const auto& d : optimisationData) {
        std::string key = std::to_string(d.index); // turns the json key to a string for writing
        data[key] = d; // where to_json is used internally
    }

    // write the file
    std::ofstream file(optimisationFileName);
    file << data.dump(4);
    file.close();

}

// for reading the user-set optimisations
std::vector<OptimisationHandler::OptimisationData> OptimisationHandler::readOptimisationData() {
    std::ifstream file(optimisationFileName);
    json data;
    file >> data; // dump file into json object

    std::vector<OptimisationData> optimisationData;

    for (auto& [key, value] : data.items()) {
        OptimisationData d = value.get<OptimisationData>(); // from_json used internally
        d.index = std::stoi(key);
        optimisationData.push_back(d); // could easily be replaced with emplace_back for speed?
    }

    return optimisationData;
}