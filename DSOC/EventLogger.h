#pragma once

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

class EventLogger {
public:
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
		bool notified = false;
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
			{"notified", event.notified},
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
		j.at("notified").get_to(event.notified);
	};


	EventLogger(const std::string& data_file_name);

	void writeEvents(std::vector<CalendarEvent>& events);
	std::vector<CalendarEvent> readEvents();

private:
	std::string dataFileName;
};
