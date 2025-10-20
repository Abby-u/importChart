#include <Geode/Geode.hpp>
#include "mdm.hpp"
#include "mainVar.hpp"
#include "utills.hpp"
#include <string>
#include <regex>
#include "md.hpp"

std::vector<notestruct> tempnotes;
std::vector<notestruct> speedchanges;
std::vector<notestruct> durnotes;
std::vector<notestruct> scenechanges;

using namespace geode::prelude;

int getPathway(std::string lane){
    if (lane == "11"||lane == "31"||lane=="51"||lane=="D1"){
        return 3;
    }else if (lane=="12"||lane == "32"||lane=="52"||lane=="D2"){
        return 2;
    }else if (lane=="13"||lane == "33"||lane=="53"||lane=="D3"){
        return 1;
    }else if (lane=="15"){
        return 1;
    }
    return 0;
}

int findSimilar(std::string thebms, int thepathway){
    for (int i=0;i<durnotes.size();i++){
        if (durnotes[i].ibms_id==thebms&&durnotes[i].pathway==thepathway){
            return i;
        }
    }
    return -1;
}

int findNextSimilarSpeedChanger(int thepathway, double thetime){
    int nearest = -1;
    double dist = sizeof(double);
    for (int i=0;i<speedchanges.size();i++){
        if (speedchanges[i].pathway==thepathway&&speedchanges[i].time>thetime&&(speedchanges[i].time-thetime)<dist){
            nearest = i;
            dist = speedchanges[i].time-thetime;
        }
    }
    return nearest;
}

int findNextSimilarSceneChanger(double thetime){
    int nearest = -1;
    double dist = sizeof(double);
    for (int i=0;i<scenechanges.size();i++){
        if (scenechanges[i].time>thetime&&(scenechanges[i].time-thetime)<dist){
            nearest = i;
            dist = scenechanges[i].time-thetime;
        }
    }
    return nearest;
}

void setSpeeds(){
    for (int i=0;i<speedchanges.size();i++){
        int nexti = findNextSimilarSpeedChanger(speedchanges[i].pathway,speedchanges[i].time);
        if (nexti==-1){
            for (int j=0;j<tempnotes.size();j++){
                if (tempnotes[j].pathway%2==speedchanges[i].pathway&&tempnotes[j].time>=speedchanges[i].time){
                    tempnotes[j].speed = speedchanges[i].speed;
                }
            }
        }else{
            for (int j=0;j<tempnotes.size();j++){
                if (tempnotes[j].pathway%2==speedchanges[i].pathway&&tempnotes[j].time>=speedchanges[i].time&&tempnotes[j].time<speedchanges[nexti].time){
                    tempnotes[j].speed = speedchanges[i].speed;
                }
            }
        }
    }
}

void setScenes(){
    for (int i=0;i<scenechanges.size();i++){
        int nexti = findNextSimilarSceneChanger(scenechanges[i].time);
        if (nexti==-1){
            for (int j=0;j<tempnotes.size();j++){
                if (tempnotes[j].time>=scenechanges[i].time){
                    tempnotes[j].scene = scenechanges[i].scene;
                }
            }
        }else{
            for (int j=0;j<tempnotes.size();j++){
                if (tempnotes[j].time>=scenechanges[i].time&&tempnotes[j].time<scenechanges[nexti].time){
                    tempnotes[j].scene = scenechanges[i].scene;
                }
            }
        }
    }
}

std::string getScene(std::string note){
    if (note=="1O") return "scene_01";
    if (note=="1P") return "scene_02";
    if (note=="1Q") return "scene_03";
    if (note=="1R") return "scene_04";
    if (note=="1S") return "scene_05";
    if (note=="1T") return "scene_06";
    if (note=="1U") return "scene_07";
    if (note=="1V") return "scene_08";
    if (note=="1W") return "scene_09";
    return "";
}

