#include "stdafx.h"
#include "Windows_Focus_Tracker_rev2.h"
#include "../../GA_Library/GA_Library/GA_Library.h"

// for turning on/off features
namespace ProgramControl
{
	extern bool runProgram = true;
	namespace Threads
	{
		extern bool displayThread = true;
		extern bool checkFocusThread = true;
		extern bool checkUserActivityThread = true;
		extern bool getTheTimeThread = true;
		extern bool regularlyWriteDataThread = true;
		extern bool checkForUserCommandsThread = true;
	}
	extern bool writeRelease = false;
	extern bool readRelease = true;
	extern bool userHoldingKey = false;
	extern bool userHoldingEscape = false;
	extern bool userInputOK = true;
	extern bool displayOK = true;
	extern bool overrideMenu = false;
	extern bool bypassOverrideQuery = false;
}

// for defining specifics of features
namespace ProgramSettings
{
	extern unsigned int scanInterval = 5;
	extern unsigned int displayInterval = 5;
	extern std::string userTimeZone = "EST-05:00:00EDT+01:00:00,M4.1.0/02:00:00,M10.5.0/02:00:00";
	extern unsigned int userActivityPollingRate = 30;
	extern unsigned int detectionMode = 1;
	extern unsigned int lastDetectionMode = 1;
	extern std::vector<std::string> titleBlack = {};
	extern std::vector<std::string> titleWhite = {};
	extern std::vector<std::string> classBlack = {};
	extern std::vector<std::string> classWhite = {};
	extern std::vector<std::string> titleGroup = {};
	extern std::vector<std::string> classGroup = {};

	extern std::string lastFiveFocusTitle[5] = {};
	extern std::string lastFiveFocusClass[5] = {};
	extern std::string currentOverride = "";
	
	namespace Display
	{
		extern unsigned int displayMode = 1;
		extern unsigned int displayFilterType = 1;
		extern unsigned int displayChronologicalType = 1;
		extern bool groupMode = true;
		extern bool conduitMode = true;
		extern bool spotlightMode = true;
		extern bool overrideMode = true;
	}
	extern std::vector<focusWindowGroup> groupList = {};
}

// fluid data
namespace ProgramCache
{
	/// create a menu object
	extern Menu programMenu = {};
	extern bool timeToSave = false;
	extern struct tm * theTime = {};
	extern bool thisYearLeap = false;
	extern bool lastYearLeap = false;
	extern int lastYearSize = 365;
	extern int thisYearSize = 365;
	extern int leapModifier = 0;
	extern bool userActivity = true;
	extern POINT mouseSnapA = {};
	extern POINT mouseSnapB = {};
	extern std::vector<focusWindow> trackingSession = {};
	extern focusWindow afkFocus = {};

	extern focusWindow lastFiveFocus[5] = {};
	extern focusWindow topFiveFocus[5] = {};
	extern focusWindow spotlightFiveFocus[5] = {};

	extern std::string systemMessage = "";
}

bool miniSANDRA::requestNexus()
{
	do {
		if (!runThisProgram)
			return false;
		std::this_thread::sleep_for(std::chrono::microseconds(5000 + RandomOps::range(1, 5000)));
	} while (readWrite.try_lock());
	return true;
}

void miniSANDRA::releaseNexus()
{
	readWrite.unlock();
}

bool miniSANDRA::runProgram()
{
	coreSANDRA.lock();
	bool returnThisValue = runThisProgram;
	coreSANDRA.unlock();
	return returnThisValue;
}

void miniSANDRA::earlyShutdown()
{
	coreSANDRA.lock();
	runThisProgram = false;
	coreSANDRA.unlock();
}