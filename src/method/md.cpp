#include <Geode/Geode.hpp>
#include "md.hpp"
#include "mainVar.hpp"
#include "uid.hpp"
#include "objectCache.hpp"
#include <string>

using namespace geode::prelude;

bool overflow = false;
std::vector<int> overflowObjects;

std::string convertStageType(std::string originalUid){
	std::string newuid = originalUid;
	return newuid = "99" + newuid.substr(2,4);
}

note_data getNoteData(std::string daUid){
	std::string thisUid = daUid;
	for (const auto& a : note_uids){// for specific stage/boss bcs each ofthem has different bms code
		if (a.note_uid==thisUid){
			return a;
		}
	}
	if (thisUid.substr(0,2)=="00"){return{};}// prevents universal (00)id to be checked
	thisUid = convertStageType(thisUid);
	for (const auto& a : note_uids){
		if (a.note_uid==thisUid){
			return a;
		}
	}
	return {};
}

void addNoteMD(LevelEditorLayer* editor,matjson::Value data){
	auto ui = editor->m_editorUI;

	if (data["Config"]["note_uid"].isNull()){return;}
	if (data["isLongPressing"].asBool().unwrap()||data["isLongPressEnd"].asBool().unwrap()){return;}

	double daX = data["Config"]["time"].asDouble().unwrap();
	double daY = (data["Config"]["pathway"].asInt().unwrap()==0)?0:1;
	double daDur = data["Config"]["length"].asDouble().unwrap();
	std::string suid = data["Config"]["note_uid"].asString().unwrap();

	note_data thisNote = getNoteData(suid);
	objectsData* noteObject = thisNote.useObjects;

	if (thisNote.note_uid.empty()) {return;}

	double offset = 0;
	double objX = ((noteOffset*grid)+(daX/1*(grid/timepergrid)))+Xpos;
	double objY = (((daY+offset)*grid)+Ypos);
	CCPoint pos = {(float)objX,(float)objY};
	auto obj = ui->createObject(spawnID,pos);

	bool found = false;

	for (int i = 0;i<noteObject->groups.size();i++){
		if (!noteObject->groups[i].lastUsed||
			((daX)-(noteObject->groups[i].lastUsed)>2.1&&
			(daX)-(noteObject->groups[i].lastUsed+noteObject->groups[i].dur)>2.1)){
			noteObject->index = i;
			noteObject->groups[i].lastUsed = daX;
			noteObject->groups[i].dur = daDur;
			found = true;
			break;
		}
	}

	if (!found){
		overflow = true;
		bool foundSimilar = false;
		for (int i=0;i<overflowObjects.size();i++){
			if (overflowObjects[i]==noteObject->groups[0].group){
				foundSimilar = true;
				break;
			}
		}
		if (!foundSimilar){
			overflowObjects.push_back(noteObject->groups[0].group);
		}
	}

	if (thisNote.noteType==4){// normal masher and boss masher for now
		std::vector<ChanceObject> thisRemap = {
			{1,1,noteObject->groups[noteObject->index].group,0},
			{2,2,thisNote.pos,0},
			{3,3,noteObject->groups[noteObject->index].center,0},
			//{5,5,(daY==0)?66:67,0},
			//{6,6,(daY==0)?99:97,0},
			{7,7,thisNote.enter,0},
			{17,17,thisNote.speed,0},
			{41,41,thisNote.sound,0}
		};

		auto trigger = dynamic_cast<SpawnTriggerGameObject*>(obj);
		if (trigger){
			trigger->m_targetGroupID = thisNote.spawnGroup;
			trigger->m_remapObjects = thisRemap;
		}

		auto itemedit= ui->createObject(itemEditID,pos);

		auto trigger0 = dynamic_cast<ItemTriggerGameObject*>(itemedit);
		if (trigger0){
			trigger0->m_mod1 = daDur*1000;
			trigger0->m_targetItemMode = 2;
			trigger0->m_resultType3 = 2;
			trigger0->m_targetGroupID = noteObject->groups[noteObject->index].center;
		}
		return;
	}

	if (thisNote.noteType==16){// raider
		std::vector<ChanceObject> thisRemap = {
			{1,1,noteObject->groups[noteObject->index].group,0},
			{2,2,thisNote.pos,0},
			{3,3,noteObject->groups[noteObject->index].center,0},
			{5,5,(daY==0)?66:67,0},
			{6,6,(daY==0)?99:97,0},
			{7,7,thisNote.enter,0},
			{17,17,thisNote.speed,0},
			{41,41,thisNote.sound,0}
		};

		auto trigger = dynamic_cast<SpawnTriggerGameObject*>(obj);
		if (trigger){
			trigger->m_targetGroupID = thisNote.spawnGroup;
			trigger->m_remapObjects = thisRemap;
		}
		return;
	}

	if (thisNote.noteType==15){// hammer
		std::vector<ChanceObject> thatRemap = {
			{1,1,noteObject->groups[noteObject->index].group,0},
			{2,2,thisNote.pos,0},
			{3,3,noteObject->groups[noteObject->index].center,0},
			{5,5,(daY==0)?66:67,0},
			{6,6,(daY==0)?99:97,0},
			{7,7,thisNote.enter,0},
			{17,17,thisNote.speed,0},
			{41,41,thisNote.sound,0}
		};

		auto trigger2 = dynamic_cast<SpawnTriggerGameObject*>(obj);
		if (trigger2){
			trigger2->m_targetGroupID = thisNote.spawnGroup;
			trigger2->m_remapObjects = thatRemap;
		}

		return;
	}

	if (thisNote.noteType==2){// hold note

		objectsData* secondObjects = thisNote.endNote;

		std::vector<ChanceObject> anotherRemap = {
			{1,1,noteObject->groups[noteObject->index].group,0},
			{2,2,thisNote.pos,0},
			{3,3,noteObject->groups[noteObject->index].center,0},
			{4,4,noteObject->groups[noteObject->index].group,0},
			{5,5,(daY==0)?66:67,0},
			{6,6,(daY==0)?99:97,0},
			{7,7,(daY==0)?10:11,0},
			{8,8,noteObject->groups[noteObject->index].center,0},
			{9,9,secondObjects->groups[noteObject->index].group,0},
			{10,10,secondObjects->groups[noteObject->index].center,0},
			{17,17,thisNote.speed,0},
			{41,41,thisNote.sound,0}
		};

		auto trigger3 = dynamic_cast<SpawnTriggerGameObject*>(obj);
		if (trigger3){
			trigger3->m_targetGroupID = thisNote.spawnGroup;
			trigger3->m_remapObjects = anotherRemap;
		}

		auto itemedit= ui->createObject(itemEditID,pos);

		auto trigger0 = dynamic_cast<ItemTriggerGameObject*>(itemedit);
		if (trigger0){
			trigger0->m_mod1 = daDur*1000;
			trigger0->m_targetItemMode = 2;
			trigger0->m_resultType3 = 2;
			trigger0->m_targetGroupID = noteObject->groups[noteObject->index].center;
		}
		
		double offset2 = 0;
		double objX2 = ((noteOffset*grid)+((daX+daDur)/1*(grid/timepergrid)))+Xpos;
		double objY2 = (((daY+offset2)*grid)+Ypos);
		CCPoint pos2 = {(float)objX2,(float)objY2};
		auto obj2 = ui->createObject(spawnID,pos2);

		std::vector<ChanceObject> secondRemap = {
			{1,1,secondObjects->groups[noteObject->index].group,0}
		};

		auto trigger2 = dynamic_cast<SpawnTriggerGameObject*>(obj2);
		if (trigger2){
			trigger2->m_targetGroupID = thisNote.speed;
			trigger2->m_remapObjects = secondRemap;
		}
		return;
	}

	if (thisNote.noteType==-2){// heart
		std::vector<ChanceObject> thatRemap = {
			{1,1,noteObject->groups[noteObject->index].group,0},
			{2,2,thisNote.pos,0},
			{3,3,noteObject->groups[noteObject->index].center,0},
			{4,4,noteObject->groups[noteObject->index].group,0},
			{5,5,(daY==0)?90:91,0},
			{6,6,(thisNote.speed==16)?96:(thisNote.speed==9)?97:98},
			{7,7,thisNote.enter,0},
			{17,17,thisNote.speed,0}
		};

		auto trigger2 = dynamic_cast<SpawnTriggerGameObject*>(obj);
		if (trigger2){
			trigger2->m_targetGroupID = thisNote.spawnGroup;
			trigger2->m_remapObjects = thatRemap;
		}

		return;
	}

	if (thisNote.noteType==-3){// note
		std::vector<ChanceObject> thatRemap = {
			{1,1,noteObject->groups[noteObject->index].group,0},
			{2,2,thisNote.pos,0},
			{3,3,noteObject->groups[noteObject->index].center,0},
			{4,4,noteObject->groups[noteObject->index].group,0},
			{5,5,(daY==0)?90:91,0},
			{6,6,(thisNote.speed==16)?96:(thisNote.speed==9)?97:98},
			{7,7,thisNote.enter,0},
			{17,17,thisNote.speed,0}
		};

		auto trigger2 = dynamic_cast<SpawnTriggerGameObject*>(obj);
		if (trigger2){
			trigger2->m_targetGroupID = thisNote.spawnGroup;
			trigger2->m_remapObjects = thatRemap;
		}

		return;
	}

	if (thisNote.noteType==3){// normal gear
		std::vector<ChanceObject> thatRemap = {
			{1,1,noteObject->groups[noteObject->index].group,0},
			{2,2,thisNote.pos,0},
			{3,3,noteObject->groups[noteObject->index].center,0},
			{4,4,noteObject->groups[noteObject->index].group,0},
			{5,5,(daY==0)?81:80,0},
			{7,7,thisNote.enter,0},
			{17,17,thisNote.speed,0}
		};

		auto trigger2 = dynamic_cast<SpawnTriggerGameObject*>(obj);
		if (trigger2){
			trigger2->m_targetGroupID = thisNote.spawnGroup;
			trigger2->m_remapObjects = thatRemap;
		}

		return;
	}
	// normal note
	std::vector<ChanceObject> thisRemap = {
		{1,1,noteObject->groups[noteObject->index].group,0},
		{2,2,thisNote.pos,0},
		{3,3,noteObject->groups[noteObject->index].center,0},
		{4,4,noteObject->groups[noteObject->index].group,0},
		{5,5,(daY==0)?66:67,0},
		{6,6,(daY==0)?99:97,0},
		{7,7,thisNote.enter,0},
		{17,17,thisNote.speed,0},
		{41,41,thisNote.sound,0}
	};

	auto trigger = dynamic_cast<SpawnTriggerGameObject*>(obj);
	if (trigger){
		trigger->m_targetGroupID = thisNote.spawnGroup;
		trigger->m_remapObjects = thisRemap;
	}

}

int doMD(LevelEditorLayer* editor, matjson::Value data){
    if (!editor){
		log::warn("editor gone??");
		return 0;
	}
    grid = 30;
    timepergrid = 0.09628343;
    Ypos = 5025.0;
    Xpos = 0.0;
    noteOffset = (1.451/0.09628343)*-1;
    spawnID = 1268;
    pickupID = 1817;
    areaScaleID = 3008;
    itemEditID = 3619;
    daTime = 0;
    bpm = 120;
    scroll = 1.0;
    beat = 0;
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
	for (const auto& [a, b]:data){
		addNoteMD(editor,b);
	}
	if (overflow){
		std::string message = "This chart uses too many notes in under 2 seconds threshold. You may notice some notes behaving unexpectedly. Overflow object variants:";
		for (int i=0;i<overflowObjects.size();i++){
			message = message + " " + std::to_string(overflowObjects[i]);
		}
		message = message+".";
		FLAlertLayer::create("Notes overflow", message, "Oh :(")->show();
	}
    return 0;
}