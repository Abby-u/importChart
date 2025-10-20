#include <Geode/Geode.hpp>
#include "md.hpp"
#include "mainVar.hpp"
#include "uid.hpp"
#include "mdm.hpp"
#include "objectCache.hpp"
#include <string>
#include "utills.hpp"

using namespace geode::prelude;

bool overflow = false;
std::vector<int> overflowObjects;

std::optional<double> lastFlashTime;
std::optional<CCPoint> lastFlashPos;
bool flashStart = false;

int curBossPhase = -1;

note_data getNoteData(std::string daUid){
	std::string thisUid = daUid;
	for (const auto& a : note_uids){
		if (a.ibms_id==thisUid){
			return a;
		}
	}
	return {};
}

int getBossPhase(int group){
	switch (group)
	{
		case 290: return 0;
		case 306: return -1;
		case 291: return 1;
		case 293: return 0;
		case 295: return 2;
		case 300: return 0;
		case 302: return 2;
		case 304: return 1;
		case 337: return 1;
		case 343: return 2;
		default: return -2;
	}
}

int getSceneGroup(std::string scene){
    if (scene=="scene_01") return 239;
    if (scene=="scene_02") return 240;
    if (scene=="scene_03") return 241;
    if (scene=="scene_04") return 242;
    if (scene=="scene_05") return 243;
    if (scene=="scene_06") return 244;
    if (scene=="scene_07") return 245;
    if (scene=="scene_08") return 246;
    if (scene=="scene_09") return 247;
	if (scene=="scene_10") return 383;
	if (scene=="scene_11") return 384;
	if (scene=="scene_12") return 385;
    return 0;
}

int getBossPhaseSpawn(int from,int to){
	if (from==-1&&to==0){
		return 290;
	}
	if (from==0&&to==1){
		return 291;
	}
	if (from==1&&to==0){
		return 293;
	}
	if (from==0&&to==2){
		return 295;
	}
	if (from==2&&to==0){
		return 300;
	}
	if (from==1&&to==2){
		return 302;
	}
	if (from==2&&to==1){
		return 304;
	}
	if (from==0&&to==-1){
		return 306;
	}
	return 0;
}

void findUnusedObject(objectsData* daObjects, double theX, double theDur){
	bool found = false;

	for (int i = 0;i<daObjects->groups.size();i++){
		if (!daObjects->groups[i].lastUsed||
			((theX)-(daObjects->groups[i].lastUsed)>2.1&&
			(theX)-(daObjects->groups[i].lastUsed+daObjects->groups[i].dur)>2.1)){
			daObjects->index = i;
			daObjects->groups[i].lastUsed = theX;
			daObjects->groups[i].dur = theDur;
			found = true;
			break;
		}
	}

	if (!found){
		overflow = true;
		bool foundSimilar = false;
		for (int i=0;i<overflowObjects.size();i++){
			if (overflowObjects[i]==daObjects->groups[0].group){
				foundSimilar = true;
				break;
			}
		}
		if (!foundSimilar){
			overflowObjects.push_back(daObjects->groups[0].group);
		}
	}
}

void setInitScene(std::string scene){
	log::info("{}.",scene);
	auto ui = GameManager::sharedState()->getEditorLayer()->m_editorUI;
	auto obj = ui->createObject(spawnID,{0,(float)Ypos});
	auto trigger = static_cast<SpawnTriggerGameObject*>(obj);
	if (trigger){
		trigger->m_targetGroupID = getSceneGroup(scene);
	}
}

void checkBossPhase(int targetPhase, double time){
	auto ui = GameManager::sharedState()->getEditorLayer()->m_editorUI;
	if (curBossPhase!=targetPhase){
		if (std::abs(targetPhase-curBossPhase)>1){
			if (targetPhase>curBossPhase){
				checkBossPhase(targetPhase-1,time-0.5);
			}else{
				checkBossPhase(targetPhase+1,time-0.5);
			}
		}
		if (std::abs(targetPhase-curBossPhase)==1){
			double objX = ((noteOffset*grid)+(time/1*(grid/timepergrid)))+Xpos;
			double objY = ((2*grid)+Ypos);
			CCPoint pos = {(float)objX,(float)objY};
			auto obj = ui->createObject(spawnID,pos);
			auto trigger = static_cast<SpawnTriggerGameObject*>(obj);
			if (trigger){
				trigger->m_targetGroupID = getBossPhaseSpawn(curBossPhase,targetPhase);
			}
			curBossPhase = targetPhase;
			return;
		}
		return;
	}
}

