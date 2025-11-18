#include "fnf.hpp"
#include "utills.hpp"
#include <Geode/Geode.hpp>
#include "mainVar.hpp"

using namespace geode::prelude;

int leftG = 50;
int downG = 51;
int upG = 52;
int rightG = 53;

int leftHold = 300;
int downHold = 301;
int upHold = 302;
int rightHold = 303;

int leftType = 308;
int downType = 309;
int upType = 310;
int rightType = 311;

int dadleftG = 700;
int daddownG = 710;
int dadupG = 720;
int dadrightG = 730;

int dadleftHold = 304;
int daddownHold = 305;
int dadupHold = 306;
int dadrightHold = 307;

int dadleftType = 312;
int daddownType = 313;
int dadupType = 314;
int dadrightType = 315;

int bfSideG = 27;
int dadSideG = 28;

float lerp(float a, float b,float c){
	return a+(b-a)*c;
}

float lastdt=0;
bool justplayed=false;

FMOD::Sound* inst;
FMOD::Channel* instchannel;
FMOD::Sound* voices;
FMOD::Channel* voiceschannel;

CCLabelBMFont* instName;
CCLabelBMFont* voicesName;

struct singleEvent{
	std::string etype;
	std::string param1;
	std::string param2;
};

struct fnfevent{
	double time;
	std::vector<singleEvent> daEvents;
};

struct fnfnote{
	double time;
	int dir;
	double dur;
	std::string ntype;
};

struct section{
	double time;
	double endtime;
	int stype;
	std::vector<fnfnote> notes;
	std::vector<fnfevent> events;
	int sectionSize;
	bool alt;
	bool gf;
	double bpm;
	bool changeBpm;
	bool mustHit;
};

struct songData{
	std::string player1;
	std::string gfVersion;
	std::string player2;
	std::string song;
	double bpm;
	double speed;
	std::vector<section> sections;
};

songData thissong;
const std::string infotext = "unfinished\n\ntodo:\nselector & viewer for notes & events\nrewrite importing method\ncomplex ui for spawn remap\nfinish other tabs\nqol stuff";

bool validatePsychChart(matjson::Value data){
	if (data.contains("song")&&data["song"].isString()){
		if (data.contains("notes") && data.contains("speed") && data.contains("bpm")){
			return true;
		}
	}else if(data.contains("song")&&!data["song"].isString()){
		if (data["song"].contains("notes") && data["song"].contains("speed") && data["song"].contains("bpm")){
			return true;
		}
	}
	return false;
}

std::string tryGetString(std::string target,matjson::Value *value){
	if (!value->contains(target) || value->isNull()){
		return "";
	}
	auto &v = (*value)[target];
	return (v.isString())?v.asString().unwrap():"";
}
std::string tryGetString(std::string target,const matjson::Value *value){
	if (!value->contains(target) || value->isNull()){
		return "";
	}
	auto &v = (*value)[target];
	return (v.isString())?v.asString().unwrap():"";
}
double tryGetDouble(std::string target,matjson::Value *value){
	if (!value->contains(target) || value->isNull()){
		return 0;
	}
	auto &v = (*value)[target];
	return (v.isNumber())?v.asDouble().unwrap():0;
}
double tryGetDouble(std::string target,const matjson::Value *value){
	if (!value->contains(target) || value->isNull()){
		return 0;
	}
	auto &v = (*value)[target];
	return (v.isNumber())?v.asDouble().unwrap():0;
}
int tryGetInt(std::string target,matjson::Value *value){
	if (!value->contains(target) || value->isNull()){
		return 0;
	}
	auto &v = (*value)[target];
	return (v.isNumber())?v.asInt().unwrap():0;
}
int tryGetInt(std::string target,const matjson::Value *value){
	if (!value->contains(target) || value->isNull()){
		return 0;
	}
	auto &v = (*value)[target];
	return (v.isNumber())?v.asInt().unwrap():0;
}
bool tryGetBool(std::string target,matjson::Value *value){
	if (!value->contains(target) || value->isNull()){
		return false;
	}
	auto &v = (*value)[target];
	return (v.isBool())?v.asBool().unwrap():false;
}
bool tryGetBool(std::string target,const matjson::Value *value){
	if (!value->contains(target) || value->isNull()){
		return false;
	}
	auto &v = (*value)[target];
	return (v.isBool())?v.asBool().unwrap():false;
}

float getRot(int id){
	switch (id){
	case 0: return 180;
	case 1: return 90;
	case 2: return 270;
	case 3: return 0;
	case 4: return 180;
	case 5: return 90;
	case 6: return 270;
	case 7: return 0;
	default: return 0;
	}
}

ccColor3B ourple = {193, 75, 153};
ccColor3B ble = {0, 255, 255};
ccColor3B gren = {18, 250, 6};
ccColor3B red = {250, 56, 64};

ccColor3B gettColor(int id){
	switch (id){
	case 0: return ourple;
	case 1: return ble;
	case 2: return gren;
	case 3: return red;
	case 4: return ourple;
	case 5: return ble;
	case 6: return gren;
	case 7: return red;
	default: return red;
	}
}

std::string booltostr(bool a){
	if(a)return "True";
	return "False";
}

class fnfChartsPopup : public geode::Popup<>{
protected:
	unsigned int instseektime = 0;
	unsigned int voicesseektime = 0;
	unsigned int instlength = 0;
	float previewseektime = 0;
	double previewbpm = -1;
	bool instPlaying = false;
	bool voicesPlaying = false;
	bool previewPlaying = false;
	ScrollLayer* m_scrolllayer;
	float xgap = 16;
	float ygap = 16;
	float xoffset = 25;
	float yoffset = 40;
	int cursec=0;
	int lastsec=-1;

	std::vector<std::pair<std::string,CCLabelBMFont*>> labelpointer;

	void doFnf(CCObject* the){
		FLAlertLayer::create("holdup","ihavent finished recreating fnf triggers yet","o")->show();
		return;
	}

	void assignSongPreview(CCObject* from){
		auto who = static_cast<fnfChartsPopup*>(from);
		int tag = who->getTag();
		unsigned int *takethis = &instlength;
		float *alsotakethis = &previewseektime;
		bool *preview = &previewPlaying;
		// auto root = CCDirector::sharedDirector()->getRunningScene()->getChildByID("fnfPopup")->getChildByIDRecursive("songinfolayer");
		utils::file::pick(utils::file::PickMode::OpenFile,{}).listen([tag,takethis,alsotakethis,preview](geode::Result<std::filesystem::path, std::string>* file){
			if (!file||file->isErr()){
				// FLAlertLayer::create("No file selected", "Import cancelled", "OK")->show();
				log::warn("nvm or error");
				return 0;
			}
			auto path = file->unwrap();
			if (tag==1){
				instchannel->stop();
				inst->release();
				inst = nullptr;
				FMOD_RESULT test = FMODAudioEngine::sharedEngine()->m_system->createSound(path.string().c_str(),FMOD_CREATESTREAM,nullptr,&inst);
				if (test==FMOD_OK){
					log::info("loaded inst");
					if (instName){
						instName->setString(("Inst: "+path.filename().string()).c_str());
					}
					FMOD_RESULT length = inst->getLength(takethis,FMOD_TIMEUNIT_MS);
					FMOD_RESULT play = FMODAudioEngine::sharedEngine()->m_system->playSound(inst,nullptr,true,&instchannel);
					if (play==FMOD_OK){
						log::info("inst play and paused");
						FMOD_RESULT setpos = instchannel->setPosition(static_cast<unsigned int>((*alsotakethis)*1000),FMOD_TIMEUNIT_MS);
					}
				}else{
					log::warn("cant load inst");
				}
			}else if(tag==2){
				voiceschannel->stop();
				voices->release();
				voices= nullptr;
				FMOD_RESULT test = FMODAudioEngine::sharedEngine()->m_system->createSound(path.string().c_str(),FMOD_CREATESTREAM,nullptr,&voices);
				if (test==FMOD_OK){
					log::info("loaded voices");
					if (voicesName){
						voicesName->setString(("Voices: "+path.filename().string()).c_str());
					}
					FMOD_RESULT play = FMODAudioEngine::sharedEngine()->m_system->playSound(voices,nullptr,true,&voiceschannel);
					if (play==FMOD_OK){
						log::info("voices play and paused");
						FMOD_RESULT setpos = voiceschannel->setPosition(static_cast<unsigned int>((*alsotakethis)*1000),FMOD_TIMEUNIT_MS);
					}
				}else{
					log::warn("cant load voices");
				}
			}
			return 0;
		});
		return;
	}

