#ifndef WFTr2
#define WFTr2

// Includes

#include "../../GA_Library/GA_Library/GA_Library.h"

#include <Windows.h>										// HWND and others
#include "boost/date_time/gregorian/gregorian.hpp"			// Gregorian calendar
#include "boost/date_time/time_zone_base.hpp"				// Time Zone
#include "boost/date_time/local_time/local_time.hpp"		// Local Time
#include <atlbase.h>										// A2W and W2A conversion
#include <algorithm>										/// needed?
#include <Mmsystem.h>										// Sound playback
#pragma comment(lib,"winmm.lib")	
#include <thread>											// threads
#include <chrono>											// chrono::seconds, etc
#include <atomic>											// cross-thread variables
#include <mutex>
#include <time.h>

namespace FileOps
{
	struct Data;
}

/// experimental (needs testing)
struct focusWindowID
{
	std::string className;
	std::string titleName;
};
// An object which describes a focusWindow group
struct focusWindowGroup
{
	std::string groupName;
	std::string groupDescription;
	std::vector<focusWindowID> members;
	bool writeMe;
};

// An object which contains focus information
struct focusWindow
{
	std::string windowClass;								// Converted HWND class
	std::string windowTitle;								// Converted HWND title
	std::string description;								// Plaintext description
	bool overridesAFK;										// switch to override the AFK detector while this focusWindow is active
	bool isInSpotlight;										// Spotlight this focus?
	bool conduits[24];										// Hourly conduit bools
	unsigned int focusLimits[8];							// Hours/day (x2), hours/week (x2), hours/month (x2), hours/year (x2)
	unsigned int warningTimings[2];							// Pre-lower limit warning, pre-upper limit warning
	unsigned int totalDayHourMinSec[4];								// Total time spent in this focus
	unsigned int yearlyStats[366][24];								// Year-long hourly stats for this focus object
	bool writeMe;											// Indicates to the program that changes have been made
	bool deleteMe;											// Indicates to the program that it should be deleted from disk.
};

// Nexus namespaces
namespace ProgramControl
{
	namespace Threads
	{
		extern bool displayThread;
		extern bool checkFocusThread;
		extern bool checkUserActivityThread;
		extern bool getTheTimeThread;
		extern bool regularlyWriteDataThread;
		extern bool checkForUserCommandsThread;
	}
	extern bool userHoldingKey;
	extern bool userHoldingEscape;
	extern bool userInputOK;
	extern bool displayOK;
	extern bool overrideMenu;
	extern bool bypassOverrideQuery;
}
namespace ProgramSettings
{
	extern unsigned int scanInterval;
	extern unsigned int displayInterval;
	extern std::string userTimeZone;
	extern unsigned int userActivityPollingRate;
	extern unsigned int detectionMode;
	extern unsigned int lastDetectionMode;
	extern std::vector<std::string> titleBlack;
	extern std::vector<std::string> titleWhite;
	extern std::vector<std::string> classBlack;
	extern std::vector<std::string> classWhite;
	extern std::vector<std::string> titleGroup;
	extern std::vector<std::string> classGroup;

	/// experimental (needs testing)
	extern std::string lastFiveFocusTitle[5];
	extern std::string lastFiveFocusClass[5];
	extern std::string currentOverride;
	namespace Display 
	{
		extern unsigned int displayMode;
		extern unsigned int displayFilterType;
		extern unsigned int displayChronologicalType;
		extern bool groupMode;
		extern bool conduitMode;
		extern bool spotlightMode;
		extern bool overrideMode;
	}
	extern std::vector<focusWindowGroup> groupList;
}
namespace ProgramCache
{
	extern Menu programMenu;

	extern bool timeToSave;
	// extern tm theTime;
	extern struct tm * theTime;
	extern bool thisYearLeap;
	extern bool lastYearLeap;
	extern int thisYearSize;
	extern int lastYearSize;
	extern int leapModifier;
	extern bool userActivity;
	extern POINT mouseSnapA;
	extern POINT mouseSnapB;
	extern std::vector<focusWindow> trackingSession;
	extern focusWindow afkFocus;
	
	/// experimental (needs testing)
	extern focusWindow lastFiveFocus[5];
	extern focusWindow topFiveFocus[5];
	extern focusWindow spotlightFiveFocus[5];

	extern std::string systemMessage;
}

// Standard functions
void getTheTime();
void checkUserActivity();
void checkFocus();
void displayData();
void regularlyWriteData();

// Support functions
void updateTotalDayHourMinSec(focusWindow &passThrough);
void makeFocusWindow(focusWindow &passThrough);
void displayFilterBase(focusWindow &sortMe);
void sortLastFive(focusWindow &sortMe);
void sortTopFive(focusWindow &sortMe);
double getTotalTime(focusWindow &readMe);

// File operations
void readSettings();
void writeSettings();
void readData();
void writeData();
/// experimental (needs testing)
void readGroups();
void writeGroups();

/// experimental (needs testing)
void checkForUserCommands();
void populateFilters();
void displayChronoStats(focusWindow &passThrough);
void setLeapYearStatus();
void floodDataFile(FileOps::Data &passThrough);
int determineFocusLimitColor(focusWindow &passThrough);

void cycleDisplayMode();
void cycleChronoMode();

void toggleConduitMode();
void toggleSpotlightMode();
void toggleOverrideMode();

void displayDisplayStatus();
void determineFocusColor(focusWindow &passThrough);
void createOverride();
void chooseOverride(Menu &passThrough);

namespace WFT_Menus
{
	void populateOverrideMenu(Menu &passThrough);
	void overrideMenu(Menu &passThrough);
	void populateGroupMenu(Menu &passThrough, std::string custom);
	void groupMenu(Menu &passThrough);
}
 
void enterMenu();
void exitMenu();

void viewGroup(Menu &passThrough);
void populateGroup(Menu&passThrough);
void setOverrideAFKFlag(Menu &passThrough);
void descriptionManagement(Menu &passThrough);
void conduitManagement(Menu &passThrough);
void spotlightManagement(Menu &passThrough);
void setTrackingSessionMenu(Menu &passThrough);
void setConduitMenu(Menu &passThrough);

void infoDisplay(Menu &passThrough);

unsigned int groupSelectDisplay(Menu &passThrough);
void createGroup(Menu &passThrough);
void deleteGroup(Menu &passThrough);
#endif

#ifndef miniSANDRADefine
#define miniSANDRADefine

class miniSANDRA
{
private:
	std::mutex coreSANDRA;
	std::timed_mutex readWrite;
	bool runThisProgram = true;
public:
	bool requestNexus();
	void releaseNexus();
	bool runProgram();
	void earlyShutdown();
};

#endif