void addNoteMD(LevelEditorLayer* editor,matjson::Value data = nullptr, notestruct mdmnote = notestruct()){
	auto ui = editor->m_editorUI;

	double daX;
	double daY;
	double daDur;
	int daSpeed;
	std::string ibms_id;

	if (data != nullptr){
		if (data["note"]["ibms_id"].isNull()){return;}
		if (data["isLongPressing"].asBool().unwrap()||data["isLongPressEnd"].asBool().unwrap()){return;}

		daX = data["config"]["time"].asDouble().unwrap();
		daY = data["note"]["pathway"].asInt().unwrap();
		daDur = data["config"]["length"].asDouble().unwrap();
		daSpeed = data["note"]["speed"].asInt().unwrap();
		ibms_id = data["note"]["ibms_id"].asString().unwrap();
	}else if(!mdmnote.ibms_id.empty()){
		daX = mdmnote.time;
		daY = mdmnote.pathway%2;
		daDur = mdmnote.duration;
		daSpeed = mdmnote.speed;
		ibms_id = mdmnote.ibms_id;
	}else{
		return;
	}

	// log::info("{}",daX);

	note_data thisNote = getNoteData(ibms_id);

	if (thisNote.ibms_id.empty()) {return;}

	double thisoffset = noteOffset;
	if (thisNote.noteType>=50){// non interactable/event stuff
		thisoffset = 0;
	}

	double objX = ((thisoffset*grid)+(daX/1*(grid/timepergrid)))+Xpos;
	double objY = ((daY*grid)+Ypos);
	CCPoint pos = {(float)objX,(float)objY};
	auto obj = ui->createObject(spawnID,pos);

	if (thisNote.noteType>=50&&thisNote.spawnGroup!=0){// events & effects
		if (thisNote.noteType==62){//flash
			if (ibms_id=="2I"){
				lastFlashTime = daX;
				lastFlashPos = pos;
				flashStart = true;
			}else if (ibms_id=="2J"){
				if (!lastFlashPos.has_value()){
					lastFlashPos = {(float)Xpos,(float)Ypos};
					lastFlashTime = 0.0;
				}
				if (!flashStart){
					lastFlashPos = {(float)pos.x-1,(float)pos.y};
					lastFlashTime = daX;
				}
				auto color = ui->createObject(899,lastFlashPos.value());
				auto colort = static_cast<EffectGameObject*>(color);
				if (colort){
					colort->m_opacity = 1.0f;
					colort->m_targetColor = 12;
					colort->m_duration = daX-lastFlashTime.value();
				}
				lastFlashTime = daX;
				lastFlashPos = pos;
				flashStart = false;
			}else if (ibms_id=="2K"){
				auto color = ui->createObject(899,lastFlashPos.value());
				auto colort = static_cast<EffectGameObject*>(color);
				if (colort){
					colort->m_opacity = 0.0f;
					colort->m_targetColor = 12;
					colort->m_duration = daX-lastFlashTime.value();
				}
				lastFlashTime = daX;
				lastFlashPos = pos;
			}
			ui->deleteObject(obj,true);
			return;
		}
		if (thisNote.noteType==65){
			curBossPhase = getBossPhase(thisNote.spawnGroup);
		}
		auto trigger = static_cast<SpawnTriggerGameObject*>(obj);
		if (trigger){
			trigger->m_targetGroupID = thisNote.spawnGroup;
		}
		return;
	}

	if (thisNote.spawnGroup==369){// boss melee
		objectsData* noteObject = thisNote.objDown;
		findUnusedObject(noteObject,daX,daDur);
		
		int curPathway = 0;
		int curPos = thisNote.pos+(daSpeed-1);
		int curSpeed = thisNote.speed+(daSpeed-1);
		int curEnter = thisNote.enter;

		if (ibms_id=="11"){
			checkBossPhase(0,daX-0.5);
		}else if (ibms_id=="12"){
			checkBossPhase(0,daX+0.2255);
		}
		

		std::vector<ChanceObject> thisRemap = {
			{1,1,noteObject->groups[noteObject->index].group,0},
			{2,2,curPos,0},
			{3,3,noteObject->groups[noteObject->index].center,0},
			// {4,4,curPathway,0},
			// {5,5,(daY==0)?66:67,0},
			// {6,6,(daY==0)?99:97,0},
			{7,7,curEnter,0},
			{17,17,curSpeed,0},
			{41,41,thisNote.sound,0}
		};

		auto trigger = static_cast<SpawnTriggerGameObject*>(obj);
		if (trigger){
			trigger->m_targetGroupID = thisNote.spawnGroup;
			trigger->m_remapObjects = thisRemap;
		}
		return;
	}

	if (thisNote.spawnGroup==317){// boss note
		objectsData* noteObject = (daY==0)?thisNote.objDown:thisNote.objUp;
		findUnusedObject(noteObject,daX,daDur);

		int thisbossphase = getBossPhase(thisNote.bossAction);
		checkBossPhase(thisbossphase,daX);
		
		int curPos = thisNote.pos+(daSpeed-1);
		int curSpeed = thisNote.speed+(daSpeed-1);
		int curEnter = thisNote.enter+((daSpeed+(daY*3)))-1;
		int curPathway = thisNote.bossAction+((daSpeed+(daY*3)))-1;

		std::vector<ChanceObject> thisRemap = {
			{1,1,noteObject->groups[noteObject->index].group,0},
			{2,2,curPos,0},
			{3,3,noteObject->groups[noteObject->index].center,0},
			{4,4,curPathway,0},
			{5,5,(daY==0)?66:67,0},
			{6,6,(daY==0)?99:97,0},
			{7,7,curEnter,0},
			{17,17,curSpeed,0},
			{41,41,thisNote.sound,0}
		};

		auto trigger = static_cast<SpawnTriggerGameObject*>(obj);
		if (trigger){
			trigger->m_targetGroupID = thisNote.spawnGroup;
			trigger->m_remapObjects = thisRemap;
		}
		return;
	}


	if (thisNote.spawnGroup==179){// masher
		objectsData* noteObject = thisNote.objDown;
		findUnusedObject(noteObject,daX,daDur);
		
		int curPathway = (daY==0)?191:192;
		int curPos = thisNote.pos+(daSpeed-1);
		int curSpeed = thisNote.speed+(daSpeed-1);
		int curEnter = (thisNote.enter>0)?thisNote.enter+(daSpeed-1):0;

		std::vector<ChanceObject> thisRemap = {
			{1,1,noteObject->groups[noteObject->index].group,0},
			{2,2,curPos,0},
			{3,3,noteObject->groups[noteObject->index].center,0},
			{4,4,curPathway,0},
			{7,7,curEnter,0},
			{17,17,curSpeed,0},
			{41,41,thisNote.sound,0}
		};

		auto trigger = static_cast<SpawnTriggerGameObject*>(obj);
		if (trigger){
			trigger->m_targetGroupID = thisNote.spawnGroup;
			trigger->m_remapObjects = thisRemap;
		}

		auto itemedit= ui->createObject(itemEditID,pos);

		auto trigger0 = static_cast<ItemTriggerGameObject*>(itemedit);
		if (trigger0){
			trigger0->m_mod1 = daDur*1000;
			trigger0->m_targetItemMode = 2;
			trigger0->m_resultType3 = 2;
			trigger0->m_targetGroupID = noteObject->groups[noteObject->index].center;
		}

		return;
	}

	if (thisNote.spawnGroup==144){// hammer
		objectsData* noteObject = (daY==0)?thisNote.objDown:thisNote.objUp;
		findUnusedObject(noteObject,daX,daDur);
		
		int curPathway = (daY==0)?191:192;
		int curPos = thisNote.pos+(daSpeed-1);
		int curSpeed = thisNote.speed+(daSpeed-1);
		int curEnter = (thisNote.enter>0)?thisNote.enter+(daSpeed-1):0;

		std::vector<ChanceObject> thisRemap = {
			{1,1,noteObject->groups[noteObject->index].group,0},
			{2,2,curPos,0},
			{3,3,noteObject->groups[noteObject->index].center,0},
			{4,4,curPathway,0},
			{5,5,(daY==0)?66:67,0},
			{6,6,(daY==0)?99:97,0},
			{7,7,curEnter,0},
			{17,17,curSpeed,0},
			{41,41,thisNote.sound,0}
		};

		auto trigger = static_cast<SpawnTriggerGameObject*>(obj);
		if (trigger){
			trigger->m_targetGroupID = thisNote.spawnGroup;
			trigger->m_remapObjects = thisRemap;
		}
		return;
	}

	if (thisNote.spawnGroup==104){// hold
		objectsData* noteObject = (daY==0)?thisNote.objDown:thisNote.objUp;
		objectsData* secondObjects = (daY==0)?thisNote.endNoteDown:thisNote.endNoteUp;
		findUnusedObject(noteObject,daX,daDur);
		
		int curPathway = (daY==0)?191:192;
		int curPos = thisNote.pos+(daSpeed-1);
		int curSpeed = thisNote.speed+(daSpeed-1);
		int curEnter = (thisNote.enter>0)?thisNote.enter+(daSpeed-1):0;

		std::vector<ChanceObject> thisRemap = {
			{1,1,noteObject->groups[noteObject->index].group,0},
			{2,2,curPos,0},
			{3,3,noteObject->groups[noteObject->index].center,0},
			{4,4,curPathway,0},
			{5,5,(daY==0)?66:67,0},
			{6,6,(daY==0)?99:97,0},
			{7,7,(daY==0)?32:33,0},
			{8,8,noteObject->groups[noteObject->index].center,0},
			{9,9,secondObjects->groups[noteObject->index].group,0},
			{10,10,secondObjects->groups[noteObject->index].center,0},
			{11,11,(daY==0)?10:11,0},
			{17,17,curSpeed,0},
			{41,41,thisNote.sound,0}
		};

		auto trigger = static_cast<SpawnTriggerGameObject*>(obj);
		if (trigger){
			trigger->m_targetGroupID = thisNote.spawnGroup;
			trigger->m_remapObjects = thisRemap;
		}

		auto itemedit= ui->createObject(itemEditID,pos);

		auto trigger0 = static_cast<ItemTriggerGameObject*>(itemedit);
		if (trigger0){
			trigger0->m_mod1 = daDur*1000;
			trigger0->m_targetItemMode = 2;
			trigger0->m_resultType3 = 2;
			trigger0->m_targetGroupID = noteObject->groups[noteObject->index].center;
		}
		
		double objX2 = ((noteOffset*grid)+((daX+daDur)/1*(grid/timepergrid)))+Xpos;
		double objY2 = ((daY*grid)+Ypos);
		CCPoint pos2 = {(float)objX2,(float)objY2};
		auto obj2 = ui->createObject(spawnID,pos2);

		std::vector<ChanceObject> secondRemap = {
			{1,1,secondObjects->groups[noteObject->index].group,0}
		};

		auto trigger2 = static_cast<SpawnTriggerGameObject*>(obj2);
		if (trigger2){
			trigger2->m_targetGroupID = curSpeed;
			trigger2->m_remapObjects = secondRemap;
		}

		return;
	}

	if (thisNote.spawnGroup==99){// note/heart
		objectsData* noteObject = (daY==0)?thisNote.objDown:thisNote.objUp;
		findUnusedObject(noteObject,daX,daDur);
		
		int curPathway = (daY==0)?191:192;
		int curPos = thisNote.pos+(daSpeed-1);
		int curSpeed = thisNote.speed+(daSpeed-1);
		int curEnter = (thisNote.enter>0)?thisNote.enter+(daSpeed-1):0;

		std::vector<ChanceObject> thisRemap = {
			{1,1,noteObject->groups[noteObject->index].group,0},
			{2,2,curPos,0},
			{3,3,noteObject->groups[noteObject->index].center,0},
			{4,4,curPathway,0},
			{5,5,(daY==0)?90:91,0},
			{6,6,(thisNote.speed==16)?96:(thisNote.speed==9)?97:98},
			{7,7,curEnter,0},
			{17,17,curSpeed,0}
		};

		auto trigger = static_cast<SpawnTriggerGameObject*>(obj);
		if (trigger){
			trigger->m_targetGroupID = thisNote.spawnGroup;
			trigger->m_remapObjects = thisRemap;
		}
		return;
	}

	if (thisNote.spawnGroup==84){// gear
		objectsData* noteObject = (daY==0)?thisNote.objDown:thisNote.objUp;
		findUnusedObject(noteObject,daX,daDur);
		
		int curPathway = (daY==0)?191:192;
		int curPos = thisNote.pos+(daSpeed-1);
		int curSpeed = thisNote.speed+(daSpeed-1);
		int curEnter = (thisNote.enter>0)?thisNote.enter+(daSpeed-1):0;
		int additional = 0;

		if (thisNote.noteType==15){
			if (curBossPhase<1){
				checkBossPhase(1,daX);
			}
			if (curBossPhase==1){
				curEnter = 325+((daSpeed+(daY*3)))-1;
				additional = 337+((daSpeed+(daY*3)))-1;
			}else if (curBossPhase==2){
				curEnter = 331+((daSpeed+(daY*3)))-1;
				additional = 343+((daSpeed+(daY*3)))-1;
			}
			curPathway = 0;
		}

		std::vector<ChanceObject> thisRemap = {
			{1,1,noteObject->groups[noteObject->index].group,0},
			{2,2,curPos,0},
			{3,3,noteObject->groups[noteObject->index].center,0},
			{4,4,curPathway,0},
			{5,5,(daY==0)?81:80,0},
			{6,6,additional,0},
			{7,7,curEnter,0},
			{17,17,curSpeed,0}
		};

		auto trigger = static_cast<SpawnTriggerGameObject*>(obj);
		if (trigger){
			trigger->m_targetGroupID = thisNote.spawnGroup;
			trigger->m_remapObjects = thisRemap;
		}
		return;
	}
	
	if (thisNote.spawnGroup==13){// normal note
		objectsData* noteObject = (daY==0)?thisNote.objDown:thisNote.objUp;
		findUnusedObject(noteObject,daX,daDur);
		
		int curPathway = (daY==0)?191:192;
		int curPos = thisNote.pos+(daSpeed-1);
		int curSpeed = thisNote.speed+(daSpeed-1);
		int curEnter = (thisNote.enter>0)?thisNote.enter+(daSpeed-1):0;

		if (thisNote.noteType==6){
			curPathway = 0;
			curEnter = thisNote.enter+((daSpeed+(daY*3)))-1;
		}

		std::vector<ChanceObject> thisRemap = {
			{1,1,noteObject->groups[noteObject->index].group,0},
			{2,2,curPos,0},
			{3,3,noteObject->groups[noteObject->index].center,0},
			{4,4,curPathway,0},
			{5,5,(daY==0)?66:67,0},
			{6,6,(daY==0)?99:97,0},
			{7,7,curEnter,0},
			{17,17,curSpeed,0},
			{41,41,thisNote.sound,0}
		};

		auto trigger = static_cast<SpawnTriggerGameObject*>(obj);
		if (trigger){
			trigger->m_targetGroupID = thisNote.spawnGroup;
			trigger->m_remapObjects = thisRemap;
		}
		return;
	}

	ui->deleteObject(obj,true);
}

