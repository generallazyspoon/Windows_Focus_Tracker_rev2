/// Feature request (finish other things first):
// Mobile override selector (for Android)
// Chrono range viewing
// Change displayData to be more responsive to keys by displaying data first, before imposing loop wait cycles (and cancelling early if a key or trigger is found) [or moving current wait cycles to end of displayData?]
// Manual Data save
// Add another tracking form (like conduits, but...?)
// Add automatic nonsense-sensing:  Auto-handle focuses with < x time:
//		1-Check for time constraint (Long interval)
//		2-If focus < x time, increment flag for special handling bool, else reset flags
//		3-If y [5?] # of flags reached, set special handling bool to true, skip 4
//		4-goto 1 as necessary
// ^How to handle applicable focuses?
// ^Ignore them for menu displays?
// ^Auto-blacklist class or title?
// ^Just set a minimum of 5 minutes (reset every day?) for tracking focuses?
// ^^If it doesn't make it into the minimum, push into a special focus for... "Various intermediary focuses"? [I kind of like this one the best]
// Auto-scanning for multiple items, putting similar items in a [white/black][titleClass][group][what?]

// Override creation rework:
/*

title:
description:
overridesAFK?
spotlight (if available)
set conduits?
	.set conduits
set focus limits?
	.set focus limits

*/

// Add More Keys to AFK Detection (or find a way to detect any keystrokes whatsoever)
// Add mouse buttons to AFK detection

// Fix AFK detection to report AFK - AFK

// function list:
/*

^ = key assignment coded
_ = function programmed
& = info written (or intentionally skipped)

^_&(a) AFK detection override >> target focus, turn on/off
^_&(c) conduit management >> target focus, toggle bool using scrolling select
^_&(d) description management >> view, modify
   (f) filter management >> add filter by type, clean up old files >> change keyword priority using scrolling grab & move interface
^_ (g) manage groups >> transfer hotkey assignment to "g"
   (h) toggle groups displayed in the tracker >> use a toggleSelect w/no max
 _&(i) info >> simple menu [in progress]
  &(l) focus limits >> create UI::selectorRange or modify UI::selector to take a range instead of just max
^_&(o) override toggle >> transfer hotkey assignment to "o"
^_&(p) override management >> transfer hotkey assignment to "p"
^_&(q) conduit mode toggle >> transfer hotkey assignment to "q"
   (r) warning timings >> select sounds or select focus -- select warning timing scheme (scaled against focus limits)
^_ (s) spotlight five management >> scrolling [?] toggle (up to 5)
   (t) program timings (save, afk, focus, display) >> display current, modify a timing, select from pre-assigned values, all ratios scaled to fit together
   (v) view stats >> select focus, select day (or range), target day (or range)
^_ (w) spotlight mode toggle >> transfer hotkey assignment to "w"
^_&(x) chronometric filter >> transfer hotkey assignment to "x"
^_&(z) display filter >> transfer hotkey assignment to "z"
^ &(esc) exit >> change hotkey to prompt for y/n (allowing tracker to run in background)

*/

#include "stdafx.h"
#include "Windows_Focus_Tracker_rev2.h"
#include "../../GA_Library/GA_Library/GA_Library.h"

miniSANDRA WFT;

int main()
{
	/// load settings & data
	readSettings();
	std::this_thread::sleep_for(std::chrono::seconds(1));
	readData();
	std::this_thread::sleep_for(std::chrono::seconds(1));
	readGroups();
	std::this_thread::sleep_for(std::chrono::seconds(1));
	UI::clearIt();

	/// populate other fluid cache
	setLeapYearStatus();
	std::this_thread::sleep_for(std::chrono::seconds(1));
	populateFilters();
	std::this_thread::sleep_for(std::chrono::seconds(1));

	UI::clearIt();
	UI::setTextColors(UI::black, UI::dark_green);
	std::cout << "Starting Focus Tracker.  (Please wait...)";
	UI::resetTextColors();

	/// start user activity thread
	std::thread eyes(checkUserActivity);
	std::thread hands(regularlyWriteData);
	std::thread skin(checkForUserCommands);
	std::thread mouth(displayData);
	std::thread continuity(getTheTime);
	std::thread consciousness(checkFocus);

	eyes.join();
	hands.join();
	skin.join();
	mouth.join();
	continuity.join();
	consciousness.join();

	/// saves on exit
	
	writeSettings();
	Sleep(100);
	writeData();
	Sleep(100);
	writeGroups();
	
	UI::setTextColors(UI::black, UI::white);
	std::cout << "Saved:  Data, Groups, Settings";
	std::cout << "\n";
	UI::resetTextColors();

	return 0;
}

void getTheTime()
{
	while (WFT.runProgram()) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		if (WFT.requestNexus())
			if (ProgramControl::Threads::getTheTimeThread) {
				time_t ldt;
				time(&ldt);
				localtime_s(ProgramCache::theTime, &ldt);
				/*
				boost::local_time::time_zone_ptr boostZone(new boost::local_time::posix_time_zone(ProgramSettings::userTimeZone));
				boost::local_time::local_date_time ldt = boost::local_time::local_sec_clock::local_time(boostZone);
				ProgramCache::theTime = boost::local_time::to_tm(ldt);
				*/
				WFT.releaseNexus(); }
	}
}

void checkUserActivity()
{
	while (WFT.runProgram()) {
		// get a snapshot
		if (WFT.requestNexus()) {
			GetCursorPos(&ProgramCache::mouseSnapA);
			ProgramCache::userActivity = false; 
			WFT.releaseNexus();
		// check every 30 seconds
		for (unsigned int x = 0; x < ProgramSettings::userActivityPollingRate; x++) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
				// early exit code
				if (!WFT.runProgram()) {
					// flush console buffer, just in case
					FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
					return;	}
				if (WFT.requestNexus()) {
				if (ProgramControl::Threads::checkUserActivityThread) {
					GetCursorPos(&ProgramCache::mouseSnapB);
					if (ProgramCache::mouseSnapA.x != ProgramCache::mouseSnapB.x || ProgramCache::mouseSnapA.y != ProgramCache::mouseSnapB.y)
					{
						GetCursorPos(&ProgramCache::mouseSnapA);
						ProgramCache::userActivity = true;
						x = 0;
					}
					// was a key pressed?
					for (unsigned int z = 0; z < 256; z++)
						if (GetAsyncKeyState(z) & 0x8000) {
							ProgramCache::userActivity = true;
							FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
							x = 0;
							break;
						}
					if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
						ProgramCache::userActivity = true;
						FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
						x = 0;
					}
					// flush console buffer, just in case
					FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE)); }
				WFT.releaseNexus();	}
			}
		}
	}
}

