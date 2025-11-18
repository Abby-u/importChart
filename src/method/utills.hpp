#ifndef utills
#define utills

#include <vector>
#include <utility>

int getData(std::function<void(matjson::Value)> onResult);

void notif(std::string themsg, std::string icon, float time = 1);

std::vector<ChanceObject> rempaFromTuple(std::vector<std::tuple<int,int,int,int>> vector);

#endif