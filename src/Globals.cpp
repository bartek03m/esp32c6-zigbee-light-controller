#include "Globals.h"

bool lightState = false;
ZigbeeSwitch *mySwitch = NULL;
Preferences preferences;
WebServer server(80);

int clapCount = 0;
unsigned long firstClapTime = 0;
unsigned long soundStartTime = 0;
unsigned long lastLoudMoment = 0;
bool isSoundActive = false;
int32_t maxAmpDuringEvent = 0;
int32_t micSensitivity = 8500;
