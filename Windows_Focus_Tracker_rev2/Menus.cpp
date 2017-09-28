#include "stdafx.h"
#include "../../GA_Library/GA_Library/GA_Library.h"
#include "Windows_Focus_Tracker_rev2.h"

void enterMenu()
{
	ProgramControl::Threads::displayThread = false;
	ProgramControl::Threads::checkForUserCommandsThread = false;
	ProgramControl::Threads::checkUserActivityThread = false;
	ProgramControl::Threads::regularlyWriteDataThread = false;
	std::this_thread::sleep_for(std::chrono::seconds(2));
}

void exitMenu()
{
	ProgramControl::Threads::displayThread = true;
	ProgramControl::Threads::checkForUserCommandsThread = true;
	ProgramControl::Threads::checkUserActivityThread = true;
	ProgramControl::Threads::regularlyWriteDataThread = true;
}

namespace WFT_Menus
{
	void populateOverrideMenu(Menu &passThrough) {
		passThrough.clearMenu();
		passThrough.setMenu("Create an Override");
		passThrough.setMenu("Select from known Overrides");
		passThrough.setMenu("Toggle Override mode");
	}
	void overrideMenu(Menu &passThrough);
	void populateGroupMenu(Menu &passThrough, std::string custom) {
		passThrough.clearMenu();
		passThrough.setMenu("View focus groups");
		passThrough.setMenu("Create a focus group");
		passThrough.setMenu("Delete a focus group");
		passThrough.setMenu("Add a focus to a group");
	}
	void groupMenu(Menu &passThrough);
}

void WFT_Menus::overrideMenu(Menu &passThrough)
{
	std::string previousOverride = ProgramSettings::currentOverride;
	ProgramControl::bypassOverrideQuery = false;
	enterMenu();
	unsigned int thisMenuSelection;
	do {
		UI::clearIt();
		UI::setTextColors(UI::light_red, UI::white);
		std::cout << "Current Override:";
		UI::resetTextColors();
		std::cout << "  " << ProgramSettings::currentOverride;
		std::cout << "\n";
		UI::setTextColors(UI::light_red, UI::white);
		std::cout << "Override status:";
		UI::resetTextColors();
		std::cout << "  ";
		if (ProgramSettings::detectionMode == 2)
			UI::setTextColors(UI::light_red, UI::white);
		else
			UI::resetTextColors();
		std::cout << StringTools::onOffString(ProgramSettings::detectionMode == 2);
		std::cout << "\n\n";
		UI::resetTextColors(); 
		std::cout << "Override selection:" << "\n\n";
		// Display the menu
		passThrough.clearMenu();
		WFT_Menus::populateOverrideMenu(passThrough);
		passThrough.displayMenu();
		thisMenuSelection = UI::selector(passThrough.choices.size());
		switch (thisMenuSelection)
		{
		case 0:
			break;
		// create an override
		case 1:
			createOverride();
			break;
		// select an existing override
		case 2:
			chooseOverride(passThrough);
			break;
		case 3:
			toggleOverrideMode();
			break;
		default:
			DebugTools::consoleDebugMarker("Warning: menu default case reached", true);
			break;
		}
	} while (thisMenuSelection > 0 && ProgramControl::overrideMenu);
	UI::setTextColors(UI::black, UI::dark_green);
	std::cout << "\n";
	if (ProgramSettings::currentOverride != previousOverride && ProgramSettings::detectionMode != 2 &&
		!ProgramControl::bypassOverrideQuery)
	{
		UI::setTextColors(UI::black, UI::light_red);
		std::cout << "Override has changed.  Activate Override mode ";
		bool engageOverride = UI::yesNo();
		if (engageOverride)
		{
			ProgramSettings::detectionMode = 2;
			UI::setTextColors(UI::black, UI::dark_green);
			std::cout << "\n\n";
			std::cout << "Override mode activated...";
			std::cout << "\n\n";
		}
	}
	std::cout << "Returning to tracker.  (Please wait...)";
	std::this_thread::sleep_for(std::chrono::seconds(2));
	UI::resetTextColors();
	exitMenu();
}

void WFT_Menus::groupMenu(Menu &passThrough)
{
	enterMenu();
	focusWindowGroup xContainer;
	unsigned int thisMenuSelection;
	std::list<unsigned int> focusIndex;
	do {
		UI::clearIt();
		std::cout << "Group menu:" << "\n\n";
		UI::resetTextColors();
		passThrough.clearMenu();
		// Establish the menu
		WFT_Menus::populateGroupMenu(passThrough, "");
		// Display the menu
		passThrough.displayMenu();
		thisMenuSelection = UI::selector(passThrough.choices.size());
		switch (thisMenuSelection)
		{
		case 0:
			break;
		case 1:
			// view groups
			if (ProgramSettings::groupList.size() == 0)
			{
				UI::clearIt();
				UI::setTextColors(UI::black, UI::light_red);
				std::cout << "No groups found.";
				std::this_thread::sleep_for(std::chrono::seconds(2));
				break;
			}
			else
				viewGroup(passThrough);
			break;
		case 2:
			// create group
			createGroup(passThrough);
			break;
		case 3:
			/// delete a group
			deleteGroup(passThrough);
			break;
		case 4:
			/// add focus to group
			populateGroup(passThrough);
			break;
		default:
			DebugTools::consoleDebugMarker("Warning: menu default case reached", true);
			break;
		}
	} while (thisMenuSelection > 0);
	UI::setTextColors(UI::black, UI::dark_green);
	std::cout << "Returning to tracker.  (Please wait...)";
	std::this_thread::sleep_for(std::chrono::seconds(2));
	UI::resetTextColors();
	exitMenu();
}

unsigned int groupSelectDisplay(Menu &passThrough)
{
	passThrough.clearMenu();
	for (unsigned int x = 0; x < ProgramSettings::groupList.size(); x++)
		passThrough.setMenu(ProgramSettings::groupList[x].groupName);
	if (passThrough.choices.size() == 0)
	{
		UI::clearIt();
		UI::setTextColors(UI::black, UI::light_red);
		std::cout << "No groups to select from...";
		UI::resetTextColors();
		std::this_thread::sleep_for(std::chrono::seconds(2));
		return 0;
	}
	
	passThrough.displayMenu();
	return UI::selector(passThrough.choices.size());
}

