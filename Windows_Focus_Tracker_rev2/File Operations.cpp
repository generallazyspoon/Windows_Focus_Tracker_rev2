#include "stdafx.h"
#include "Windows_Focus_Tracker_rev2.h"
#include "../../GA_Library/GA_Library/GA_Library.h"

/// File Operations:

void readSettings()
{
	// read & smartBuffer the file
	FileOps::Data readSettingsFile;
	FileOps::TargetManage::setFileName(readSettingsFile, "settings");
	FileOps::TargetManage::setExtension(readSettingsFile, "cfg");
	FileOps::DBManage::smartBuffer(readSettingsFile);
	// program control information filters
	ProgramSettings::userTimeZone = readSettingsFile.infoDatabase[0].data[0];
	ProgramSettings::userActivityPollingRate = std::stoi(readSettingsFile.infoDatabase[1].data[0]);
	ProgramSettings::scanInterval = std::stoi(readSettingsFile.infoDatabase[2].data[0]);
	ProgramSettings::displayInterval = ProgramSettings::scanInterval;	// "synchronized"
	ProgramSettings::Display::displayMode = std::stoi(readSettingsFile.infoDatabase[3].data[0]);
	ProgramSettings::Display::displayFilterType = std::stoi(readSettingsFile.infoDatabase[4].data[0]);
	ProgramSettings::Display::displayChronologicalType = std::stoi(readSettingsFile.infoDatabase[5].data[0]);
	ProgramSettings::detectionMode = std::stoi(readSettingsFile.infoDatabase[6].data[0]);
	ProgramSettings::currentOverride = readSettingsFile.infoDatabase[7].data[0];
	// blacklist filters (exclude)
	for (unsigned int x = 0; x < readSettingsFile.infoDatabase[8].data.size(); x++)
		ProgramSettings::titleBlack.push_back(readSettingsFile.infoDatabase[8].data[x]);
	for (unsigned int x = 0; x < readSettingsFile.infoDatabase[9].data.size(); x++)
		ProgramSettings::classBlack.push_back(readSettingsFile.infoDatabase[9].data[x]);
	// whitelist filters (keyword)
	for (unsigned int x = 0; x < readSettingsFile.infoDatabase[10].data.size(); x++)
		ProgramSettings::titleWhite.push_back(readSettingsFile.infoDatabase[10].data[x]);
	for (unsigned int x = 0; x < readSettingsFile.infoDatabase[11].data.size(); x++)
		ProgramSettings::classWhite.push_back(readSettingsFile.infoDatabase[11].data[x]);
	// grouplist filters (keyword)
	for (unsigned int x = 0; x < readSettingsFile.infoDatabase[12].data.size(); x++)
		ProgramSettings::titleGroup.push_back(readSettingsFile.infoDatabase[12].data[x]);
	for (unsigned int x = 0; x < readSettingsFile.infoDatabase[13].data.size(); x++)
		ProgramSettings::classGroup.push_back(readSettingsFile.infoDatabase[13].data[x]);
	// cross-session information filters
	std::vector<std::string> tempStringVector = StringTools::parseStrings(readSettingsFile.infoDatabase[14].data[0], ',');
	for (unsigned int x = 0; x < tempStringVector.size() - 1; x++)
		ProgramSettings::lastFiveFocusTitle[x] = tempStringVector[x];
	tempStringVector = StringTools::parseStrings(readSettingsFile.infoDatabase[15].data[0], ',');
	for (unsigned int x = 0; x < tempStringVector.size() - 1; x++)
		ProgramSettings::lastFiveFocusClass[x] = tempStringVector[x];
	ProgramSettings::playSounds = StringTools::onOffBool(readSettingsFile.infoDatabase[16].data[0]);
	ProgramSettings::playFocusZoneSounds = StringTools::onOffBool(readSettingsFile.infoDatabase[17].data[0]);
	// report
	UI::clearIt();
	UI::setTextColors(UI::black, UI::dark_green);
	std::cout << "Read setting file OK.";
	UI::resetTextColors();
}

