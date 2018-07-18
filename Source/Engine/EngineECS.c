#include "EngineECS.h"

/////////////////////////////////External data//////////////////////////////////

//From Engine.c
extern unsigned char initializedEngine;

////////////////////////////////////////////////////////////////////////////////

engineECS ECS = {.numberOfComponents = 0, .maxEntities = 0, .maxUsedIndex = 0, .Entities = 0};
unsigned char initializedECS = 0;

//----------------- ECS Functions -----------------

//Initialize the Entity Component System lists and arrays
//Needs to be called before registering any system 
int InitECS(unsigned max_entities){
	if(initializedECS){
		PrintLog(Warning,"ECS already initialized!\n");
		return 0;
	}
	if(initializedEngine){
		PrintLog(Warning,"Engine already initialized! Initialize and configure ECS before initializing the engine!\n");
		return 0;
	}

	ECS.maxEntities = max_entities;
	ECS.Entities = calloc(max_entities,sizeof(Entity));
	ECS.AvaliableEntitiesIndexes = InitList(sizeof(int));

	//Populate avaliable entities with all indexes
	int i;
	for(i=0;i<max_entities;i++){
		InsertListEnd(&ECS.AvaliableEntitiesIndexes,(void*)&i);
	}	

	ECS.SystemList = InitList(sizeof(System));

	initializedECS = 1;
	return 1;
}

int RegisterNewComponent(char componentName[25],void (*constructorFunc)(void** data),void (*destructorFunc)(void** data),void*(*copyFunc)(void*),cJSON*(*encodeFunc)(void** data, cJSON* currentData),void* (*decodeFunc)(cJSON** data)){
	if(!ECS.Components){
		ECS.Components = malloc(sizeof(Component*));
		ECS.Components[0] = calloc(ECS.maxEntities,sizeof(Component));
	}else{
		ECS.Components = realloc(ECS.Components,(ECS.numberOfComponents + 1) * sizeof(Component*));
		ECS.Components[ECS.numberOfComponents] = calloc(ECS.maxEntities,sizeof(Component));
	}

	ComponentType newType;
	strncpy(newType.name,componentName,25);
	newType.constructor = constructorFunc;
	newType.destructor = destructorFunc;
	newType.copy = copyFunc;
	newType.encode = encodeFunc;
	newType.decode = decodeFunc;

	if(!ECS.ComponentTypes){
		ECS.ComponentTypes = malloc(sizeof(ComponentType));
		ECS.ComponentTypes[0] = newType;
	}else{
		ECS.ComponentTypes = realloc(ECS.ComponentTypes,(ECS.numberOfComponents + 1) * sizeof(ComponentType));
		ECS.ComponentTypes[ECS.numberOfComponents] = newType;
	}
	ECS.numberOfComponents++;

	return ECS.numberOfComponents-1;
}

int RegisterNewSystem(char systemName[25], int priority, ComponentMask required, ComponentMask excluded, void (*initFunc)(), void (*updateFunc)(), void (*freeFunc)()){
	if(!initializedECS){
		PrintLog(Warning,"ECS not initialized! Initialize ECS before registering the systems\n");
		return -1;
	}
	System newSystem = (System){.priority = priority, .enabled = 1, .required = required, .excluded = excluded, .systemInit = initFunc, .systemUpdate = updateFunc, .systemFree = freeFunc};
	strncpy(newSystem.name,systemName,25);

	ListCellPointer current = GetLastCell(ECS.SystemList);
	unsigned index = GetLength(ECS.SystemList);
	while(current){
		System *curSystem = (System*)GetElement(*current);
		if(curSystem->priority >= priority) break;

		index--;
		current = GetPreviousCell(current);
	}
	
	InsertListIndex(&ECS.SystemList,(void*)&newSystem,index<0? 0: index);
	return GetLength(ECS.SystemList)-1;
}




