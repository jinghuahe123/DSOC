# DSOC (Device-Specific Outlook Calendar)

A City of London School SPARC Project submitted to CREST.

## Introduction

This piece of software adds device-specific notifications to Outlook Calendar. As of writing this software, the official Microsoft implementation only allows configuring generic global settings for each device, rather than per-notification control. 

DSOC gives the user full flexibility of exactly which device a notification will be sent to. Additionally, it also allows for scheduling a queue of notifications for an offline device to show the next time it is powered back on. This is useful if the user desires setting different reminders to independent workstations. 

*A example of a use-case: an office worker has his work email and calendar logged in on his personal device to ensure he can recieve on-call meeting notifications, but he prefers not to be notified of non-urgent messages on his personal devices. DSOC can specifically reroute emergency meetings to his personal device, but filter everything else to his work devices.*

*A second example: a student prefers to work on a more powerful desktop tower where possible, but requires using a laptop during lectures. DSOC allows the student to set reminders for certain assignments to the tower computer so they will show when he returns to his accomodation, even if his tower computer is powered off when he sends the reminder.*

## Features

- Simple, scalable configuration for device-specificity
- ~Automatic, user-customisable, event grouping based on keywords for simpler configurations~ (in progress)
- Unobtrusive background application
- Full offline support

## DSOC Prerequisites 

**Currently, DSOC only supports Windows Clients**

Due to offline support, Classic Microsoft Outlook is required for this application to function. 

This is usually pre-installed alongside the Microsoft Office Suite. If this is not included on your device, please download it from [here](https://support.microsoft.com/en-gb/office/install-or-reinstall-classic-outlook-on-a-windows-pc-5c94902b-31a5-4274-abb0-b07f4661edf5).

More documentation will be added accordingly as the project progresses. 
