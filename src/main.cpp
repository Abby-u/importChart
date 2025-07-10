#include <Geode/Geode.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include <fstream>
#include <Geode/binding/SetupTriggerPopup.hpp>
#include <math.h>

int grid = 30;
double timepergrid = 0.09628343;
double Ypos = 5025.0;
double Xpos = 0.0;
double noteOffset = (1.56/0.09628343)*-1;
int spawnID = 1268;
int pickupID = 1817;
int areaScaleID = 3008; //lol
int itemEditID = 3619;

double daTime = 0;
double bpm = 120;
double scroll = 1.0;
int beat = 0;

int leftG = 50;
int downG = 51;
int upG = 52;
int rightG = 53;

int leftHold = 300;
int downHold = 301;
int upHold = 302;
int rightHold = 303;

int dadleftG = 700;
int daddownG = 710;
int dadupG = 720;
int dadrightG = 730;

int dadleftHold = 304;
int daddownHold = 305;
int dadupHold = 306;
int dadrightHold = 307;

int bfSideG = 27;
int dadSideG = 28;

using namespace geode::prelude;

std::string getHighestDiff(matjson::Value data){
	if (data["notes"].size()==3){
		if (data["notes"].contains("hard")){
			return "hard";
		}else if(data["notes"].contains("normal")){
			return "normal";
		}else if(data["notes"].contains("easy")){
			return "easy";
		}else {
			log::warn("unknown difficulty");
			return nullptr;
		}
	} else if (data["notes"].size()==2){
		if (data["notes"].contains("nightmare")){
			return "nightmare";
		}else if(data["notes"].contains("erect")){
			return "erect";
		}else {
			log::warn("unknown difficulty");
			return nullptr;
		}
	} else{
		log::warn("custom difficulty or no difficulty?");
		return nullptr;
	}
}

bool validatePsychChart(matjson::Value data){
	if (data.contains("song") && data["song"].contains("notes") && data["song"].contains("events") && data["song"].contains("bpm")){
		return true;
	}
	return false;
}

bool validateVSliceChart(matjson::Value data){
	if (data.contains("version") && data.contains("notes") && data.contains("events")){
		return true;
	}
	return false;
}

bool validateVSliceMetadata(matjson::Value data){
	if (data.contains("version") && data.contains("playData") && data.contains("timeChanges")){
		return true;
	}
	return false;
}

void getData(std::function<void(matjson::Value)> onResult){
	utils::file::pick(utils::file::PickMode::OpenFile,{}).listen([onResult](geode::Result<std::filesystem::path, std::string>* file){
		if (!file){
			FLAlertLayer::create("No file selected", "Import cancelled", "OK")->show();
			onResult(matjson::Value());
			return;
		}
		auto rawData = file->unwrap();
		auto jsonData = utils::file::readJson(rawData);
		if (!jsonData) {
			FLAlertLayer::create("Invalid File Type", "This ain't json file. Import cancelled", "OK")->show();
			log::warn("this aint json");
			onResult(matjson::Value());
			return;
		}
		log::info("ok");
		onResult(jsonData.unwrap());
	});
}

int getTargetGroup(int num){
	switch (num){
		case 0: return leftG;
		case 1: return downG;
		case 2: return upG;
		case 3: return rightG;
		case 4: return dadleftG;
		case 5:	return daddownG;
		case 6: return dadupG;
		case 7: return dadrightG;
		default: return 0;
	}
}
int getTargetID1(int num){
	switch (num){
		case 0: return leftHold;
		case 1: return downHold;
		case 2: return upHold;
		case 3: return rightHold;
		case 4: return dadleftHold;
		case 5: return daddownHold;
		case 6: return dadupHold;
		case 7: return dadrightHold;
		default: return 0;
	}
}