//----------------- Component functions -----------------
ComponentID GetComponentID(char componentName[25]){
	ComponentID c;
	for(c=0; c<ECS.numberOfComponents; c++){
		if(StringCompareEqual(ECS.ComponentTypes[c].name, componentName)) return c;
	}
	PrintLog(Warning,"GetComponentID: Component named %s not found\n",componentName);
	return -1;
}

//Receives components name strings[25] and return a ComponentMask containing these components
ComponentMask CreateComponentMaskByName(int numComp, ...){
	ComponentMask newMask = {0};

	va_list args;
	va_start(args, numComp);
	int i;
	for(i=0;i<numComp;i++){
		int index = GetComponentID(va_arg(args,char *));
		if(index<0 || index>31){
			PrintLog(Warning,"CreateComponentMaskByName: Component index out of range! (%d)\n",index);
			continue;
		}

		newMask.mask |= 1<<index;
	}
	va_end(args);

	return newMask;
}

//Receives components index ints and return a ComponentMask containing these components
ComponentMask CreateComponentMaskByID(int numComp, ...){
	ComponentMask newMask = {0};

	va_list args;
	va_start(args, numComp);
	int i;
	for(i=0;i<numComp;i++){
		int index = va_arg(args,int);
		if(index<0 || index>31){
			PrintLog(Warning,"CreateComponentMaskByID: Component index out of range! (%d)\n",index);
			continue;
		}

		newMask.mask |= 1<<index;
	}
	va_end(args);

	return newMask;
}





//----------------- Entity functions -----------------
EntityID CreateEntity(){
	int *avaliableIndex = (int *)GetFirstElement(ECS.AvaliableEntitiesIndexes);
	if(avaliableIndex){
		int index = *avaliableIndex;
		if(index>ECS.maxUsedIndex) ECS.maxUsedIndex = index;
		//Clear the entity before returning
		ECS.Entities[index].mask = CreateComponentMaskByID(0);
		ECS.Entities[index].isSpawned = 1;
		ECS.Entities[index].childs = InitList(sizeof(EntityID));

		RemoveListStart(&ECS.AvaliableEntitiesIndexes);
		return index;
	}
	PrintLog(Error,"CreateEntity: No entity avaliable to spawn! (Max %ud)\n",ECS.maxEntities);
	return -1;
}

void DestroyEntity(EntityID entity){
	if(!IsValidEntity(entity)){
		PrintLog(Warning,"DestroyEntity: Entity is not spawned or out of range!(%d)\n",entity);
		return;
	}

	if(EntityIsParent(entity)){
		ListCellPointer childCell;
		ListForEach(childCell,ECS.Entities[entity].childs){
			DestroyEntity(GetElementAsType(childCell, EntityID));
		}
	}

	int mask = ECS.Entities[entity].mask.mask;
	ComponentID c;
	for(c=0; c<ECS.numberOfComponents; c++){
		if(mask & 1){
			ECS.ComponentTypes[c].destructor(&ECS.Components[c][entity].data);
		}
		mask >>=1;
	}

	ECS.Entities[entity].mask.mask = 0;
	ECS.Entities[entity].isSpawned = 0;
	FreeList(&ECS.Entities[entity].childs);
	ECS.Entities[entity].isChild = 0;
	ECS.Entities[entity].isParent = 0;
	ECS.Entities[entity].isPrefab = 0;
	ECS.Entities[entity].prefabPath[0] = '\0';
	ECS.Entities[entity].prefabName[0] = '\0';

	InsertListStart(&ECS.AvaliableEntitiesIndexes, (void*)&entity);
}

int IsValidEntity(EntityID entity){
	if(entity<0 || entity>=ECS.maxEntities) return 0;
	return ECS.Entities[entity].isSpawned;
}

int EntityIsPrefab(EntityID entity){
	if(entity<0 || entity>=ECS.maxEntities) return 0;
	return ECS.Entities[entity].isPrefab;
}

char *GetPrefabPath(EntityID entity){
	if(!EntityIsPrefab(entity)){
		PrintLog(Warning,"GetPrefabPath: Entity is not a prefab! (%d)", entity);
		return NULL;
	}
	return ECS.Entities[entity].prefabPath;
}
char *GetPrefabName(EntityID entity){
	if(!EntityIsPrefab(entity)){
		PrintLog(Warning,"GetPrefabName: Entity is not a prefab! (%d)", entity);
		return NULL;
	}
	return ECS.Entities[entity].prefabName;
}

