# DSOC (Device-Specific Outlook Calendar)

A City of London School SPARC Project submitted to CREST.

## Introduction

This piece of software adds device-specific notifications to Outlook Calendar. As of writing this software, the official Microsoft implementation only allows configuring generic global settings for each device, rather than per-notification control. 

DSOC gives the user full flexibility of exactly which device a notification will be sent to. Additionally, it also allows for scheduling a queue of notifications for an offline device to show the next time it is powered back on. This is useful if the user desires setting different reminders to independent workstations. 

*A example of a use-case: an office worker has his work email and calendar logged in on his personal device to ensure he can recieve on-call meeting notifications, but he prefers not to be notified of non-urgent messages on his personal devices. DSOC can specifically reroute emergency meetings to his personal device, but filter everything else to his work devices.*


## Features

- Simple, scalable configuration for device-specificity
- Automatic, user-customisable, event grouping based on keywords for simpler configurations
- Unobtrusive background application
- Full offline support


## DSOC Prerequisites 

**Currently, DSOC only supports Windows Clients**

Due to offline support, Classic Microsoft Outlook is required for this application to function. 

This is usually pre-installed alongside the Microsoft Office Suite. If this is not included on your device, please download it from [here](https://support.microsoft.com/en-gb/office/install-or-reinstall-classic-outlook-on-a-windows-pc-5c94902b-31a5-4274-abb0-b07f4661edf5).

Microsoft Visual C++ Redistributable for Visual Studio 2015–2022 (x64) is also required for the non-standalone release. Please download it from [Microsoft Visual C++ Redistributable for Visual Studio 2015–2022 (x64).](https://www.microsoft.com/en-gb/download/details.aspx?id=48145)


## Usage

On the first run, DSOC will generate a configuration file called 'DSOC-config.json'. Fill out the fields and run the program again.

| Parameter                         | Description                                                             |
| --------------------------------- | ----------------------------------------------------------------------- |
| _console_                         | _Boolean for whether console window should spawn when run (debugging)_  |
| device_name                       | Unique ID for each computer                                             |
| enable_automatic_optimisations    | Toggles the automatic event groupings.                                  |
| update_time                       | Time (seconds) between updating the event cache from Outlook.           |
| reminder_time                     | Time (seconds) before an event starts to send a reminder.               |
| start_hour                        | (24hr) Hour that the program will begin filtering events.               |
| start_minute                      | Minute that the program will begin filtering events.                    |
| end_hour                          | (24hr) Hour that the program will stop filtering events.                |
| end_minute                        | Minute that the program will stop filtering events.                     |

If automatic optimisations are enabled, DSOC will then also generate a file called 'optimisations.json'. Fill out the fields, and copy paste the existing format if more fields are required.

**Please note**, the 'key' (leading number before each block of optimisation data) has to be unique for each piece of data. The easiest way to implement this is just to label them chronologically starting from 0.

| Parameter    | Description                                                         |
| ------------ | ------------------------------------------------------------------- |
| keyword      | The word/phrase that will be scanned for in the event.              |
| target       | The ID of the computer to send the notification to.                 |

If an event contains a keyword that has been set, DSOC will route that event to the specified target, so that the 'SENDTO: ' header is not always required. 

Once these are filled out, run the program a third time to start it. 

To Quit the program, press the sequence of keys: **Ctrl+Shift+Q**

## Error Codes

| Code | Description                                                                 |
| ---- | --------------------------------------------------------------------------- |
| 10   | Failed to initialise Outlook Calendar Object.                               | 
| 21   | No parameters in .json config file / config file corrupt.                   |
| 23   | Failed to load parameters from config file.                                 |
| 27   | No parameters in .json optimisation file / optimisation file corrupt.       |
| 31   | Failed to register notification service.                                    |

