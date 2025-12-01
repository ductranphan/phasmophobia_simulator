#ifndef DEFS_H
#define DEFS_H

#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>

/*
    You are free to rename all of the types and functions defined here.

    The ghost ID must remain the same for the validator to work correctly.
*/

#define MAX_ROOM_NAME 64
#define MAX_HUNTER_NAME 64
#define MAX_ROOMS 24
#define MAX_ROOM_OCCUPANCY 8
#define MAX_CONNECTIONS 8
#define MAX_HUNTER 4
#define ENTITY_BOREDOM_MAX 15
#define HUNTER_FEAR_MAX 15
#define DEFAULT_GHOST_ID 68057

typedef unsigned char EvidenceByte;

enum LogReason {
    LR_EVIDENCE = 0,
    LR_BORED = 1,
    LR_AFRAID = 2,
    LR_UNKNOWN = 3
};

enum EvidenceType {
    EV_EMF          = 1 << 0,
    EV_ORBS         = 1 << 1,
    EV_RADIO        = 1 << 2,
    EV_TEMPERATURE  = 1 << 3,
    EV_FINGERPRINTS = 1 << 4,
    EV_WRITING      = 1 << 5,
    EV_INFRARED     = 1 << 6,
};

enum GhostType {
    GH_POLTERGEIST  = EV_FINGERPRINTS | EV_TEMPERATURE | EV_WRITING,
    GH_THE_MIMIC    = EV_FINGERPRINTS | EV_TEMPERATURE | EV_RADIO,
    GH_HANTU        = EV_FINGERPRINTS | EV_TEMPERATURE | EV_ORBS,
    GH_JINN         = EV_FINGERPRINTS | EV_TEMPERATURE | EV_EMF,
    GH_PHANTOM      = EV_FINGERPRINTS | EV_INFRARED    | EV_RADIO,
    GH_BANSHEE      = EV_FINGERPRINTS | EV_INFRARED    | EV_ORBS,
    GH_GORYO        = EV_FINGERPRINTS | EV_INFRARED    | EV_EMF,
    GH_BULLIES      = EV_FINGERPRINTS | EV_WRITING     | EV_RADIO,
    GH_MYLING       = EV_FINGERPRINTS | EV_WRITING     | EV_EMF,
    GH_OBAKE        = EV_FINGERPRINTS | EV_ORBS        | EV_EMF,
    GH_YUREI        = EV_TEMPERATURE  | EV_INFRARED    | EV_ORBS,
    GH_ONI          = EV_TEMPERATURE  | EV_INFRARED    | EV_EMF,
    GH_MOROI        = EV_TEMPERATURE  | EV_WRITING     | EV_RADIO,
    GH_REVENANT     = EV_TEMPERATURE  | EV_WRITING     | EV_ORBS,
    GH_SHADE        = EV_TEMPERATURE  | EV_WRITING     | EV_EMF,
    GH_ONRYO        = EV_TEMPERATURE  | EV_RADIO       | EV_ORBS,
    GH_THE_TWINS    = EV_TEMPERATURE  | EV_RADIO       | EV_EMF,
    GH_DEOGEN       = EV_INFRARED     | EV_WRITING     | EV_RADIO,
    GH_THAYE        = EV_INFRARED     | EV_WRITING     | EV_ORBS,
    GH_YOKAI        = EV_INFRARED     | EV_RADIO       | EV_ORBS,
    GH_WRAITH       = EV_INFRARED     | EV_RADIO       | EV_EMF,
    GH_RAIJU        = EV_INFRARED     | EV_ORBS        | EV_EMF,
    GH_MARE         = EV_WRITING      | EV_RADIO       | EV_ORBS,
    GH_SPIRIT       = EV_WRITING      | EV_RADIO       | EV_EMF,
};

struct Hunter;
struct Room;
struct Ghost;

struct CaseFile {
    EvidenceByte collected; // Union of all of the evidence bits collected between all hunters
    bool         solved;    // True when >=3 unique bits set
    sem_t        mutex;     // Used for synchronizing both fields when multithreading    
};

// Implement here based on the requirements, should all be allocated to the House structure
struct Room {
    char name[MAX_ROOM_NAME];
    struct Room* connections[MAX_CONNECTIONS];  //Ppinters to its neighbour doors
    int connection_count;
    
    struct Ghost* ghost;                          //check if the ghost standing at the room (pointer or null)
    struct Hunter* hunters[MAX_ROOM_OCCUPANCY];       //which hunter are standing here?
    
    int hunter_count;      
    bool is_exit;
    EvidenceByte evidence;  //this is the game's imporatnat info, ghost drops it and hunter collects it, checks what clues are in the room
    sem_t mutex;     //The room lock
};

struct RoomNode {
    struct Room* room;
    struct RoomNode* next;
};

struct RoomStack {  //Linkedlist, mainly for hunter to retrace their steps when they want to go back to van
    struct RoomNode* head;
    int count;
};


// Implement here based on the requirements, should be allocated to the House structure
struct Ghost {
    int id;
    enum GhostType type;   //type of ghost 
    struct Room* current_room;    //location of ghost
    int boredom;    //boredom counter
    bool running;     
    pthread_t thread;    //the actual brain (thread)
};

struct Hunter {
    char name[MAX_HUNTER_NAME];
    int id;

    struct Room* current_room;
    struct CaseFile* shared_casefile;    //pointer to their shared infos
    struct RoomStack breadcrumb_stack;    //path back home

    enum EvidenceType device;   //the tool hunter is holding
    int fear;        
    int boredom; 
    
    enum LogReason exit_reason;
    bool running;
    pthread_t thread;
};

// Can be either stack or heap allocated
struct House {
    struct Room rooms[MAX_ROOMS];
    int room_count;
    struct Room* starting_room;   //pointer to the starting room

    struct Hunter* hunters;          
    int hunter_count;
    
    struct CaseFile casefile;
    struct Ghost ghost;
};

/* The provided `house_populate_rooms()` function requires the following functions.
   You are free to rename them and change their parameters and modify house_populate_rooms()
   as needed as long as the house has the correct rooms and connections after calling it.
*/

void house_init(struct House* house);
void house_cleanup(struct House* house);
void room_init(struct Room* room, const char* name, bool is_exit);
void room_connect(struct Room* a, struct Room* b); 

void ghost_init(struct Ghost* ghost, int id, enum GhostType type, struct Room* room);
void* ghost_thread(void* arg);

void hunter_init(struct Hunter* hunter, int id, const char* name, struct Room* start_room, enum EvidenceType device, struct CaseFile* casefile);
void hunter_cleanup(struct Hunter* hunter);
void* hunter_thread(void* arg);

#endif // DEFS_H