void AddComponentToEntity(ComponentID component, EntityID entity){
	if(!IsValidEntity(entity)){
		PrintLog(Warning,"AddComponentToEntity: Entity is not spawned or out of range!(%d)\n",entity);
		return;
	}
	if(component<0 || component>=ECS.numberOfComponents){
		PrintLog(Warning,"AddComponentToEntity: Component index out of range!(%d)\n",component);
		return;
	}
	if(EntityContainsMask(entity, CreateComponentMaskByID(1,component))){
		PrintLog(Warning,"AddComponentToEntity: An entity can only have one of each component type! (E:%d C:%d)\n",entity, component);
		return;
	}
	

	ECS.Entities[entity].mask.mask |= 1 << component;
	
	//Call the component constructor
	ECS.ComponentTypes[component].constructor(&ECS.Components[component][entity].data);
}

void RemoveComponentFromEntity(ComponentID component, EntityID entity){
	if(!IsValidEntity(entity)){
		PrintLog(Warning,"RemoveComponentToEntity: Entity is not spawned or out of range!(%d)\n",entity);
		return;
	}
	if(component<0 || component>=ECS.numberOfComponents){
		PrintLog(Warning,"RemoveComponentToEntity: Component index out of range!(%d)\n",component);
		return;
	}
	if(!EntityContainsMask(entity, CreateComponentMaskByID(1,component))){
		PrintLog(Warning,"AddComponentToEntity: This entity doesnt have this component! (E:%d C:%d)\n",entity, component);
		return;
	}
	ECS.Entities[entity].mask.mask &= ~(1 << component);

	//Call the component destructor
	ECS.ComponentTypes[component].destructor(&ECS.Components[component][entity].data);
}

EntityID DuplicateEntity(EntityID entity){
	if(!IsValidEntity(entity)){
		PrintLog(Warning,"DuplicateEntity: Entity is not spawned or out of range!(%d)\n",entity);
		return -1;
	}

	EntityID newEntity = CreateEntity();
	
	ComponentID c;
	for(c=0; c<ECS.numberOfComponents; c++){
		if(EntityContainsComponent(entity,c)){
			ECS.Components[c][newEntity].data = ECS.ComponentTypes[c].copy(ECS.Components[c][entity].data);
		}
	}
	ECS.Entities[newEntity].mask.mask = ECS.Entities[entity].mask.mask;

	if(EntityIsParent(entity)){
		ListCellPointer childCell;
		ListForEach(childCell,ECS.Entities[entity].childs){
			EntityID newChild = DuplicateEntity(GetElementAsType(childCell, EntityID));
			SetEntityParent(newChild, newEntity);
		}
	}

	if(EntityIsChild(entity)){
		SetEntityParent(newEntity, GetEntityParent(entity));
	}

	if(EntityIsPrefab(entity)){
		ECS.Entities[newEntity].isPrefab = 1;
		strncpy(ECS.Entities[newEntity].prefabPath, ECS.Entities[entity].prefabPath, 4096);
		strncpy(ECS.Entities[newEntity].prefabName, ECS.Entities[entity].prefabName, 256);
	}

	return newEntity;
}


ComponentMask GetEntityComponents(EntityID entity){
	if(!IsValidEntity(entity)){
		PrintLog(Warning,"GetEntityComponents: Entity is not spawned or out of range!(%d)\n",entity);
		return (ComponentMask){0};
	}
	return ECS.Entities[entity].mask;
}

int IsEmptyComponentMask(ComponentMask mask){
	return mask.mask? 0:1;
}

int EntityContainsMask(EntityID entity, ComponentMask mask){
	if(!mask.mask) return 0;
	return (ECS.Entities[entity].mask.mask & mask.mask) == mask.mask;
}

