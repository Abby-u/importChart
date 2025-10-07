#ifndef uid
#define uid

#include <vector>
#include <tuple>
#include <string>
#include "objectCache.hpp"

struct note_data{
    std::string note_uid;
    int spawnGroup; //which spawn group to trigger
    int noteType; //what kind of note is this
    int pos; //up =1 or down =0
    int speed; //speed 1-2-3
    int enter; //
    objectsData* useObjects;
    int sound;
    objectsData* endNote; //optional
};

extern std::vector<note_data> note_uids;

#endif