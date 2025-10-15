#ifndef mdm
#define mdm

struct notestruct {
    std::string ibms_id;
    double time;
    double duration;
    int pathway;
    int speed;
};

struct head{
    double bpm;
    std::vector<notestruct> notes;
};

int mdmChart(LevelEditorLayer* editor, std::string rawdata);

#endif