int EntityContainsComponent(EntityID entity, ComponentID component){
	if(!IsValidEntity(entity)){
		PrintLog(Warning,"EntityContainsComponent: Entity is not spawned or out of range!(%d)\n",entity);
		return 0;
	}
	if(component<0 || component>ECS.numberOfComponents){
		PrintLog(Warning,"EntityContainsComponent: Component index out of range!(%d)\n",component);
		return 0;
	}
	unsigned long compMask = 1<<component;
	return (ECS.Entities[entity].mask.mask & compMask) == compMask;
}

int MaskContainsComponent(ComponentMask mask, ComponentID component){
	if(component<0 || component>ECS.numberOfComponents){
		PrintLog(Warning,"MaskContainsComponent: Component index out of range!(%d)\n",component);
		return 0;
	}

	return mask.mask>>component & 1;
}

ComponentMask IntersectComponentMasks(ComponentMask mask1, ComponentMask mask2){
	return (ComponentMask){mask1.mask & mask2.mask};
}





//----------------- Parenting functions -----------------
int EntityIsParent(EntityID entity){
    if(!IsValidEntity(entity)){
        PrintLog(Warning,"EntityIsParent: Entity is not spawned or out of range!(%d)\n",entity);
        return 0;
    }
    return ECS.Entities[entity].isParent;
}

int EntityIsChild(EntityID entity){
    if(!IsValidEntity(entity)){
        PrintLog(Warning,"EntityIsChild: Entity is not spawned or out of range!(%d)\n",entity);
        return 0;
    }
    return ECS.Entities[entity].isChild;
}

void SetEntityParent(EntityID child, EntityID parent){
    if(child == parent){
        PrintLog(Warning,"SetEntityParent: Child and parent can't be the same! (%d) (%d)\n",child,parent);
        return;
    }
	if(!IsValidEntity(child)){
        PrintLog(Warning,"EntityIsParent: Child is not spawned or out of range!(%d)\n",child);
        return;
    }
	if(!IsValidEntity(parent)){
        PrintLog(Warning,"EntityIsParent: Parent is not spawned or out of range!(%d)\n",parent);
        return;
    }

    //If is already a child of another parent, remove it first
    if(EntityIsChild(child)){
        UnsetParent(child);
    }

    InsertListEnd(&ECS.Entities[parent].childs, &child);
    ECS.Entities[child].parent = parent;

    ECS.Entities[child].isChild = 1;
    ECS.Entities[parent].isParent = 1;
}

EntityID GetEntityParent(EntityID entity){
	if(!IsValidEntity(entity)){
        PrintLog(Warning,"GetEntityParent: Entity is not spawned or out of range!(%d)\n",entity);
        return 0;
    }
    return ECS.Entities[entity].isChild? ECS.Entities[entity].parent:-1;
}

List* GetChildsList(EntityID parent){
	if(!IsValidEntity(parent)){
        PrintLog(Warning,"GetChildsList: Parent is not spawned or out of range!(%d)\n",parent);
        return NULL;
    }
    return ECS.Entities[parent].isParent? &ECS.Entities[parent].childs : NULL;
}

int UnsetParent(EntityID child){
	if(!IsValidEntity(child)){
        PrintLog(Warning,"UnsetParent: Child is not spawned or out of range!(%d)\n",child);
        return 0;
    }
    if(!EntityIsChild(child)){
        PrintLog(Warning,"UnsetParent: Entity is not anyone's child. (%d)\n",child);
        return 0;
    }

    EntityID parentID = GetEntityParent(child);

    //Find the index of the child in the list of childs
    int index = 0;
    ListCellPointer current = GetFirstCell(ECS.Entities[parentID].childs);
    while(current){
        EntityID cID = *((EntityID*) GetElement(*current));

        //If found
        if(cID == child){
            RemoveListIndex(&ECS.Entities[parentID].childs,index);

            ECS.Entities[child].isChild = 0;

            //If his old parent has an empty list of childs, set isParent to zero
            if(IsListEmpty(ECS.Entities[parentID].childs)){
                ECS.Entities[parentID].isParent = 0;
            }

            return 1;
        }
        current = GetNextCell(current);
        index++;
    }

    //Return zero if can't find the child's index in the list (Indicative of implementation error)
    PrintLog(Error,"RemoveChild: Child is not an parent's child (Shouldn't happen). (P:%d  C:%d)\n",parentID,child);
    return 0;
}