	void doMusic(CCObject* ye){
		auto who = static_cast<CCMenuItemToggler*>(ye);
		if (!who->isOn()){
			previewPlaying = true;
			if (inst!=nullptr&&instchannel){
				if (instseektime>instlength){
					log::info("reset");
					instseektime = 0;
					voicesseektime = 0;
				}
				FMOD_RESULT setpos = instchannel->setPosition(static_cast<unsigned int>(previewseektime*1000),FMOD_TIMEUNIT_MS);
				FMOD_RESULT play = instchannel->setPaused(false);
				if (play==FMOD_RESULT::FMOD_OK){
					log::info("inst resumed");
					instPlaying = true;
				}
			}
			if (voices!=nullptr&&voiceschannel){
				FMOD_RESULT setpos = voiceschannel->setPosition(static_cast<unsigned int>(previewseektime*1000),FMOD_TIMEUNIT_MS);
				FMOD_RESULT play = voiceschannel->setPaused(false);
				if (play==FMOD_RESULT::FMOD_OK){
					log::info("voices resumed");
					voicesPlaying = true;
				}
			}
		}else{
			previewPlaying = false;
			if (inst!=nullptr&&instchannel){
				FMOD_RESULT pause = instchannel->setPaused(true);
				if (pause==FMOD_RESULT::FMOD_OK){
					log::info("inst paused");
					FMOD_RESULT seek = instchannel->getPosition(&instseektime,FMOD_TIMEUNIT_MS);
					log::info("{}",instseektime);
					instPlaying = false;
				}
			}
			if (voices!=nullptr&&voiceschannel){
				FMOD_RESULT pause = voiceschannel->setPaused(true);
				if (pause==FMOD_RESULT::FMOD_OK){
					log::info("voices paused");
					FMOD_RESULT seek = voiceschannel->getPosition(&voicesseektime,FMOD_TIMEUNIT_MS);
					log::info("{}",voicesseektime);
					voicesPlaying= false;
				}
			}
		}
	}

	void controlSection(CCObject* sendr){
		auto who = static_cast<CCMenuItemToggler*>(sendr);
		int tag = who->getTag();
		if(tag==-2){
			cursec=0;
		}else{
			cursec+=tag;
		}
		if (cursec>thissong.sections.size()){
			cursec=thissong.sections.size();
		}
		previewseektime=thissong.sections[cursec].time+0.0001;
		if (inst!=nullptr&&instchannel){
			FMOD_RESULT setpos = instchannel->setPosition(static_cast<unsigned int>(previewseektime*1000),FMOD_TIMEUNIT_MS);
		}
		if (voices!=nullptr&&voiceschannel){
			FMOD_RESULT setpos = voiceschannel->setPosition(static_cast<unsigned int>(previewseektime*1000),FMOD_TIMEUNIT_MS);
		}
	}

	void onSectionChange(){
		// auto sectionlayer = GameManager::sharedState()->getChildByIDRecursive("sectionlayer");
		// if (!sectionlayer){
		// 	return;
		// }
		for (auto a:labelpointer){
			if(a.first=="step"){a.second->setString(("Steps: "+numToString(thissong.sections[cursec].sectionSize)).c_str());continue;}
			if(a.first=="type"){a.second->setString(("Type: "+numToString(thissong.sections[cursec].stype,3)).c_str());continue;}
			if(a.first=="musthit"){a.second->setString(("Must hit: "+booltostr(thissong.sections[cursec].mustHit)).c_str());continue;}
			if(a.first=="bpm"){a.second->setString(("BPM: "+numToString(thissong.sections[cursec].bpm,3)).c_str());continue;}
			if(a.first=="changebpm"){a.second->setString(("Change BPM: "+booltostr(thissong.sections[cursec].changeBpm)).c_str());continue;}
			if(a.first=="alt"){a.second->setString(("Alt section: "+booltostr(thissong.sections[cursec].alt)).c_str());continue;}
			if(a.first=="gf"){a.second->setString(("Gf section: "+booltostr(thissong.sections[cursec].gf)).c_str());continue;}
		}
	}

	void update(float dt) override {
		if (previewbpm<1){
			previewbpm=thissong.bpm;
		}
		if (instchannel&&inst&&instPlaying==true){
			if(previewseektime*1000>instlength){
				previewseektime=0;
				previewbpm=thissong.bpm;
				FMOD_RESULT play = FMODAudioEngine::sharedEngine()->m_system->playSound(inst,nullptr,false,&instchannel);
				FMOD_RESULT sek = instchannel->setPosition(0,FMOD_TIMEUNIT_MS);
				if(voiceschannel&&voices){
					FMOD_RESULT play = FMODAudioEngine::sharedEngine()->m_system->playSound(voices,nullptr,false,&voiceschannel);
					FMOD_RESULT sek = voiceschannel->setPosition(0,FMOD_TIMEUNIT_MS);
				}
			}
		}
		// if (previewPlaying&&instPlaying&&((voices)?voicesPlaying:true)){
		// 	if (justplayed==false){
		// 		previewseektime=static_cast<float>(instseektime)/1000;
		// 		justplayed=true;
		// 	}else{
		// 		previewseektime += dt;
		// 	}
		// }else{
		// 	justplayed=false;
		// }
		if (previewPlaying){
			previewseektime += dt;
		}
		
		// m_scrolllayer->setScale(lerp(m_scrolllayer->getScale(),1,dt*4));
		if (!thissong.sections.empty()){
			int index=0;
			CCObject* the = NULL;
			CCARRAY_FOREACH(m_scrolllayer->m_contentLayer->getChildren(),the){
				static_cast<CCNode*>(the)->setVisible(false);
				static_cast<CCNode*>(the)->setPositionX(1000);
			}
			for (auto const& a:thissong.sections){
				if(a.changeBpm){
					previewbpm=a.bpm;
				}
				if (previewseektime>=a.time&&previewseektime<a.endtime){
					if(a.changeBpm){
						previewbpm=a.bpm;
					}
					if(previewPlaying)cursec=index;
					auto thing = m_scrolllayer->m_contentLayer->getChildByTag(index);
					if (thing){
						thing->setVisible(true);
						thing->setPositionX(90);
					}
					float thespeed = previewbpm/60;
					float they = ((previewseektime)-a.time)*thespeed*64;
					m_scrolllayer->m_contentLayer->setPosition({0,they-yoffset});

					if (cursec!=lastsec&&previewPlaying){
						// m_scrolllayer->setScale(m_scrolllayer->getScale()+0.05);
					}

					break;
				}
				index+=1;
			}
		}
		if (cursec!=lastsec){
			onSectionChange();
		}

		lastsec=cursec;

		CCLOG("Frame dt = %f",dt);
	}