int MDchart(LevelEditorLayer* editor, matjson::Value data = nullptr, head mdmdata = head()){
	if (!editor){
		log::warn("editor gone??");
		return 1;
	}
    grid = 30;
    timepergrid = 0.09628343;
    Ypos = 5025.0;
    Xpos = 0.0;
    noteOffset = (1.451/0.09628343)*-1;
    daTime = 0;
	std::string curscene;
	if (data!=nullptr){
		bpm = data["bpm"].asDouble().unwrap();
		curscene = data["scene"].asString().unwrap();
	}else if (!mdmdata.notes.empty()){
		bpm = mdmdata.bpm;
		curscene = mdmdata.stage;
	}else{
		log::warn("none");
		return 1;
	}
	overflow = false;
	overflowObjects.clear();
	resetObjectsData();
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
	}
	setInitScene(curscene);
	if (data!=nullptr){
		for (const auto& [a, b]:data["notes"].asArray
			().unwrap()){
			addNoteMD(editor,b,notestruct{});
		}
	}else if (!mdmdata.notes.empty()){
		for (const auto& b : mdmdata.notes){
			addNoteMD(editor,nullptr,b);
		}
	}
	auto root = CCDirector::sharedDirector()->getRunningScene();
	root->removeChildByID("mdPopup");
	root->removeChildByID("mdmPopup");

	if (overflow){
		std::string message = "This chart uses too many notes in under 2 seconds threshold. You may notice some notes behaving unexpectedly. Overflow object variants:";
		for (int i=0;i<overflowObjects.size();i++){
			message = message + " " + std::to_string(overflowObjects[i]);
		}
		message = message+".";
		FLAlertLayer::create("Notes overflow", message, "Oh :(")->show();
	}
	
	notif("Done","GJ_completesIcon_001.png");
    return 0;
}