void viewGroup(Menu &passThrough)
{
	passThrough.clearMenu();
	unsigned int groupSelection;
	
	do {
		UI::clearIt();
		// select group to view
		std::cout << "Select a group to view:" << "\n\n";
		groupSelection = groupSelectDisplay(passThrough);
		// display group selected
		if (groupSelection != 0)
		{
			UI::clearIt();
			UI::setTextColors(UI::black, UI::white);
			std::cout << "Group Name:   ";
			UI::resetTextColors();
			std::cout << ProgramSettings::groupList[groupSelection - 1].groupName << "\n";
			UI::setTextColors(UI::black, UI::white);
			std::cout << "Description:  ";
			UI::resetTextColors();
			std::cout << ProgramSettings::groupList[groupSelection - 1].groupDescription << "\n";
			UI::setTextColors(UI::black, UI::white);
			// display members of that group
			std::cout << "Members:" << "\n";
			if (ProgramSettings::groupList[groupSelection - 1].members.size() == 0)
			{
				UI::setTextColors(UI::black, UI::dark_green);
				std::cout << "(No members in group.)";
				UI::resetTextColors();
				break;
			}
			// member info
			for (unsigned int x = 0; x < ProgramSettings::groupList[groupSelection - 1].members.size(); x++) {
				UI::setTextColors(UI::black, UI::dark_green);
				std::cout << ProgramSettings::groupList[groupSelection - 1].members[x].className;
				std::cout << " - ";
				std::cout << ProgramSettings::groupList[groupSelection - 1].members[x].titleName;
				std::cout << "\n";
				UI::setTextColors(UI::black, UI::dark_cyan);
				for (unsigned int y = 0; y < ProgramCache::trackingSession.size(); y++)
					if (ProgramSettings::groupList[groupSelection - 1].members[x].className == ProgramCache::trackingSession[y].windowClass &&
						ProgramSettings::groupList[groupSelection - 1].members[x].titleName == ProgramCache::trackingSession[y].windowTitle) {
						std::cout << ProgramCache::trackingSession[y].description;
						break; }
				UI::resetTextColors(); }
			UI::pressEnterToContinue(); }
	} while (groupSelection > 0);
}

void createGroup(Menu &passThrough)
{
	// set a few standards
	passThrough.clearMenu();
	focusWindowGroup xContainer;
	// display
	UI::clearIt();
	std::cout << "Current groups:" << "\n\n";
	if (ProgramSettings::groupList.size() == 0)
	{
		UI::setTextColors(UI::black, UI::dark_green);
		std::cout << "No groups found.";
		UI::resetTextColors();
		std::cout << "\n";
	}
	else
		for (unsigned int x = 0; x < ProgramSettings::groupList.size(); x++)
			passThrough.setMenu(ProgramSettings::groupList[x].groupName);
	passThrough.displayMenu();
	std::cout << "\n";
	// gather the new group name
	bool groupExistsAlready = false;
	do {
		UI::setTextColors(UI::black, UI::white);
		std::cout << "Please enter a new group name (press enter to exit):  ";
		UI::resetTextColors();
		xContainer.groupName = UI::getField(true);
		// check for existing groups (by name only)
		if (ProgramSettings::groupList.size() != 0)
			for (unsigned int x = 0; x < ProgramSettings::groupList.size(); x++)
				if (xContainer.groupName == ProgramSettings::groupList[x].groupName)
				{
					groupExistsAlready = true;
					std::cout << "\n";
					UI::setTextColors(UI::black, UI::light_red);
					std::cout << "Group already exists... Please try again." << "\n\n";
					UI::resetTextColors();
					break;
				}
				else
					groupExistsAlready = false;
	} while (groupExistsAlready == true && xContainer.groupName != "");
	// safe exit code
	if (xContainer.groupName == "")
		return;
	// gather description
	std::cout << "\n";
	UI::setTextColors(UI::black, UI::white);
	std::cout << "Please enter a description for this group (press enter to skip):  ";
	UI::resetTextColors();
	xContainer.groupDescription = UI::getField(true);
	// confirmation process
	UI::clearIt();
	std::cout << "You have entered:" << "\n\n";
	UI::setTextColors(UI::black, UI::white);
	std::cout << "Name:";
	UI::resetTextColors();
	std::cout << "  " << xContainer.groupName << "\n";
	UI::setTextColors(UI::black, UI::white);
	std::cout << "Description:";
	UI::resetTextColors();
	std::cout << "  " << xContainer.groupDescription << "\n\n";
	UI::setTextColors(UI::black, UI::light_red);
	bool commitGroup = UI::commitChanges();
	std::cout << "\n\n";
	if (commitGroup)
	{
		xContainer.writeMe = true;
		ProgramSettings::groupList.push_back(xContainer);
		UI::setTextColors(UI::black, UI::dark_green);
		std::cout << "Commited group to database.  (Please wait...)";
		UI::resetTextColors();
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}
	else
	{
		UI::setTextColors(UI::black, UI::light_red);
		std::cout << "Group creation aborted.  (Please wait...)";
		UI::resetTextColors();
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}
}