void addNote(LevelEditorLayer* editor, double daX, double daY, double dur = 0){
	auto ui = editor->m_editorUI;
	double offset = 0;
	double objX = ((noteOffset*grid)+(daX/1000*(grid/timepergrid)))+Xpos;
	double objY = (((daY+offset)*grid)+Ypos);
	CCPoint pos = {(float)objX,(float)objY};
	auto obj = ui->createObject(spawnID,pos);
	auto trigger = dynamic_cast<EffectGameObject*>(obj);
	if (trigger){
		trigger->m_targetGroupID = getTargetGroup(daY);
	}
	double obj1X = (noteOffset*grid)+(daX/1000*(grid/timepergrid))-(1.0f)+Xpos;
	double obj1Y = (((daY+offset)*grid)+Ypos);
	CCPoint pos1 = {(float)obj1X,(float)obj1Y};
	auto obj1 = ui->createObject(itemEditID,pos1);
	auto trigger1 = dynamic_cast<ItemTriggerGameObject*>(obj1);
	if (trigger1){
		trigger1->m_mod1 = dur;
		trigger1->m_targetItemMode = 2;
		trigger1->m_resultType3 = 2;
	}
	auto trigger3 = dynamic_cast<EffectGameObject*>(obj1);
	if (trigger3){
		trigger3->m_targetGroupID = getTargetID1(daY);
	}

	editor->updateObjectLabel(obj);
	editor->updateObjectLabel(obj1);
	// selection->addObject(obj);
	// selection->addObject(obj1);
}

void addSpawn(LevelEditorLayer* editor, double daX, double daY, int group, bool parent = false){
	auto ui = editor->m_editorUI;

	double objX = (daX/1000*(grid/timepergrid))+Xpos;
	double objY = ((daY*grid)+Ypos);
	CCPoint pos = {(float)objX,(float)objY};
	auto obj = ui->createObject(spawnID,pos);
	auto trigger = dynamic_cast<EffectGameObject*>(obj);
	if (trigger){
		trigger->m_targetGroupID = group;
	}
	obj->m_hasGroupParent = parent;

	editor->updateObjectLabel(obj);
	// selection->addObject(obj);

}

void addAreaScale(LevelEditorLayer* editor, double daX, double daY, int target, int center, double length, int daeffectID, bool early = false){
	auto ui = editor->m_editorUI;
	double doffset = (early == true)?(noteOffset*grid):0.0;
	int daLength = ceilf(length*30);
	double dX = doffset+(daX/1000*(grid/timepergrid))+Xpos;
	double dY = (((daY+1)*grid)+Ypos);
	CCPoint daPos = {(float)dX,(float)dY};
	auto daObj = ui->createObject(areaScaleID,daPos);
	auto daAreaTrigger = dynamic_cast<EnterEffectObject*>(daObj);
	if (daAreaTrigger){
		daAreaTrigger->m_length = daLength;
		daAreaTrigger->m_offset = daLength*-1;
		daAreaTrigger->m_areaScaleY = 0.0f;
		daAreaTrigger->m_areaScaleX = 1.0f;
		daAreaTrigger->m_deadzone = 0.0f;
		daAreaTrigger->m_effectID = daeffectID;
		daAreaTrigger->m_priority = 2;
		daAreaTrigger->m_dontEditAreaParent = false;
		daAreaTrigger->m_directionType = 2;
		daAreaTrigger->m_inbound = true;
	}
	auto normalLookingTrigger = dynamic_cast<EffectGameObject*>(daObj);
	if (normalLookingTrigger){
		normalLookingTrigger->m_targetGroupID = target;
		normalLookingTrigger->m_centerGroupID = center;
	}

	editor->updateObjectLabel(daObj);
	// selection->addObject(daObj);
}

