#ifndef mdChart
#define mdChart

std::string convertStageType(std::string originalUid);

void addNoteMD(LevelEditorLayer* editor,matjson::Value data);

int MDchart(LevelEditorLayer* editor, matjson::Value data);

#endif