void checkFocus()
{
	// memory
	std::string titleContainer, classContainer = "";
	bool newEntry = true;
	bool blacklistedEntry = false;
	bool titleGroupPriority = false;

	while (WFT.runProgram())
	{
		if (WFT.requestNexus()) {
			if (ProgramControl::Threads::checkFocusThread) {
				WFT.releaseNexus();
				// scan interval
				std::this_thread::sleep_for(std::chrono::seconds(ProgramSettings::scanInterval));
				// track foreground window
				if (WFT.requestNexus()) {
					if (ProgramSettings::detectionMode == 1 && ProgramCache::userActivity) {
						// reset variables
						titleContainer.clear();
						classContainer.clear();
						newEntry = true;
						blacklistedEntry = false;
						titleGroupPriority = false;
						// gather class & title info
						USES_CONVERSION;
						HWND foreground;
						foreground = GetForegroundWindow();
						int bufsize = GetWindowTextLength(foreground) + 1;
						LPWSTR foregroundTitle = new WCHAR[bufsize];
						GetWindowText(foreground, foregroundTitle, bufsize);
						int bufsize2 = 32;
						LPWSTR foregroundClass = new WCHAR[bufsize2];
						RealGetWindowClass(foreground, foregroundClass, 32);
						// convert the strings:
						titleContainer = W2A(foregroundTitle);
						classContainer = W2A(foregroundClass);

						delete[] foregroundTitle;
						delete[] foregroundClass;

						// format the strings
						StringTools::formatForFileIO(titleContainer, false);
						StringTools::formatForFileIO(classContainer, false);

						// check for blacklisted title (to ignore statistics gathering)
						if (ProgramSettings::titleBlack.size() > 0)
							for (unsigned int x = 0; x < ProgramSettings::titleBlack.size(); x++)
								// if whitelisted, change title
								if (titleContainer.find(ProgramSettings::titleBlack[x]) != titleContainer.npos) {
									blacklistedEntry = true;
									break;
								}

						// check for blacklisted class (to ignore statistics gathering)
						if (ProgramSettings::classBlack.size() > 0)
							for (unsigned int x = 0; x < ProgramSettings::classBlack.size(); x++)
								// if whitelisted, change title
								if (classContainer.find(ProgramSettings::classBlack[x]) != classContainer.npos) {
									blacklistedEntry = true;
									break;
								}

						// check for whitelisted keyword in title (for condensing title)
						if (!blacklistedEntry)
							if (ProgramSettings::titleWhite.size() > 0)
								for (unsigned int x = 0; x < ProgramSettings::titleWhite.size(); x++)
									// if whitelisted, change title
									if (titleContainer.find(ProgramSettings::titleWhite[x]) != titleContainer.npos) {
										titleContainer = ProgramSettings::titleWhite[x];
										break;
									}

						// check for whitelisted keyword in class (for condensing class)
						if (!blacklistedEntry)
							if (ProgramSettings::classWhite.size() > 0)
								for (unsigned int x = 0; x < ProgramSettings::classWhite.size(); x++)
									// if whitelisted, change class
									if (classContainer.find(ProgramSettings::classWhite[x]) != classContainer.npos) {
										classContainer = ProgramSettings::classWhite[x];
										break;
									}

						// check for title group (for combining data by title keyword)
						if (!blacklistedEntry)
							for (unsigned int x = 0; x < ProgramSettings::titleGroup.size(); x++)
								if (titleContainer.find(ProgramSettings::titleGroup[x]) != titleContainer.npos) {
									classContainer = "[Title Group]";
									titleContainer = ProgramSettings::titleGroup[x];
									titleGroupPriority = true;
								}

						// check for class group (for combining data by class keyword)
						if (!blacklistedEntry && !titleGroupPriority)
							for (unsigned int x = 0; x < ProgramSettings::classGroup.size(); x++)
								if (classContainer.find(ProgramSettings::classGroup[x]) != classContainer.npos) {
									classContainer = ProgramSettings::classGroup[x];
									titleContainer = "[Class Group]";
								}

						if (!blacklistedEntry && ProgramCache::userActivity) {
							// check for existing, if found, break
							if (ProgramCache::trackingSession.size() > 0)
								for (unsigned int x = 0; x < ProgramCache::trackingSession.size(); x++)
									if (titleContainer == ProgramCache::trackingSession[x].windowTitle &&
										classContainer == ProgramCache::trackingSession[x].windowClass) {
										// cycle its stored clock
										updateTotalDayHourMinSec(ProgramCache::trackingSession[x]);
										// update at the proper write block at entry [x]
										ProgramCache::trackingSession[x].yearlyStats[ProgramCache::theTime->tm_yday][ProgramCache::theTime->tm_hour] += ProgramSettings::scanInterval;
										ProgramCache::trackingSession[x].writeMe = true;
										// push to display filter
										displayFilterBase(ProgramCache::trackingSession[x]);
										// use a bool to skip the next code
										newEntry = false;
										// override AFK, if indicated
										if (ProgramCache::trackingSession[x].overridesAFK)
										{
											ProgramControl::Threads::checkUserActivityThread = false;
											ProgramCache::userActivity = true;
										}
										else ProgramControl::Threads::checkUserActivityThread = true;
									}
							// if not found
							if (newEntry && ProgramCache::userActivity) {
								// create & populate a new focusWindow object
								focusWindow xContainer;
								makeFocusWindow(xContainer);
								xContainer.windowTitle = titleContainer;
								xContainer.windowClass = classContainer;
								xContainer.description = "";
								xContainer.writeMe = true;
								// push object into session
								ProgramCache::trackingSession.push_back(xContainer);
								ProgramCache::trackingSession[ProgramCache::trackingSession.size() - 1].yearlyStats[ProgramCache::theTime->tm_yday][ProgramCache::theTime->tm_hour] += 5;
								updateTotalDayHourMinSec(ProgramCache::trackingSession[ProgramCache::trackingSession.size() - 1]);
								// push to display filter
								displayFilterBase(xContainer);
							}
						}
					}
					else if (ProgramSettings::detectionMode == 1 && !ProgramCache::userActivity) {
						ProgramCache::afkFocus.yearlyStats[ProgramCache::theTime->tm_yday][ProgramCache::theTime->tm_hour] += 5;
						updateTotalDayHourMinSec(ProgramCache::afkFocus);
						displayFilterBase(ProgramCache::afkFocus);
						ProgramCache::afkFocus.writeMe = true;
					}
					/// track override - needs testing
					else if (ProgramSettings::detectionMode == 2) {
						if (ProgramSettings::currentOverride == "")
						{
							ProgramCache::systemMessage = "No override selected, setting normal tracking mode.";
							ProgramSettings::Display::displayMode = 1;
						}
						ProgramControl::Threads::checkUserActivityThread = false;
						ProgramCache::userActivity = true;
						for (unsigned int x = 0; x < ProgramCache::trackingSession.size(); x++)
							if (ProgramCache::trackingSession[x].windowClass == "[Custom Override]" &&
								ProgramSettings::currentOverride == ProgramCache::trackingSession[x].windowTitle) {
								ProgramCache::trackingSession[x].yearlyStats[ProgramCache::theTime->tm_yday][ProgramCache::theTime->tm_hour] += 5;
								updateTotalDayHourMinSec(ProgramCache::trackingSession[x]);
								displayFilterBase(ProgramCache::trackingSession[x]);
								if (ProgramCache::trackingSession[x].overridesAFK) {
									ProgramControl::Threads::checkUserActivityThread = false;
									ProgramCache::userActivity = true;
								}
								else ProgramControl::Threads::checkUserActivityThread = true;
							}
					}
					/// my only "backdoor"
					else if (ProgramSettings::detectionMode == 3) {
						// do secret things... @_o
					}
					WFT.releaseNexus();
				}
			}
		}
	}
}