void addBPMChanger(LevelEditorLayer* editor, double daX, double daY, bool early = false){
	auto ui = editor->m_editorUI;
	double doffset = (early == true)?(noteOffset*grid):0.0;
	double objX = doffset+(daX/1000*(grid/timepergrid))+Xpos;
	double objY = ((daY*grid)+Ypos);
	CCPoint pos = {(float)objX,(float)objY};
	auto obj = ui->createObject(spawnID,pos);
	auto trigger = dynamic_cast<EffectGameObject*>(obj);
	if (trigger){
		trigger->m_targetGroupID = 396;
	}
	double obj1X = doffset+(daX/1000*(grid/timepergrid))-(1.0f)+Xpos;
	double obj1Y = ((daY*grid)+Ypos);
	CCPoint pos1 = {(float)obj1X,(float)obj1Y};
	auto obj1 = ui->createObject(itemEditID,pos1);
	auto trigger1 = dynamic_cast<ItemTriggerGameObject*>(obj1);
	if (trigger1){
		trigger1->m_mod1 = bpm;
		trigger1->m_targetItemMode = 2;
		trigger1->m_resultType3 = 2;
	}
	auto trigger3 = dynamic_cast<EffectGameObject*>(obj1);
	if (trigger3){
		trigger3->m_targetGroupID = 2;
	}
	double length = ((60/bpm/4)/0.1538461538461538)+0.1;
	double obj2X = doffset+(daX/1000*(grid/timepergrid))+Xpos;
	double obj2Y = (((daY+1)*grid)+Ypos);
	CCPoint pos2 = {(float)obj2X,(float)obj2Y};
	auto obj2 = ui->createObject(areaScaleID,pos2);
	auto trigger4 = dynamic_cast<EnterEffectObject*>(obj2);
	if (trigger4){
		trigger4->m_areaScaleY = length;
		trigger4->m_areaScaleX = 1.0f;
		trigger4->m_deadzone = 1.0f;
		trigger4->m_effectID = 20;
		trigger4->m_priority = 1;
		trigger4->m_dontEditAreaParent = true;
		trigger4->m_directionType = 0; //this is very much time wasting
		trigger4->m_inbound = true;
	}
	auto trigger5 = dynamic_cast<EffectGameObject*>(obj2);
	if (trigger5){
		trigger5->m_targetGroupID = 280;
		trigger5->m_centerGroupID = 997;
	}
	addAreaScale(editor,daX,daY,1040,275,length,21,early);
	addAreaScale(editor,daX,daY,1041,281,length,22,early);
	addAreaScale(editor,daX,daY,1042,282,length,23,early);
	addAreaScale(editor,daX,daY,1043,283,length,24,early);

	addAreaScale(editor,daX,daY,1044,360,length,25,early);
	addAreaScale(editor,daX,daY,1045,361,length,26,early);
	addAreaScale(editor,daX,daY,1046,362,length,27,early);
	addAreaScale(editor,daX,daY,1047,363,length,28,early);

	editor->updateObjectLabel(obj);
	editor->updateObjectLabel(obj1);
	// selection->addObject(obj);
	// selection->addObject(obj1);
}

void addScrollSpeed(LevelEditorLayer* editor, double daX, double daY, double daScroll,double daDur,bool early = false){
	auto ui = editor->m_editorUI;

	double doffset = (early == true)?(noteOffset*grid):0.0;
	double resultscroll = scroll*daScroll;
	double length = ((60/bpm/4)/0.1538461538461538)+0.1;
	double obj2X = doffset+(daX/1000*(grid/timepergrid))+Xpos;
	double obj2Y = (((daY)*grid)+Ypos);
	CCPoint pos2 = {(float)obj2X,(float)obj2Y};
	auto obj2 = ui->createObject(3011,pos2);
	auto trigger4 = dynamic_cast<EnterEffectObject*>(obj2);
	if (trigger4){
		trigger4->m_moveDistance = (std::max((double)1.0,resultscroll)-1.0)*300;
		trigger4->m_duration = daDur;
		trigger4->m_useEffectID = true;
		trigger4->m_targetGroupID = 29; //bruh it should be m_effectID
	}
	auto obj3 = ui->createObject(3011,pos2);
	auto trigger5 = dynamic_cast<EnterEffectObject*>(obj3);
	if (trigger5){
		trigger5->m_moveDistance = (std::max((double)1.0,resultscroll)-1.0)*300;
		trigger5->m_duration = daDur;
		trigger5->m_useEffectID = true;
		trigger5->m_targetGroupID = 30;
	}
	auto obj4 = ui->createObject(3013,pos2);
	auto trigger6 = dynamic_cast<EnterEffectObject*>(obj4);
	if (trigger6){
		trigger6->m_areaScaleY = (std::max((double)1.0,resultscroll));
		trigger6->m_areaScaleX = 1.0;
		trigger6->m_duration = daDur;
		trigger6->m_useEffectID = true;
		trigger6->m_targetGroupID = 31;
	}

	// selection->addObject(obj);
	// selection->addObject(obj1);
}