void writeSettings()
{
	// open the file stream
	std::ofstream writeSettingsFile("settings.cfg");
	// write the settings:
	writeSettingsFile << "userTimeZone:  " << ProgramSettings::userTimeZone << "\n";
	writeSettingsFile << "userActivityPollingRate:  " << ProgramSettings::userActivityPollingRate << "\n";
	writeSettingsFile << "focusPollingRate:  " << ProgramSettings::scanInterval << "\n";
	writeSettingsFile << "displayMode:  " << ProgramSettings::Display::displayMode << "\n";
	writeSettingsFile << "displayFilterType:  " << ProgramSettings::Display::displayFilterType << "\n";
	writeSettingsFile << "displayChronologicalType:  " << ProgramSettings::Display::displayChronologicalType << "\n";
	writeSettingsFile << "detectionMode:  " << ProgramSettings::detectionMode << "\n";
	writeSettingsFile << "currentOverride:  " << ProgramSettings::currentOverride << "\n";
	for (unsigned int x = 0; x < ProgramSettings::titleBlack.size(); x++)
		writeSettingsFile << "titleBlack:  " << ProgramSettings::titleBlack[x] << "\n";
	for (unsigned int x = 0; x < ProgramSettings::classBlack.size(); x++)
		writeSettingsFile << "classBlack:  " << ProgramSettings::classBlack[x] << "\n";
	for (unsigned int x = 0; x < ProgramSettings::titleWhite.size(); x++)
		writeSettingsFile << "titleWhite:  " << ProgramSettings::titleWhite[x] << "\n";
	for (unsigned int x = 0; x < ProgramSettings::classWhite.size(); x++)
		writeSettingsFile << "classWhite:  " << ProgramSettings::classWhite[x] << "\n";
	for (unsigned int x = 0; x < ProgramSettings::titleGroup.size(); x++)
		writeSettingsFile << "titleGroup:  " << ProgramSettings::titleGroup[x] << "\n";
	for (unsigned int x = 0; x < ProgramSettings::classGroup.size(); x++)
		writeSettingsFile << "classGroup:  " << ProgramSettings::classGroup[x] << "\n";
	writeSettingsFile << "lastFiveFocusTitle:  ";
	for (unsigned int x = 0; x < 4; x++)
		writeSettingsFile << ProgramCache::lastFiveFocus[x].windowTitle << ",";
	writeSettingsFile << ProgramCache::lastFiveFocus[4].windowTitle << "\n";
	writeSettingsFile << "lastFiveFocusClass:  ";
	for (unsigned int x = 0; x < 4; x++)
		writeSettingsFile << ProgramCache::lastFiveFocus[x].windowClass << ",";
	writeSettingsFile << ProgramCache::lastFiveFocus[4].windowClass << "\n";
	writeSettingsFile << "playSounds:  ";
	writeSettingsFile << StringTools::onOffString(ProgramSettings::playSounds) << "\n";
	writeSettingsFile << "playFocusLimitSounds:  ";
	writeSettingsFile << StringTools::onOffString(ProgramSettings::playFocusZoneSounds) << "\n";
	// close the file stream
	writeSettingsFile.close();
}

