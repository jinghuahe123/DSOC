#pragma once

#include <Windows.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

class ParameterHandler {
public:
	// struct for program settings
	struct ParameterData {
		std::string currentDevice;
		bool enableAutomaticOptimisations;
		unsigned long updateTime;
		unsigned long reminderTime;
	};


	// using json = nlohmann::json;

	ParameterHandler(const std::string& file_name);

	ParameterData getData();

private:
	std::string fileName;

	using json = nlohmann::json;
	// default program settings
	json params = {
		{"parameters", {
			{"device_name", ""},
			{"enable_automatic_optimisations", true},
			{"update_time", 15}, // 15 seconds - how often the thread to get new events should update
			{"reminder_time", 600}, // 10*60 seconds; 10mins - how long before an event a notification should occur
		}}
	};
	json loaded_params;

	bool fileExists();
};