void displayData()
{

	// memory
	bool systemTalking = false;
	while (WFT.runProgram())
	{
		// reset system message status
		systemTalking = false;
		// delay
		for (unsigned int x = 0; x < ProgramSettings::displayInterval * 10; x++) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			if (WFT.requestNexus()) {
				if (!ProgramControl::userInputOK) {
					// display the system message, if found
					if (ProgramCache::systemMessage != "" && !systemTalking)
					{
						if (!WFT.runProgram())
							return;
						UI::setTextColors(UI::black, UI::dark_green);
						std::cout << ProgramCache::systemMessage << "\n\n";
						UI::resetTextColors();
						systemTalking = true;
						// give the other thread time to display the message
						std::this_thread::sleep_for(std::chrono::seconds(2));
						// give the other thread time to convey its dissatisfaction with the user's input
						if (ProgramControl::userHoldingKey)
							std::this_thread::sleep_for(std::chrono::seconds(2));
						// give the other thread time to process an exit command
						if (ProgramControl::userHoldingEscape)
							std::this_thread::sleep_for(std::chrono::seconds(4));
						if (!WFT.runProgram())
							return;
						ProgramControl::userInputOK = true;
						ProgramControl::userHoldingKey = false;
						ProgramControl::userHoldingEscape = false;
						break;
					}
				}
				WFT.releaseNexus();
			}
		}
		if (WFT.requestNexus()){
			if (ProgramControl::Threads::displayThread)
			{
				// display
				if (ProgramControl::displayOK) {
					UI::clearIt();
					displayDisplayStatus();
					std::cout << "--------------------------------" << "\n\n";
					// off
					if (ProgramSettings::Display::displayMode == 0)
					{
						std::cout << "Get out, now.  It's not safe." << "\n\n";  // ;)
					}
					// last 5
					else if (ProgramSettings::Display::displayMode == 1)
					{
						UI::setTextColors(UI::black, UI::dark_green);
						std::cout << "(Most recent at the top.)" << "\n\n";
						for (unsigned int x = 0; x < 5; x++) {
							if (ProgramCache::lastFiveFocus[x].windowClass == "[Custom Override]")
								UI::setTextColors(UI::dark_red, UI::white);
							else UI::setTextColors(UI::black, UI::white);
							std::cout << ProgramCache::lastFiveFocus[x].windowClass;
							UI::resetTextColors();
							std::cout << " - ";
							determineFocusColor(ProgramCache::lastFiveFocus[x]);
							std::cout << ProgramCache::lastFiveFocus[x].windowTitle << "\n";
							displayChronoStats(ProgramCache::lastFiveFocus[x]);
							UI::resetTextColors();
						}
					}
					// top 5
					else if (ProgramSettings::Display::displayMode == 2)
					{
						for (unsigned int x = 0; x < 5; x++) {
							if (ProgramCache::topFiveFocus[x].windowClass == "[Custom Override]")
								UI::setTextColors(UI::dark_red, UI::white);
							std::cout << ProgramCache::topFiveFocus[x].windowClass;
							UI::resetTextColors();
							std::cout << " - ";
							determineFocusColor(ProgramCache::topFiveFocus[x]);
							std::cout << ProgramCache::topFiveFocus[x].windowTitle << "\n";
							displayChronoStats(ProgramCache::topFiveFocus[x]);
							UI::resetTextColors();
						}
					}
					/// groups
					else if (ProgramSettings::Display::displayMode == 3)
					{
						focusWindow xContainer = {};
						if (ProgramSettings::groupList.size() > 0)
							for (unsigned int x = 0; x < ProgramSettings::groupList.size(); x++) {
								xContainer = {};
								if (ProgramSettings::groupList[x].members.size() > 0) {
									xContainer.windowClass = "[Custom Group]";
									xContainer.windowTitle = ProgramSettings::groupList[x].groupName;
									xContainer.description = ProgramSettings::groupList[x].groupDescription;
									UI::setTextColors(UI::black, UI::dark_green);
									std::cout << xContainer.windowClass << " - " << xContainer.windowTitle << "\n";
									if (xContainer.description != "")
									{
										UI::setTextColors(UI::black, UI::dark_cyan);
										std::cout << "\t" << xContainer.description << "\n";
									}
									// process data
									for (unsigned int y = 0; y < ProgramSettings::groupList[x].members.size(); y++)
									{
										for (unsigned int z = 0; z < ProgramCache::trackingSession.size(); z++)
											if (ProgramSettings::groupList[x].members[y].className == ProgramCache::trackingSession[z].windowClass &&
												ProgramSettings::groupList[x].members[y].titleName == ProgramCache::trackingSession[z].windowTitle)
											{
												UI::setTextColors(UI::black, UI::white);
												std::cout << ProgramCache::trackingSession[z].windowClass << " - " << ProgramCache::trackingSession[z].windowTitle << "\n";
												if (ProgramCache::trackingSession[z].description != "")
												{
													UI::resetTextColors();
													std::cout << "\t";
													UI::setTextColors(UI::dark_cyan, UI::light_gray);
													std::cout << ProgramCache::trackingSession[z].description << "\n";
												}
												for (unsigned int a = 0; a < 4; a++)
													xContainer.totalDayHourMinSec[a] += ProgramCache::trackingSession[z].totalDayHourMinSec[a];
												for (unsigned int a = 0; a < 366; a++)
													for (unsigned int b = 0; b < 24; b++)
														xContainer.yearlyStats[a][b] += ProgramCache::trackingSession[z].yearlyStats[a][b];
												for (unsigned int a = 0; a < 8; a++)
													xContainer.focusLimits[a] += ProgramCache::trackingSession[z].focusLimits[a];
											}
									}
									// display
									displayChronoStats(xContainer);
									std::cout << "\n";
									xContainer = {};
								}
							}
					}
					// afk
					else if (ProgramSettings::Display::displayMode == 4)
					{
						std::cout << "AFK" << "\n";
						displayChronoStats(ProgramCache::afkFocus);
					}
					// debug
					else if (ProgramSettings::Display::displayMode == 5)
					{
						// debug info (if any)
						std::cout << "\n\n";
					}
					std::cout << "--------------------------------" << "\n\n";
				}
			}
			WFT.releaseNexus();
		}
	}
}

void regularlyWriteData()
{
	// memory
	int writeTimes[12] = { 0, 4, 9, 14, 19, 24, 29, 34, 39, 44, 49, 54 };
	while (WFT.runProgram()) {
		// check every 30 seconds (sectioned off in case of early termination)
		for (unsigned int x = 0; x < 300; x++) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			if (!WFT.runProgram())
				return;
		}
		if (WFT.requestNexus()){
		// check for the write cue (5 minute mark)
			if (ProgramControl::Threads::regularlyWriteDataThread)
			{
				for (unsigned int timeCheck = 0; timeCheck < 12; timeCheck++)
					if (ProgramCache::theTime->tm_min == writeTimes[timeCheck]) {
						writeSettings();
						std::this_thread::sleep_for(std::chrono::seconds(1));
						writeGroups();
						std::this_thread::sleep_for(std::chrono::seconds(1));
						writeData();
						ProgramControl::Threads::checkFocusThread = true;
						// wait another minute, checking every second for early program termination
						for (unsigned int x = 0; x < 60; x++) {
							std::this_thread::sleep_for(std::chrono::seconds(1));
							if (!WFT.runProgram())
								return;
						}
					}
			}
			WFT.releaseNexus();
		}
	}
}

