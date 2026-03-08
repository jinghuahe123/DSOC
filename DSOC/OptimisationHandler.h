#pragma once

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

class OptimisationHandler {
public:
	struct OptimisationData {
		int index;
		std::string targetDevice;
		std::string keyword;
	};

	using json = nlohmann::json;

	// internally used by nlohmann for turning OptimisationData object to json object
	friend void to_json(json& j, const OptimisationData& data) {
		j = json{
			{"target", data.targetDevice},
			{"keyword", data.keyword},
		};
	};

	// internally used by nlohmann for turning json object to OptimisationData object
	friend void from_json(const json& j, OptimisationData& data) {
		j.at("target").get_to(data.targetDevice);
		j.at("keyword").get_to(data.keyword);
	};

	OptimisationHandler(const std::string& optimisation_file_name);
	
	void init(); // need to create instance of class and initialise them separately depending on the main parameters
	std::vector<OptimisationData> readOptimisationData();

private:
	std::string optimisationFileName;

	bool fileExists();
	void writeOptimisationData(std::vector<OptimisationData>& optimisationData);
};

