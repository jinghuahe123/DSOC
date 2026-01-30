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

	// struct for logged events
	struct CalendarEvent {
		int index;
		std::string title;
		int startHour;
		int startMinute;
		int endHour;
		int endMinute;
		std::string description;
		std::string targetDevice;
	};



	using json = nlohmann::json;
	// write events to data file
	// figure out why requires friend attribute to compile
	friend void to_json(json& j, const CalendarEvent event) {
		j = json{
			{"title", event.title},
			{"startHour", event.startHour},
			{"startMinute", event.startMinute},
			{"endHour", event.endHour},
			{"endMinute", event.endMinute},
			{"target", event.targetDevice},
			{"description", event.description},
		};
	};

	// read events from data file
	friend void from_json(const json& j, CalendarEvent event) {
		j.at("title").get_to(event.title);
		j.at("startHour").get_to(event.startHour);
		j.at("startMinute").get_to(event.startMinute);
		j.at("endHour").get_to(event.endHour);
		j.at("endMinute").get_to(event.endMinute);
		j.at("target").get_to(event.targetDevice);
		j.at("description").get_to(event.description);
	};

	// NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CalendarEvent, index, title, description, date)

	ParameterHandler(const std::string& file_name, const std::string& data_file_name);

	ParameterData getData();
	void writeEvents(std::vector<CalendarEvent>& events);
	std::vector<CalendarEvent> readEvents();

private:
	std::string fileName;
	std::string dataFileName;

	using json = nlohmann::json;
	// default program settings
	json params = {
		{"parameters", {
			{"device_name", ""},
			{"enable_automatic_optimisations", true},
			{"update_time", 300}, // 5*60 seconds; 5mins
			{"reminder_time", 600}, // 10*60 seconds; 10mins
		}}
	};
	json loaded_params;

	bool fileExists();
};