//----------------- Import/Export Scenes and Entities functions -----------------

//Warning: the encodingToPrefab flag is only valid for the first caller of this function,
//as if you are encoding to prefab, you will only encode the full data of the first, and the childs will still
//encode as normal prefabs (prefab path, name and changes) or normal objects

//Limitations:
//	* When not encoding to prefab (like when saving a scene with a prefab), only the local changes made to the first parent
//    are saved. Ex: Saving a scene with a character prefab positioned and with his child weapon translated, the translation of
//    the weapon will not be saved on the scene. But if you save the character prefab with his child weapon translated before saving
//    the scene, the translation will not be discarted

cJSON *EncodeEntity(EntityID entity,int encodingToPrefab){
	cJSON *entityObj = cJSON_CreateObject();
	cJSON *currentData = NULL;

	//When encoding to a scene or to a child of a prefab json, encode the path to this prefab and the changes made to the prefab
	if(EntityIsPrefab(entity) && !encodingToPrefab){
		cJSON_AddStringToObject(entityObj, "prefabPath", ECS.Entities[entity].prefabPath);
		cJSON_AddStringToObject(entityObj, "prefabName", ECS.Entities[entity].prefabName);

		currentData = OpenJSON(ECS.Entities[entity].prefabPath, ECS.Entities[entity].prefabName);
	}

	ComponentID c;
	for(c=0; c<ECS.numberOfComponents; c++){
		if(EntityContainsComponent(entity, c)){
			//Get current json data if it exists
			cJSON *currentCompJSON = NULL;
			if(currentData)
				currentCompJSON = cJSON_GetObjectItemCaseSensitive(currentData, ECS.ComponentTypes[c].name);

			cJSON *compEncoded = ECS.ComponentTypes[c].encode(&ECS.Components[c][entity].data,currentCompJSON);
			if(compEncoded){
				cJSON_AddItemToObject(entityObj, ECS.ComponentTypes[c].name, compEncoded);
			}
		}
	}

	if(currentData)
		cJSON_Delete(currentData);

	if(EntityIsParent(entity) && ((EntityIsPrefab(entity) && encodingToPrefab) || !EntityIsPrefab(entity))){

		cJSON *childsArray = cJSON_AddArrayToObject(entityObj, "childs");

		ListCellPointer childCell;
		ListForEach(childCell, *GetChildsList(entity)){
			cJSON_AddItemToArray(childsArray, EncodeEntity(GetElementAsType(childCell,EntityID),0));
		}
	}

	return entityObj;
}

EntityID DecodeEntity(cJSON **entityObj){
	EntityID newEntity;

	cJSON *prefabPath = cJSON_GetObjectItem(*entityObj,"prefabPath");
	//Entity being decoded is a prefab
	if(prefabPath){
		cJSON *prefabName = cJSON_GetObjectItem(*entityObj,"prefabName");
		newEntity = ImportEntityPrefab(prefabPath->valuestring,prefabName->valuestring);
		if(newEntity<0)
			return -1;
	}else{
		//Entity being decoded is not a prefab
		newEntity = CreateEntity();
	}

	ComponentID c;
	for(c=0; c<ECS.numberOfComponents; c++){

		cJSON *comp = cJSON_GetObjectItemCaseSensitive(*entityObj, ECS.ComponentTypes[c].name);
		if(comp){
			void *compData = ECS.ComponentTypes[c].decode(&comp);

			//In case of a prefab, replace the prefab component with the override component
			if(ECS.Components[c][newEntity].data){
				ECS.ComponentTypes[c].destructor(&ECS.Components[c][newEntity].data);
			}

			ECS.Components[c][newEntity].data = compData;
			ECS.Entities[newEntity].mask.mask |= 1<<c;
		}
	}

	cJSON *childsArray = cJSON_GetObjectItem(*entityObj, "childs");
	if(childsArray){
		cJSON *child;
		cJSON_ArrayForEach(child,childsArray){
			EntityID newChild = DecodeEntity(&child);

			if(newChild>=0)
				SetEntityParent(newChild,newEntity);
		}
	}
	return newEntity;
}