	void changeTab(CCObject* sendr){
		auto who = static_cast<fnfChartsPopup*>(sendr);
		int tag = who->getTag();
		auto menu = who->getParent();
		auto layers = menu->getParent();
		if (tag>=0&&tag<5){
			CCNode* the = NULL;
			if ((menu->getChildren()) && (menu->getChildren())->data->num > 0){//thanks Bin
				for(CCObject** __arr__ = (menu->getChildren())->data->arr, **__end__ = (menu->getChildren())->data->arr + (menu->getChildren())->data->num-1; __arr__ <= __end__ && (((the) = static_cast<CCNode*>(*__arr__)) != 0 ); __arr__++){
					static_cast<CCMenuItemToggler*>(the)->toggle(false);
					static_cast<CCMenuItemToggler*>(the)->updateSprite();
				}
			}
			layers->getChildByID("songinfolayer")->setVisible(false);
			layers->getChildByID("sectionlayer")->setVisible(false);
			layers->getChildByID("notelayer")->setVisible(false);
			layers->getChildByID("eventlayer")->setVisible(false);
			layers->getChildByID("remaplayer")->setVisible(false);
			switch (tag){
			case 0: layers->getChildByID("songinfolayer")->setVisible(true);break;
			case 1:layers->getChildByID("sectionlayer")->setVisible(true);break;
			case 2:layers->getChildByID("notelayer")->setVisible(true);break;
			case 3:layers->getChildByID("eventlayer")->setVisible(true);break;
			case 4:layers->getChildByID("remaplayer")->setVisible(true);break;
			default:break;
			}
		}
		return;
	}

