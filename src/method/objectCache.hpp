#ifndef objectcache
#define objectcache

#include <vector>
#include <utility>
#include <string>

struct objectInfo{
    int group;
    int center;
    double lastUsed;
    double dur;
};

struct objectsData{
    int index;
    double lastUsed;
    std::vector<objectInfo> groups;
};

extern objectsData smallUp;
extern objectsData smallDown;
extern objectsData med1Up;
extern objectsData med1Down;
extern objectsData med2Up;
extern objectsData med2Down;
extern objectsData large1Up;
extern objectsData large1Down;
extern objectsData large2Up;
extern objectsData large2Down;
extern objectsData raiderUp;
extern objectsData raiderDown;
extern objectsData hammerUp;
extern objectsData hammerDown;
extern objectsData hammerFlipUp;
extern objectsData hammerFlipDown;
extern objectsData geminiUp;
extern objectsData geminiDown;
extern objectsData holdUp;
extern objectsData holdDown;
extern objectsData holdUpEnd;
extern objectsData holdDownEnd;
extern objectsData masherUp;
extern objectsData masherDown;
extern objectsData gearUp;
extern objectsData gearDown;
extern objectsData bossProjectile1Up;
extern objectsData bossProjectile1Down;
extern objectsData bossProjectile2Up;
extern objectsData bossProjectile2Down;
extern objectsData bossProjectile3Up;
extern objectsData bossProjectile3Down;
extern objectsData bossMelee;
extern objectsData bossMasher;
extern objectsData bossMasherExit;
extern objectsData bossGearUp;
extern objectsData bossGearDown;
extern objectsData pItemUp;
extern objectsData pItemDown;
extern objectsData ghostUp;
extern objectsData ghostDown;
extern objectsData heartUp;
extern objectsData heartDown;
extern objectsData noteUp;
extern objectsData noteDown;

void resetObjectsData();

#endif
// //smallup
// extern int smallUpIndex;
// extern double lastUsedSmallUp;
// extern std::vector<std::pair<int,int>> smallUpObjects;

// //smalldown
// extern int smallDownIndex;
// extern double lastUsedSmallDown;
// extern std::vector<std::pair<int,int>> smallDownObjects;

// //medium1up
// extern int med1UpIndex;
// extern double lastUsedMed1Up;
// extern std::vector<std::pair<int,int>> med1UpObjects;

// //medium1down
// extern int med1DownIndex;
// extern double lastUsedMed1Down;
// extern std::vector<std::pair<int,int>> med1DownObjects;

// //medium2up
// extern int med2UpIndex;
// extern double lastUsedMed2Up;
// extern std::vector<std::pair<int,int>> med2UpObjects;

// //medium2down
// extern int med2DownIndex;
// extern double lastUsedMed2Down;
// extern std::vector<std::pair<int,int>> med2DownObjects;

// //large1up
// extern int large1UpIndex;
// extern double lastUsedLarge1Up;
// extern std::vector<std::pair<int,int>> large1UpObjects;

// //large1down
// extern int large1DownIndex;
// extern double lastUsedLarge1Down;
// extern std::vector<std::pair<int,int>> large1DownObjects;

// //large2up
// extern int large2UpIndex;
// extern double lastUsedLarge2Up;
// extern std::vector<std::pair<int,int>> large2UpObjects;

// //large2down
// extern int large2DownIndex;
// extern double lastUsedLarge2Down;
// extern std::vector<std::pair<int,int>> large2DownObjects;

// //raiderUp
// extern int raiderUpIndex;
// extern double lastUsedRaiderUp;
// extern std::vector<std::pair<int,int>> raiderUpObjects;

// //raiderDown
// extern int raiderDownIndex;
// extern double lastUsedRaiderDown;
// extern std::vector<std::pair<int,int>> raiderDownObjects;

// //hammerUp
// extern int hammerUpIndex;
// extern double lastUsedHammerUp;
// extern std::vector<std::pair<int,int>> hammerUpObjects;

// //hammerDown
// extern int hammerDownIndex;
// extern double lastUsedHammerDown;
// extern std::vector<std::pair<int,int>> hammerDownObjects;

