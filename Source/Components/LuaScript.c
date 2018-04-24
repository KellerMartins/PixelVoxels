#include "LuaScript.h"

//Function that returns the ID of this component
static ComponentID ThisComponentID(){
    static ComponentID CompID = -1;
    if(CompID<0)
        CompID = GetComponentID("LuaScript");
    
    return CompID;
}

extern engineECS ECS;

typedef struct LuaScript{
    char scriptPath[512];
	char scriptName[256];
    int index;
}LuaScript;


//Runs on AddComponentToEntity
void LuaScriptConstructor(void** data){
    if(!data) return;
    *data = calloc(1,sizeof(LuaScript));
    ((LuaScript*)*data)->index = -1;
}


//Runs on RemoveComponentFromEntity
void LuaScriptDestructor(void** data){
    if(!data) return;
    free(*data);
    *data = NULL;
}


//Runs on DuplicateEntity
void* LuaScriptCopy(void* data){
    if(!data) return NULL;
    LuaScript *newLuaScript = malloc(sizeof(LuaScript));
    memcpy(newLuaScript,data,sizeof(LuaScript));
	return newLuaScript;
}


//Runs on EncodeEntity
cJSON* LuaScriptEncode(void** data, cJSON* currentData){
    if(!data) return NULL;
    LuaScript *ls = *data; 

    int hasChanged = 0;
    if(currentData){
        //Check if any data has changed 
        if(!StringCompareEqual(ls->scriptName, cJSON_GetObjectItem(currentData, "scriptName")->valuestring)){
            hasChanged = 1;
        }
        if(!StringCompareEqual(ls->scriptPath, cJSON_GetObjectItem(currentData, "scriptPath")->valuestring)){
            hasChanged = 1;
        }
    }

    //Encode this component if its not from a prefab (who has currentData) or if it has changed
    if(!currentData || hasChanged){
        cJSON *obj = cJSON_CreateObject();

        cJSON_AddStringToObject(obj, "scriptName", ls->scriptName);
        cJSON_AddStringToObject(obj, "scriptPath", ls->scriptPath);

        return obj;
    }
    return NULL;
}


//Runs on DecodeEntity
void* LuaScriptDecode(cJSON **data){
    LuaScript *ls = malloc(sizeof(LuaScript));

    ls->index = -1;
    strcpy(ls->scriptName,cJSON_GetObjectItem(*data, "scriptName")->valuestring);
    strcpy(ls->scriptPath, cJSON_GetObjectItem(*data, "scriptPath")->valuestring);

    return ls;
}

void SetLuaScript(EntityID entity, char* scriptPath, char* scriptName){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("SetLuaScript: Entity doesn't have a LuaScript component. (%d)\n",entity);
        return;
    }

    LuaScript *ls = (LuaScript *)ECS.Components[ThisComponentID()][entity].data;
    strcpy(ls->scriptName, scriptName);
    strcpy(ls->scriptPath, scriptPath);
    ls->index = -1;
}

int GetLuaScriptIndex(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("GetLuaScriptIndex: Entity doesn't have a LuaScript component. (%d)\n",entity);
        return -1;
    }

    return ((LuaScript *)ECS.Components[ThisComponentID()][entity].data)->index;
}

void SetLuaScriptIndex(EntityID entity, int index){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("SetLuaScriptIndex: Entity doesn't have a LuaScript component. (%d)\n",entity);
        return;
    }

    ((LuaScript *)ECS.Components[ThisComponentID()][entity].data)->index = index;
}

char *GetLuaScriptName(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("GetLuaScriptName: Entity doesn't have a LuaScript component. (%d)\n",entity);
        return NULL;
    }

    return ((LuaScript *)ECS.Components[ThisComponentID()][entity].data)->scriptName;
}

char *GetLuaScriptPath(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("GetLuaScriptPath: Entity doesn't have a LuaScript component. (%d)\n",entity);
        return NULL;
    }

    return ((LuaScript *)ECS.Components[ThisComponentID()][entity].data)->scriptPath;
}