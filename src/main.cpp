#include <Geode/Geode.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <cstdio>
#include <regex>
#include <Geode/binding/SetupTriggerPopup.hpp>
#include <math.h>
#include "method/fnf.hpp"
#include "method/md.hpp"
#include "method/utills.hpp"

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

matjson::Value thefilepaths;

bool checkFile(std::filesystem::path thepath){
	std::ifstream file(thepath);
	if (!file.good()){
		return false;
	}
	// log::info("{} {}",thepath.filename(),thepath.filename()=="mdextract.exe");
	return thepath.filename()=="mdextract.exe";
}

std::string convertQuotes(const std::string& thestring){
	std::string converted;
	converted.reserve(thestring.size());
	for (char c:thestring){
		if (c == '"') {converted += "\\\"";}
        else {converted += c;};
    }
    return converted;
}

std::string nameOnly(const std::string& input) {
    std::string result = input;
    std::replace(result.begin(), result.end(), '_', ' ');
    result = std::regex_replace(result, std::regex("\\s+$"), "");
    return result;
}

using extractTask = Task<std::string, std::monostate>;

extractTask runMDextract(const std::string& thecmd){
	return extractTask::run([thecmd](auto progress, auto hasbeencancelled)->extractTask::Result{
		FILE* pipe = _popen(thecmd.c_str(),"r");
		if (!pipe){
			log::info("something happened (error)");
			return extractTask::Cancel();
		}
		log::info("pipe starts");
		char buffer[sizeof(int)];
		std::string test;
		while (fgets(buffer, sizeof(buffer), pipe)) {
			// log::info("{}",buffer);
            test += buffer;
        }
		_pclose(pipe);
		log::info("pipe finished");
		// log::info("{} {}",test.size(),test.c_str());
		return test;
	}, "run");
}

int getmapNumber(const std::string& s) {
    std::regex re("(\\d+)$");
    std::smatch match;
    if (std::regex_search(s, match, re)) {
        return std::stoi(match.str(1));
    }
    return 0;
}

const char * getfaces(int num){
	switch (num)
	{
	case 1: return "diffIcon_01_btn_001.png";
	case 2: return "diffIcon_03_btn_001.png";
	case 3: return "diffIcon_05_btn_001.png";
	default:return "diffIcon_00_btn_001.png";
	}
}

class $modify(editedPauseLayer,EditorPauseLayer) {

	struct Fields {
		EventListener<extractTask> m_extractListener;
		
	};
	
	class optionPopup : public geode::Popup<std::string const&>{//taken from docs
	protected:
		bool setup(std::string const& value) override{
			this->setTitle("Import Chart");
			auto menu = CCMenu::create();
			auto defMenu = this->m_closeBtn->getParent();

			auto sAxis = SimpleAxisLayout::create(geode::Axis::Column);
			sAxis->setMainAxisDirection(AxisDirection::TopToBottom);
			sAxis->setGap(5.0f);

			auto baseButtonSprite = CCSprite::createWithSpriteFrameName("GJ_button_01.png");
			auto infoSprite = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");

			auto fnfButton = ButtonSprite::create("FNF chart (.json)",0.70f);
			auto mdButton = ButtonSprite::create("MD chart (.bundle)",0.70f);
			auto mdjsonButton = ButtonSprite::create("MD chart (.json)",0.70f);
			auto mdmButton = ButtonSprite::create("MDM chart (.mdm)",0.70f);

			auto daFnf = CCMenuItemSpriteExtra::create(fnfButton,menu,menu_selector(editedPauseLayer::doFnf));
			auto daMd = CCMenuItemSpriteExtra::create(mdButton,menu,menu_selector(editedPauseLayer::dotheMD));
			auto daMdjson = CCMenuItemSpriteExtra::create(mdjsonButton,menu,menu_selector(editedPauseLayer::dotheMD));
			auto daMdm = CCMenuItemSpriteExtra::create(mdmButton,menu,menu_selector(editedPauseLayer::doFnf));
			auto infoButton = InfoAlertButton::create("Help","<cl>MDExtract</c> is required to import <cp>Muse Dash</c> chart. Also owning <cp>Muse Dash</c> is required (to get Unity .bundle file, which is where the chart was saved). You can download the tool from mod desc, then save its directory in the mod settings.",1.0f);

			daFnf->setID("fnf"_spr);
			daMd->setID("md"_spr);
			daMdjson->setID("mdjson"_spr);
			daMdm->setID("mdm"_spr);

			menu->addChild(daFnf,0,-1);
			menu->addChild(daMd,0,-1);
			menu->addChild(daMdjson,0,-1);
			menu->addChild(daMdm,0,-1);
			defMenu->addChild(infoButton);

			CCPoint pos = {220.0f,140.0f};
			infoButton->setPosition(pos);

			CCPoint offset = {0.0f,-15.0};

			m_mainLayer->addChildAtPosition(menu,Anchor::Center,offset);

			menu->ignoreAnchorPointForPosition(false);
			menu->setContentWidth(240.0f);
			menu->setContentHeight(100.0f);

			sAxis->apply(menu);

			ccColor3B grey = {127,127,127};
			daMdm->setEnabled(false);
			mdmButton->setCascadeColorEnabled(true);
			mdmButton->setColor(grey);

			auto file = Mod::get()->getSettingValue<std::filesystem::path>("md-extractor-path");
			if (!checkFile(file)){
				
				daMd->setEnabled(false);
				mdButton->setCascadeColorEnabled(true);
				mdButton->setColor(grey);
			}

			return true;
		}
	public:
		static optionPopup* create(){
			auto ret = new optionPopup();
			if (ret->initAnchored(240.f, 160.f,"w")) {
				ret->autorelease();
				return ret;
			}

			delete ret;
			return nullptr;
		}
	};

