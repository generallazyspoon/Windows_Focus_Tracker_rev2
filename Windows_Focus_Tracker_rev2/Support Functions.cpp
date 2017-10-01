#include "stdafx.h"
#include "Windows_Focus_Tracker_rev2.h"
#include "../../GA_Library/GA_Library/GA_Library.h"

// function for updating the total days, hours, minutes and seconds inside the specified focusWindow object
void updateTotalDayHourMinSec(focusWindow &passThrough)
{
	// a couple of control bools
	bool rotateMinute = false;
	bool rotateHour = false;
	bool rotateDay = false;
	// screen for any clock changes
	if (passThrough.totalDayHourMinSec[3] + ProgramSettings::scanInterval > 59) {
		rotateMinute = true;
		if (passThrough.totalDayHourMinSec[2] + 1 > 59) {
			rotateHour = true;
			if (passThrough.totalDayHourMinSec[1] + 1 > 23)
				rotateDay = true; } }
	// modify the applicable values 
	if (!rotateMinute) {
		passThrough.totalDayHourMinSec[3] += ProgramSettings::scanInterval;
		return; }
	else if (rotateMinute) {
		passThrough.totalDayHourMinSec[3] = passThrough.totalDayHourMinSec[3] + ProgramSettings::scanInterval - 60;
		passThrough.totalDayHourMinSec[2] += 1;
		if (rotateHour) {
			passThrough.totalDayHourMinSec[2] = 0;
			passThrough.totalDayHourMinSec[1] += 1;
			if (rotateDay) {
				passThrough.totalDayHourMinSec[1] = 0;
				passThrough.totalDayHourMinSec[0] += 1; } } }
}

// function to populate a new focusWindow object with default parameters
void makeFocusWindow(focusWindow &passThrough)
{
	passThrough.windowClass = "";
	passThrough.windowTitle = "";
	passThrough.description = "";
	passThrough.overridesAFK = false;
	passThrough.isInSpotlight = false;
	for (unsigned int x = 0; x < 24; x++)
		passThrough.conduits[x] = false;
	passThrough.focusLimits[0] = 0;
	passThrough.focusLimits[1] = 24;
	passThrough.focusLimits[2] = 0;
	passThrough.focusLimits[3] = 168;
	passThrough.focusLimits[4] = 0;
	passThrough.focusLimits[5] = 744;
	passThrough.focusLimits[6] = 0;
	passThrough.focusLimits[7] = 8784;

	passThrough.warningTimings[0] = 15;
	passThrough.warningTimings[1] = 15;

	for (unsigned int x = 0; x < 4; x++)
		passThrough.totalDayHourMinSec[x] = 0;

	for (unsigned int x = 0; x < 366; x++)
		for (unsigned int y = 0; y < 24; y++)
			passThrough.yearlyStats[x][y] = 0;

	passThrough.writeMe = true;
}

void displayFilterBase(focusWindow &sortMe)
{
	sortLastFive(sortMe);
	sortTopFive(sortMe);
}

void sortLastFive(focusWindow &sortMe)
{
	/// type "A"
	// start assuming the item is not on the list
	unsigned int foundAtIndex = 5;
	// check for the item in the list
	for (unsigned int x = 0; x < 5; x++)
		if (sortMe.windowClass == ProgramCache::lastFiveFocus[x].windowClass &&
			sortMe.windowTitle == ProgramCache::lastFiveFocus[x].windowTitle) {
			foundAtIndex = x;
			break; }
	// if it's the most recent, just replace it
	if (foundAtIndex == 0)
	{
		ProgramCache::lastFiveFocus[0] = sortMe;
		return;
	}
	// if it's not found, put it at the top, shift the others down
	if (foundAtIndex == 5)
	{
		for (unsigned int x = 4; x > 0; x = x - 1)
			ProgramCache::lastFiveFocus[x] = ProgramCache::lastFiveFocus[x - 1];
		ProgramCache::lastFiveFocus[0] = sortMe;
		return;
	}
	// if it's found, shift the more recent entries down one and put the new entry at the top
	for (unsigned int x = foundAtIndex; x > 0; x = x - 1)
		ProgramCache::lastFiveFocus[x] = ProgramCache::lastFiveFocus[x - 1];
	ProgramCache::lastFiveFocus[0] = sortMe;
	/// end type "A"

	/// type "B"
	/*
	// check for existing entries, update if found.
	std::vector<unsigned int> entriesFound = {};
	for (unsigned int x = 0; x < 5; x++)
		if (sortMe.windowClass == ProgramCache::lastFiveFocus[x].windowClass &&
			sortMe.windowTitle == ProgramCache::lastFiveFocus[x].windowTitle)
			entriesFound.push_back(x);
	// update all existing entries
	if (entriesFound.size() > 0) {
		for (unsigned int x = 0; x < entriesFound.size(); x++)
			ProgramCache::lastFiveFocus[entriesFound[x]] = sortMe; }
	// if current, update & move on
	if (ProgramCache::lastFiveFocus[0].windowTitle == sortMe.windowTitle) {
		ProgramCache::lastFiveFocus[0] = sortMe;
		return;
	}
	// otherwise, pull all the other entries down, add sortMe to the top of the list
	ProgramCache::lastFiveFocus[4] = ProgramCache::lastFiveFocus[3];
	ProgramCache::lastFiveFocus[3] = ProgramCache::lastFiveFocus[2];
	ProgramCache::lastFiveFocus[2] = ProgramCache::lastFiveFocus[1];
	ProgramCache::lastFiveFocus[1] = ProgramCache::lastFiveFocus[0];
	ProgramCache::lastFiveFocus[0] = sortMe;
	*/
	/// end type "B"
	
	// update the cross-session data
	for (unsigned int x = 0; x < 5; x++) {
		ProgramSettings::lastFiveFocusTitle[x] = ProgramCache::lastFiveFocus[x].windowTitle;
		ProgramSettings::lastFiveFocusClass[x] = ProgramCache::lastFiveFocus[x].windowClass; }
}