void deleteGroup(Menu &passThrough)
{
	passThrough.clearMenu();
	UI::clearIt();
	// early exit code if no groups found
	if (ProgramSettings::groupList.size() == 0)
	{
		UI::setTextColors(UI::black, UI::light_red);
		std::cout << "No groups found.  (Please wait...)";
		std::this_thread::sleep_for(std::chrono::seconds(2));
		return;
	}
	// otherwise...
	UI::setTextColors(UI::black, UI::white);
	std::cout << "Select a group to delete:" << "\n\n";
	UI::resetTextColors();
	unsigned int chooseThisGroup = groupSelectDisplay(passThrough);
	if (chooseThisGroup == 0)
		return;
	else
	{
		UI::clearIt();
		std::cout << "You have selected:" << "\n\n";
		UI::setTextColors(UI::black, UI::white);
		std::cout << "Group name:  ";
		UI::setTextColors(UI::black, UI::dark_green);
		std::cout << ProgramSettings::groupList[chooseThisGroup - 1].groupName << "\n";
		UI::setTextColors(UI::black, UI::white);
		std::cout << "Group description:  ";
		UI::setTextColors(UI::black, UI::dark_green);
		std::cout << ProgramSettings::groupList[chooseThisGroup - 1].groupName << "\n";
		UI::setTextColors(UI::black, UI::white);
		std::cout << "Group members:  ";
		if (ProgramSettings::groupList[chooseThisGroup - 1].members.size() > 0)
			for (unsigned int x = 0; x < ProgramSettings::groupList[chooseThisGroup - 1].members.size(); x++) {
				UI::setTextColors(UI::black, UI::dark_green);
				std::cout << ProgramSettings::groupList[chooseThisGroup - 1].members[x].className;
				UI::resetTextColors();
				std::cout << " - ";
				UI::setTextColors(UI::black, UI::dark_green);
				std::cout << ProgramSettings::groupList[chooseThisGroup - 1].members[x].titleName;
				UI::resetTextColors();
				std::cout << "\n";
				for (unsigned int y = 0; y < ProgramCache::trackingSession.size(); y++)
					if (ProgramSettings::groupList[chooseThisGroup - 1].members[x].className == ProgramCache::trackingSession[y].windowClass &&
						ProgramSettings::groupList[chooseThisGroup - 1].members[x].titleName == ProgramCache::trackingSession[y].windowTitle) {
						std::cout << ProgramCache::trackingSession[y].description;
						break; } }
		std::cout << "\n\n";
		UI::setTextColors(UI::black, UI::light_red);
		bool commitGroup = UI::commitChanges();
		std::cout << "\n\n";
		if (commitGroup)
		{
			remove(("groups/" + ProgramSettings::groupList[chooseThisGroup - 1].groupName + ".cfg").c_str());
			ProgramSettings::groupList.erase(ProgramSettings::groupList.begin() + chooseThisGroup - 1);
			std::cout << "Group deleted.  (Please wait...)";
			std::this_thread::sleep_for(std::chrono::seconds(2));
			return;
		}
		else
		{
			std::cout << "Group delete aborted.  (Please wait...)";
			std::this_thread::sleep_for(std::chrono::seconds(2));
		}
	}
}

void populateGroup(Menu &passThrough)
{
	passThrough.clearMenu();
	UI::clearIt();
	// select group to populate
	std::cout << "Select a group to populate:" << "\n\n";
	unsigned int groupSelection = groupSelectDisplay(passThrough);
	// safe exit code
	if (groupSelection == 0)
		return;
	
	/// display total # group members (when selecting group)


	unsigned int addThisFocusToGroup;

	do {
		passThrough.clearMenu();
		for (unsigned int x = 0; x < ProgramCache::trackingSession.size(); x++) {
			passThrough.setMenu(ProgramCache::trackingSession[x].windowClass + " - " + ProgramCache::trackingSession[x].windowTitle);
			passThrough.toggledOn.push_back(false);	}
		
		for (unsigned int x = 0; x < ProgramCache::trackingSession.size(); x++)
			for (unsigned int y = 0; y < ProgramSettings::groupList[groupSelection - 1].members.size(); y++)
				if (ProgramSettings::groupList[groupSelection - 1].members[y].className == ProgramCache::trackingSession[x].windowClass &&
					ProgramSettings::groupList[groupSelection - 1].members[y].titleName == ProgramCache::trackingSession[x].windowTitle)
					passThrough.toggledOn[x] = true;
		
		// display group selected
		UI::clearIt();
		UI::setTextColors(UI::black, UI::white);
		std::cout << "Group Name:   ";
		UI::resetTextColors();
		std::cout << ProgramSettings::groupList[groupSelection - 1].groupName << "\n";
		UI::setTextColors(UI::black, UI::white);
		std::cout << "Description:  ";
		UI::resetTextColors();
		std::cout << ProgramSettings::groupList[groupSelection - 1].groupDescription << "\n";		
		std::cout << "\n";

		std::cout << "Toggle the members you wish to include in this group:" << "\n\n";
		std::cout << "(Dark green is toggled \"on\")";
		std::cout << "\n\n";

		for (unsigned int x = 0; x < passThrough.choices.size(); x++) {
			if (passThrough.toggledOn[x])
				UI::setTextColors(UI::black, UI::dark_green);
			std::cout << x + 1 << ".  " << passThrough.choices[x] << "\n";
			UI::resetTextColors(); }
		std::cout << "\n";
		// select the group to toggle on/off
		addThisFocusToGroup = UI::selector(passThrough.choices.size());
		// early exit code
		if (addThisFocusToGroup == 0) return;
		// add to group struct
		if (!passThrough.toggledOn[addThisFocusToGroup - 1])
		{
			passThrough.toggledOn[addThisFocusToGroup - 1] = true; 
			ProgramSettings::groupList[groupSelection - 1].members.push_back({
			ProgramCache::trackingSession[addThisFocusToGroup - 1].windowClass,
			ProgramCache::trackingSession[addThisFocusToGroup - 1].windowTitle });
		}
		else {
			passThrough.toggledOn[addThisFocusToGroup - 1] = false;
			bool foundIt = false;
			for (unsigned int x = 0; x < ProgramCache::trackingSession.size(); x++) {
				for (unsigned int y = 0; y < ProgramSettings::groupList[groupSelection - 1].members.size(); y++) {
					DebugTools::consoleDebugMarker(std::to_string(ProgramSettings::groupList[groupSelection - 1].members.size()), false);
					if (ProgramCache::trackingSession[x].windowClass == ProgramSettings::groupList[groupSelection - 1].members[y].className &&
						ProgramCache::trackingSession[x].windowTitle == ProgramSettings::groupList[groupSelection - 1].members[y].titleName) {
						ProgramSettings::groupList[groupSelection - 1].members.erase(ProgramSettings::groupList[groupSelection - 1].members.begin() + y);
						DebugTools::consoleDebugMarker(std::to_string(ProgramSettings::groupList[groupSelection - 1].members.size()), true);
						foundIt = true;
						break; } }
				if (foundIt)
					break; } }
		std::cout << "\n";
	} while (addThisFocusToGroup > 0);
}

