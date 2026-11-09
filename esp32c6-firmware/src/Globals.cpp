// Definitions of the shared globals declared in Globals.h.
#include "Globals.h"

bool lightState = false;
ZigbeeSwitch *mySwitch = NULL;
Preferences preferences;

int clapCount = 0;
unsigned long firstClapTime = 0;
unsigned long soundStartTime = 0;
unsigned long lastLoudMoment = 0;
bool isSoundActive = false;
int32_t maxAmpDuringEvent = 0;
int32_t micSensitivity = 8000; // Fixed loudness threshold; raise it if background noise triggers claps