	bool setup() override{
		this->setTitle("Chart Preview");
		auto title = this->m_title;
		auto originBg = this->m_bgSprite;

		ccColor3B black = {0,0,0};
		GLubyte op = 70;
		originBg->setColor(black);
		originBg->setZOrder(-10);

		auto plusSprite = CCSprite::createWithSpriteFrameName("GJ_plus2Btn_001.png");
		plusSprite->setScale(0.5f);

		auto arrowsprite = CCSprite::createWithSpriteFrameName("navArrowBtn_001.png");
		auto sectionarrow = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
		auto sectionarrow2 = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
		auto returnsectionarrow = CCSprite::createWithSpriteFrameName("GJ_backBtn_001.png");

		title->setPositionX(title->getPositionX()-100);

		auto bg = CCScale9Sprite::create("GJ_square01.png");
		bg->setContentSize({200.0f,300.0f});
		bg->setZOrder(1);
		m_mainLayer->addChildAtPosition(bg,Anchor::Left,{100.0f,0.0f});

		// auto menu = CCMenu::create();
		auto defMenu = this->m_closeBtn->getParent();
		
		auto songinfolayer = CCLayer::create();
		auto sectionlayer = CCLayer::create();
		auto notelayer = CCLayer::create();
		auto eventlayer = CCLayer::create();
		auto remaplayer = CCLayer::create();

		auto contentsize = CCSize(200.0f,300.0f);
		auto anchorp = CCPoint(0.0f,0.5f);
		
		songinfolayer->setContentSize(contentsize);
		sectionlayer->setContentSize(contentsize);
		notelayer->setContentSize(contentsize);
		eventlayer->setContentSize(contentsize);
		remaplayer->setContentSize(contentsize);

		songinfolayer->setZOrder(5);
		sectionlayer->setZOrder(5);
		notelayer->setZOrder(5);
		eventlayer->setZOrder(5);
		remaplayer->setZOrder(5);

		songinfolayer->setAnchorPoint(anchorp);
		sectionlayer->setAnchorPoint(anchorp);
		notelayer->setAnchorPoint(anchorp);
		eventlayer->setAnchorPoint(anchorp);
		remaplayer->setAnchorPoint(anchorp);

		songinfolayer->setID("songinfolayer");
		sectionlayer->setID("sectionlayer");
		notelayer->setID("notelayer");
		eventlayer->setID("eventlayer");
		remaplayer->setID("remaplayer");

		sectionlayer->setVisible(false);
		notelayer->setVisible(false);
		eventlayer->setVisible(false);
		remaplayer->setVisible(false);

		m_mainLayer->addChildAtPosition(songinfolayer,Anchor::Left);
		m_mainLayer->addChildAtPosition(sectionlayer,Anchor::Left);
		m_mainLayer->addChildAtPosition(notelayer,Anchor::Left);
		m_mainLayer->addChildAtPosition(eventlayer,Anchor::Left);
		m_mainLayer->addChildAtPosition(remaplayer,Anchor::Left);

		auto songinfomenu = CCMenu::create();
		auto sectionmenu = CCMenu::create();
		auto notemenu = CCMenu::create();
		auto eventmenu = CCMenu::create();
		auto remapmenu = CCMenu::create();

		songinfomenu->setContentSize(contentsize);
		sectionmenu->setContentSize(contentsize);
		notemenu->setContentSize(contentsize);
		eventmenu->setContentSize(contentsize);
		remapmenu->setContentSize(contentsize);

		songinfomenu->setAnchorPoint(anchorp);
		sectionmenu->setAnchorPoint(anchorp);
		notemenu->setAnchorPoint(anchorp);
		eventmenu->setAnchorPoint(anchorp);
		remapmenu->setAnchorPoint(anchorp);

		songinfolayer->addChildAtPosition(songinfomenu,Anchor::Left);
		sectionlayer->addChildAtPosition(sectionmenu,Anchor::Left);
		notelayer->addChildAtPosition(notemenu,Anchor::Left);
		eventlayer->addChildAtPosition(eventmenu,Anchor::Left);
		remaplayer->addChildAtPosition(remapmenu,Anchor::Left);

		// songinfomenu->setConte

		auto scrollable = geode::ScrollLayer::create({180.0f,300.f},true,true);
		scrollable->setZOrder(-5);
		scrollable->ignoreAnchorPointForPosition(false);
		m_mainLayer->addChildAtPosition(scrollable,Anchor::Right,{-90.0f,0.0f});
		m_scrolllayer = scrollable;

		auto line = CCSprite::createWithSpriteFrameName("floorLine_01_001.png");
		line->setZOrder(1);
		// line->setColor(black);
		line->setScaleX(0.35f);
		scrollable->addChildAtPosition(line,Anchor::Top,{0,-80});

		auto sAxis = SimpleAxisLayout::create(geode::Axis::Column);
		sAxis->setMainAxisDirection(AxisDirection::TopToBottom);
		sAxis->setMainAxisAlignment(MainAxisAlignment::Start);
		sAxis->setGap(20.0f);

		auto togglerMenu = CCMenu::create();
		togglerMenu->setContentSize({20.0f,269.0f});
		togglerMenu->setID("toggler");
		togglerMenu->setLayout(sAxis);
		m_mainLayer->addChildAtPosition(togglerMenu,Anchor::Center,{9.0f,0.0f});

		auto songinfoB = CCMenuItemToggler::createWithSize("GJ_tabOff_001.png","GJ_tabOn_001.png",this,menu_selector(fnfChartsPopup::changeTab),1.0f);
		songinfoB->setID("songinfoB");
		songinfoB->setRotation(90);
		songinfoB->setTag(0);
		togglerMenu->addChildAtPosition(songinfoB,Anchor::Center);

		auto sectionB = CCMenuItemToggler::createWithSize("GJ_tabOff_001.png","GJ_tabOn_001.png",this,menu_selector(fnfChartsPopup::changeTab),1.0f);
		sectionB->setID("sectionB");
		sectionB->setRotation(90);
		sectionB->setTag(1);
		togglerMenu->addChildAtPosition(sectionB,Anchor::Center);

		auto noteB = CCMenuItemToggler::createWithSize("GJ_tabOff_001.png","GJ_tabOn_001.png",this,menu_selector(fnfChartsPopup::changeTab),1.0f);
		noteB->setID("noteB");
		noteB->setRotation(90);
		noteB->setTag(2);
		togglerMenu->addChildAtPosition(noteB,Anchor::Center);

		auto eventB = CCMenuItemToggler::createWithSize("GJ_tabOff_001.png","GJ_tabOn_001.png",this,menu_selector(fnfChartsPopup::changeTab),1.0f);
		eventB->setID("eventB");
		eventB->setRotation(90);
		eventB->setTag(3);
		togglerMenu->addChildAtPosition(eventB,Anchor::Center);

		auto remapB = CCMenuItemToggler::createWithSize("GJ_tabOff_001.png","GJ_tabOn_001.png",this,menu_selector(fnfChartsPopup::changeTab),1.0f);
		remapB->setID("remapB");
		remapB->setRotation(90);
		remapB->setTag(4);
		togglerMenu->addChildAtPosition(remapB,Anchor::Center);

		sAxis->apply(togglerMenu);

		//songinfo stuff

		auto songinfoT = CCLabelBMFont::create("Song Info","bigFont.fnt",200,CCTextAlignment::kCCTextAlignmentCenter);
		songinfoT->setScale(0.6f);
		songinfolayer->addChildAtPosition(songinfoT,Anchor::Top,{0.0f,-40.0f});

		auto songT = CCLabelBMFont::create(("Song: "+thissong.song).c_str(),"bigFont.fnt",200,CCTextAlignment::kCCTextAlignmentLeft);
		songT->ignoreAnchorPointForPosition(true);
		songT->setAnchorPoint({0.0f,0.5f});
		songT->setScale(0.4f);
		songinfolayer->addChildAtPosition(songT,Anchor::Left,{10.0f,70.0f-(15*0)});

		auto p1T = CCLabelBMFont::create(("Player 1: "+thissong.player1).c_str(),"bigFont.fnt",200,CCTextAlignment::kCCTextAlignmentLeft);
		p1T->ignoreAnchorPointForPosition(true);
		p1T->setAnchorPoint({0.0f,0.5f});
		p1T->setScale(0.4f);
		songinfolayer->addChildAtPosition(p1T,Anchor::Left,{10.0f,70.0f-(15*1)});

		auto gfT = CCLabelBMFont::create(("Gf: "+thissong.gfVersion).c_str(),"bigFont.fnt",200,CCTextAlignment::kCCTextAlignmentLeft);
		gfT->ignoreAnchorPointForPosition(true);
		gfT->setAnchorPoint({0.0f,0.5f});
		gfT->setScale(0.4f);
		songinfolayer->addChildAtPosition(gfT,Anchor::Left,{10.0f,70.0f-(15*2)});

		auto p2T = CCLabelBMFont::create(("Player 2: "+thissong.player2).c_str(),"bigFont.fnt",200,CCTextAlignment::kCCTextAlignmentLeft);
		p2T->ignoreAnchorPointForPosition(true);
		p2T->setAnchorPoint({0.0f,0.5f});
		p2T->setScale(0.4f);
		songinfolayer->addChildAtPosition(p2T,Anchor::Left,{10.0f,70.0f-(15*3)});

		auto bpmT = CCLabelBMFont::create(("BPM: "+numToString(thissong.bpm,3)).c_str(),"bigFont.fnt",200,CCTextAlignment::kCCTextAlignmentLeft);
		bpmT->ignoreAnchorPointForPosition(true);
		bpmT->setAnchorPoint({0.0f,0.5f});
		bpmT->setScale(0.4f);
		songinfolayer->addChildAtPosition(bpmT,Anchor::Left,{10.0f,70.0f-(15*4)});

		auto speedT = CCLabelBMFont::create(("Speed: "+numToString(thissong.speed,3)).c_str(),"bigFont.fnt",200,CCTextAlignment::kCCTextAlignmentLeft);
		speedT->ignoreAnchorPointForPosition(true);
		speedT->setAnchorPoint({0.0f,0.5f});
		speedT->setScale(0.4f);
		songinfolayer->addChildAtPosition(speedT,Anchor::Left,{10.0f,70.0f-(15*5)});

		auto instT = CCLabelBMFont::create("Inst:","bigFont.fnt",2000,CCTextAlignment::kCCTextAlignmentLeft);
		instT->ignoreAnchorPointForPosition(true);
		instT->setAnchorPoint({0.0f,0.5f});
		instT->setScale(0.4f);
		instT->setID("instlabel");
		songinfolayer->addChildAtPosition(instT,Anchor::Left,{10.0f,70.0f-(15*6)});
		instName = instT;

		auto voicesT = CCLabelBMFont::create("Voices:","bigFont.fnt",2000,CCTextAlignment::kCCTextAlignmentLeft);
		voicesT->ignoreAnchorPointForPosition(true);
		voicesT->setAnchorPoint({0.0f,0.5f});
		voicesT->setScale(0.4f);
		voicesT->setID("voiceslabel");
		songinfolayer->addChildAtPosition(voicesT,Anchor::Left,{10.0f,70.0f-(15*7)});
		voicesName = voicesT;

		auto instbg = CCScale9Sprite::create("square02_small.png");
		instbg->setContentSize({125.0f*2,15.0f*2});
		instbg->setColor(black);
		instbg->setOpacity(op);
		instbg->setZOrder(-1);
		instbg->ignoreAnchorPointForPosition(true);
		instbg->setAnchorPoint({0.0f,0.5f});
		instbg->setScale(0.5f);
		songinfolayer->addChildAtPosition(instbg,Anchor::Left,{43.0f,70.0f-(15*6)});

		auto voicesbg = CCScale9Sprite::create("square02_small.png");
		voicesbg->setContentSize({108.0f*2,15.0f*2});
		voicesbg->setColor(black);
		voicesbg->setOpacity(op);
		voicesbg->setZOrder(-1);
		voicesbg->ignoreAnchorPointForPosition(true);
		voicesbg->setAnchorPoint({0.0f,0.5f});
		voicesbg->setScale(0.5f);
		songinfolayer->addChildAtPosition(voicesbg,Anchor::Left,{60.0f,70.0f-(15*7)});

		auto instP = CCMenuItemSpriteExtra::create(plusSprite,this,menu_selector(fnfChartsPopup::assignSongPreview));
		instP->ignoreAnchorPointForPosition(true);
		instP->setAnchorPoint({0.5f,0.5f});
		instP->setTag(1);
		songinfomenu->addChildAtPosition(instP,Anchor::Left,{174.0f,79.0f-(15*6)});
		
		auto voicesP = CCMenuItemSpriteExtra::create(plusSprite,this,menu_selector(fnfChartsPopup::assignSongPreview));
		voicesP->ignoreAnchorPointForPosition(true);
		voicesP->setAnchorPoint({0.5f,0.5f});
		voicesP->setTag(2);
		songinfomenu->addChildAtPosition(voicesP,Anchor::Left,{174.0f,79.0f-(15*7)});

		//section

		auto sectionT = CCLabelBMFont::create("Section","bigFont.fnt",200,CCTextAlignment::kCCTextAlignmentCenter);
		sectionT->setScale(0.6f);
		sectionlayer->addChildAtPosition(sectionT,Anchor::Top,{0.0f,-40.0f});

		auto stepsT = CCLabelBMFont::create("Steps: 0","bigFont.fnt",200,CCTextAlignment::kCCTextAlignmentLeft);
		stepsT->ignoreAnchorPointForPosition(true);
		stepsT->setAnchorPoint({0.0f,0.5f});
		stepsT->setScale(0.4f);
		sectionlayer->addChildAtPosition(stepsT,Anchor::Left,{10.0f,70.0f-(15*0)});
		labelpointer.push_back({"step", stepsT});

		auto typesT = CCLabelBMFont::create("Type: 0","bigFont.fnt",200,CCTextAlignment::kCCTextAlignmentLeft);
		typesT->ignoreAnchorPointForPosition(true);
		typesT->setAnchorPoint({0.0f,0.5f});
		typesT->setScale(0.4f);
		sectionlayer->addChildAtPosition(typesT,Anchor::Left,{10.0f,70.0f-(15*1)});
		labelpointer.push_back({"type", typesT});

		auto musthitT = CCLabelBMFont::create("Must hit: False","bigFont.fnt",200,CCTextAlignment::kCCTextAlignmentLeft);
		musthitT->ignoreAnchorPointForPosition(true);
		musthitT->setAnchorPoint({0.0f,0.5f});
		musthitT->setScale(0.4f);
		sectionlayer->addChildAtPosition(musthitT,Anchor::Left,{10.0f,70.0f-(15*2)});
		labelpointer.push_back({"musthit", musthitT});

		auto bpmsT = CCLabelBMFont::create("BPM: 0","bigFont.fnt",200,CCTextAlignment::kCCTextAlignmentLeft);
		bpmsT->ignoreAnchorPointForPosition(true);
		bpmsT->setAnchorPoint({0.0f,0.5f});
		bpmsT->setScale(0.4f);
		sectionlayer->addChildAtPosition(bpmsT,Anchor::Left,{10.0f,70.0f-(15*3)});
		labelpointer.push_back({"bpm", bpmsT});

		auto changebpmsT = CCLabelBMFont::create("Change BPM: False","bigFont.fnt",200,CCTextAlignment::kCCTextAlignmentLeft);
		changebpmsT->ignoreAnchorPointForPosition(true);
		changebpmsT->setAnchorPoint({0.0f,0.5f});
		changebpmsT->setScale(0.4f);
		sectionlayer->addChildAtPosition(changebpmsT,Anchor::Left,{10.0f,70.0f-(15*4)});
		labelpointer.push_back({"changebpm", changebpmsT});

		auto altsT = CCLabelBMFont::create("Alt section: False","bigFont.fnt",200,CCTextAlignment::kCCTextAlignmentLeft);
		altsT->ignoreAnchorPointForPosition(true);
		altsT->setAnchorPoint({0.0f,0.5f});
		altsT->setScale(0.4f);
		sectionlayer->addChildAtPosition(altsT,Anchor::Left,{10.0f,70.0f-(15*5)});
		labelpointer.push_back({"alt", altsT});

		auto gfsT = CCLabelBMFont::create("Gf section: False","bigFont.fnt",200,CCTextAlignment::kCCTextAlignmentLeft);
		gfsT->ignoreAnchorPointForPosition(true);
		gfsT->setAnchorPoint({0.0f,0.5f});
		gfsT->setScale(0.4f);
		sectionlayer->addChildAtPosition(gfsT,Anchor::Left,{10.0f,70.0f-(15*6)});
		labelpointer.push_back({"gf",gfsT});

		//note

		auto noteT = CCLabelBMFont::create("Note","bigFont.fnt",200,CCTextAlignment::kCCTextAlignmentCenter);
		noteT->setScale(0.6f);
		notelayer->addChildAtPosition(noteT,Anchor::Top,{0.0f,-40.0f});

		//event

		auto eventT = CCLabelBMFont::create("Event","bigFont.fnt",200,CCTextAlignment::kCCTextAlignmentCenter);
		eventT->setScale(0.6f);
		eventlayer->addChildAtPosition(eventT,Anchor::Top,{0.0f,-40.0f});

		//remap

		auto remapT = CCLabelBMFont::create("Spawn Remaps","bigFont.fnt",200,CCTextAlignment::kCCTextAlignmentCenter);
		remapT->setScale(0.6f);
		remaplayer->addChildAtPosition(remapT,Anchor::Top,{0.0f,-40.0f});

		//otherbutton

		float morexoffset = 110.0f;

		auto previewchartb = CCMenuItemToggler::createWithSize("GJ_playMusicBtn_001.png","GJ_stopMusicBtn_001.png",this,menu_selector(fnfChartsPopup::doMusic),0.6f);
		defMenu->addChildAtPosition(previewchartb,Anchor::Bottom,{morexoffset,15.0f});

		auto prevsec = CCMenuItemSpriteExtra::create(sectionarrow,this,menu_selector(fnfChartsPopup::controlSection));
		prevsec->setTag(-1);
		prevsec->getChildByIndex(0)->setScale(0.6f);
		defMenu->addChildAtPosition(prevsec,Anchor::Bottom,{morexoffset-30,15.0f});

		auto nextsec = CCMenuItemSpriteExtra::create(sectionarrow2,this,menu_selector(fnfChartsPopup::controlSection));
		nextsec->setTag(1);
		nextsec->getChildByIndex(0)->setScaleX(-0.6f);
		nextsec->getChildByIndex(0)->setScaleY(0.6f);
		defMenu->addChildAtPosition(nextsec,Anchor::Bottom,{morexoffset+30,15.0f});

		auto returnsec = CCMenuItemSpriteExtra::create(returnsectionarrow,this,menu_selector(fnfChartsPopup::controlSection));
		returnsec->setTag(-2);
		returnsec->getChildByIndex(0)->setScale(0.4f);
		defMenu->addChildAtPosition(returnsec,Anchor::Bottom,{morexoffset-60,15.0f});

		auto importb = ButtonSprite::create("Import",0.6f);
		auto import = CCMenuItemSpriteExtra::create(importb,this,menu_selector(fnfChartsPopup::doFnf));
		defMenu->addChildAtPosition(import,Anchor::Bottom,{-100,25});

		int cursection=0;
		// log::info("a:{}, {}",cursection,thissong.sections.size());
		double thisbpm=thissong.bpm;
		for (auto const& a:thissong.sections){
			auto thisnode = CCNode::create();
			thisnode->setTag(cursection);
			thisnode->setContentSize({180.0f,300.0f});
			thisnode->setAnchorPoint({0.5f,1.0f});
			thisnode->setVisible(false);

			auto thisbglayer = CCSpriteBatchNode::create("square.png");
			thisbglayer->setContentSize({180.0f,300.0f});
			thisbglayer->setAnchorPoint({0.5f,1.0f});

			// @geode-ignore(unknown-resource)
			auto thisnotelayer = CCSpriteBatchNode::create("GJ_GameSheet.png");
			thisnotelayer->setContentSize({180.0f,300.0f});
			thisnotelayer->setAnchorPoint({0.5f,1.0f});
			thisnotelayer->setZOrder(10);

			auto thedurnode = CCDrawNode::create();
			thedurnode->setContentSize({180.0f,300.0f});
			thedurnode->setAnchorPoint({0.5f,1.0f});
			thedurnode->setZOrder(9);

			if (a.changeBpm){
				thisbpm=a.bpm;
			}
			
			int alt=0;
			for (int i=0;i<a.sectionSize;i++){
				for (int j=0;j<9;j++){
					auto checker = CCSprite::create("square.png");
					checker->setScale(2);
					checker->setAnchorPoint({0.5f,1.0f});
					if ((alt+j)%2==0){
						checker->setColor({200,200,200});
						thisbglayer->addChildAtPosition(checker,Anchor::TopLeft,{(j*xgap)+xoffset,(-i*ygap)});
					}else{
						checker->setColor({150,150,150});
						thisbglayer->addChildAtPosition(checker,Anchor::TopLeft,{(j*xgap)+xoffset,(-i*ygap)});
					}
				}
				alt++;
			}

			for (auto const& b:a.notes){
				auto temparr = CCSprite::createWithSpriteFrameName("d_arrow_03_001.png");
				float thetiem = (thisbpm/60)*(b.time-(a.time*1000))*-1/15.62;
				float thedur = (thisbpm/60)*((b.time+b.dur)-(a.time*1000))*-1/15.62;
				int thisdir = (b.dir<4)?b.dir+4:b.dir-4;
				temparr->setAnchorPoint({0.5f,0.5f});
				temparr->setScaleX(0.705f);
				temparr->setScaleY(0.35f);
				temparr->setRotation(getRot(thisdir));
				temparr->setColor(gettColor(thisdir));
				thisnotelayer->addChildAtPosition(temparr,Anchor::TopLeft,{((thisdir+1)*xgap)+xoffset,(thetiem-8)});
				if(b.dur>0){
					float theyoffset = 300.0f;
					thedurnode->drawRect({((thisdir+1)*xgap)+xoffset-2,thetiem+theyoffset},{((thisdir+1)*xgap)+xoffset+2,thedur+theyoffset},{1,1,1,1},1,{1,1,1,1});
				}
			}

			for (auto const& b:a.events){
				auto temparr = CCSprite::createWithSpriteFrameName("d_arrow_03_001.png");
				float thetiem = (thisbpm/60)*(b.time-(a.time*1000))*-1/15.62;
				// float thedur = (thisbpm/60)*((b.time+b.dur)-(a.time*1000))*-1/15.62;
				// int thisdir = (b.dir<4)?b.dir+4:b.dir-4;
				temparr->setAnchorPoint({0.5f,0.5f});
				temparr->setScaleX(0.705f);
				temparr->setScaleY(0.35f);
				// temparr->setRotation(getRot(thisdir));
				temparr->setColor({100,100,100});
				thisnotelayer->addChildAtPosition(temparr,Anchor::TopLeft,{((0)*xgap)+xoffset,(thetiem-8)});
				// if(b.dur>0){
				// 	float theyoffset = 300.0f;
				// 	thedurnode->drawRect({((thisdir+1)*xgap)+xoffset-2,thetiem+theyoffset},{((thisdir+1)*xgap)+xoffset+2,thedur+theyoffset},{1,1,1,1},1,{1,1,1,1});
				// }
			}

			thisnode->addChildAtPosition(thisbglayer,Anchor::Top);
			thisnode->addChildAtPosition(thisnotelayer,Anchor::Top);
			thisnode->addChildAtPosition(thedurnode,Anchor::Top);

			scrollable->m_contentLayer->addChildAtPosition(thisnode,Anchor::Top,{0.0f,-yoffset});
			cursection++;
		}
		
		/*
		for (auto const& pathValue : filepaths.asArray().unwrap()){
			log::info("{}",std::filesystem::path(pathValue.asString().unwrap()).stem().string().substr(
				std::filesystem::path(pathValue.asString().unwrap()).stem().string().size()-4
			));
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
				auto title = CCLabelBMFont::create(cleanName(mapname).c_str(),"bigFont.fnt",1000,CCTextAlignment::kCCTextAlignmentLeft);
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
			}else if (std::filesystem::path(pathValue.asString().unwrap()).stem().string().substr(
				std::filesystem::path(pathValue.asString().unwrap()).stem().string().size()-5
			)=="music"){
				auto result = geode::utils::clipboard::write(pathValue.asString().unwrap());
				if (!result){continue;}
				notif("Copied song directory","GJ_infoIcon_001.png");
				music->release();
				music = nullptr;
				FMOD_RESULT musicsound = FMODAudioEngine::sharedEngine()->m_system->createSound(pathValue.asString().unwrap().c_str(),FMOD_CREATESTREAM,nullptr,&music);
				if (musicsound == FMOD_OK){
					FMOD_RESULT reult = music->getLength(&songLengthInt,FMOD_TIMEUNIT_MS);
					if (reult == FMOD_OK){
						log::info("song length :{}",songLengthInt);
					}else{
						log::warn("cant get song length");
					}
				}else{
					log::warn("cant create audiostream");
				}
			}
			else if (std::filesystem::path(pathValue.asString().unwrap()).stem().string().substr(
				std::filesystem::path(pathValue.asString().unwrap()).stem().string().size()-4
			)=="demo"){
				if (Mod::get()->getSettingValue<bool>("md-extract-demo")!=true){continue;}
				demochannel->stop();
				demo->release();
				demo = nullptr;
				FMOD_RESULT test2 = FMODAudioEngine::sharedEngine()->m_system->createSound(pathValue.asString().unwrap().c_str(),FMOD_CREATESTREAM|FMOD_LOOP_NORMAL,nullptr,&demo);
				if (test2 == FMOD_OK){
					FMOD_RESULT reult = FMODAudioEngine::sharedEngine()->m_system->playSound(demo,nullptr,false,&demochannel);
					if (reult == FMOD_OK){
						log::info("demo play");
					}else{
						log::warn("demo fail");
					}
				}else{
					log::warn("cant create audiostream");
				}
			}
		}
		*/
		// auto sAxis = SimpleAxisLayout::create(geode::Axis::Column);
		// sAxis->setMainAxisDirection(AxisDirection::TopToBottom);
		// sAxis->setGap(30.0f);
		auto infoSprite = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
		auto createButton = ButtonSprite::create("Create",0.70f);

		auto infoButton = InfoAlertButton::create("Info",infotext,0.6f);
		defMenu->addChildAtPosition(infoButton,Anchor::Top,{-20.0f,-20.0f},true);

		// CCPoint pos = {380.0f,230.0f};
		// infoButton->setPosition(pos);
		// m_mainLayer->addChildAtPosition(bg,Anchor::Right,offset);
		

		// scrollable->m_contentLayer->setLayout(sAxis);
		// sAxis->apply(scrollable->m_contentLayer);

		return true;
	}
	void onClose(CCObject* sender) override {
		// if (demochannel) 
		// if (demo) {
		// 	
		// }
		instchannel->stop();
		inst->release();
		inst = nullptr;
		voiceschannel->stop();
		voices->release();
		voices= nullptr;
		geode::Popup<>::onClose(sender);
		// auto popup = optionPopup::create();
		// popup->m_noElasticity = true;
		// popup->setID("importChartPopup");
		// popup->show();
	}
public:
	static fnfChartsPopup* create(){
		auto ret = new fnfChartsPopup();
		if (ret->initAnchored(400.f, 300.f)) {
			ret->autorelease();
			ret->scheduleUpdate();
			return ret;
		}
		delete ret;
		return nullptr;
	}
};