void setOverrideAFKFlag(Menu &passThrough)
{
	enterMenu();
	unsigned int overrideAFKToggle = {};
	passThrough.clearMenu();
	for (unsigned int x = 0; x < ProgramCache::trackingSession.size(); x++)
		passThrough.setMenu(ProgramCache::trackingSession[x].windowClass + " - " + ProgramCache::trackingSession[x].windowTitle);
	for (unsigned int x = 0; x < ProgramCache::trackingSession.size(); x++)
		if (ProgramCache::trackingSession[x].overridesAFK)
			passThrough.toggledOn.push_back(true);
		else passThrough.toggledOn.push_back(false);
	do {
		UI::clearIt();
		UI::setTextColors(UI::black, UI::white);
		std::cout << "Select the focus or override whose properties you wish to change:" << "\n\n";
		std::cout << "Focuses that disable AFK detection are marked in ";
		UI::setTextColors(UI::black, UI::dark_green);
		std::cout << "dark green";
		UI::resetTextColors();
		std::cout << "." << "\n\n";
		overrideAFKToggle = passThrough.toggleItem(UI::dark_green);
		if (overrideAFKToggle != 0)
		{
			ProgramCache::trackingSession[overrideAFKToggle - 1].overridesAFK = !ProgramCache::trackingSession[overrideAFKToggle - 1].overridesAFK;
			ProgramCache::trackingSession[overrideAFKToggle - 1].writeMe = true;
		}
	} while (overrideAFKToggle != 0);
	std::cout << "\n\n";
	UI::setTextColors(UI::black, UI::dark_green);
	std::cout << "Returning to tracker.  (Please wait...)";
	std::this_thread::sleep_for(std::chrono::seconds(2));
	UI::resetTextColors();
	exitMenu();
}

void descriptionManagement(Menu &passThrough)
{
	enterMenu();
	// memory
	unsigned int chosenTask = NULL;
	do {
		UI::clearIt();
		UI::setTextColors(UI::black, UI::white);
		std::cout << "Description Management Menu:" << "\n\n";
		UI::resetTextColors();
		passThrough.clearMenu();
		passThrough.setMenu("View a focus description");
		passThrough.setMenu("Modify a focus description");
		chosenTask = passThrough.selectItem();
		if (chosenTask != 0)
		{
			UI::clearIt();
			UI::setTextColors(UI::black, UI::white);
			if (chosenTask == 1)
				std::cout << "Select the focus whose description you would like to view:" << "\n\n";
			else if (chosenTask == 2)
				std::cout << "Select the focus whose description you would like to modify:" << "\n\n";
			UI::resetTextColors();
			for (unsigned int x = 0; x < ProgramCache::trackingSession.size(); x++)
			{
				std::cout << x + 1 << ".  ";
				UI::setTextColors(UI::black, UI::white);
				for (unsigned int y = 0; y < ProgramCache::trackingSession[x].windowClass.size(); y++)
					std::cout << ProgramCache::trackingSession[x].windowClass[y];
				UI::resetTextColors();
				std::cout << " - ";
				UI::setTextColors(UI::black, UI::white);
				for (unsigned int y = 0; y < ProgramCache::trackingSession[x].windowTitle.size(); y++)
					std::cout << ProgramCache::trackingSession[x].windowTitle[y];
				std::cout << "\n";
			}
			std::cout << "\n";
			unsigned int chosenFocus = UI::selector(ProgramCache::trackingSession.size());
			if (chosenFocus == 0)
				break;
			UI::clearIt();
			if (chosenTask == 1)
			{
				UI::resetTextColors();
				std::cout << "Class:  ";
				UI::setTextColors(UI::black, UI::white);
				std::cout << ProgramCache::trackingSession[chosenFocus - 1].windowClass;
				std::cout << "\n";
				UI::resetTextColors();
				std::cout << "Title:  ";
				UI::setTextColors(UI::black, UI::white);
				std::cout << ProgramCache::trackingSession[chosenFocus - 1].windowTitle;
				std::cout << "\n";
				UI::resetTextColors();
				std::cout << "Description:  ";
				UI::setTextColors(UI::black, UI::light_cyan);
				std::cout << ProgramCache::trackingSession[chosenFocus - 1].description;
				UI::resetTextColors();
				UI::pressEnterToContinue();
			}
			else if (chosenTask == 2)
			{
				UI::resetTextColors();
				std::cout << "Class:  ";
				UI::setTextColors(UI::black, UI::white);
				std::cout << ProgramCache::trackingSession[chosenFocus - 1].windowClass;
				std::cout << "\n";
				UI::resetTextColors();
				std::cout << "Title:  ";
				UI::setTextColors(UI::black, UI::white);
				std::cout << ProgramCache::trackingSession[chosenFocus - 1].windowTitle;
				std::cout << "\n";
				UI::resetTextColors();
				std::cout << "Description:  ";
				UI::setTextColors(UI::black, UI::light_cyan);
				std::cout << ProgramCache::trackingSession[chosenFocus - 1].description;
				std::cout << "\n\n";
				UI::resetTextColors();
				std::cout << "Please enter a new description:  ";
				UI::setTextColors(UI::black, UI::light_cyan);
				std::string newDescription = UI::getField(false);
				UI::resetTextColors();
				std::cout << "\n";
				bool commitNewDescription = UI::commitChanges();
				if (commitNewDescription)
				{
					ProgramCache::trackingSession[chosenFocus - 1].description = newDescription;
					ProgramCache::trackingSession[chosenFocus - 1].writeMe = true;
					std::cout << "\n\n";
					UI::setTextColors(UI::black, UI::dark_green);
					std::cout << "Description updated.  (Returning to menu...)";
					UI::resetTextColors();
					std::this_thread::sleep_for(std::chrono::seconds(2));
				}
				else
				{
					std::cout << "\n\n";
					UI::setTextColors(UI::black, UI::light_red);
					std::cout << "Changes aborted.  (Returning to menu...)";
					UI::resetTextColors();
					std::this_thread::sleep_for(std::chrono::seconds(2));
				}
			}
		}
	} while (chosenTask != 0);
	UI::setTextColors(UI::black, UI::dark_green);
	std::cout << "\n";
	std::cout << "Returning to tracker.  (Please wait...)";
	std::this_thread::sleep_for(std::chrono::seconds(2));
	UI::resetTextColors();
	exitMenu();
}