// //geminiUp
// extern int geminiUpIndex;
// extern double lastUsedGeminiUp;
// extern std::vector<std::pair<int,int>> geminiUpObjects;

// //geminiDown
// extern int geminiDownIndex;
// extern double lastUsedGeminiDown;
// extern std::vector<std::pair<int,int>> geminiDownObjects;

// //holdUp
// extern int holdUpIndex;
// extern double lastUsedHoldUp;
// extern std::vector<std::pair<int,int>> holdUpObjects;

// //holdDown
// extern int holdDownIndex;
// extern double lastUsedHoldDown;
// extern std::vector<std::pair<int,int>> holdDownObjects;

// //masherUp
// extern int masherUpIndex;
// extern double lastUsedMasherUp;
// extern std::vector<std::pair<int,int>> masherUpObjects;

// //masherDown
// extern int masherDownIndex;
// extern double lastUsedMasherDown;
// extern std::vector<std::pair<int,int>> masherDownObjects;

// //gearUp
// extern int gearUpIndex;
// extern double lastUsedGearUp;
// extern std::vector<std::pair<int,int>> gearUpObjects;

// //gearDown
// extern int gearDownIndex;
// extern double lastUsedGearDown;
// extern std::vector<std::pair<int,int>> gearDownObjects;

// //bossProjectile1Up
// extern int bossProjectile1UpIndex;
// extern double lastUsedBossProjectile1Up;
// extern std::vector<std::pair<int,int>> bossProjectile1UpObjects;

// //bossProjectile1Down
// extern int bossProjectile1DownIndex;
// extern double lastUsedBossProjectile1Down;
// extern std::vector<std::pair<int,int>> bossProjectile1DownObjects;

// //bossProjectile2Up
// extern int bossProjectile2UpIndex;
// extern double lastUsedBossProjectile2Up;
// extern std::vector<std::pair<int,int>> bossProjectile2UpObjects;

// //bossProjectile2Down
// extern int bossProjectile2DownIndex;
// extern double lastUsedBossProjectile2Down;
// extern std::vector<std::pair<int,int>> bossProjectile2DownObjects;

// //bossProjectile3Up
// extern int bossProjectile3UpIndex;
// extern double lastUsedBossProjectile3Up;
// extern std::vector<std::pair<int,int>> bossProjectile3UpObjects;

// //bossProjectile3Down
// extern int bossProjectile3DownIndex;
// extern double lastUsedBossProjectile3Down;
// extern std::vector<std::pair<int,int>> bossProjectile3DownObjects;

// //bossGearUp
// extern int bossGearUpIndex;
// extern double lastUsedBossGearUp;
// extern std::vector<std::pair<int,int>> bossGearUpObjects;

// //bossGearDown
// extern int bossGearDownIndex;
// extern double lastUsedBossGearDown;
// extern std::vector<std::pair<int,int>> bossGearDownObjects;

// //pItemUp
// extern int pItemUpIndex;
// extern double lastUsedPItemUp;
// extern std::vector<std::pair<int,int>> pItemUpObjects;

// //pItemDown
// extern int pItemDownIndex;
// extern double lastUsedPItemDown;
// extern std::vector<std::pair<int,int>> pItemDownObjects;

// //ghostUp
// extern int ghostUpIndex;
// extern double lastUsedGhostUp;
// extern std::vector<std::pair<int,int>> ghostUpObjects;

// //ghostDown
// extern int ghostDownIndex;
// extern double lastUsedGhostDown;
// extern std::vector<std::pair<int,int>> ghostDownObjects;

// //heartUp
// extern int heartUpIndex;
// extern double lastUsedHeartUp;
// extern std::vector<std::pair<int,int>> heartUpObjects;

// //heartDown
// extern int heartDownIndex;
// extern double lastUsedHeartDown;
// extern std::vector<std::pair<int,int>> heartDownObjects;

// //noteUp
// extern int noteUpIndex;
// extern double lastUsedNoteUp;
// extern std::vector<std::pair<int,int>> noteUpObjects;

// //noteDown
// extern int noteDownIndex;
// extern double lastUsedNoteDown;
// extern std::vector<std::pair<int,int>> noteDownObjects;