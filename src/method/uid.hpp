#ifndef uid
#define uid

#include <vector>
#include <tuple>
#include <string>
#include "objectCache.hpp"

struct note_data{
    std::string ibms_id;
    int spawnGroup; // which spawn group to trigger
    int noteType; // what kind of note is this
    int pos; // spawn pos
    int speed; // speed 1-2-3
    int enter; // enter animation
    int sound; // sound to play
    objectsData* objDown; // pointer to which groups of obecsjt
    objectsData* objUp;
    objectsData* endNoteDown; // for hold notes
    objectsData* endNoteUp;
};

extern std::vector<note_data> note_uids;

#endif