void conduitManagement(Menu &passThrough)
{
	enterMenu();
	unsigned int modifyTheseConduits;
	do {
		UI::clearIt();
		setTrackingSessionMenu(passThrough);
		UI::setTextColors(UI::black, UI::white);
		std::cout << "Please select the focus whose conduits you wish to view/modify:" << "\n\n";
		UI::resetTextColors();
		modifyTheseConduits = passThrough.selectItem();
		if (modifyTheseConduits == 0)
			break;
		UI::clearIt();
		setConduitMenu(passThrough);
		for (unsigned int x = 0; x < 24; x++)
			passThrough.toggledOn.push_back(ProgramCache::trackingSession[modifyTheseConduits - 1].conduits[x]);
		passThrough.toggleSelect(NULL, NULL, 
		{ 
			"Please select the conduit you wish to toggle for ", 
			ProgramCache::trackingSession[modifyTheseConduits - 1].windowClass,
			" - ",
			ProgramCache::trackingSession[modifyTheseConduits - 1].windowTitle,
			": ", 
			"\n\n"
		});
		for (unsigned int x = 0; x < 24; x++)
			ProgramCache::trackingSession[modifyTheseConduits - 1].conduits[x] = passThrough.toggledOn[x];
		ProgramCache::trackingSession[modifyTheseConduits - 1].writeMe = true;
	} while (modifyTheseConduits != 0);
	UI::setTextColors(UI::black, UI::dark_green);
	std::cout << "\n";
	std::cout << "Returning to tracker.  (Please wait...)";
	UI::resetTextColors();
	std::this_thread::sleep_for(std::chrono::seconds(2));
	exitMenu();
}

void spotlightManagement(Menu &passThrough)
{
	enterMenu();
	UI::clearIt();
	setTrackingSessionMenu(passThrough);
	for (unsigned int x = 0; x < ProgramCache::trackingSession.size(); x++)
		if (ProgramCache::trackingSession[x].isInSpotlight)
			passThrough.toggledOn.push_back(true);
		else passThrough.toggledOn.push_back(false);
	passThrough.toggleSelect(NULL, 5, {	"Please select the focuses (up to 5) you wish to place in the spotlight:", "\n\n" });
	for (unsigned int x = 0; x < ProgramCache::trackingSession.size(); x++) {
		if (ProgramCache::trackingSession[x].isInSpotlight != passThrough.toggledOn[x]) {
			ProgramCache::trackingSession[x].isInSpotlight = !ProgramCache::trackingSession[x].isInSpotlight;
			ProgramCache::trackingSession[x].writeMe = true; } }
	UI::setTextColors(UI::black, UI::dark_green);
	std::cout << "\n";
	std::cout << "Returning to tracker.  (Please wait...)";
	UI::resetTextColors();
	std::this_thread::sleep_for(std::chrono::seconds(2));
	exitMenu();
}

void setTrackingSessionMenu(Menu &passThrough)
{
	passThrough.clearMenu();
	for (unsigned int x = 0; x < ProgramCache::trackingSession.size(); x++)
		passThrough.setMenu(ProgramCache::trackingSession[x].windowClass + " - " + ProgramCache::trackingSession[x].windowTitle);
}

void setConduitMenu(Menu &passThrough) 
{
	passThrough.clearMenu();
	passThrough.setMenu("00:00 - 01:00");
	passThrough.setMenu("01:00 - 02:00");
	passThrough.setMenu("02:00 - 03:00");
	passThrough.setMenu("03:00 - 04:00");
	passThrough.setMenu("04:00 - 05:00");
	passThrough.setMenu("05:00 - 06:00");
	passThrough.setMenu("06:00 - 07:00");
	passThrough.setMenu("07:00 - 08:00");
	passThrough.setMenu("08:00 - 09:00");
	passThrough.setMenu("09:00 - 10:00");
	passThrough.setMenu("10:00 - 11:00");
	passThrough.setMenu("11:00 - 12:00");
	passThrough.setMenu("12:00 - 13:00");
	passThrough.setMenu("13:00 - 14:00");
	passThrough.setMenu("14:00 - 15:00");
	passThrough.setMenu("15:00 - 16:00");
	passThrough.setMenu("16:00 - 17:00");
	passThrough.setMenu("17:00 - 18:00");
	passThrough.setMenu("18:00 - 19:00");
	passThrough.setMenu("19:00 - 20:00");
	passThrough.setMenu("20:00 - 21:00");
	passThrough.setMenu("21:00 - 22:00");
	passThrough.setMenu("22:00 - 23:00");
	passThrough.setMenu("23:00 - 00:00");
}

