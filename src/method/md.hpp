#ifndef mdChart
#define mdChart

#include "mdm.hpp"

std::string convertStageType(std::string originalUid);

void addNoteMD(LevelEditorLayer* editor,matjson::Value data, notestruct mdmnote);

int MDchart(LevelEditorLayer* editor, matjson::Value data, head mdmdata);

#endif