int mdmChart(LevelEditorLayer* editor, std::string rawdata){
    if (!editor){
		log::warn("editor gone??");
		return 1;
	}
    double thisbpm;

    tempnotes.clear();
    durnotes.clear();
    speedchanges.clear();
    scenechanges.clear();

    std::istringstream stream(rawdata);
    std::string line;

    int lane;

    int curspeed = 1;
    std::string curscene;

    double timeperbeat;

    std::smatch valuematch;
    while (std::getline(stream, line)){//refer to CustomAlbums/BmsLoader.cs
        if (line.empty()||!line.starts_with('#')){
            continue;
        }
        line = line.substr(1);
        if (line.find(' ') != std::string::npos){
            std::vector<std::string> splitline = utils::string::split(line, " ");
            if (splitline.size() < 2){continue;}
            std::string key = splitline[0];
            std::string value = splitline[1];

            // Correctly parse BPM lines; ensure we initialize thisbpm and timeperbeat.
            if (key == "BPM") {
                try {
                    thisbpm = std::stod(value);
                    if (thisbpm > 0.0) {
                        timeperbeat = 60.0 / thisbpm;
                    } else {
                        continue;
                    }
                } catch (const std::exception&) {
                    continue;
                }
                continue;
            }
            if (key == "PLAYER") {
                try {
                    curspeed = std::stoi(value);
                } catch (const std::exception&) {
                    continue;
                }
                continue;
            }
            if (key == "GENRE") {
                try {
                    curscene = value;
                } catch (const std::exception&) {
                    continue;
                }
                continue;
            }

        }else if (line.find(':')){
            std::vector<std::string> splitline = utils::string::split(line,":");
            if (splitline.size() < 2){continue;}
            std::string beatnlane = splitline[0];
            std::string bms = splitline[1];
            int beat = std::stoi(beatnlane.substr(0,3));
            std::string lane = beatnlane.substr(3,2);
            int pathway = getPathway(lane);//pathway
            for (int i=0;i<bms.size();i=i+2){
                // log::info("145");
                std::string note = bms.substr(i,2);//ibms id
                if (note=="00"){
                    continue;
                }
                double thisdur = 0.0;
                double ratio = static_cast<double>(i)/static_cast<double>(bms.size()-1);
                double temptime = (ratio+beat)*(timeperbeat*4);//time
                // log::info("{} {} {} {}",temptime,i,bms.size()-1,ratio);
                if (note=="0F"||note=="0G"||note=="16"||note=="17"){
                    if (lane.substr(0,1)=="1"||lane.substr(0,1)=="D"){
                        notestruct thisnote ={
                            note,
                            temptime,
                            0.001,
                            pathway,
                            curspeed,
                            curscene
                        };
                        tempnotes.push_back(thisnote);
                    }else{
                        int j = findSimilar(note,pathway);
                        if (j != -1){
                            thisdur = temptime-durnotes[j].time;
                            notestruct thisnote ={
                                note,
                                durnotes[j].time,
                                thisdur,
                                durnotes[j].pathway,
                                curspeed,
                                curscene
                            };
                            tempnotes.push_back(thisnote);
                            durnotes.erase(durnotes.begin() + j);
                        }else{
                            notestruct thisnote ={
                                note,
                                temptime,
                                thisdur,
                                pathway,
                                curspeed,
                                curscene
                            };
                            durnotes.push_back(thisnote);
                        }
                    }
                }else if ("0O"<=note&&note<="0W"){
                    int changePathInt = 0;
                    int thespeed = 1;
                    
                    if ("0O"<=note&&note<="0Q"){
                        changePathInt = -1;
                    }else if("0U"<=note&&note<="0W"){
                        changePathInt = 1;
                    }
                    //man
                    if (note=="0P"||note=="0S"||note=="0V"){
                        thespeed = 2;
                    }else if(note=="0Q"||note=="0T"||note=="0W"){
                        thespeed = 3;
                    }

                    if (changePathInt==-1){
                        notestruct thisnoteup ={
                            note,
                            temptime,
                            thisdur,
                            1,
                            thespeed,
                            curscene
                        };
                        speedchanges.push_back(thisnoteup);
                        notestruct thisnotedown ={
                            note,
                            temptime,
                            thisdur,
                            0,
                            thespeed,
                            curscene
                        };
                        speedchanges.push_back(thisnotedown);
                    }else{
                        notestruct thisnote ={
                            note,
                            temptime,
                            thisdur,
                            changePathInt,
                            thespeed,
                            curscene
                        };
                        speedchanges.push_back(thisnote);
                    }
                }else if ("1O"<=note&&note<="1W"){
                    notestruct thisnote ={
                        note,
                        temptime,
                        thisdur,
                        pathway,
                        curspeed,
                        getScene(note)
                    };
                    scenechanges.push_back(thisnote);
                }else{
                    notestruct thisnote ={
                        note,
                        temptime,
                        thisdur,
                        pathway,
                        curspeed,
                        curscene
                    };
                    tempnotes.push_back(thisnote);
                }
            }
        }
    }
    std::sort(tempnotes.begin(),tempnotes.end(), [](const notestruct& a, const notestruct& b){
        return a.time < b.time;
    });
    std::sort(speedchanges.begin(),speedchanges.end(), [](const notestruct& a, const notestruct& b){
        return a.time < b.time;
    });
    std::sort(scenechanges.begin(),scenechanges.end(), [](const notestruct& a, const notestruct& b){
        return a.time < b.time;
    });
    setSpeeds();
    setScenes();
    head thismdm = {
        thisbpm,
        curscene,
        tempnotes
    };
    int res = MDchart(GameManager::sharedState()->getEditorLayer(),nullptr,thismdm);

    return res;
}