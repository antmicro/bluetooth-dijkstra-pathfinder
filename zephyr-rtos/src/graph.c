#include "../include/graph.h"

void reset_tentative_distances(){
    for(uint8_t i = 0; i < MAX_MESH_SIZE; i++){
        if((graph + i)->reserved) (graph + i)->tentative_distance = INF;
    }
}

