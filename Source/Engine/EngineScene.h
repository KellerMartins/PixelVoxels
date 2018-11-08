#ifndef ENGINESCENE_H
#define ENGINESCENE_H

#include "../Libs/cJSON.h"
#include "../utils.h"
#include "EngineECS.h"

typedef struct engineScene{
    Trie data;
    
    char filename[FILENAME_MAX];
    char path[PATH_MAX];
}engineScene;


int ExportScene(char path[], char name[]);
int LoadScene(char path[], char name[]);
int LoadSceneAdditive(char path[], char name[], int loadData);

#endif