	class mdChartsPopup : public geode::Popup<matjson::Value const&>{

	protected:
		bool setup(matjson::Value const& filepaths) override{
			thefilepaths = filepaths;

			this->setTitle("Select Difficulty");
			auto menu = CCMenu::create();
			auto defMenu = this->m_closeBtn->getParent();

			int selectedIndex = -1;
			CCSize size = {360.0f,180.0f};
			std::array<matjson::Value, 20> thedata;
			auto scrollable = geode::ScrollLayer::create(size,true,true);

			auto baseButtonSprite = CCSprite::createWithSpriteFrameName("GJ_button_01.png");
			auto bgsprite = CCSprite::create("square02b_001.png");
			auto diffNotes = CCArray::create();

			ccColor3B black = {0,0,0};
			GLubyte op = 70;

			auto bg = CCScale9Sprite::create("square02b_001.png");
			bg->setContentSize(size);
			bg->setColor(black);
			bg->setOpacity(op);

			int i = 0;

			for (auto const& pathValue : filepaths.asArray().unwrap()){
				if (std::filesystem::path(pathValue.asString().unwrap()).extension()==".json"){
					auto file = geode::utils::file::readJson(pathValue.asString().unwrap()).unwrap();
					std::string mapname = file["mapName"].asString().unwrap();
					int sumnotes = file["notes"].size();
					int diff = getmapNumber(mapname);
					auto face = CCSprite::createWithSpriteFrameName(getfaces(diff));
					auto damenu = CCMenu::create();
					auto btn = CCMenuItemSpriteExtra::create(face, this, menu_selector(editedPauseLayer::doMd));
					btn->setTag(i);
					CCPoint anchor0 = {0.0,0.0};
					damenu->setAnchorPoint(anchor0);
					damenu->ignoreAnchorPointForPosition(false);
					damenu->setScale(0.75);
					damenu->addChild(btn,i,i);
					
					auto cell = CCNode::create();
					cell->setContentWidth(360.0f);
					auto title = CCLabelBMFont::create(nameOnly(mapname).c_str(),"bigFont.fnt",1000,CCTextAlignment::kCCTextAlignmentLeft);
					auto noteCount = CCLabelBMFont::create((std::to_string(sumnotes)+" notes").c_str(),"bigFont.fnt",1000,CCTextAlignment::kCCTextAlignmentLeft);
					CCPoint anchor = {0.0,0.5};
					title->setAnchorPoint(anchor);
					title->setScale(0.5f);
					noteCount->setAnchorPoint(anchor);
					noteCount->setScale(0.3f);
					
					cell->addChildAtPosition(noteCount,Anchor::Left,CCPoint(0.0,-5.0));
					cell->addChildAtPosition(title,Anchor::Left,CCPoint(0.0,5.0));
					CCPoint offset = {-15.0,0.0};
					cell->addChildAtPosition(damenu,Anchor::Right,offset);
					scrollable->m_contentLayer->addChild(cell,diff,i);
					diffNotes->addObject(cell);
					i++;
				}
			}

			auto sAxis = SimpleAxisLayout::create(geode::Axis::Column);
			sAxis->setMainAxisDirection(AxisDirection::TopToBottom);
			sAxis->setGap(30.0f);
			auto infoSprite = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
			auto createButton = ButtonSprite::create("Create",0.70f);

			auto infoButton = InfoAlertButton::create("Help","One song can have 1 to 6 (maybe more) difficulties. The differences are amounts of notes and effects. <co>map</c> higher than 3 is usually a hidden chart",1.0f);
			defMenu->addChild(infoButton);

			CCPoint pos = {380.0f,230.0f};
			infoButton->setPosition(pos);

			CCPoint offset = {0.0f,-10.0};
			m_mainLayer->addChildAtPosition(bg,Anchor::Center,offset);
			m_mainLayer->addChildAtPosition(scrollable,Anchor::Center,offset);

			scrollable->ignoreAnchorPointForPosition(false);

			scrollable->m_contentLayer->setLayout(sAxis);
			sAxis->apply(scrollable->m_contentLayer);

			return true;
		}
	public:
		static mdChartsPopup* create(matjson::Value the){
			auto ret = new mdChartsPopup();
			if (ret->initAnchored(400.f, 250.f,the)) {
				ret->autorelease();
				return ret;
			}

			delete ret;
			return nullptr;
		}
	};

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
			sprite, actionsButtons, menu_selector(editedPauseLayer::initImportChartPopup)
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