void checkForUserCommands()
{
	while (WFT.runProgram())
	{
		// flush the console buffer
		FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));

		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		if (WFT.requestNexus())
		if (_kbhit() && ProgramControl::userInputOK) {
			// "a" - Override AFK toggling
			if (GetAsyncKeyState(0x41))
			{
				ProgramCache::systemMessage = "Starting AFK Detection override menu.  (Please wait...)";
				ProgramControl::userInputOK = false;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				if (GetAsyncKeyState(0x41) & 0x8000)
				{
					ProgramControl::userHoldingKey = true;
					UI::setTextColors(UI::black, UI::light_red);
					std::cout << "WFT isn't the clingy type." << "\n\n";
					UI::resetTextColors();
				}
				FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				setOverrideAFKFlag(ProgramCache::programMenu);
			}
			// "c" - Manage Conduits
			if (GetAsyncKeyState(0x43))
			{
				ProgramCache::systemMessage = "Starting Conduit management.  (Please wait...)";
				ProgramControl::userInputOK = false;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				if (GetAsyncKeyState(0x43) & 0x8000)
				{
					ProgramControl::userHoldingKey = true;
					UI::setTextColors(UI::black, UI::light_red);
					std::cout << "I \"c\" \"u\" have issues, but not as many as \"i\"." << "\n\n";
					UI::resetTextColors();
				}
				FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				conduitManagement(ProgramCache::programMenu);
			}
			// "d" - Description management
			else if (GetAsyncKeyState(0x44))
			{
				ProgramCache::systemMessage = "Entering Description management.  (Please wait...)";
				ProgramControl::userInputOK = false;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				if (GetAsyncKeyState(0x44) & 0x8000)
				{
					ProgramControl::userHoldingKey = true;
					UI::setTextColors(UI::black, UI::light_red);
					std::cout << "Finger used continuous attack.  User is confused." << "\n\n";
					UI::resetTextColors();
				}
				FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				descriptionManagement(ProgramCache::programMenu);
			}
			// "o" - toggle Override state
			else if (GetAsyncKeyState(0x4F))
			{
				ProgramCache::systemMessage = "Toggling override mode.  (Please wait...)";
				ProgramControl::userInputOK = false;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				if (GetAsyncKeyState(0x4F) & 0x8000)
				{
					ProgramControl::userHoldingKey = true;
					UI::setTextColors(UI::black, UI::light_red);
					std::cout << "WFT is can't focus with  you holding down keys--ha ha ha ok I'll go away now." << "\n\n";
					UI::resetTextColors();
				}
				toggleOverrideMode();
			}
			// "p" - configure Overrides
			else if (GetAsyncKeyState(0x50))
			{
				ProgramCache::systemMessage = "Starting Override menu.  (Please wait...)";
				ProgramControl::userInputOK = false;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				if (GetAsyncKeyState(0x50) & 0x8000)
				{
					ProgramControl::userHoldingKey = true;
					UI::setTextColors(UI::black, UI::light_red);
					std::cout << "Not so loud!  WFT has sensitive senses." << "\n\n";
					UI::resetTextColors();
				}
				// flush the console buffer
				FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				ProgramControl::overrideMenu = true;
				WFT_Menus::overrideMenu(ProgramCache::programMenu);
			}
			// "q" - toggle Conduit mode
			else if (GetAsyncKeyState(0x51))
			{
				ProgramCache::systemMessage = "Toggling conduit mode.  (Please wait...)";
				ProgramControl::userInputOK = false;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				if (GetAsyncKeyState(0x51) & 0x8000)
				{
					ProgramControl::userHoldingKey = true;
					UI::setTextColors(UI::black, UI::light_red);
					std::cout << "You could be baking something right now." << "\n\n";
					UI::resetTextColors();
				}
				toggleConduitMode();
			}
			// "s" - Spotlight management
			else if (GetAsyncKeyState(0x53))
			{
				ProgramCache::systemMessage = "Entering Spotlight management.  (Please wait...)";
				ProgramControl::userInputOK = false;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				if (GetAsyncKeyState(0x53) & 0x8000)
				{
					ProgramControl::userHoldingKey = true;
					UI::setTextColors(UI::black, UI::light_red);
					std::cout << "SSSSSSSSSSSSSsssssssssssss... (You're leaking.)" << "\n\n";
					UI::resetTextColors();
				}
				spotlightManagement(ProgramCache::programMenu);
			}
			// "w" - toggle Spotlight mode
			else if (GetAsyncKeyState(0x57))
			{
				ProgramCache::systemMessage = "Toggling spotlight mode.  (Please wait...)";
				ProgramControl::userInputOK = false;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				if (GetAsyncKeyState(0x57) & 0x8000)
				{
					ProgramControl::userHoldingKey = true;
					UI::setTextColors(UI::black, UI::light_red);
					std::cout << "WFT enjoys your hug, but it doesn't need it." << "\n\n";
					UI::resetTextColors();
				}
				toggleSpotlightMode();
			}
			// "x" - change Chronometric filter
			else if (GetAsyncKeyState(0x58))
			{
				ProgramCache::systemMessage = "Changing chrono mode.  (Please wait...)";
				ProgramControl::userInputOK = false;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				if (GetAsyncKeyState(0x58) & 0x8000)
				{
					ProgramControl::userHoldingKey = true;
					UI::setTextColors(UI::black, UI::light_red);
					std::cout << "WFT needs some space (don't press space)..." << "\n\n";
					UI::resetTextColors();
				}
				cycleChronoMode();
			}
			// "z" - change Display mode
			else if (GetAsyncKeyState(0x5A))
			{
				ProgramCache::systemMessage = "Changing display mode.  (Please wait...)";
				ProgramControl::userInputOK = false;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				if (GetAsyncKeyState(0x5A) & 0x8000)
				{
					ProgramControl::userHoldingKey = true;
					UI::setTextColors(UI::black, UI::light_red);
					std::cout << "Smothering the key won't bring her back." << "\n\n";
					UI::resetTextColors();
				}
				cycleDisplayMode();
			}
			/// others
			else if (GetAsyncKeyState(VK_F9))
			{
				ProgramCache::systemMessage = "No function associated with this key.  (Please wait...)";
				ProgramControl::userInputOK = false;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				if (GetAsyncKeyState(VK_F9) & 0x8000)
				{
					ProgramControl::userHoldingKey = true;
					UI::setTextColors(UI::black, UI::light_red);
					std::cout << "F9 carries the weight of both responsibilty /and/ your finger.  Give it a break." << "\n\n";
					UI::resetTextColors();
				}
			}
			else if (GetAsyncKeyState(VK_F10))
			{
				ProgramCache::systemMessage = "No function associated with this key.  (Please wait...)";
				ProgramControl::userInputOK = false;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				if (GetAsyncKeyState(VK_F10) & 0x8000)
				{
					ProgramControl::userHoldingKey = true;
					UI::setTextColors(UI::black, UI::light_red);
					std::cout << "If it would talk, F10 would ask you to stop pointing." << "\n\n";
					UI::resetTextColors();
				}
			}
			else if (GetAsyncKeyState(VK_F11))
			{
				ProgramCache::systemMessage = "No function associated with this key.  (Please wait...)";
				ProgramControl::userInputOK = false;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				if (GetAsyncKeyState(VK_F11) & 0x8000)
				{
					ProgramControl::userHoldingKey = true;
					UI::setTextColors(UI::black, UI::light_red);
					std::cout << "F11 agrees with F10." << "\n\n";
					UI::resetTextColors();
				}
			}
			else if (GetAsyncKeyState(VK_F12))
			{
				ProgramCache::systemMessage = "No function associated with this key.  (Please wait...)";
				ProgramControl::userInputOK = false;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				if (GetAsyncKeyState(VK_F12) & 0x8000)
				{
					ProgramControl::userHoldingKey = true;
					UI::setTextColors(UI::black, UI::light_red);
					std::cout << "Wake up!  You're crushing F12 with one of your chins (or your fatty fat finger)!" << "\n\n";
					UI::resetTextColors();
				}
			}
			else if (GetAsyncKeyState(VK_TAB))
			{
				ProgramCache::systemMessage = "No function associated with this key.  (Please wait...)";
				ProgramControl::userInputOK = false;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				if (GetAsyncKeyState(VK_TAB) & 0x8000)
				{
					ProgramControl::userHoldingKey = true;
					UI::setTextColors(UI::black, UI::light_red);
					std::cout << "There's no need to be pushy..." << "\n\n";
					UI::resetTextColors();
				}
			}
			else if (GetAsyncKeyState(VK_BACK))
			{
				ProgramCache::systemMessage = "No function associated with this key.  (Please wait...)";
				ProgramControl::userInputOK = false;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				if (GetAsyncKeyState(VK_BACK) & 0x8000)
				{
					ProgramControl::userHoldingKey = true;
					UI::setTextColors(UI::black, UI::light_red);
					std::cout << "Yes, apply more pressure.  That'll work." << "\n\n";
					UI::resetTextColors();
				}
			} 
			else if (GetAsyncKeyState(VK_ESCAPE))
			{
				ProgramCache::systemMessage = "Hold escape for 3 more seconds to exit...";
				ProgramControl::userInputOK = false;
				unsigned int secondCounter = 0;
				for (unsigned int x = 0; x < 3; x++) {
					std::this_thread::sleep_for(std::chrono::seconds(1));
					if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
						ProgramControl::userHoldingEscape = true;
						secondCounter++;
						if (secondCounter == 3) {
							UI::setTextColors(UI::dark_red, UI::white);
							std::cout << "THERE IS NO ESCAPE oh wait is that a power button...?" << "\n\n";
							UI::resetTextColors();
							WFT.earlyShutdown();
							return;
						}
					}
				}
			}
			// g (manage group states)
			else if (GetAsyncKeyState(0x47))
			{
				ProgramCache::systemMessage = "Entering Group Management.  (Please wait...)";
				ProgramControl::userInputOK = false;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				if (GetAsyncKeyState(0x47) & 0x8000)
				{
					ProgramControl::userHoldingKey = true;
					UI::setTextColors(UI::black, UI::light_red);
					std::cout << "Getting there faster is extra, pal..." << "\n\n";
					UI::resetTextColors();
				}
				FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				WFT_Menus::groupMenu(ProgramCache::programMenu);
			}

			while (!ProgramControl::userInputOK)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(25));
			}
			ProgramControl::displayOK = true;

			// flush the console buffer
			FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			WFT.releaseNexus();
		}
	}
}