/// Info [In progress]:
void infoDisplay(Menu &passThrough)
{
	enterMenu();
	passThrough.clearMenu();
	passThrough.setMenu("Window Tracking");						// learn about checkFocus()
	passThrough.setMenu("AFK Detection");						// learn about checkUserActivity() and afkFocus.data
	passThrough.setMenu("Overrides");							// learn about the override system
	passThrough.setMenu("Chronometrics");						// learn about the data storage scheme (calendar year only)
	passThrough.setMenu("Focus Limits");						// learn about focus limits
	passThrough.setMenu("Groups");								// learn about groups
	passThrough.setMenu("Display Filters");						// learn about top 5, last 5, and spotlights
	passThrough.setMenu("Conduits");							// learn about conduits
	passThrough.setMenu("Focus Limit Warning Timings");			// learn about focus limit warning timings
	passThrough.setMenu("Program Timings");						// learn about program timings (afk detection, write interval, scan interval, etc)
	passThrough.setMenu("Contact Info");						// Provide contact info for Sebastian Cobb
	UI::clearIt();
	UI::setTextColors(UI::black, UI::white);
	std::cout << "Information Menu:" << "\n\n";
	UI::resetTextColors();
	unsigned int whichInfo = passThrough.selectItem();
	if (whichInfo == 0)
	{
		UI::setTextColors(UI::black, UI::dark_green);
		std::cout << "\n";
		std::cout << "Returning to tracker.  (Please wait...)";
		std::this_thread::sleep_for(std::chrono::seconds(2));
		return;
	}
	UI::clearIt();
	UI::setTextColors(UI::black, UI::white);
	std::cout << "Information:  " << passThrough.choices[whichInfo - 1] << "\n\n";
	UI::resetTextColors();
	UI::clearIt();
	switch (whichInfo)
	{
	case 1:
	{
		// Window Tracking
		std::cout << "Windows Focus Tracker scans the foreground window regularly (see ";
		UI::setTextColors(UI::black, UI::white);
		std::cout << "Program Timings";
		UI::resetTextColors();
		std::cout << ") for the active foreground window.  ";
		std::cout << "Time spent on the foreground window while not AFK is automatically added to the internal database.  ";
		std::cout << "Regular archiving to disk takes place regularly (can be configured), as well.";
		std::cout << "\n\n";
		std::cout << "Window tracking is also run through 6 types of keyword filters (can be configured) to help organize data collection.  ";
		std::cout << "Listed below are the filter types, in their chronological implimentation.  ";
		std::cout << "For each list, there is a linear keyword priority.  The functionality of these filters also scales into the next format.  All lists have linear priority, meaning that when they reach a keyword that is found, the effects of that filter immediately apply and the next filter will now scan based on any modified attributes.";
		std::cout << "\n\n";
		UI::setTextColors(UI::black, UI::light_green);
		std::cout << "1.  Blacklisted titles" << "\n";
		UI::resetTextColors();
		std::cout << "If WFT scans the foreground window and the title contains a keyword on this list, all tracking is immediately ignored, subsequent filters are also ignored.  For example, you might want to not track \"Foobar\", because you only ever touch it (or do anything related to it) once in a blue moon.";
		std::cout << "\n\n";
		UI::setTextColors(UI::black, UI::light_green);
		std::cout << "2.  Blacklisted classes" << "\n";
		UI::resetTextColors();
		std::cout << "If WFT scans the foreground window and the class contains a keyword on this list, again, all subsequent tracking and filters are ignored.  For example, class \"32770\" is for dialogue boxes.  You may not want to track that seemingly negligible time one might spend looking at a dialogue box.";
		std::cout << "\n\n";
		UI::setTextColors(UI::black, UI::light_green);
		std::cout << "3.  Whitelisted titles" << "\n";
		UI::resetTextColors();
		std::cout << "If the foreground window title contains a keyword on this list, the focus tracker will treat the data as if it had the keyword alone as the title.  For example, a keyword of \"Winamp\" would condense all windows that contained \"Winamp\" in the title to just \"Winamp\", but leave the class alone.  This filter also changes the data which is passed to the title group filter.";
		std::cout << "\n\n";
		UI::setTextColors(UI::black, UI::light_green);
		std::cout << "4.  Whitelisted classes" << "\n";
		UI::resetTextColors();
		std::cout << "This is the same as whitelisted titles, but the keyword applies to class keywords discovered.";
		std::cout << "\n\n";
		UI::setTextColors(UI::black, UI::light_green);
		std::cout << "5.  Title Groups" << "\n";
		UI::resetTextColors();
		std::cout << "When a user wants to create a topic filter for the window tracker, this is where they would go to do so.  Where the previous filters worked by screening or condensing the title or class fields, the focus group filters actually group all instances of the keywords into one dataset.  For example, if the user chose \"Winamp\" as their title group, then all windows which contain the title keyword, including Chrome tabs, notepad, and Winamp itself, would be grouped into one dataset.  When this is found, class groups are skipped.";
		std::cout << "\n\n"; 
		UI::setTextColors(UI::black, UI::light_green);
		std::cout << "6.  Class Groups" << "\n";
		UI::resetTextColors();
		std::cout << "The same rules apply here for class groups.";
		std::cout << "\n\n";
		UI::pressEnterToContinue();
		UI::clearIt();
		std::cout << "WFT will pass the window to each filter, in order, changing the properties of the data it stores.";
		std::cout << "\n\n";
		std::cout << "For example, if we had the raw window data:  ";
		std::cout << "Class:  ";
		UI::setTextColors(UI::black, UI::white);
		std::cout << "SuperClass" << "\n";
		std::cout << "Title:  ";
		std::cout << "Winamp & Chrome" << "\n";
		UI::resetTextColors();
		std::cout << "...and our filters are as follows: ";
		std::cout << "\n\n";
		std::cout << "blackTitle:  VLC, Angry Birds, Toilet Brushes" << "\n";
		std::cout << "blackClass:  asdf, sdfa, megadoop";
		std::cout << "\n\n";
		std::cout << "When it finds a blacklisted title or class, it discards the focus data for that detection.  In this case, it has found no blacklisted title keyword, then no blacklisted title keyword.";
		std::cout << "\n\n";
		UI::pressEnterToContinue();
		std::cout << "\n\n";
		std::cout << "whiteTitle:  ";
		std::cout << "Winamp";
		UI::resetTextColors();
		std::cout << ", ";
		UI::setTextColors(UI::black, UI::white);
		std::cout << "Chrome";
		UI::resetTextColors();
		std::cout << "\n\n";
		std::cout << "When it finds a title that is on the whiteTitle list, it condenses the title to read that of the keyword, so our new information is ";
		UI::setTextColors(UI::black, UI::white);
		std::cout << "SuperClass";
		UI::resetTextColors();
		std::cout << " - ";
		UI::setTextColors(UI::black, UI::white);
		std::cout << "Winamp";
		std::cout << "\n\n";
		UI::resetTextColors();
		std::cout << "Once a filter is found, it stops checking for more keywords in that filter (linear priority).";
		std::cout << "\n\n";
		UI::pressEnterToContinue();
		std::cout << "\n\n";
		std::cout << "whiteClass:  ";
		UI::setTextColors(UI::black, UI::white);
		std::cout << "Class";
		UI::resetTextColors();
		std::cout << ", Super";
		std::cout << "\n\n";
		UI::resetTextColors();
		std::cout << "The same applies to classes, stopping and applying the first filter it comes across, only.  Our new window data is:  ";
		std::cout << "\n\n";
		UI::setTextColors(UI::black, UI::white);
		std::cout << "Class";
		UI::resetTextColors();
		std::cout << " - ";
		UI::setTextColors(UI::black, UI::white);
		std::cout << "Winamp";
		std::cout << "\n\n";
		UI::pressEnterToContinue();
		std::cout << "\n\n";
		std::cout << "titleGroup:  ";
		UI::setTextColors(UI::black, UI::white);
		std::cout << "Winamp";
		UI::resetTextColors();
		std::cout << ", Chrome";
		std::cout << "classGroup:  abc123";
		std::cout << "\n\n";
		std::cout << "When the data is passed to the titleGroup, it scans for the keyword in the title.  If it finds it, then all classes with the title keyword from here are grouped together in one data file.  In this stage, it would skip classGroup if it found a keyword.  classGroup wouldn't have found anything, anyways.";
		UI::pressEnterToContinue();
		std::cout << "\n\n";
		UI::clearIt();
		std::cout << "You can set and prioritize these filters by pressing \"";
		UI::setTextColors(UI::black, UI::light_cyan);
		std::cout << "f";
		UI::resetTextColors();
		std::cout << "\" while the tracker is active.";
		std::cout << "\n\n";
		std::cout << "To set or modify a description for any focus (including Overrides), press \"";
		UI::setTextColors(UI::black, UI::light_cyan);
		std::cout << "d";
		UI::resetTextColors();
		std::cout << "\" while the tracker is active.";
		std::cout << "\n\n";
		UI::pressEnterToContinue();
	}
		break;
	case 2:
	{
		// AFK Detection
		std::cout << "While not in a menu, WFT will track user activity on the mouse and most keys.  ";
		std::cout << "The default detection status is off and checked every second until user activity is found.  ";
		std::cout << "When it is found, it resets the ";
		UI::setTextColors(UI::black, UI::light_green);
		std::cout << ProgramSettings::userActivityPollingRate;
		UI::resetTextColors();
		std::cout << " second (can be configured) timer to count down before setting detection status to false again.  ";
		std::cout << "This works similarly to your screensaver timer, though not all keys included may be the same.  ";
		std::cout << "\n\n";
		std::cout << "Each focus entries (including Overrides) can be configured to disable AFK detection.  ";
		std::cout << "One example of where this might be appropriate is to disable AFK detection on VLC.  ";
		std::cout << "This would assume that when VLC is in focus, the user is presently engaged in the activities that VLC may provide which do not require user input.  ";
		std::cout << "\n\n";
		std::cout << "WFT also stores a separate dataset for afk, allowing the user to reflect on their relative non-being.";
		UI::pressEnterToContinue();
		std::cout << "\n\n";
		std::cout << "AFK Detection for individual focuses can be set by pressing the \"";
		UI::setTextColors(UI::black, UI::light_cyan);
		std::cout << "a";
		UI::resetTextColors();
		std::cout << "\" key while the tracker is active.";
		std::cout << "\n\n";
		UI::pressEnterToContinue();
	}
		break;
	case 3:
	{
		// Overrides
		std::cout << "WFT recognizes its limitation in that it can only see the foreground window.  ";
		std::cout << "However, not all activities take place here.  Override mode adds an option for the ";
		std::cout << "user to override the Window tracker and track a specific activity, not necessarily digital.  ";
		std::cout << "Some of these activities might include exercise, house-cleaning, or shaving a cat (if you need to track that sort of thing).";
		std::cout << "\n\n";
		std::cout << "Overrides are stored in the tracking session and saved at the regular save interval, with other focuses.";
		std::cout << "Overrides may also be included in groups.";
		UI::pressEnterToContinue();
		std::cout << "\n\n";
		std::cout << "Override mode can be toggled by pressing the \"";
		UI::setTextColors(UI::black, UI::light_cyan);
		std::cout << "o";
		UI::resetTextColors();
		std::cout << "\" key while the tracker is active.";
		std::cout << "\n\n";
		std::cout << "Overrides can be managed (including selecting and setting the active override) by pressing the \"";
		UI::setTextColors(UI::black, UI::light_cyan);
		std::cout << "p";
		UI::resetTextColors();
		std::cout << "\" key while the tracker is active.";
		std::cout << "\n\n";
		UI::pressEnterToContinue();
	}
		break;
	case 4:
	{
		// Chronometrics
		std::cout << "Focus information is stored in individual data files after they have passed any filters.  ";
		std::cout << "The data files include the total time, and hourly stats for every day of the year.  ";
		std::cout << "When viewing tracking data, each filter provides a different set of chronometrics.";
		std::cout << "\n\n";
		std::cout << "The hourly filter shows your ";
		UI::setTextColors(UI::black, UI::light_cyan);
		std::cout << "minutes";
		UI::resetTextColors();
		std::cout << " spent on the task in ";
		UI::setTextColors(UI::black, UI::dark_green);
		std::cout << "this hour block";
		UI::resetTextColors();
		std::cout << ".  For example, if you have spent 15 minutes on a task and it is 4:07, it will show you that you have spent 8 minutes (:00 counts as the first minute) in the relevant focus.";
		std::cout << "\n\n";
		std::cout << "The daily filter shows your ";
		UI::setTextColors(UI::black, UI::light_cyan);
		std::cout << "hours";
		UI::resetTextColors();
		std::cout << " per ";
		UI::setTextColors(UI::black, UI::dark_green);
		std::cout << "this calendar day.";
		UI::resetTextColors();
		std::cout << "\n\n";
		std::cout << "The weekly filter shows your ";
		UI::setTextColors(UI::black, UI::light_cyan);
		std::cout << "hours";
		UI::resetTextColors();
		std::cout << " per ";
		UI::setTextColors(UI::black, UI::dark_green);
		std::cout << "last 7 days, up to this hour block.  This includes days from the previous year, if any data is found.";
		UI::resetTextColors();
		std::cout << "\n\n";
		std::cout << "The yearly filter shows you the number of ";
		UI::setTextColors(UI::black, UI::dark_green);
		std::cout << "days";
		UI::resetTextColors();
		std::cout << " per ";
		UI::setTextColors(UI::black, UI::dark_green);
		std::cout << "this current year, up to the current day.  ";
		UI::resetTextColors();
		std::cout << "This is to stay focused on yearly goals, but give a fresh start each calendar year.  ";
		UI::setTextColors(UI::black, UI::light_green);
		std::cout << "The data is not erased at the new year, but is overwritten upon passing the time again in the next year.";
		std::cout << "\n\n";
		UI::pressEnterToContinue();
		std::cout << "\n\n";
		std::cout << "Chronometric focus can be cycled by pressing the \"";
		UI::setTextColors(UI::black, UI::light_cyan);
		std::cout << "x";
		UI::resetTextColors();
		std::cout << "\" key while the tracker is active.";
		std::cout << "\n\n";
		UI::pressEnterToContinue();
	}
		break;
	case 5:
	{
		// Focus Limits
		std::cout << "When filtering by time in the tracker, hourly stats are not passed through the focus limits.  ";
		std::cout << "That would be a little nitpicky.  ";
		std::cout << "However, daily stats, weekly stats, and yearly stats are all passed through a focus limit filter, to better optimize your time.";
		std::cout << "\n\n";
		UI::pressEnterToContinue();
		std::cout << "\n\n";
		std::cout << "Setting a lower limit and and upper limit (each with individual hours/period limitations) can help throttle user time, preventing burnout on a single focus.";
		std::cout << "\n\n";
		UI::pressEnterToContinue();
		std::cout << "\n\n";
		std::cout << "The lower and upper limits are not strictly enforced.  ";
		std::cout << "Data is still collected for the given focus regardless of focus limit status.";
		std::cout << "\n\n";
		UI::pressEnterToContinue();
		std::cout << "\n\n";
		UI::clearIt();
		std::cout << "Focus limits may be set by pressing \"";
		UI::setTextColors(UI::black, UI::light_cyan);
		std::cout << "l (L)";
		UI::resetTextColors();
		std::cout << "\" in the tracker.";
		std::cout << "\n\n";
		UI::pressEnterToContinue();
	}
		break;
	case 6:
	{
		// Groups
		std::cout << "";
		UI::pressEnterToContinue();
		std::cout << "\n\n";
		UI::clearIt();
		std::cout << "You can manage these groups by pressing \"";
		UI::setTextColors(UI::black, UI::light_cyan);
		std::cout << "g";
		UI::resetTextColors();
		std::cout << "\" while the tracker is active.";
		std::cout << "\n\n";
		UI::pressEnterToContinue();
	}
		break;
	case 7:
	{
		// Display Filters
		std::cout << "";
		UI::pressEnterToContinue();
		std::cout << "\n\n";
		UI::clearIt();
		std::cout << "You can cycle the display filter by pressing \"";
		UI::setTextColors(UI::black, UI::light_cyan);
		std::cout << "z";
		UI::resetTextColors();
		std::cout << "\" while the tracker is active.";
		std::cout << "\n\n";
		UI::pressEnterToContinue();
	}
		break;
	case 8:
	{
		// Conduits
		std::cout << "For every calendar day, there are 24 hour blocks.  ";
		std::cout << "Setting conduits indicates to the user the preferred time to be performing the focus.  ";
		std::cout << "For example, a user might wish to prefer playing Overwatch competitive at a certain hour.  ";
		std::cout << "Toggling the hour blocks on for the focus will highlight the focus on the tracker when it is found being done within the specified ";
		UI::setTextColors(UI::black, UI::light_cyan);
		std::cout << "\"on\"";
		UI::resetTextColors();
		std::cout << " hour blocks.";
		UI::pressEnterToContinue();
		std::cout << "\n\n";
		UI::clearIt();
		std::cout << "You can set conduits for any given focus by pressing \"";
		UI::setTextColors(UI::black, UI::light_cyan);
		std::cout << "c";
		UI::resetTextColors();
		std::cout << "\" while the tracker is active.";
		std::cout << "\n\n";
		std::cout << "Toggle conduit detection by pressing \"";
		UI::setTextColors(UI::black, UI::light_cyan);
		std::cout << "q";
		UI::resetTextColors();
		std::cout << "\" while the tracker is active.";
		std::cout << "\n\n";
		UI::pressEnterToContinue();
	}
		break;
	case 9:
	{
		// Spotlight 5
		std::cout << "";
		UI::pressEnterToContinue();
		std::cout << "\n\n";
		UI::clearIt();
		std::cout << "You can manage focus spotlight assignments by pressing \"";
		UI::setTextColors(UI::black, UI::light_cyan);
		std::cout << "s";
		UI::resetTextColors();
		std::cout << "\" while the tracker is active.";
		std::cout << "\n\n";
		std::cout << "You can toggle spotlight detection by pressing \"";
		UI::setTextColors(UI::black, UI::light_cyan);
		std::cout << "w";
		UI::resetTextColors();
		std::cout << "\" while the tracker is active.";
		std::cout << "\n\n";
		UI::pressEnterToContinue();
	}
		break;
	case 10:
	{
		// Warning Timings
		std::cout << "";
		UI::pressEnterToContinue();
		std::cout << "\n\n";
		UI::clearIt();
		std::cout << "You can manage focus limit warning sounds & timings by pressing \"";
		UI::setTextColors(UI::black, UI::light_cyan);
		std::cout << "r";
		UI::resetTextColors();
		std::cout << "\" while the tracker is active.";
		std::cout << "\n\n";
		UI::pressEnterToContinue();
	}
		break;
	case 11:
	{
		// Program Timings
		std::cout << "";
		UI::pressEnterToContinue();
		std::cout << "\n\n";
		UI::clearIt();
		std::cout << "You can manage the program timings by pressing \"";
		UI::setTextColors(UI::black, UI::light_cyan);
		std::cout << "t";
		UI::resetTextColors();
		std::cout << "\" while the tracker is active.";
		std::cout << "\n\n";
		UI::pressEnterToContinue();
	}
		break;
	case 12:
	{
		// Contact Sebastian Cobb
		std::cout << "";
		std::cout << "";
		std::cout << "";
		std::cout << "";
		UI::pressEnterToContinue();
	}
		break;
	default:
		break;
	}
	exitMenu();
}