void readData()
{
	// read the file
	FileOps::Data readDataFiles;
	readDataFiles.fileLines.reserve(399);
	FileOps::TargetManage::setDir(readDataFiles, "data");
	FileOps::TargetManage::setExtension(readDataFiles, "data");
	FileOps::FileManage::setFileList(readDataFiles);
	if (readDataFiles.fileList.size() == 0)
	{
		UI::clearIt();
		std::cout << "No data found (Is this the first-time run?)";
		std::this_thread::sleep_for(std::chrono::seconds(2));
		return;
	}
	focusWindow xContainer;
	std::vector<unsigned int> temporaryVector = {};
	temporaryVector.reserve(24);
	std::vector<std::string> tempConduitVector = {};
	tempConduitVector.reserve(24);
	for (unsigned int z = 0; z < readDataFiles.fileList.size(); z++) {
		// set the file name & load the relevant information
		FileOps::TargetManage::setFileName(readDataFiles, readDataFiles.fileList[z]);
		FileOps::FileContent::gatherLines(readDataFiles, ":  ");
		// clear the container
		xContainer = {};
		// filter the smartBuffer to load data
		xContainer.windowClass = readDataFiles.fileLines[0];
		xContainer.windowTitle = readDataFiles.fileLines[1];
		xContainer.description = readDataFiles.fileLines[2];
		xContainer.overridesAFK = StringTools::onOffBool(readDataFiles.fileLines[3]);
		xContainer.isInSpotlight = StringTools::onOffBool(readDataFiles.fileLines[4]);
		tempConduitVector = StringTools::parseStrings(readDataFiles.fileLines[5], ',');
		for (unsigned int y = 0; y < 24; y++)
			xContainer.conduits[y] = StringTools::onOffBool(tempConduitVector[y]);
		temporaryVector = StringTools::parseUnsignedInts(readDataFiles.fileLines[6]);
		for (unsigned int y = 0; y < 8; y++)
			xContainer.focusZones[y] = temporaryVector[y];
		temporaryVector = StringTools::parseUnsignedInts(readDataFiles.fileLines[7]);
		for (unsigned int y = 0; y < 2; y++)
			xContainer.warningTimings[y] = temporaryVector[y];
		temporaryVector = StringTools::parseUnsignedInts(readDataFiles.fileLines[8]);
		for (unsigned int y = 0; y < 4; y++)
			xContainer.totalDayHourMinSec[y] = temporaryVector[y];
		// yearly Stats
		for (unsigned int zz = 9; zz < readDataFiles.fileLines.size(); zz++) {
			temporaryVector = StringTools::parseUnsignedInts(readDataFiles.fileLines[zz]);
			for (unsigned int a = 0; a < 24; a++)
				xContainer.yearlyStats[zz - 9][a] = temporaryVector[a];
		}
		ProgramCache::trackingSession.push_back(xContainer);
		UI::clearIt();
		UI::setTextColors(UI::black, UI::dark_green);
		std::cout << "Loaded " << z + 1 << "/" << readDataFiles.fileList.size() << " data files OK.";
		UI::resetTextColors();
		readDataFiles.fileLines.resize(0);
	}
	// clear the container, other objects
	FileOps::Data readAFKFile;
	focusWindow yContainer = {};
	// set the AFK file
	FileOps::TargetManage::setFileName(readAFKFile, "afk");
	FileOps::TargetManage::setExtension(readAFKFile, "data");
	// read the AFK data
	FileOps::FileContent::gatherLines(readAFKFile, ":  ");
	// filter the data
	yContainer.windowClass = readAFKFile.fileLines[0];
	yContainer.windowTitle = readAFKFile.fileLines[1];
	yContainer.description = readAFKFile.fileLines[2];
	temporaryVector = StringTools::parseUnsignedInts(readAFKFile.fileLines[3]);
	for (unsigned int y = 0; y < 4; y++)
		yContainer.totalDayHourMinSec[y] = temporaryVector[y];
	// yearly Stats
	for (unsigned int zz = 4; zz < readAFKFile.fileLines.size(); zz++) {
		temporaryVector = StringTools::parseUnsignedInts(readAFKFile.fileLines[zz]);
		for (unsigned int a = 0; a < 24; a++)
			xContainer.yearlyStats[zz - 4][a] = temporaryVector[a];
		ProgramCache::afkFocus = xContainer;
	}
	UI::clearIt();
	UI::setTextColors(UI::black, UI::dark_green);
	std::cout << "Read AFK data OK.";
	UI::resetTextColors();
}

void writeData()
{
	// write focus data
	for (unsigned int x = 0; x < ProgramCache::trackingSession.size(); x++)
		if (ProgramCache::trackingSession[x].writeMe) {
			std::ofstream writeIt;
			// file object
			writeIt.open("data\\" + ProgramCache::trackingSession[x].windowClass + "_-_" + ProgramCache::trackingSession[x].windowTitle + ".data");
			// write the fields
			writeIt << "Class:  " << ProgramCache::trackingSession[x].windowClass << "\n";
			writeIt << "Title:  " << ProgramCache::trackingSession[x].windowTitle << "\n";
			writeIt << "Description:  " << ProgramCache::trackingSession[x].description << "\n";
			writeIt << "overridesAFK:  " << StringTools::onOffString(ProgramCache::trackingSession[x].overridesAFK) << "\n";
			writeIt << "Spotlight Status:  " << StringTools::onOffString(ProgramCache::trackingSession[x].isInSpotlight) << "\n";
			writeIt << "Conduits:  ";
			for (unsigned int y = 0; y < 24; y++) {
				writeIt << StringTools::onOffString(ProgramCache::trackingSession[x].conduits[y]);
				if (y != 23)
					writeIt << ","; }
			writeIt << "\n";
			writeIt << "Focus Zones:  ";
			for (unsigned int y = 0; y < 8; y++) {
				writeIt << ProgramCache::trackingSession[x].focusZones[y];
				if (y != 8)
					writeIt << ", "; }
			writeIt << "\n";
			writeIt << "Warning Timings:  " << ProgramCache::trackingSession[x].warningTimings[0] <<
				", " << ProgramCache::trackingSession[x].warningTimings[1] << "\n";
			writeIt << "Total Time:  ";
			for (unsigned int y = 0; y < 4; y++) {
				writeIt << ProgramCache::trackingSession[x].totalDayHourMinSec[y];
				if (y != 4)
					writeIt << ", "; } 
			writeIt << "\n";
			for (unsigned int y = 0; y < 366; y++) {
				writeIt << y << ":  ";
				for (unsigned int z = 0; z < 24; z++) {
					writeIt << ProgramCache::trackingSession[x].yearlyStats[y][z];
					if (z != 23)
						writeIt << ", "; } 
				writeIt << "\n"; }
			writeIt.close(); 
			ProgramCache::trackingSession[x].writeMe = false; }
	// write AFK data
	if (ProgramCache::afkFocus.writeMe) {
		// file object
		std::ofstream writeIt("afk.data");
		// write the fields
		writeIt << "Class:  " << ProgramCache::afkFocus.windowClass << "\n";
		writeIt << "Title:  " << ProgramCache::afkFocus.windowTitle << "\n";
		writeIt << "Description:  " << ProgramCache::afkFocus.description << "\n";
		writeIt << "Total Time:  ";
		for (unsigned int y = 0; y < 4; y++) {
			writeIt << ProgramCache::afkFocus.totalDayHourMinSec[y];
			if (y != 3)
				writeIt << ", "; }
		writeIt << "\n";
		for (unsigned int y = 0; y < 366; y++) {
			writeIt << y << ":  ";
			for (unsigned int z = 0; z < 24; z++) {
				writeIt << ProgramCache::afkFocus.yearlyStats[y][z];
				if (z != 23)
					writeIt << ", "; }
			writeIt << "\n"; }
		writeIt.close(); 
		ProgramCache::afkFocus.writeMe = false;
	}
}