void importChart(LevelEditorLayer* editor,matjson::Value data){
	if (!editor){
		log::warn("editor gone??");
		return;
	}
	// spawnPos.setPoint(5025.0,0.0);
	// selection->removeAllObjects();
	Ypos = 5025.0f;
	Xpos = 0.0f;
	auto ui = editor->m_editorUI;
	auto objects = ui->getSelectedObjects();
	if (objects && objects->count() > 0){
		double minX = -4000.0;
		double minY = -4000.0;
		for (int i = 0; i < objects->count(); ++i){
			auto obj = static_cast<GameObject*>(objects->objectAtIndex(i));
			CCPoint daPos = obj->getPosition();
			minX = (daPos.x>minX)?daPos.x:minX;
			minY = (daPos.y>minY)?daPos.y:minY;
		}
		Ypos = minY;
		Xpos = minX;
		// spawnPos.setPoint(minX,minY);
	}

	daTime = 0.0;
	if (validatePsychChart(data)){
		//old chart pre v-slice or psychEngine
		bool converted = false;
		if (data["song"].contains("format") && data["song"]["format"] == "psych_v1_convert"){
			converted = true;
		}
		// log::info("{}",data["song"].dump());
		scroll = data["song"]["speed"].asDouble().unwrap();
		bpm = data["song"]["bpm"].asDouble().unwrap();
		addBPMChanger(editor,0.0,20.0f,true);
		addScrollSpeed(editor,0.0,22.0,1.0,0.0,true);
		addSpawn(editor,0.0,23.0,741,true);
		for (const auto& [a, b]:data["song"]["notes"]){
			for (const auto& [c, d]:b){
				if (c == "changeBPM" && d == true) {
					bpm = b["bpm"].asDouble().unwrap();
					addBPMChanger(editor,daTime,20.0f);
				}else if (c == "mustHitSection"){
					if (d == true){
						addSpawn(editor,daTime,15.0f,bfSideG);
					}else{
						addSpawn(editor,daTime,16.0f,dadSideG);
					}
				}else if (c == "sectionNotes"){
					for (const auto& [e,f]:d){
						if (b["mustHitSection"].asBool().unwrap() || converted){
							addNote(editor,f[0].asDouble().unwrap(),f[1].asDouble().unwrap(),f[2].asDouble().unwrap());
						}else{
							if (f[1]<4){
								addNote(editor,f[0].asDouble().unwrap(),f[1].asDouble().unwrap()+4,f[2].asDouble().unwrap());
							} else {
								addNote(editor,f[0].asDouble().unwrap(),f[1].asDouble().unwrap()-4,f[2].asDouble().unwrap());
							}
						}
					}
				}
			}
			int section = (b.contains("sectionBeats")) ? b["sectionBeats"].asInt().unwrap() : b["lengthInSteps"].asInt().unwrap()/4;
			for (int i = 0;i < section;++i){
				if (beat%4==0){
					addSpawn(editor,daTime+(i*60/bpm*1000),17.0f,26);
				}
				if (beat%2==0){
					addSpawn(editor,daTime+(i*60/bpm*1000),18.0f,945);
				}
				if (beat%1==0){
					addSpawn(editor,daTime+(i*60/bpm*1000),19.0f,1224);
				}
				++beat;
			}
			daTime = daTime + (60/bpm)*section*1000;
		}
		if (data["song"].contains("events") && data["song"]["events"].isArray()){
			for (const auto& [a,b]:data["song"]["events"]){
				double thisTime = b[0].asDouble().unwrap();
				for (const auto& [c,d]:b[1]){
					if (d[0] == "Change Scroll Speed"){
						double daScroll = (!d[1].isNull())?std::stod(d[1].asString().unwrap()):1.0;
						double daDur = (!d[2].isNull()&&!d[2].asString().unwrap().empty())?std::stod(d[2].asString().unwrap()):0.0;
						addScrollSpeed(editor,thisTime,22.0,daScroll,daDur);
					} else if (d[0] == "Add Camera Zoom"){
						addSpawn(editor,thisTime,17.0f,26);
					}
				}
			}
		}
		editor->updateEditor(0.0f);
	}else if (validateVSliceChart(data)) {
		//vslice
		getData([data,editor](matjson::Value metadata){
			if (!metadata.isNull() && metadata.size()>0 && validateVSliceMetadata(metadata)){
				std::string difficulty = getHighestDiff(data);
				scroll = data["scrollSpeed"][difficulty].asDouble().unwrap();
				bpm = metadata["timeChanges"][0]["bpm"].asDouble().unwrap();
				double lastNote = data["notes"][difficulty][data["notes"][difficulty].size()-1]["t"].asDouble().unwrap();
				addBPMChanger(editor,0.0,20.0f,true);
				addScrollSpeed(editor,0.0,22.0,1.0,0.0,true);
				addSpawn(editor,0.0,23.0,741,true);
				for (const auto&[a,b]:data["notes"][difficulty]){
					addNote(editor,b["t"].asDouble().unwrap(),b["d"].asInt().unwrap(),(b.contains("l")?b["l"].asDouble().unwrap():(double)0.0));
				}
				for (const auto&[a,b]:data["events"]){
					auto event = b["e"].asString().unwrap();
					double daTime = b["t"].asDouble().unwrap();
					if (event == "FocusCamera"){
						if (b["v"]["char"].asInt().unwrap() == 0){
							addSpawn(editor,daTime,15.0f,bfSideG);
						}else if (b["v"]["char"].asInt().unwrap() == 1){
							addSpawn(editor,daTime,16.0f,dadSideG);
						}
					}
				}
				for (double dTime = 0.0;dTime<lastNote;dTime = dTime + ((60/bpm)*4000)){
					addSpawn(editor,dTime,17.0f,26);
				}
				for (double dTime = 0.0;dTime<lastNote;dTime = dTime + ((60/bpm)*2000)){
					addSpawn(editor,dTime,18.0f,945);
				}
				for (double dTime = 0.0;dTime<lastNote;dTime = dTime + ((60/bpm)*1000)){
					addSpawn(editor,dTime,19.0f,1224);
				}
			}
		});
		return;
	}else {
		FLAlertLayer::create("Invalid Chart Type", "Unknown chart from unknown engine? or maybe its not even an FNF chart file.", "OK")->show();
		//this aint chart data
		return;
	}
}

