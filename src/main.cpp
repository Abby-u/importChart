#include <Geode/Geode.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include <fstream>
#include <iostream>
#include <Geode/binding/SetupTriggerPopup.hpp>
#include <math.h>
#include "method/fnf.hpp"
#include "method/md.hpp"

using namespace geode::prelude;

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

void initiateInterface(LevelEditorLayer* editor){// not finished yet
	auto root = CCDirector::sharedDirector()->getRunningScene();
	auto screenDmnsn = CCDirector::sharedDirector()->getWinSize();

	auto daMenu = CCMenu::create();
	daMenu->setID("menu-container"_spr);
	daMenu->setPosition(screenDmnsn*0);
	daMenu->setTouchPriority(-504);
	root->addChild(daMenu,106);

	auto blacktransp = CCLayerColor::create(ccc4(0,0,0,128));
	blacktransp->setID("bg"_spr);
	daMenu->addChild(blacktransp,-10);

	auto daLayer = CCLayer::create();
	daLayer->setID("layer"_spr);
	daMenu->addChild(daLayer);
	
}

class $modify(editedPauseLayer,EditorPauseLayer) {
	bool init(LevelEditorLayer* p0) {
		if (!EditorPauseLayer::init(p0)) {
			return false;
		}
		
		auto actionsButtons = this->getChildByID("actions-menu");
		// @geode-ignore(unknown-resource)
		auto topSprite = CCSprite::createWithSpriteFrameName("GJ_button_04.png");
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
		auto sprite2 = ButtonSprite::create(
			"View Raw Remap",
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
		auto remapButton = CCMenuItemSpriteExtra::create(
			sprite2, actionsButtons, menu_selector(editedPauseLayer::showSpawnRemap)
		);

		importButton->setID("import-chart"_spr);
		actionsButtons->addChild(importButton,0,-1);

		remapButton->setID("spawn-remap"_spr);
		actionsButtons->addChild(remapButton,0,-1);

		actionsButtons->updateLayout(true);

		return true;
	}

	void showSpawnRemap(CCObject*){
		auto editor = GameManager::sharedState()->getEditorLayer()->m_editorUI;
		auto objects = editor->getSelectedObjects();
		if (objects->count()<0){return;}
		for (int i=0;i<objects->count();i++){
			auto object = static_cast<GameObject*>(objects->objectAtIndex(i));
			if (object->m_objectID==1268){
				auto remaps = static_cast<SpawnTriggerGameObject*>(object)->m_remapObjects;
				auto keys = static_cast<SpawnTriggerGameObject*>(object)->m_remapKeys;
				for (auto thej = remaps.begin(); thej != remaps.end(); ++thej){
					log::info("object remaps: {} {} {} {}",thej->m_groupID,thej->m_oldGroupID,thej->m_chance,thej->m_unk00c);
				}
				for (auto thej2 = keys.begin(); thej2 != keys.end(); ++thej2){
					log::info("key remaps: {}",*thej2);
				}
			}
			log::info(" ");
		}
	}

	void buttonPress(CCObject*){
		utils::file::pick(utils::file::PickMode::OpenFile,{}).listen([this](geode::Result<std::filesystem::path, std::string>* file){
		if (!file){
			log::info("nvm");
			return;
		}
		auto filePath = file->unwrap();
		std::string ext = (filePath.has_extension())?filePath.extension().string() : "";
		log::info("{}",ext);
		auto editor = GameManager::sharedState()->getEditorLayer();
		if (ext == ".json"){
			//fnfChart(editor,utils::file::readJson(filePath).unwrap());
			doMD(editor,utils::file::readJson(filePath).unwrap());
		} else {
			FLAlertLayer::create("Invalid File Type", "This file isnt supported. (ig)", "OK")->show();
			log::warn("this aint supported");
			return;
		}
		// initiateInterface(editor);
		});
	}
};