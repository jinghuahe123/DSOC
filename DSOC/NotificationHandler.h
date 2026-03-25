#pragma once

#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include <winrt/Windows.Data.Xml.Dom.h> 
#include <winrt/Windows.UI.Notifications.h> 
#include <winrt/Windows.Foundation.h> 
#include <shobjidl.h>
#include <propkey.h>
#include <propvarutil.h>
#include <string>

class NotificationHandler {
public:
	inline static bool appInitialised = false; // static as each app needs to be registerd once not per object instance
	static std::wstring getExecutablePath();

	NotificationHandler(const std::wstring& appId, const std::wstring& shortcutPath, const std::wstring exePath);
	
	void sendNotification(const std::wstring& title, const std::wstring& message, int startHour, int startMinute);
	void sendNotification(const std::wstring& title, const std::wstring& message);

private:
	
	std::wstring APP_ID;
	std::wstring appShortcutPath;
	std::wstring executablePath;

	bool registerAppID();
	bool createAppIDShortcut();
};

