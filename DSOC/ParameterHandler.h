#pragma once

#include <Windows.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

// https://github.com/nlohmann/json // Main Github for nlohmann
// https://json.nlohmann.me/features/arbitrary_types/ // for to_json and from_json implementations

class ParameterHandler {
public:
	// struct for program settings
	struct ParameterData {
		std::string currentDevice;
		bool enableAutomaticOptimisations;
		unsigned long updateTime;
		unsigned long reminderTime;
		bool consoleEnabled;
		int startHour;
		int startMinute;
		int endHour;
		int endMinute;
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
			{"device_name", ""}, // unique device name for filtering
			{"enable_automatic_optimisations", true}, // whether keyword checking is enabled
			{"update_time", 15}, // 15 seconds - how often the thread to get new events should update
			{"reminder_time", 600}, // 10*60 seconds; 10mins - how long before an event a notification should occur
			{"console", false}, // debugging console enabled flag
			{"start_hour", 0}, // program active from this hour
			{"start_minute", 0}, // program active from this minute
			{"end_hour", 0}, // program deactive from this hour
			{"end_minute", 0} // program deactive from this minute
		}}
	};
	json loaded_params;

	bool fileExists();
};

