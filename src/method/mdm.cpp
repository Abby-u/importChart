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

using namespace geode::prelude;

int getPathway(std::string lane){
    if (lane == "11"||lane=="51"||lane=="D1"){
        return 3;
    }else if (lane=="12"||lane=="52"||lane=="D2"){
        return 2;
    }else if (lane=="13"||lane=="53"||lane=="D3"){
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
        if ((speedchanges[i].pathway==thepathway||speedchanges[i].pathway==-1)&&speedchanges[i].time>thetime&&(speedchanges[i].time-thetime)<dist){
            nearest = i;
            dist = speedchanges[i].time-thetime;
        }
    }
    return nearest;
}

void setSpeeds(){
    for (int i=0;i<speedchanges.size();i++){
        int nexti = findNextSimilarSpeedChanger(speedchanges[i].pathway,speedchanges[i].time);
        if (nexti==-1){
            for (int j=0;j<tempnotes.size();j++){
                if ((tempnotes[j].pathway%2==speedchanges[i].pathway||speedchanges[i].pathway==-1)&&tempnotes[j].time>=speedchanges[i].time){
                    tempnotes[j].speed = speedchanges[i].speed;
                }
            }
        }else{
            for (int j=0;j<tempnotes.size();j++){
                if ((tempnotes[j].pathway%2==speedchanges[i].pathway||speedchanges[i].pathway==-1)&&tempnotes[j].time>=speedchanges[i].time&&tempnotes[j].time<speedchanges[nexti].time){
                    tempnotes[j].speed = speedchanges[i].speed;
                }
            }
        }
    }
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

    std::istringstream stream(rawdata);
    std::string line;
    int section = 0;
    std::regex bpmr("#BPM ([+-]?(?=\\.\\d|\\d)(?:\\d+)?(?:\\.?\\d*))(?:[Ee]([+-]?\\d+))?");
    std::regex datafield("\\*-+ MAIN DATA FIELD");

    std::regex sectionsr("#[0-9]+:[A-Za-z0-9]+");
    double timestep;
    int lane;

    int curspeed = 1;
    int curpathway =0;

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
                log::info("{} {} {} {}",temptime,i,bms.size()-1,ratio);
                if (note=="0F"||note=="0G"||note=="16"||note=="17"){
                    if (lane.substr(0,1)=="1"||lane.substr(0,1)=="D"){
                        notestruct thisnote ={
                            note,
                            temptime,
                            0.001,
                            pathway,
                            curspeed
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
                                curspeed
                            };
                            tempnotes.push_back(thisnote);
                            durnotes.erase(durnotes.begin() + j);
                        }else{
                            notestruct thisnote ={
                                note,
                                temptime,
                                thisdur,
                                pathway,
                                curspeed
                            };
                            durnotes.push_back(thisnote);
                        }
                    }
                }else if ("0O"<note&&note<"0W"){
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
                    notestruct thisnote ={
                        note,
                        temptime,
                        thisdur,
                        changePathInt,
                        thespeed
                    };
                    speedchanges.push_back(thisnote);
                }else{
                    notestruct thisnote ={
                        note,
                        temptime,
                        thisdur,
                        pathway,
                        curspeed
                    };
                    tempnotes.push_back(thisnote);
                }
            }
        }
    }
    setSpeeds();
    std::sort(tempnotes.begin(),tempnotes.end(), [](const notestruct& a, const notestruct& b){
        return a.time < b.time;
    });
    head thismdm = {
        thisbpm,
        tempnotes
    };
    int res = MDchart(GameManager::sharedState()->getEditorLayer(),nullptr,thismdm);

    return res;
}