class $modify(editedPauseLayer,EditorPauseLayer) {
	bool init(LevelEditorLayer* p0) {
		if (!EditorPauseLayer::init(p0)) {
			return false;
		}
		
		auto actionsButtons = this->getChildByID("actions-menu");
		// @geode-ignore(unknown-resource)
		auto topSprite = CCSprite::createWithSpriteFrameName("GJ_button_04-uhd.png");
		auto sprite = ButtonSprite::create(
			"Import Chart",
			40,
    		true,
    		"bigFont.fnt",
    		// @geode-ignore(unknown-resource)
    		"GJ_button_04-uhd.png",
    		24.f,
    		1.f     
		);
		auto importButton = CCMenuItemSpriteExtra::create(
			sprite, actionsButtons, menu_selector(editedPauseLayer::buttonPress)
		);
		importButton->setID("import-chart"_spr);
		actionsButtons->addChild(importButton,0,-1);

		actionsButtons->updateLayout(true);

		return true;
	}

	void buttonPress(CCObject*){
		utils::file::pick(utils::file::PickMode::OpenFile,{}).listen([](geode::Result<std::filesystem::path, std::string>* file){
		if (!file){
			log::info("nvm");
			return;
		}
		auto rawData = file->unwrap();
		auto jsonData = utils::file::readJson(rawData);
		if (!jsonData) {
			FLAlertLayer::create("Invalid File Type", "This ain't json file. (ig)", "OK")->show();
			log::warn("this aint json");
			return;
		}
		log::info("ok");
		auto editor = GameManager::sharedState()->getEditorLayer();
		importChart(editor,jsonData.unwrap());
		//initiateInterface();
		});
	}

	void initiateInterface(){// not finished yet
		auto root = CCDirector::sharedDirector()->getRunningScene();
		auto screenDmnsn = CCDirector::sharedDirector()->getWinSize();

		auto daMenu = CCMenu::create();
		daMenu->setID("layer-container"_spr);
		daMenu->setPosition(screenDmnsn/2);
		daMenu->setTouchPriority(-504);
		root->addChild(daMenu,106);

		auto blacktransp = CCLayerColor::create(ccc4(0,0,0,128));
		blacktransp->setID("bg"_spr);
		daMenu->addChild(blacktransp,-10);

		// @geode-ignore(unknown-resource)
		auto bg1 = CCScale9Sprite::createWithSpriteFrameName("GJ_square01.png");
		if (!bg1){
			log::warn("the bg tweakin");
			return;
		}
		bg1->setContentSize({800.f,600.f});
		bg1->setID("containerbg1"_spr);
		daMenu->addChild(bg1,-10);
	}
};