void displayChronoStats(focusWindow &passThrough)
{
	/// debug
	if (ProgramSettings::Display::displayChronologicalType == 0)
	{

	}
	/// this hour
	if (ProgramSettings::Display::displayChronologicalType == 1)
	{
		UI::setTextColors(UI::black, UI::dark_gray);
		std::cout << std::setprecision(2) <<
			double(passThrough.yearlyStats[ProgramCache::theTime->tm_yday][ProgramCache::theTime->tm_hour]) / 60 << " minutes (this hour block)." << "\n\n";
		UI::resetTextColors();
	}
	/// this day
	if (ProgramSettings::Display::displayChronologicalType == 2)
	{
		double dailyStats = 0;
		for (int x = 0; x <= ProgramCache::theTime->tm_hour; x++)
			dailyStats += passThrough.yearlyStats[ProgramCache::theTime->tm_yday][x];
		int thisFocusColor = determineFocusLimitColor(passThrough);
		UI::setTextColors(UI::black, thisFocusColor);
		std::cout << std::setprecision(2) << dailyStats / 60 / 60;
		UI::resetTextColors(); 
		std::cout << " hours (so far, this calendar day)." << "\n\n";
	}
	/// this week
	if (ProgramSettings::Display::displayChronologicalType == 3)
	{
		double weekTotal = 0;
		// if crossing backwards over year line
		if (ProgramCache::theTime->tm_yday <= 5)
		{
			// calculate the number of days which will spill over
			int overflow = 6 - ProgramCache::theTime->tm_yday;
			// grab the overflow data
			for (int x = 0; x < overflow; x++)
				for (int y = 0; y < 24; y++)
					weekTotal += passThrough.yearlyStats[ProgramCache::lastYearSize - x][y];
			// grab the data prior to the current day
			if (ProgramCache::theTime->tm_yday > 0)
				for (int x = 0; x < ProgramCache::theTime->tm_yday; x++)
					for (unsigned int y = 0; y < 24; y++)
						weekTotal += passThrough.yearlyStats[ProgramCache::theTime->tm_yday][y];
			// grab the data for today
			for (int x = 0; x <= ProgramCache::theTime->tm_hour; x++)
				weekTotal += passThrough.yearlyStats[ProgramCache::theTime->tm_yday][x];
		}
		// all other inquiries
		else if (ProgramCache::theTime->tm_yday >= 6) {
			for (int x = 1; x <= 6; x++)
				for (unsigned int y = 0; y < 24; y++)
					weekTotal += passThrough.yearlyStats[ProgramCache::theTime->tm_yday - x][y];
			for (int x = 0; x <= ProgramCache::theTime->tm_hour; x++)
				weekTotal += passThrough.yearlyStats[ProgramCache::theTime->tm_yday][x]; }
		int thisFocusColor = determineFocusLimitColor(passThrough);
		UI::setTextColors(UI::black, thisFocusColor);
		std::cout << std::setprecision(2) << weekTotal / 60 / 60;
		UI::resetTextColors();
		std::cout << " hours (so far, these past 7 days)." << "\n\n";
	}
	/// this month
	if (ProgramSettings::Display::displayChronologicalType == 4)
	{
		double monthTotal = 0;
		int firstDayOfMonth = 0;
		// month array
		if (ProgramCache::theTime->tm_mon == 0)
			firstDayOfMonth = 0;
		else if (ProgramCache::theTime->tm_mon == 1)
			firstDayOfMonth = 31;
		else if (ProgramCache::theTime->tm_mon == 2)
			firstDayOfMonth = 59 + ProgramCache::leapModifier;
		else if (ProgramCache::theTime->tm_mon == 3)
			firstDayOfMonth = 81 + ProgramCache::leapModifier;
		else if (ProgramCache::theTime->tm_mon == 4)
			firstDayOfMonth = 112 + ProgramCache::leapModifier;
		else if (ProgramCache::theTime->tm_mon == 5)
			firstDayOfMonth = 144 + ProgramCache::leapModifier;
		else if (ProgramCache::theTime->tm_mon == 6)
			firstDayOfMonth = 175 + ProgramCache::leapModifier;
		else if (ProgramCache::theTime->tm_mon == 7)
			firstDayOfMonth = 207 + ProgramCache::leapModifier;
		else if (ProgramCache::theTime->tm_mon == 8)
			firstDayOfMonth = 239 + ProgramCache::leapModifier;
		else if (ProgramCache::theTime->tm_mon == 9)
			firstDayOfMonth = 270 + ProgramCache::leapModifier;
		else if (ProgramCache::theTime->tm_mon == 10)
			firstDayOfMonth = 302 + ProgramCache::leapModifier;
		else if (ProgramCache::theTime->tm_mon == 11)
			firstDayOfMonth = 333 + ProgramCache::leapModifier;
		// math
		for (int x = firstDayOfMonth; x < ProgramCache::theTime->tm_yday; x++)
			for (unsigned int y = 0; y < 24; y++)
				monthTotal += passThrough.yearlyStats[x][y];
		for (int x = 0; x <= ProgramCache::theTime->tm_hour; x++)
			monthTotal += passThrough.yearlyStats[ProgramCache::theTime->tm_yday][x];
		// output	
		int thisFocusColor = determineFocusLimitColor(passThrough);
		UI::setTextColors(UI::black, thisFocusColor);
		std::cout << std::setprecision(2) << monthTotal / 60 / 60;
		UI::resetTextColors();
		std::cout << " hours (so far, this current month)." << "\n\n";
	}
	/// this year
	if (ProgramSettings::Display::displayChronologicalType == 5)
	{
		double yearTotal = 0;
		for (int x = 0; x < ProgramCache::theTime->tm_yday; x++)
			for (unsigned int y = 0; y < 24; y++)
				yearTotal += passThrough.yearlyStats[x][y];
		for (int x = 0; x < ProgramCache::theTime->tm_hour; x++)
			yearTotal += passThrough.yearlyStats[ProgramCache::theTime->tm_yday][x];
		int thisFocusColor = determineFocusLimitColor(passThrough);
		UI::setTextColors(UI::black, thisFocusColor);
		std::cout << std::setprecision(2) <<
			yearTotal / 24 / 60 / 60;
		UI::resetTextColors();
		std::cout << " days (so far, this year)." << "\n\n";
	}
}

