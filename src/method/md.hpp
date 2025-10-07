#ifndef mdChart
#define mdChart

std::string convertStageType(std::string originalUid);

void addNoteMD(LevelEditorLayer* editor,matjson::Value data);

int doMD(LevelEditorLayer* editor, matjson::Value data);

#endif