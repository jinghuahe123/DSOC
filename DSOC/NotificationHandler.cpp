#include "NotificationHandler.h"
// #include <wtypes.h>

std::wstring NotificationHandler::getExecutablePath() {
    wchar_t buffer[MAX_PATH];
    DWORD size = GetModuleFileNameW(nullptr, buffer, MAX_PATH);

    return std::wstring(buffer, size);
}

NotificationHandler::NotificationHandler(const std::wstring& appId, const std::wstring& shortcutPath, const std::wstring exePath) {
#ifndef _WIN32
    std::cerr << "Does not support current operating system. Please run on Windows." << std::endl;
#endif

	APP_ID = appId;
    appShortcutPath = shortcutPath;
    executablePath = exePath;

    // initialise the windows notification api
	//winrt::init_apartment();
    winrt::init_apartment(winrt::apartment_type::multi_threaded);
	SetCurrentProcessExplicitAppUserModelID(APP_ID.c_str()); // is this required if registry is set?

    // checks if notification api has been registered already (initialisation per application not per thread)
    // if not registered, set app ID in registry, and create start menu shortcut
    // annoyingly both are required for windows to display a notification
    if (!appInitialised && (!registerAppID() || !createAppIDShortcut())) {
        std::cerr << "App registration failed. \n";
        std::exit(31);
    }
}

// registers the app into the registry
bool NotificationHandler::registerAppID() {
    // variables for the key and the registry path to put it in
	HKEY hKey; // 'Handle to Registry Key', like a pointer to the registry key - represents the open key
	std::wstring registryKeyPath = L"Software\\Classes\\" + APP_ID;

    // create/open the key
    // hKeyStatus stores the error code thrown by RegCreateKeyExW and RegSetValueExW
    LONG hKeyStatus = RegCreateKeyExW(
        HKEY_CURRENT_USER, // root hive where the key is created
        registryKeyPath.c_str(), // path within the root hive to create the key to
        0, // always 0
        nullptr, // default, never used
        REG_OPTION_NON_VOLATILE, // key is not deleted when rebooting
        KEY_WRITE, // permission parameter to open key for writing
        nullptr, // default security
        &hKey, // handle to point to the new key
        nullptr // default, not needed
    );

    if (hKeyStatus != ERROR_SUCCESS) {
        std::cerr << "Unable to open registry for writing. \n";
        return false;
    }

    // write the app id value to the key AppUserModelID which is referenced as hKey
    hKeyStatus = RegSetValueExW(
        hKey, // passes the pointer to the key created
        L"AppUserModelID", // name of the value (type String) to write
        0, // always 0
        REG_SZ, // defines a null termination for the String
        reinterpret_cast<const BYTE*>(APP_ID.c_str()), // data to write (APP_ID) casted to bytes
        static_cast<DWORD>((APP_ID.size() + 1) * sizeof(wchar_t)) // calculates the size of the written data
    );

    RegCloseKey(hKey); // closes the key 
    appInitialised = true;
    return hKeyStatus == ERROR_SUCCESS;
}

bool NotificationHandler::createAppIDShortcut() {
    CoInitialize(nullptr);

    // hresult is written to when an operation success/failure is logged
    // shellLink is written to to configure the shorcut itself

    // create a shorcut
    // Microsoft refers to these as shell links
    IShellLinkW* shellLink = nullptr; // shellLink refers to an in-memory (cached) .lnk file
    HRESULT hresult = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&shellLink)); // constructs shellLink into a COM object
    if (FAILED(hresult)) return false;

    shellLink->SetPath(executablePath.c_str()); // sets the executable path the link points to
    shellLink->SetWorkingDirectory(L"."); // sets working directory when exe is launched i.e. the directory the program should look for its data files

    // gets the IPropertyStore interface of the shortcut (where the AppUserModelID will be stored)
    IPropertyStore* propStore = nullptr; // pointer to the properties store of the shortcut
    hresult = shellLink->QueryInterface(IID_PPV_ARGS(&propStore));
    if (FAILED(hresult)) return false;

    PROPVARIANT pv; // a struct that stores a certain property's value
    InitPropVariantFromString(APP_ID.c_str(), &pv); // fills the struct with the APP_ID (AppUserModelID)
    propStore->SetValue(PKEY_AppUserModel_ID, pv); // writes the set value into the shortcut under PKEY_AppUserModel_ID
    propStore->Commit(); // saves changes to the shortcut object
    PropVariantClear(&pv);

    // gets the IPersistFile interface to allow writing changes to disk
    IPersistFile* persistFile = nullptr; // pointer to the interface
    hresult = shellLink->QueryInterface(IID_PPV_ARGS(&persistFile));
    if (FAILED(hresult)) return false;

    // writes the shortcut to the disk (TRUE flag overwrites previous shortcut if it exists)
    hresult = persistFile->Save(appShortcutPath.c_str(), TRUE);

    // avoids memory leaks
    persistFile->Release();
    propStore->Release();
    shellLink->Release();
    CoUninitialize();

    appInitialised = true;

    return SUCCEEDED(hresult);
}

void NotificationHandler::sendNotification(const std::wstring& title, const std::wstring& message) {
	using namespace winrt;
	using namespace Windows::UI::Notifications;
	using namespace winrt::Windows::Data::Xml::Dom;

	// notification in xml format
    // could turn this into a separate document, but don't want a random .xml file floating around, accessable to the user?
	std::wstring xmlPayload = LR"(<toast> <visual> <binding template="ToastGeneric"> <text>)" + title + LR"(</text> <text>)" + message + LR"(</text> </binding> </visual> </toast>)";

	// parse as xml
	XmlDocument xml;
	xml.LoadXml(xmlPayload);

    // create the notification element
	ToastNotification toast(xml);

	// create the notifing action
	ToastNotifier notifier = ToastNotificationManager::CreateToastNotifier(APP_ID);

	// push the notification
	notifier.Show(toast);
	
}