int determineFocusLimitColor(focusWindow &passThrough)
{
	// default the color to standard light gray
	int appropriateColor = 7;
	
	if (ProgramSettings::Display::displayChronologicalType == 2)
	{
		double dayTotal = 0;
		for (int x = 0; x <= ProgramCache::theTime->tm_hour; x++)
			dayTotal += passThrough.yearlyStats[ProgramCache::theTime->tm_yday][x];

		if (double(passThrough.focusLimits[1]) <= dayTotal / 60 / 60)
			appropriateColor = UI::light_green;
		else if (double(passThrough.focusLimits[1]) > dayTotal / 60 / 60 && double(passThrough.focusLimits[0]) < dayTotal / 60 / 60)
			appropriateColor = UI::light_yellow;
		else if (double(passThrough.focusLimits[0]) >= dayTotal / 60 / 60)
			appropriateColor = UI::light_red;
	}
	else if (ProgramSettings::Display::displayChronologicalType == 3)
	{
		double weekTotal = 0;
		// if crossing backwards over year line
		if (ProgramCache::theTime->tm_yday <= 5)
		{
			// calculate the number of days which will spill over
			int overflow = 6 - ProgramCache::theTime->tm_yday;
			// grab the overflow data
			for (int x = 0; x < overflow; x++)
				for (int y = 0; y < 24; y++)
					weekTotal += passThrough.yearlyStats[ProgramCache::lastYearSize - x][y];
			// grab the data prior to the current day
			if (ProgramCache::theTime->tm_yday > 0)
				for (int x = 0; x < ProgramCache::theTime->tm_yday; x++)
					for (unsigned int y = 0; y < 24; y++)
						weekTotal += passThrough.yearlyStats[ProgramCache::theTime->tm_yday][y];
			// grab the data for today
			for (int x = 0; x <= ProgramCache::theTime->tm_hour; x++)
				weekTotal += passThrough.yearlyStats[ProgramCache::theTime->tm_yday][x];
		}
		// all other inquiries
		else if (ProgramCache::theTime->tm_yday >= 6) {
			for (int x = 1; x <= 6; x++)
				for (unsigned int y = 0; y < 24; y++)
					weekTotal += passThrough.yearlyStats[ProgramCache::theTime->tm_yday - x][y];
			for (int x = 0; x <= ProgramCache::theTime->tm_hour; x++)
				weekTotal += passThrough.yearlyStats[ProgramCache::theTime->tm_yday][x];
		}
		if (double(passThrough.focusLimits[3]) <= weekTotal / 60 / 60)
			appropriateColor = UI::light_green;
		else if (double(passThrough.focusLimits[3]) > weekTotal / 60 / 60 && double(passThrough.focusLimits[2]) < weekTotal / 60 / 60)
			appropriateColor = UI::light_yellow;
		else if (double(passThrough.focusLimits[2]) >= weekTotal / 60 / 60)
			appropriateColor = UI::light_red;


	}
	else if (ProgramSettings::Display::displayChronologicalType == 4)
	{
		double monthTotal = 0;
		int firstDayOfMonth = 0;
		// month array
		if (ProgramCache::theTime->tm_mon == 0)
			firstDayOfMonth = 0;
		else if (ProgramCache::theTime->tm_mon == 1)
			firstDayOfMonth = 31;
		else if (ProgramCache::theTime->tm_mon == 2)
			firstDayOfMonth = 59 + ProgramCache::leapModifier;
		else if (ProgramCache::theTime->tm_mon == 3)
			firstDayOfMonth = 81 + ProgramCache::leapModifier;
		else if (ProgramCache::theTime->tm_mon == 4)
			firstDayOfMonth = 112 + ProgramCache::leapModifier;
		else if (ProgramCache::theTime->tm_mon == 5)
			firstDayOfMonth = 144 + ProgramCache::leapModifier;
		else if (ProgramCache::theTime->tm_mon == 6)
			firstDayOfMonth = 175 + ProgramCache::leapModifier;
		else if (ProgramCache::theTime->tm_mon == 7)
			firstDayOfMonth = 207 + ProgramCache::leapModifier;
		else if (ProgramCache::theTime->tm_mon == 8)
			firstDayOfMonth = 239 + ProgramCache::leapModifier;
		else if (ProgramCache::theTime->tm_mon == 9)
			firstDayOfMonth = 270 + ProgramCache::leapModifier;
		else if (ProgramCache::theTime->tm_mon == 10)
			firstDayOfMonth = 302 + ProgramCache::leapModifier;
		else if (ProgramCache::theTime->tm_mon == 11)
			firstDayOfMonth = 333 + ProgramCache::leapModifier;
		// math
		for (int x = firstDayOfMonth; x < ProgramCache::theTime->tm_yday; x++)
			for (unsigned int y = 0; y < 24; y++)
				monthTotal += passThrough.yearlyStats[x][y];
		for (int x = 0; x <= ProgramCache::theTime->tm_hour; x++)
			monthTotal += passThrough.yearlyStats[ProgramCache::theTime->tm_yday][x];

		if (double(passThrough.focusLimits[5]) <= monthTotal / 60 / 60)
			appropriateColor = UI::light_green;
		else if (double(passThrough.focusLimits[5]) > monthTotal / 60 / 60 && double(passThrough.focusLimits[4]) < monthTotal / 60 / 60)
			appropriateColor = UI::light_yellow;
		else if (double(passThrough.focusLimits[4]) >= monthTotal / 60 / 60)
			appropriateColor = UI::light_red;

	}
	else if (ProgramSettings::Display::displayChronologicalType == 5)
	{
		double yearTotal = 0;
		yearTotal += passThrough.totalDayHourMinSec[0] * 24;
		yearTotal += passThrough.totalDayHourMinSec[1] * 1;
		yearTotal += passThrough.totalDayHourMinSec[2] / 60;
		yearTotal += passThrough.totalDayHourMinSec[3] / 60;

		if (double(passThrough.focusLimits[7]) <= yearTotal / 60 / 60)
			appropriateColor = UI::light_green;
		else if (double(passThrough.focusLimits[7]) > yearTotal / 60 / 60 && double(passThrough.focusLimits[6]) < yearTotal / 60 / 60)
			appropriateColor = UI::light_yellow;
		else if (double(passThrough.focusLimits[6]) >= yearTotal / 60 / 60)
			appropriateColor = UI::light_red;

	}

	return appropriateColor;
}