void sortTopFive(focusWindow &sortMe)
{
	// check if the item is already on the list
	unsigned int atPosition = 5;
	for (unsigned int x = 0; x < 5; x++)
		if (sortMe.windowTitle == ProgramCache::topFiveFocus[x].windowTitle &&
			sortMe.windowClass == ProgramCache::topFiveFocus[x].windowClass) {
			atPosition = x;
			break; }

	// get the total
	double sortMeTotal = getTotalTime(sortMe);
	// declare some memory for comparison
	double compareMeTotal = 0;

	if (atPosition == 5) {
		for (unsigned int x = 0; x < 5; x++) {
			compareMeTotal = getTotalTime(ProgramCache::topFiveFocus[x]);
			if (sortMeTotal >= compareMeTotal) {
				for (unsigned int y = 4; y > x; y--)
					ProgramCache::topFiveFocus[y] = ProgramCache::topFiveFocus[y - 1];
				ProgramCache::topFiveFocus[x] = sortMe;
				return;
			}
		}
	}
	
	if (atPosition == 0) {
		ProgramCache::topFiveFocus[0] = sortMe;
		return; }

	if (atPosition < 5 && atPosition > 0) {
		for (unsigned int x = atPosition; x > 0; x--) {
			compareMeTotal = getTotalTime(ProgramCache::topFiveFocus[x - 1]);
			if (sortMeTotal >= compareMeTotal) {
				ProgramCache::topFiveFocus[x] = ProgramCache::topFiveFocus[x - 1];
				ProgramCache::topFiveFocus[x - 1] = sortMe;
				return;
			}
			else {
				ProgramCache::topFiveFocus[atPosition] = sortMe;
				return;
			}
			return;
		}
	}
}

double getTotalTime(focusWindow &readMe)
{
	return (readMe.totalDayHourMinSec[0] * 86400) +
		(readMe.totalDayHourMinSec[1] * 3600) +
		(readMe.totalDayHourMinSec[2]) * 60 +
		readMe.totalDayHourMinSec[3];
}

void populateFilters()
{
	unsigned int z = 0;
	for (unsigned int x = 0; x < ProgramCache::trackingSession.size(); x++)
		for (unsigned int y = 0; y < 5; y++)
			if (ProgramCache::trackingSession[x].windowTitle == ProgramSettings::lastFiveFocusTitle[y] &&
				ProgramCache::trackingSession[x].windowClass == ProgramSettings::lastFiveFocusClass[y]) {
				ProgramCache::lastFiveFocus[z] = ProgramCache::trackingSession[x];
				z++;
				break;
			}
	
	if (ProgramCache::trackingSession.size() > 0)
		for (unsigned int x = 0; x < ProgramCache::trackingSession.size(); x++)
			sortTopFive(ProgramCache::trackingSession[x]);

	unsigned int spotlightFivePosition = 0;
	for (unsigned int x = 0; x < ProgramCache::trackingSession.size(); x++)
	{
		if (ProgramCache::trackingSession[x].isInSpotlight)
		{
			ProgramCache::spotlightFiveFocus[spotlightFivePosition] = ProgramCache::trackingSession[x];
			spotlightFivePosition++;
			if (spotlightFivePosition >= 5)
				return;
		}
	}
	UI::clearIt();
	UI::setTextColors(UI::black, UI::dark_green);
	std::cout << "Populated cross-session display filters OK.";
	UI::resetTextColors();

}

void setLeapYearStatus()
{
	time_t ldt;
	time(&ldt);
	localtime_s(ProgramCache::theTime, &ldt);

	int thisYear = ProgramCache::theTime->tm_year;
	int lastYear = ProgramCache::theTime->tm_year - 1;
	if (lastYear % 4 == 0 && (lastYear % 100 != 0 || lastYear % 400 == 0))
		ProgramCache::lastYearSize = 366;
	if (thisYear % 4 == 0 && (thisYear % 100 != 0 || thisYear % 400 == 0))
	{
		ProgramCache::thisYearSize = 366;
		ProgramCache::leapModifier = 1;
	}
	UI::clearIt();
	UI::setTextColors(UI::black, UI::dark_green);
	std::cout << "Set leap year status OK.";
	UI::resetTextColors();
}