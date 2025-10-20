#ifndef mdm
#define mdm

struct notestruct {
    std::string ibms_id;
    double time;
    double duration;
    int pathway;
    int speed;
    std::string scene;
};

struct head{
    double bpm;
    std::string stage;
    std::vector<notestruct> notes;
};

int mdmChart(LevelEditorLayer* editor, std::string rawdata);

#endif