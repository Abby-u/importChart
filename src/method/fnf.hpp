#ifndef fnf
#define fnf

// int grid;
// double timepergrid;
// double Ypos;
// double Xpos;
// double noteOffset;
// int spawnID;
// int pickupID;
// int areaScaleID;
// int itemEditID;

// double daTime;
// double bpm;
// double scroll;
// int beat;
using namespace geode::prelude;

std::string getHighestDiff(matjson::Value data);

bool validatePsychChart(matjson::Value data);

bool validateVSliceChart(matjson::Value data);

bool validateVSliceMetadata(matjson::Value data);

int checkNoteType(matjson::Value note,int origin);

int getData(std::function<void(matjson::Value)> onResult);

int getTargetGroup(int num);

int getTargetID1(int num);

int getTargetID2(int num);

void addNote(LevelEditorLayer* editor, double daX, double daY, double dur, int daType, int targetG, int targetDur, int targetType);

void addSpawn(LevelEditorLayer* editor, double daX, double daY, int group, bool parent);

void addAreaScale(LevelEditorLayer* editor, double daX, double daY, int target, int center, double length, int daeffectID, bool early,double scaleY);

void addBPMChanger(LevelEditorLayer* editor, double daX, double daY, bool early);

void addScrollSpeed(LevelEditorLayer* editor, double daX, double daY, double daScroll,double daDur,bool early);

void fnfChart(LevelEditorLayer* editor,matjson::Value data);

int initFnf(matjson::Value data);

#endif