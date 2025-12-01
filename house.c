#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"

/*
    house.c
*/

// Called house_populate_rooms in helpers.c
void room_init(struct Room* room, const char* name, bool is_exit) {
    strncpy(room->name, name, MAX_ROOM_NAME);
    
    room->is_exit = is_exit;
    room->connection_count = 0;
    room->hunter_count = 0;
    room->ghost = NULL; 
    room->evidence = 0;
    
    sem_init(&room->mutex, 0, 1);
    
    for(int i = 0; i < MAX_CONNECTIONS; i++) {
        room->connections[i] = NULL;
    }
    for(int i = 0; i < MAX_ROOM_OCCUPANCY; i++) {
        room->hunters[i] = NULL;
    }
}

// Called house_populate_rooms in helpers.c
void room_connect(struct Room* a, struct Room* b) {
    if (a->connection_count < MAX_CONNECTIONS) {
        a->connections[a->connection_count++] = b;
    }
    
    if (b->connection_count < MAX_CONNECTIONS) {
        b->connections[b->connection_count++] = a;
    }
}

//aetting the board
void house_init(struct House* house) {
    house->casefile.collected = 0;
    house->casefile.solved = false;
    sem_init(&house->casefile.mutex, 0, 1);
    
    house->hunters = NULL;
    house->hunter_count = 0;
}

//Clean up house, destroy unnessary data
void house_cleanup(struct House* house) {
    for (int i = 0; i < house->room_count; i++) {
        sem_destroy(&house->rooms[i].mutex);
    }
    
    sem_destroy(&house->casefile.mutex);
    
    if (house->hunters != NULL) {
        free(house->hunters);
        house->hunters = NULL;
    }
}