int initFnf(matjson::Value data){
	if(validatePsychChart(data)){
		thissong.sections.clear();
		auto check = data["song"].asString();
		double tempbpm;
		double temptime;
		matjson::Value root;
		bool converted = false;
		if (check.isOk()){
			log::info("were in root");
			root = data;
		}else{
			root = data["song"];
		}
		if (root.contains("format")&&root["format"]=="psych_v1_convert"){
			converted = true;
		}
		thissong.player1 = tryGetString("player1",&root);
		thissong.gfVersion = tryGetString("gfVersion",&root);
		thissong.player2 = tryGetString("player2",&root);
		thissong.song = tryGetString("song",&root);
		thissong.bpm = tryGetDouble("bpm",&root);
		thissong.speed = tryGetDouble("speed",&root);
		if (!root.contains("notes")){
			//no notes?
			log::warn("no notes?");
			return 1;
		}
		auto events = (root.contains("events"))?root["events"]:nullptr;
		tempbpm = thissong.bpm;
		temptime = 0;
		for (auto const& [a,b]:root["notes"]){
			// log::info("here");
			if (b.isNull()){continue;}
			double eos;
			section thissection;
			thissection.sectionSize = tryGetInt("sectionBeats",&b)*4;
			if (thissection.sectionSize==0){
				thissection.sectionSize = tryGetInt("lengthInSteps",&b);
			}
			thissection.bpm = tryGetDouble("bpm",&b);
			thissection.changeBpm = tryGetBool("changeBPM",&b);
			thissection.mustHit = tryGetBool("mustHitSection",&b);
			thissection.stype = tryGetInt("typeOfSection",&b);
			thissection.alt = tryGetBool("altAnim",&b);
			thissection.gf = tryGetBool("gfSection",&b);
			thissection.time = temptime;
			tempbpm = (thissection.changeBpm==true)?thissection.bpm:tempbpm;
			eos = temptime+(thissection.sectionSize*((60/tempbpm)/4)); 
			thissection.endtime = eos;

			if (b.contains("sectionNotes")&&!b["sectionNotes"].isNull()){
				for (auto const& [c,d]:b["sectionNotes"]){
					fnfnote thisnote;
					thisnote.time = d[0].asDouble().unwrap();
					thisnote.dur = d[2].asDouble().unwrap();
					thisnote.ntype = (d.size()>=4&&!d[3].isNull()&&d[3].isString())?d[3].asString().unwrap():"";//0x0
					if (thissection.mustHit||converted){
						thisnote.dir = d[1].asInt().unwrap();
					}else{
						if(d[1].asInt().unwrap()<4){
							thisnote.dir = d[1].asInt().unwrap()+4;
						}else{
							thisnote.dir = d[1].asInt().unwrap()-4;
						}
					}
					thissection.notes.push_back(thisnote);
				}
			}
			if (!events.isNull()){
				for (auto const& [c,d]:events){
					double etime = d[0].asDouble().unwrap();
					// log::info("{},{},{}",temptime,etime,eos);
					if (etime>=temptime*1000&&etime<eos*1000&&!d[1].isNull()){
						fnfevent thisevent;
						thisevent.time = etime;
						for (auto const& [e,f]:d[1]){
							singleEvent the;
							the.etype = (!f[0].isNull()&&f[0].isString())?f[0].asString().unwrap():nullptr;
							the.param1 = (!f[1].isNull()&&f[1].isString())?f[1].asString().unwrap():nullptr;
							the.param2 = (!f[2].isNull()&&f[2].isString())?f[2].asString().unwrap():nullptr;
							thisevent.daEvents.push_back(the);
						}
						thissection.events.push_back(thisevent);
					}
				}
			}
			thissong.sections.push_back(thissection);
			// log::info("during extarct: {}",thissong.sections.size());
			temptime = eos;
			
		}
	}else{
		log::warn("eh?");
	}
	log::info("fnf nisfished");
	auto fnfpopup = fnfChartsPopup::create();
	fnfpopup->m_noElasticity = true;
	fnfpopup->setID("fnfPopup");
	fnfpopup->show();
	
	return 0;
}
/*
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

int checkNoteType(matjson::Value note,int origin){
	int final = origin;
	if (!note[3].isNull()&&note[3].isString()){
		std::string datype = note[3].asString().unwrap();
		if (datype == "mimi"){
			final = 4;
		}
	}
	return final;
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

int getTargetID2(int num){
	switch (num){
		case 0: return leftType;
		case 1: return downType;
		case 2: return upType;
		case 3: return rightType;
		case 4: return dadleftType;
		case 5: return daddownType;
		case 6: return dadupType;
		case 7: return dadrightType;
		default: return 0;
	}
}

void addNote(LevelEditorLayer* editor, double daX, double daY, double dur = 0, int daType = 0, int targetG = 0, int targetDur = 0, int targetType = 0){
	auto ui = editor->m_editorUI;
	double offset = 0;
	double objX = ((noteOffset*grid)+(daX/1000*(grid/timepergrid)))+Xpos;
	double objY = (((daY+offset)*grid)+Ypos);
	CCPoint pos = {(float)objX,(float)objY};
	auto obj = ui->createObject(spawnID,pos);
	auto trigger = static_cast<EffectGameObject*>(obj);
	if (trigger){
		trigger->m_targetGroupID = (targetG!=0)?targetG:getTargetGroup(daY);
	}
	double obj1X = (noteOffset*grid)+(daX/1000*(grid/timepergrid))-(1.0f)+Xpos;
	double obj1Y = (((daY+offset)*grid)+Ypos);
	CCPoint pos1 = {(float)obj1X,(float)obj1Y};
	auto obj1 = ui->createObject(itemEditID,pos1);
	auto trigger1 = static_cast<ItemTriggerGameObject*>(obj1);
	if (trigger1){
		trigger1->m_mod1 = dur;
		trigger1->m_targetItemMode = 2;
		trigger1->m_resultType3 = 2;
		trigger1->m_targetGroupID = (targetDur!=0)?targetDur:getTargetID1(daY);
	}

	auto obj2 = ui->createObject(itemEditID,pos1);
	auto trigger2 = static_cast<ItemTriggerGameObject*>(obj2);
	if (trigger2){
		trigger2->m_mod1 = daType;
		trigger2->m_targetItemMode = 1;
		trigger2->m_resultType3 = 1;
		trigger2->m_targetGroupID = (targetType!=0)?targetType:getTargetID2(daY);
	}

	editor->updateObjectLabel(obj);
	editor->updateObjectLabel(obj1);
}

void addSpawn(LevelEditorLayer* editor, double daX, double daY, int group, bool parent = false){
	auto ui = editor->m_editorUI;

	double objX = (daX/1000*(grid/timepergrid))+Xpos;
	double objY = ((daY*grid)+Ypos);
	CCPoint pos = {(float)objX,(float)objY};
	auto obj = ui->createObject(spawnID,pos);
	auto trigger = static_cast<EffectGameObject*>(obj);
	if (trigger){
		trigger->m_targetGroupID = group;
	}
	obj->m_hasGroupParent = parent;

	editor->updateObjectLabel(obj);
}

//changed to editAreaScale
void addAreaScale(LevelEditorLayer* editor, double daX, double daY, int target, int center, double length, int daeffectID, bool early = false,double scaleY = 0.0f){
	auto ui = editor->m_editorUI;
	double doffset = (early == true)?(noteOffset*grid):0.0;
	int daLength = ceilf(length*30);
	double dX = doffset+(daX/1000*(grid/timepergrid))+Xpos;
	double dY = (((daY+1)*grid)+Ypos);
	CCPoint daPos = {(float)dX,(float)dY};
	auto daObj = ui->createObject(3013,daPos);
	auto daAreaTrigger = static_cast<EnterEffectObject*>(daObj);
	if (daAreaTrigger){
		daAreaTrigger->m_length = daLength;
		daAreaTrigger->m_duration = 0.0f;
		daAreaTrigger->m_areaScaleY = scaleY;
		daAreaTrigger->m_useEffectID = true;
		daAreaTrigger->m_targetGroupID = daeffectID; //bruh it should be m_effectID
	}
	editor->updateObjectLabel(daObj);
}

void addBPMChanger(LevelEditorLayer* editor, double daX, double daY, bool early = false){
	auto ui = editor->m_editorUI;
	double doffset = (early == true)?(noteOffset*grid):0.0;
	double objX = doffset+(daX/1000*(grid/timepergrid))+Xpos;
	double objY = ((daY*grid)+Ypos);
	CCPoint pos = {(float)objX,(float)objY};
	auto obj = ui->createObject(spawnID,pos);
	auto trigger = static_cast<EffectGameObject*>(obj);
	if (trigger){
		trigger->m_targetGroupID = 396;
	}
	double obj1X = doffset+(daX/1000*(grid/timepergrid))-(1.0f)+Xpos;
	double obj1Y = ((daY*grid)+Ypos);
	CCPoint pos1 = {(float)obj1X,(float)obj1Y};
	auto obj1 = ui->createObject(itemEditID,pos1);
	auto trigger1 = static_cast<ItemTriggerGameObject*>(obj1);
	if (trigger1){
		trigger1->m_mod1 = bpm;
		trigger1->m_targetItemMode = 2;
		trigger1->m_resultType3 = 2;
	}
	auto trigger3 = static_cast<EffectGameObject*>(obj1);
	if (trigger3){
		trigger3->m_targetGroupID = 2;
	}
	double length = ((60/bpm/4)/0.1538461538461538)+0.1;
	double obj2X = doffset+(daX/1000*(grid/timepergrid))+Xpos;
	double obj2Y = (((daY+1)*grid)+Ypos);
	CCPoint pos2 = {(float)obj2X,(float)obj2Y};
	addAreaScale(editor,daX,daY,0,0,0,20,early,length);

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
	auto trigger4 = static_cast<EnterEffectObject*>(obj2);
	if (trigger4){
		trigger4->m_moveDistance = (std::max((double)1.0,resultscroll)-1.0)*300;
		trigger4->m_duration = daDur;
		trigger4->m_useEffectID = true;
		trigger4->m_targetGroupID = 29; //bruh it should be m_effectID
	}
	auto obj3 = ui->createObject(3011,pos2);
	auto trigger5 = static_cast<EnterEffectObject*>(obj3);
	if (trigger5){
		trigger5->m_moveDistance = (std::max((double)1.0,resultscroll)-1.0)*300;
		trigger5->m_duration = daDur;
		trigger5->m_useEffectID = true;
		trigger5->m_targetGroupID = 30;
	}
	auto obj4 = ui->createObject(3013,pos2);
	auto trigger6 = static_cast<EnterEffectObject*>(obj4);
	if (trigger6){
		trigger6->m_areaScaleY = (std::max((double)1.0,resultscroll));
		trigger6->m_areaScaleX = 1.0;
		trigger6->m_duration = daDur;
		trigger6->m_useEffectID = true;
		trigger6->m_targetGroupID = 31;
	}
}

void fnfChart(LevelEditorLayer* editor,matjson::Value data){
	if (!editor){
		log::warn("editor gone??");
		return;
	}
	grid = 30;
	timepergrid = 0.09628343;
	Ypos = 5025.0;
	Xpos = 0.0;
	noteOffset = (1.56/0.09628343)*-1;
	spawnID = 1268;
	pickupID = 1817;
	areaScaleID = 3008; //lol
	itemEditID = 3619;

	daTime = 0;
	bpm = 120;
	scroll = 1.0;
	beat = 0;
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
	noteOffset = (1.56/0.09628343)*-1;
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
						int daType = ((!b["altAnim"].isNull()&&b["altAnim"].isBool()&&b["altAnim"].asBool().unwrap())||(!f[3].isNull()&&f[3].isString()&&f[3].asString().unwrap()=="Alt Animation"))?1:0;
						daType = ((!b["gfSection"].isNull()&&b["gfSection"].isBool()&&b["gfSection"].asBool().unwrap())||(!f[3].isNull()&&f[3].isString()&&f[3].asString().unwrap()=="GF Sing"))?daType+2:daType;
						//additional check cuz custom notetype
						daType = checkNoteType(f,daType);
						if (b["mustHitSection"].asBool().unwrap() || converted){
							addNote(editor,f[0].asDouble().unwrap(),f[1].asDouble().unwrap(),f[2].asDouble().unwrap(),daType);
						}else{
							if (f[1]<4){
								addNote(editor,f[0].asDouble().unwrap(),f[1].asDouble().unwrap()+4,f[2].asDouble().unwrap(),daType);
							} else {
								addNote(editor,f[0].asDouble().unwrap(),f[1].asDouble().unwrap()-4,f[2].asDouble().unwrap(),daType);
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
		auto root = CCDirector::sharedDirector()->getRunningScene();
		root->removeChildByID("importChartPopup");
		notif("Done","GJ_completesIcon_001.png");
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
			auto root = CCDirector::sharedDirector()->getRunningScene();
			root->removeChildByID("importChartPopup");
			notif("Done","GJ_completesIcon_001.png");
			return 0;
		});
		return;
	}else {
		FLAlertLayer::create("Invalid Chart Type", "Unknown chart from unknown engine? or maybe its not even an FNF chart file.", "OK")->show();
		//this aint chart data
		return;
	}
}
*/