int ExportEntityPrefab(EntityID entity, char path[], char name[]){
	cJSON *entityObj = EncodeEntity(entity,1);

	char fullPath[512+256];
    strncpy(fullPath,path,512);
	//Add a '/' if the path doesnt end with one
    if(path[strlen(path)-1] != '/'){
        strcat(fullPath,"/");
    }
    strcat(fullPath,name);
	//Add the ".prefab" extension to the name if it doesn't contain it already
	if(!StringCompareEqual(&name[strlen(name)-7], ".prefab")){
		strcat(fullPath,".prefab");
	}

    PrintLog(Info,"Saving prefab: (%s)\n",fullPath);
    FILE* file = fopen(fullPath,"w");

	if(file){
		char *jsonString = cJSON_Print(entityObj);
		fprintf(file,"%s",jsonString);
		free(jsonString);
		fclose(file);

		ECS.Entities[entity].isPrefab = 1;
		strncpy(ECS.Entities[entity].prefabPath,path,512);
		strncpy(ECS.Entities[entity].prefabName,name,256);
		if(!StringCompareEqual(&name[strlen(name)-7], ".prefab")){
			strcat(ECS.Entities[entity].prefabName,".prefab");
		}

		return 1;
	}else{
		PrintLog(Error,"ExportEntityPrefab: Failed to create/open json file!\n");
		return 0;
	}
	
	cJSON_Delete(entityObj);
}

EntityID ImportEntityPrefab(char path[], char name[]){
	cJSON *entityObj = OpenJSON(path, name);
	if(entityObj){
		EntityID newEntity = DecodeEntity(&entityObj);

		if(newEntity<0)
			return -1;

		ECS.Entities[newEntity].isPrefab = 1;
		strcpy(ECS.Entities[newEntity].prefabPath,path);
		strcpy(ECS.Entities[newEntity].prefabName,name);

		cJSON_Delete(entityObj);
		return newEntity;
	}
	return -1;
}

int ExportScene(char path[], char name[]){

	cJSON *sceneObj = cJSON_CreateObject();
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
	int i;
	for(i=0;i<=ECS.maxUsedIndex;i++){
		if(IsValidEntity(i)){
			DestroyEntity(i);
		}
	}
	return LoadSceneAdditive(path,name);
}

int LoadSceneAdditive(char path[], char name[]){
	cJSON *sceneObj = OpenJSON(path, name);
	if(sceneObj){
		cJSON *entityArray = cJSON_GetObjectItemCaseSensitive(sceneObj, "entities");
		cJSON *entityObj = NULL;
		cJSON_ArrayForEach(entityObj, entityArray){	
			//Entity construction
			DecodeEntity(&entityObj);
		}

		cJSON_Delete(sceneObj);
		return 1;
	}
	return 0;
}





//----------------- System functions -----------------
SystemID GetSystemID(char systemName[25]){
	int index = 0;
	ListCellPointer current;
	ListForEach(current,ECS.SystemList){
		System currSys = GetElementAsType(current,System);
		if(StringCompareEqual(currSys.name, systemName)) return index;	
		index++;
	}
	return -1;
}

void EnableSystem(SystemID system){
	ListCellPointer sys = GetCellAt(ECS.SystemList,system);
	GetElementAsType(sys,System).enabled = 1;
}

void DisableSystem(SystemID system){
	ListCellPointer sys = GetCellAt(ECS.SystemList,system);
	GetElementAsType(sys,System).enabled = 0;
}

int IsSystemEnabled(SystemID system){
	ListCellPointer sys = GetCellAt(ECS.SystemList,system);
	return GetElementAsType(sys,System).enabled;
}