	void initImportChartPopup(CCObject*){
		auto popup = optionPopup::create();
		popup->m_noElasticity = true;
		popup->setID("importChartPopup");
		popup->show();
	}

	void initMDPopup(matjson::Value thedata){
		auto popup = mdChartsPopup::create(thedata);
		popup->m_noElasticity = true;
		popup->setID("mdPopup");
		popup->show();
	}

	void doMd(CCObject* sender){
		auto obj = static_cast<editedPauseLayer*>(sender);
		// log::info("{} {}",obj->getTag(),thefilepaths.dump());
		auto content = geode::utils::file::readJson( std::filesystem::path( thefilepaths.asArray().unwrap()[obj->getTag()].asString().unwrap() ) ).unwrap();
		MDchart(GameManager::sharedState()->getEditorLayer(),content);
	}

	void doMdjson(CCObject*){
		utils::file::pick(utils::file::PickMode::OpenFile,{}).listen([this](geode::Result<std::filesystem::path, std::string>* file){
			if (!file){
				log::info("nvm");
				return;
			}
			auto filePath = file->unwrap();
			std::string ext = (filePath.has_extension())?filePath.extension().string() : "";
			// log::info("{}",ext);
			auto editor = GameManager::sharedState()->getEditorLayer();
			if (ext == ".json"){
				MDchart(editor,utils::file::readJson(filePath).unwrap());
			} else {
				FLAlertLayer::create("Invalid File Type", "This file isnt supported.", "OK")->show();
				log::warn("this aint supported");
				return;
			}
			return;
		});
	}

	void doFnf(CCObject*){
		utils::file::pick(utils::file::PickMode::OpenFile,{}).listen([this](geode::Result<std::filesystem::path, std::string>* file){
			if (!file){
				log::info("nvm");
				return;
			}
			auto filePath = file->unwrap();
			std::string ext = (filePath.has_extension())?filePath.extension().string() : "";
			// log::info("{}",ext);
			auto editor = GameManager::sharedState()->getEditorLayer();
			if (ext == ".json"){
				fnfChart(editor,utils::file::readJson(filePath).unwrap());
			} else {
				FLAlertLayer::create("Invalid File Type", "This file isnt supported.", "OK")->show();
				log::warn("this aint supported");
				return;
			}
			return;
		});
	}

	void dotheMD(CCObject* obj){
		utils::file::pick(utils::file::PickMode::OpenFile,{}).listen([this](geode::Result<std::filesystem::path, std::string>* file){
			if (!file){
				log::info("nvm");
				return;
			}
			auto filePath = file->unwrap();
			std::string ext = (filePath.has_extension())?filePath.extension().string() : "";
			// log::info("{}",ext);
			auto editor = GameManager::sharedState()->getEditorLayer();
			if (ext == ".bundle"){
				std::string daPath = filePath.string();
				std::string test;
				std::string mdextractPath = convertQuotes(Mod::get()->getSettingValue<std::filesystem::path>("md-extractor-path").string());
				std::string targetfolder = Mod::get()->getConfigDir().string();
				std::replace(daPath.begin(),daPath.end(),' ','#');
				std::replace(targetfolder.begin(),targetfolder.end(),' ','#');
				std::string cmd = "\""+mdextractPath+"\""+ " -pf "+daPath+ " -tf " +targetfolder;
				// log::info("{}",cmd);
				notif("Running extractor","GJ_infoIcon_001.png");
				m_fields->m_extractListener.bind([this](extractTask::Event* ev){
					if (ev->getValue()){
						log::info("extract finished");
						std::string res = *ev->getValue();
						auto thejson = matjson::parse(res);
						if (thejson.isErr()){
							log::warn("something happened on mdextract output, most likely an error output: {}",res);
							notif(res,"GJ_deleteIcon_001.png",5);
							return;
						}
						auto root = CCDirector::sharedDirector()->getRunningScene();
						root->removeChildByID("importChartPopup");
						initMDPopup(thejson.unwrap());
					}else if (ev->isCancelled()){
						log::warn("extract cancelled");
					}
				});
				m_fields->m_extractListener.setFilter(runMDextract(cmd));

			} else {
				FLAlertLayer::create("Invalid File Type", "This file isnt supported.", "OK")->show();
				log::warn("this aint supported");
				return;
			}
			return;
		});
	}
};