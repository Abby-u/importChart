#ifndef utills
#define utills

int getData(std::function<void(matjson::Value)> onResult);

void notif(std::string themsg, std::string icon, float time = 1);

#endif