void displayDisplayStatus()
{
	int colorScheme[2] = {};
	// Display mode
	{
		colorScheme[0] = UI::black;
		colorScheme[1] = UI::dark_green;
		UI::setTextColors(colorScheme[0], colorScheme[1]);
		std::cout << "(z) Display:		";
		// none
		if (ProgramSettings::Display::displayMode == 0)
		{
			std::cout << "[";
			UI::setTextColors(colorScheme[1], colorScheme[0]);
			std::cout << "None";
			UI::setTextColors(colorScheme[0], colorScheme[1]);
			std::cout << "] [";
			std::cout << "Last 5";
			std::cout << "] [";
			std::cout << "Top 5";
			std::cout << "] [";
			std::cout << "Groups";
			std::cout << "] [";
			std::cout << "AFK";
			std::cout << "] [";
			std::cout << "Debug";
			std::cout << "]";
		}
		// last 5
		else if (ProgramSettings::Display::displayMode == 1)
		{
			std::cout << "[";
			std::cout << "None";
			std::cout << "] [";
			UI::setTextColors(colorScheme[1], colorScheme[0]);
			std::cout << "Last 5";
			UI::setTextColors(colorScheme[0], colorScheme[1]);
			std::cout << "] [";
			std::cout << "Top 5";
			std::cout << "] [";
			std::cout << "Groups";
			std::cout << "] [";
			std::cout << "AFK";
			std::cout << "] [";
			std::cout << "Debug";
			std::cout << "]";
		}
		// top 5
		else if (ProgramSettings::Display::displayMode == 2)
		{
			std::cout << "[";
			std::cout << "None";
			std::cout << "] [";
			std::cout << "Last 5";
			std::cout << "] [";
			UI::setTextColors(colorScheme[1], colorScheme[0]);
			std::cout << "Top 5";
			UI::setTextColors(colorScheme[0], colorScheme[1]);
			std::cout << "] [";
			std::cout << "Groups";
			std::cout << "] [";
			std::cout << "AFK";
			std::cout << "] [";
			std::cout << "Debug";
			std::cout << "]";
		}
		// groups
		else if (ProgramSettings::Display::displayMode == 3)
		{
			std::cout << "[";
			std::cout << "None";
			std::cout << "] [";
			std::cout << "Last 5";
			std::cout << "] [";
			std::cout << "Top 5";
			std::cout << "] [";
			UI::setTextColors(colorScheme[1], colorScheme[0]);
			std::cout << "Groups";
			UI::setTextColors(colorScheme[0], colorScheme[1]);
			std::cout << "] [";
			std::cout << "AFK";
			std::cout << "] [";
			std::cout << "Debug";
			std::cout << "]";
		}
		// afk
		else if (ProgramSettings::Display::displayMode == 4)
		{
			std::cout << "[";
			std::cout << "None";
			std::cout << "] [";
			std::cout << "Last 5";
			std::cout << "] [";
			std::cout << "Top 5";
			std::cout << "] [";
			std::cout << "Groups";
			std::cout << "] [";
			UI::setTextColors(colorScheme[1], colorScheme[0]);
			std::cout << "AFK";
			UI::setTextColors(colorScheme[0], colorScheme[1]);
			std::cout << "] [";
			std::cout << "Debug";
			std::cout << "]";
		}
		// debug
		else if (ProgramSettings::Display::displayMode == 5)
		{
			std::cout << "[";
			std::cout << "None";
			std::cout << "] [";
			std::cout << "Last 5";
			std::cout << "] [";
			std::cout << "Top 5";
			std::cout << "] [";
			std::cout << "Groups";
			std::cout << "] [";
			std::cout << "AFK";
			std::cout << "] [";
			UI::setTextColors(colorScheme[1], colorScheme[0]);
			std::cout << "Debug";
			UI::setTextColors(colorScheme[0], colorScheme[1]);
			std::cout << "]";
		}
		std::cout << "\n";
	}
	// Chrono mode
	{
		/// strange VS2015 collapsing error
		{
			colorScheme[0] = UI::black;
			colorScheme[1] = UI::dark_yellow;
			UI::setTextColors(colorScheme[0], colorScheme[1]);
			std::cout << "(x) Chrono:		";
			// hour
			if (ProgramSettings::Display::displayChronologicalType == 1)
			{
				std::cout << "[";
				UI::setTextColors(colorScheme[1], colorScheme[0]);
				std::cout << "Hour";
				UI::setTextColors(colorScheme[0], colorScheme[1]);
				std::cout << "] [";
				std::cout << "Day";
				std::cout << "] [";
				std::cout << "Week";
				std::cout << "] [";
				std::cout << "Month";
				std::cout << "] [";
				std::cout << "Year";
				std::cout << "]";
			}
			// day
			else if (ProgramSettings::Display::displayChronologicalType == 2)
			{
				std::cout << "[";
				std::cout << "Hour";
				std::cout << "] [";
				UI::setTextColors(colorScheme[1], colorScheme[0]);
				std::cout << "Day";
				UI::setTextColors(colorScheme[0], colorScheme[1]);
				std::cout << "] [";
				std::cout << "Week";
				std::cout << "] [";
				std::cout << "Month";
				std::cout << "] [";
				std::cout << "Year";
				std::cout << "]";
			}
			// week
			else if (ProgramSettings::Display::displayChronologicalType == 3)
			{
				std::cout << "[";
				std::cout << "Hour";
				std::cout << "] [";
				std::cout << "Day";
				std::cout << "] [";
				UI::setTextColors(colorScheme[1], colorScheme[0]);
				std::cout << "Week";
				UI::setTextColors(colorScheme[0], colorScheme[1]);
				std::cout << "] [";
				std::cout << "Month";
				std::cout << "] [";
				std::cout << "Year";
				std::cout << "]";
			}
			// month
			else if (ProgramSettings::Display::displayChronologicalType == 4)
			{
				std::cout << "[";
				std::cout << "Hour";
				std::cout << "] [";
				std::cout << "Day";
				std::cout << "] [";
				std::cout << "Week";
				std::cout << "] [";
				UI::setTextColors(colorScheme[1], colorScheme[0]);
				std::cout << "Month";
				UI::setTextColors(colorScheme[0], colorScheme[1]);
				std::cout << "] [";
				std::cout << "Year";
				std::cout << "]";
			}
			// year
			else if (ProgramSettings::Display::displayChronologicalType == 5)
			{
				std::cout << "[";
				std::cout << "Hour";
				std::cout << "] [";
				std::cout << "Day";
				std::cout << "] [";
				std::cout << "Week";
				std::cout << "] [";
				std::cout << "Month";
				std::cout << "] [";
				UI::setTextColors(colorScheme[1], colorScheme[0]);
				std::cout << "Year";
				UI::setTextColors(colorScheme[0], colorScheme[1]);
				std::cout << "]";
			}
			std::cout << "\n";
		}
	}
	// Conduit filter
	{
		colorScheme[0] = UI::black;
		colorScheme[1] = UI::light_cyan;
		UI::setTextColors(colorScheme[0], colorScheme[1]);
		std::cout << "(q) Conduits:		";
		// on
		if (!ProgramSettings::Display::conduitMode)
		{
			std::cout << "[";
			UI::setTextColors(colorScheme[1], colorScheme[0]);
			std::cout << "Off";
			UI::setTextColors(colorScheme[0], colorScheme[1]);
			std::cout << "] [";
			std::cout << "On";
			std::cout << "]";
		}
		// off
		else if (ProgramSettings::Display::conduitMode)
		{
			std::cout << "[";
			std::cout << "Off";
			std::cout << "] [";
			UI::setTextColors(colorScheme[1], colorScheme[0]);
			std::cout << "On";
			UI::setTextColors(colorScheme[0], colorScheme[1]);
			std::cout << "]";
		}
		std::cout << "\n";
	}
	// Spotlight filter
	{
		colorScheme[0] = UI::black;
		colorScheme[1] = UI::light_blue;
		UI::setTextColors(colorScheme[0], colorScheme[1]);
		std::cout << "(w) Spotlights:		";
		// on
		if (!ProgramSettings::Display::spotlightMode)
		{
			std::cout << "[";
			UI::setTextColors(colorScheme[1], colorScheme[0]);
			std::cout << "Off";
			UI::setTextColors(colorScheme[0], colorScheme[1]);
			std::cout << "] [";
			std::cout << "On";
			std::cout << "]";
		}
		// off
		else if (ProgramSettings::Display::spotlightMode)
		{
			std::cout << "[";
			std::cout << "Off";
			std::cout << "] [";
			UI::setTextColors(colorScheme[1], colorScheme[0]);
			std::cout << "On";
			UI::setTextColors(colorScheme[0], colorScheme[1]);
			std::cout << "]";
		}
		std::cout << "\n";
	}
	// Override status
	{
		colorScheme[0] = UI::black;
		colorScheme[1] = UI::dark_red;
		UI::setTextColors(colorScheme[0], colorScheme[1]);
		std::cout << "(o) Override:		";
		// on
		if (ProgramSettings::detectionMode != 2)
		{
			std::cout << "[";
			UI::setTextColors(colorScheme[1], colorScheme[0]);
			std::cout << "Off";
			UI::setTextColors(colorScheme[0], colorScheme[1]);
			std::cout << "] [";
			std::cout << "On";
			std::cout << "]";
		}
		// off
		else if (ProgramSettings::detectionMode == 2)
		{
			std::cout << "[";
			std::cout << "Off";
			std::cout << "] [";
			UI::setTextColors(colorScheme[1], colorScheme[0]);
			std::cout << "On";
			UI::setTextColors(colorScheme[0], colorScheme[1]);
			std::cout << "]";
		}
		std::cout << "\n";
	}
	// Override Selection
	{
		UI::setTextColors(UI::black, UI::dark_red);
		std::cout << "(p) Current Override:	";
		UI::setTextColors(UI::dark_red, UI::white);
		std::cout << ProgramSettings::currentOverride;
		std::cout << "\n";
	}
	// a little space
	std::cout << "\n";
	// other functions
	UI::setTextColors(UI::dark_gray, UI::white);
	std::cout << "Other Functions:" << "\n";
	UI::setTextColors(UI::black, UI::white);
	std::cout << "a  :  Configure AFK detection bypasses" << "\n";
	std::cout << "c  :  Configure Conduits" << "\n";
	std::cout << "d  :  View/edit Focus descriptions" << "\n";
	std::cout << "g  :  Manage Groups" << "\n";
	std::cout << "s  :  Manage Spotlight 5" << "\n";
	std::cout << "esc:  exit" << "\n";
	std::cout << "\n";
	// user activity report
	if (ProgramCache::userActivity)
		std::cout << "(User is active.)";
	else
		std::cout << "(User is inactive.)";
	// make room
	std::cout << "\n";
	UI::resetTextColors();
}

