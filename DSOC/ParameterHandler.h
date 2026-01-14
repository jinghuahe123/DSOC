#pragma once

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>


class ParameterHandler {
public:
	struct ParameterData {
		std::string currentDevice;
		bool enableAutomaticOptimisations;
		unsigned long updateTime;
		unsigned long reminderTime;
	};

	ParameterHandler(const std::string& file_name);

	ParameterData getData();

private:
	std::string fileName;

	using json = nlohmann::json;
	json data = {
		{"parameters", {
			{"device_name", ""},
			{"enable_automatic_optimisations", true},
			{"update_time", 300}, // 5*60 seconds; 5mins
			{"reminder_time", 600}, // 10*60 seconds; 10mins
		}}
	};
	json loaded_data;

	bool fileExists();
};

