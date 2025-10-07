#include "utills.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

int getData(std::function<void(matjson::Value)> onResult){
	utils::file::pick(utils::file::PickMode::OpenFile,{}).listen([onResult](geode::Result<std::filesystem::path, std::string>* file){
		if (!file){
			FLAlertLayer::create("No file selected", "Import cancelled", "OK")->show();
			onResult(matjson::Value());
			return 0;
		}
		auto rawData = file->unwrap();
		auto jsonData = utils::file::readJson(rawData);
		if (!jsonData) {
			FLAlertLayer::create("Invalid File Type", "This ain't json file. Import cancelled", "OK")->show();
			log::warn("this aint json");
			onResult(matjson::Value());
			return 0;
		}
		log::info("ok");
		onResult(jsonData.unwrap());
        return 0;
	});
    return 0;
}