void cycleDisplayMode()
{
	if (ProgramSettings::Display::displayMode == 0)
		ProgramSettings::Display::displayMode = 1;
	else if (ProgramSettings::Display::displayMode == 1)
		ProgramSettings::Display::displayMode = 2;
	else if (ProgramSettings::Display::displayMode == 2)
		ProgramSettings::Display::displayMode = 3;
	else if (ProgramSettings::Display::displayMode == 3)
		ProgramSettings::Display::displayMode = 4;
	else if (ProgramSettings::Display::displayMode == 4)
		ProgramSettings::Display::displayMode = 5;
	else if (ProgramSettings::Display::displayMode == 5)
		ProgramSettings::Display::displayMode = 0;
}
void cycleChronoMode()
{
	if (ProgramSettings::Display::displayChronologicalType == 1)
		ProgramSettings::Display::displayChronologicalType = 2;
	else if (ProgramSettings::Display::displayChronologicalType == 2)
		ProgramSettings::Display::displayChronologicalType = 3;
	else if (ProgramSettings::Display::displayChronologicalType == 3)
		ProgramSettings::Display::displayChronologicalType = 4;
	else if (ProgramSettings::Display::displayChronologicalType == 4)
		ProgramSettings::Display::displayChronologicalType = 5;
	else if (ProgramSettings::Display::displayChronologicalType == 5)
		ProgramSettings::Display::displayChronologicalType = 1;
}
void toggleConduitMode()
{
	ProgramSettings::Display::conduitMode = !ProgramSettings::Display::conduitMode;
}
void toggleSpotlightMode()
{
	ProgramSettings::Display::spotlightMode = !ProgramSettings::Display::spotlightMode;
}
void toggleOverrideMode()
{
	if (ProgramSettings::detectionMode != 2)
	{
		ProgramControl::Threads::checkUserActivityThread = false;
		ProgramCache::userActivity = true;
		ProgramSettings::detectionMode = 2;
	}
	else if (ProgramSettings::detectionMode == 2)
	{
		ProgramControl::Threads::checkUserActivityThread = true;
		ProgramSettings::detectionMode = 1;
	}
}

void determineFocusColor(focusWindow &passThrough)
{
	int backgroundFocus = 0;
	int foregroundFocus = 7;
	if (ProgramSettings::Display::conduitMode)
		if (passThrough.conduits[ProgramCache::theTime->tm_hour])
			foregroundFocus = UI::light_cyan;
	if (ProgramSettings::Display::spotlightMode)
		if (passThrough.isInSpotlight)
			backgroundFocus = UI::dark_blue;
	UI::setTextColors(backgroundFocus, foregroundFocus);
}

void createOverride()
{
	// allocate some memory
	focusWindow xContainer = {};
	// populate the object
	makeFocusWindow(xContainer);
	UI::clearIt();
	// prompt the user for input
	std::cout << "Please enter a new Override focus:  ";
	xContainer.windowTitle = UI::getField(false);
	/// needs check for existing entries and "" escape feature
	std::cout << "Please enter a description:  ";
	xContainer.description = UI::getField(false);
	// mirror the input, confirm
	UI::clearIt();
	std::cout << "You have entered..." << "\n\n";
	std::cout << "Override:  " << xContainer.windowTitle << "\n";
	std::cout << "Description:  " << xContainer.description << "\n\n";
	bool acceptOverride = UI::commitChanges();
	std::cout << "\n\n";
	if (acceptOverride)
	{
		xContainer.windowClass = "[Custom Override]";
		xContainer.writeMe = true;
		ProgramCache::trackingSession.push_back(xContainer); 
		UI::setTextColors(UI::black, UI::dark_green);
		std::cout << "Override accepted." << "\n\n";
		UI::resetTextColors();
		std::cout << "Set as current override and return to tracker ";
		bool returnToTracker = UI::yesNo();
		std::cout << "\n\n";
		if (returnToTracker)
		{
			UI::setTextColors(UI::black, UI::dark_green);
			ProgramSettings::currentOverride = xContainer.windowTitle;
			std::cout << "Current override updated..." << "\n\n";
			bool overrideStatus;
			if (ProgramSettings::detectionMode == 2)
				overrideStatus = true;
			else overrideStatus = false;
			std::cout << "Override tracking status:  ";
			if (ProgramSettings::Display::displayMode != 2)
				UI::setTextColors(UI::black, UI::light_red);
			else UI::setTextColors(UI::black, UI::dark_green);
			std::cout << StringTools::onOffString(overrideStatus);
			if (!overrideStatus)
			{
				std::cout << "\n\n"; 
				std::cout << "Engage Override mode ";
				if (bool engageOverride = UI::yesNo())
				{
					ProgramSettings::detectionMode = 2;
				}
				else ProgramControl::bypassOverrideQuery = true;
			}
			std::this_thread::sleep_for(std::chrono::seconds(2));
			ProgramControl::overrideMenu = false;
			std::cout << "\n";
			UI::resetTextColors();
			return;
		}
	}
	else
	{
		UI::clearIt();
		UI::setTextColors(UI::black, UI::light_red);
		std::cout << "Override entry aborted.  (Please wait...)";
		UI::resetTextColors();
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}
	//UI::clearIt();
	//UI::setTextColors(UI::black, UI::dark_green);
	//std::cout << "Restarting tracking display.  (Please wait...)";
	//UI::resetTextColors();
}

void chooseOverride(Menu &passThrough)
{
	UI::clearIt();
	passThrough.clearMenu();
	for (unsigned int x = 0; x < ProgramCache::trackingSession.size(); x++)
		if (ProgramCache::trackingSession[x].windowClass == "[Custom Override]")
			passThrough.setMenu(ProgramCache::trackingSession[x].windowTitle);
	if (passThrough.choices.size() == 0)
	{
		UI::setTextColors(UI::black, UI::light_red);
		std::cout << "No Overrides found.  (Please wait...)";
		UI::resetTextColors();
		std::this_thread::sleep_for(std::chrono::seconds(2));
		return;
	}
	else if (passThrough.choices.size() > 0)
	{
		std::cout << "Select an Override:";
		std::cout << "\n\n";
		unsigned int overrideSelected = passThrough.selectItem();
		if (overrideSelected == 0)
		{
			UI::clearIt();
			UI::setTextColors(UI::black, UI::light_red);
			std::cout << "Override selection aborted.  (Please wait...)";
			UI::resetTextColors();
			std::this_thread::sleep_for(std::chrono::seconds(2));
		}
		else
			ProgramSettings::currentOverride = passThrough.choices[overrideSelected - 1];
	}
}