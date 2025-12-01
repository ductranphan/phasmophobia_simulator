#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h> 

#include "defs.h"
#include "helpers.h"


int main() {
	//Initialize House and Helper
    srand(time(NULL)); 
    struct House house;
    house_init(&house);
    house_populate_rooms(&house); 

	//Intialize Ghost
    const enum GhostType* ghost_types;
    int num_ghost_types = get_all_ghost_types(&ghost_types);
    enum GhostType random_type = ghost_types[rand_int_threadsafe(0, num_ghost_types)];

    struct Room* ghost_room = NULL;
    do {
        int r = rand_int_threadsafe(1, house.room_count);
        ghost_room = &house.rooms[r];
    } while (ghost_room == house.starting_room);

    ghost_init(&house.ghost, DEFAULT_GHOST_ID, random_type, ghost_room);

	//Initialize Hunter
    char input_buffer[MAX_HUNTER_NAME];
    int hunter_capacity = 0;
    
    printf("Run Phasmaphobia \n");
    printf("Enter hunter names ('done' to finish):\n");

	//Loop will stop if we reach MAX_HUNTERS
    while (house.hunter_count < MAX_HUNTER) {
        printf("Hunter Name: ");
        if (!fgets(input_buffer, sizeof(input_buffer), stdin)) {
        	break;
        }
        
        //Remvoe new line
        input_buffer[strcspn(input_buffer, "\n")] = 0;

        if (strcmp(input_buffer, "done") == 0) {
            break;
        }
        
        char hunter_name[MAX_HUNTER_NAME];
        strncpy(hunter_name, input_buffer, MAX_HUNTER_NAME);
        
        printf("Hunter ID: ");
        if (!fgets(input_buffer, sizeof(input_buffer), stdin)){
        	break;
        }
        
        int hunter_id = atoi(input_buffer);
		
		//Grow the hunter array dynamically
        if (house.hunter_count >= hunter_capacity) {
            hunter_capacity = (hunter_capacity == 0) ? 1 : hunter_capacity * 2;
            struct Hunter* temp = realloc(house.hunters, hunter_capacity * sizeof(struct Hunter));
            if (!temp) {
                fprintf(stderr, "Memory allocation failed for hunters.\n");
                exit(1);
            }
            house.hunters = temp;
        }

		//Pick a random device for the hunter
        const enum EvidenceType* devices;
        int num_devices = get_all_evidence_types(&devices);
        enum EvidenceType start_device = devices[rand_int_threadsafe(0, num_devices)];

		//Intialize the hunter
        hunter_init(&house.hunters[house.hunter_count], 
                    hunter_id, 
                    hunter_name, 
                    house.starting_room, 
                    start_device, 
                    &house.casefile);

        house.hunter_count++;
    }

    if (house.hunter_count == 0) {
        printf("No hunters? The ghost wins by default!\n");
        house_cleanup(&house);
        return 0;
    }

	//Create threads
    printf("\nStarting simulation with %d hunters...\n", house.hunter_count);

    pthread_create(&house.ghost.thread, NULL, ghost_thread, &house.ghost);

    for (int i = 0; i < house.hunter_count; i++) {
        pthread_create(&house.hunters[i].thread, NULL, hunter_thread, &house.hunters[i]);
    }

	//Wait for threads to finish 
    for (int i = 0; i < house.hunter_count; i++) {
        pthread_join(house.hunters[i].thread, NULL);
    }
    
    pthread_join(house.ghost.thread, NULL);

	//Prints result
    printf("\n--- Simulation Results ---\n");
    printf("Actual Ghost: %s\n", ghost_to_string(house.ghost.type));

    printf("\nHunter Results:\n");
    bool hunters_won = false;
    for (int i = 0; i < house.hunter_count; i++) {
        struct Hunter* h = &house.hunters[i];
        printf("- %s: ", h->name);
        if (h->exit_reason == LR_EVIDENCE) {
            printf("ESCAPED (Collected enough evidence)\n");
            hunters_won = true;
        } else if (h->exit_reason == LR_BORED) {
            printf("LEFT (Bored)\n");
        } else if (h->exit_reason == LR_AFRAID) {
            printf("FLED (Scared)\n");
        } else {
            printf("UNKNOWN (Did not exit properly)\n");
        }
        hunter_cleanup(h); 
    }

    printf("\nEvidence Collected: ");
    const enum EvidenceType* ev_list;
    int ev_count = get_all_evidence_types(&ev_list);
    for (int i = 0; i < ev_count; i++) {
        if (house.casefile.collected & ev_list[i]) {
            printf("[%s] ", evidence_to_string(ev_list[i]));
        }
    }
    printf("\n");

    if (hunters_won) {
        printf("Identified Ghost: %s\n", ghost_to_string((enum GhostType)house.casefile.collected));
        printf("The Hunters Win!\n");
    } else {
        printf("The Hunters Failed!\n");
    }

	//Clean up
    house_cleanup(&house);
    return 0;
}