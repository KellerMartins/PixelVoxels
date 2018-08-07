#include "EngineScene.h"

/////////////////////////////////External data//////////////////////////////////

//From EngineECS.c
extern engineECS ECS;

////////////////////////////////////////////////////////////////////////////////

engineScene Scene;

int ExportScene(char path[], char name[]){

	cJSON *sceneObj = cJSON_CreateObject();
	
	//Data encoding
	cJSON *dataObject = cJSON_AddObjectToObject(sceneObj, "data");
	cJSON_AddItemToObject(dataObject, "sunDirection", JSON_CreateVector3(GetTrieElementAs_Vector3(Scene.data, "sunDirection", (Vector3){1,0,-1})));
	cJSON_AddItemToObject(dataObject, "sunColor", JSON_CreateVector3(GetTrieElementAs_Vector3(Scene.data, "sunColor", (Vector3){1,1,1})));
	cJSON_AddItemToObject(dataObject, "backgroundColor", JSON_CreateVector3(GetTrieElementAs_Vector3(Scene.data, "backgroundColor", (Vector3){0.05,0.05,0.05})));

	//Entities encoding
	cJSON *entitiesArray = cJSON_AddArrayToObject(sceneObj, "entities");

	int i;
	for(i=0;i<=ECS.maxUsedIndex;i++){
		if(IsValidEntity(i) && !EntityIsChild(i)){
			cJSON_AddItemToArray(entitiesArray, EncodeEntity(i,0));
		}
	}

	char fullPath[512+256];
    strncpy(fullPath,path,512);
    if(path[strlen(path)-1] != '/'){
        strcat(fullPath,"/");
    }
    strcat(fullPath,name);
	strcat(fullPath,".scene");
    PrintLog(Info,"Saving scene: (%s)\n",fullPath);
    FILE* file = fopen(fullPath,"w");

	if(file){
		char *jsonString = cJSON_Print(sceneObj);
		fprintf(file,"%s",jsonString);
		free(jsonString);
		fclose(file);
		return 1;
	}else{
		PrintLog(Error,"ExportScene: Failed to create/open json file!\n");
		return 0;
	}
	
	cJSON_Delete(sceneObj);
}

int LoadScene(char path[], char name[]){
    strncpy(Scene.path, path, PATH_MAX);
    strncpy(Scene.filename, name, FILENAME_MAX);
    FreeTrie(&Scene.data);

	int i;
	for(i=0;i<=ECS.maxUsedIndex;i++){
		if(IsValidEntity(i)){
			DestroyEntity(i);
		}
	}
	return LoadSceneAdditive(path, name, 1);
}

int LoadSceneAdditive(char path[], char name[], int loadData){
	cJSON *sceneObj = OpenJSON(path, name);
	if(sceneObj){
		cJSON *entityArray = cJSON_GetObjectItemCaseSensitive(sceneObj, "entities");
		cJSON *entityObj = NULL;
		cJSON_ArrayForEach(entityObj, entityArray){	
			//Entity construction
			DecodeEntity(&entityObj);
		}

		if(loadData){
			cJSON *dataArray = cJSON_GetObjectItemCaseSensitive(sceneObj, "data");
			cJSON *value = NULL;
			cJSON_ArrayForEach(value, dataArray){	
				const char *key = value->string;

				if(cJSON_IsArray(value) && cJSON_GetArraySize(value) == 3){
					Vector3 vec = {(cJSON_GetArrayItem(value,0))->valuedouble,
								   (cJSON_GetArrayItem(value,1))->valuedouble,
						    	   (cJSON_GetArrayItem(value,2))->valuedouble};
					InsertTrie_Vector3(&Scene.data, key, vec);	
				}
				
				else if(cJSON_IsNumber(value)){
					double num = value->valuedouble;
					InsertTrie_double(&Scene.data, key, num);
				}
				
				else if(cJSON_IsBool(value)){
					int b = cJSON_IsTrue(value);
					InsertTrie_int(&Scene.data, key, b);
				}
				
				else if(cJSON_IsString(value)){
					char* str = value->valuestring;
					InsertTrieString(&Scene.data, key, str);
				}
			}
		}

		cJSON_Delete(sceneObj);
		return 1;
	}
	return 0;
}