/// experimental (needs testing)
void readGroups()
{
	FileOps::Data readGroupFiles;
	FileOps::TargetManage::setDir(readGroupFiles, "groups");
	FileOps::TargetManage::setExtension(readGroupFiles, "cfg");
	FileOps::FileManage::setFileList(readGroupFiles);
	focusWindowGroup xContainer;
	for (unsigned int x = 0; x < readGroupFiles.fileList.size(); x++) {
		// target & parse the data
		FileOps::TargetManage::setFileName(readGroupFiles, readGroupFiles.fileList[x]);
		FileOps::DBManage::clearInfoDatabase(readGroupFiles);
		FileOps::DBManage::smartBuffer(readGroupFiles);
		// contain the data
		xContainer.groupName = readGroupFiles.infoDatabase[0].data[0];
		xContainer.groupDescription = readGroupFiles.infoDatabase[1].data[0];
		std::vector<std::string> tempGroupIDInfo = {};
		for (unsigned int z = 0; z < readGroupFiles.infoDatabase[2].data.size(); z++) {
			tempGroupIDInfo = StringTools::parseStrings(readGroupFiles.infoDatabase[2].data[z], ',');
			xContainer.members.push_back({
				tempGroupIDInfo[0],
				tempGroupIDInfo[1]
			}); }
		// push the data
		ProgramSettings::groupList.push_back(xContainer);
		xContainer = {};
		UI::clearIt();
		UI::setTextColors(UI::black, UI::dark_green);
		std::cout << "Read " << x + 1 << " group files OK.";
		UI::resetTextColors();
	}
}

void writeGroups()
{
	if (ProgramSettings::groupList.size() > 0)
		for (unsigned int x = 0; x < ProgramSettings::groupList.size(); x++) {
			std::ofstream groupsFile("groups/" + ProgramSettings::groupList[x].groupName + ".cfg");
			groupsFile << "Name:  " << ProgramSettings::groupList[x].groupName << "\n";
			groupsFile << "Description:  " << ProgramSettings::groupList[x].groupDescription << "\n";
			for (unsigned int y = 0; y < ProgramSettings::groupList[x].members.size(); y++)
			{
				groupsFile << "Member:  " << ProgramSettings::groupList[x].members[y].className
					<< "," << ProgramSettings::groupList[x].members[y].titleName
					<< "\n";
			}
			groupsFile.close();
		}
}

/// ///
void floodDataFile(FileOps::Data &passThrough)
{
	FileOps::Data::databaseStruct theMoonIsInTheCode;
	theMoonIsInTheCode.data.reserve(24);
	passThrough.infoDatabase.reserve(400);
	for (unsigned int x = 0; x < 400; x++) {
		theMoonIsInTheCode.dataType = "";
		for (unsigned int y = 0; y < 24; y++)
			theMoonIsInTheCode.data.push_back("");
		passThrough.infoDatabase.push_back(theMoonIsInTheCode);
		theMoonIsInTheCode = {};
	}
}