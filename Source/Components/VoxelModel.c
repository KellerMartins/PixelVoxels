#include "VoxelModel.h"

//Function that returns the ID of this component
static ComponentID ThisComponentID(){
    static ComponentID CompID = -1;
    if(CompID<0)
        CompID = GetComponentID("VoxelModel");
    
    return CompID;
}

extern engineCore Core;
extern engineECS ECS;
extern engineRendering Rendering;

Vector3 DecodeMagicaVoxelRotation(int rot);

void InternalLoadVoxelModel(VoxelModel **modelPointer, char modelPath[], char modelName[]);
void InternalLoadMultiVoxelModelObject(VoxelModel **modelPointer, char modelPath[], char modelName[], char objectName[]);

void VoxelModelConstructor(void** data){
    if(!data) return;
    *data = calloc(1,sizeof(VoxelModel));
    VoxelModel *obj = (VoxelModel*)*data;
    glGenBuffers(3, obj->vbo); 
}
void VoxelModelDestructor(void** data){
    if(!data) return;
    VoxelModel *obj = *data;

    free(obj->model);
    glDeleteBuffers(3, obj->vbo);

    free(*data);
    *data = NULL;
}

void* VoxelModelCopy(void* data){
    if(!data) return NULL;

    VoxelModel *newVoxelModel = malloc(sizeof(VoxelModel));
    memcpy(newVoxelModel,data,sizeof(VoxelModel));
    newVoxelModel->dimension[0] = ((VoxelModel*)data)->dimension[0];
    newVoxelModel->dimension[1] = ((VoxelModel*)data)->dimension[1];
    newVoxelModel->dimension[2] = ((VoxelModel*)data)->dimension[2];

    glGenBuffers(3, newVoxelModel->vbo); 

    if(((VoxelModel*)data)->model){

        //Copy model and lighting data
        int sizeModel = newVoxelModel->dimension[0] * newVoxelModel->dimension[1] * newVoxelModel->dimension[2] * sizeof(unsigned char);
        newVoxelModel->model = malloc(sizeModel);
        memcpy(newVoxelModel->model,((VoxelModel*)data)->model,sizeModel);

        //Copy VBO data
        int fullSize = newVoxelModel->dimension[0] * newVoxelModel->dimension[1] * newVoxelModel->dimension[2] * 3 * sizeof(GLfloat);
        int dataSize = newVoxelModel->dimension[0] * newVoxelModel->dimension[1] * newVoxelModel->dimension[2] * 3 * sizeof(GLfloat);

        glBindBuffer(GL_COPY_READ_BUFFER, ((VoxelModel*)data)->vbo[0]);
        glBindBuffer(GL_COPY_WRITE_BUFFER, newVoxelModel->vbo[0]);
        glBufferData(GL_COPY_WRITE_BUFFER, fullSize, NULL, GL_DYNAMIC_DRAW);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, dataSize);
        
        glBindBuffer(GL_COPY_READ_BUFFER, ((VoxelModel*)data)->vbo[1]);
        glBindBuffer(GL_COPY_WRITE_BUFFER, newVoxelModel->vbo[1]);
        glBufferData(GL_COPY_WRITE_BUFFER, fullSize, NULL, GL_DYNAMIC_DRAW);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, dataSize);
        
        glBindBuffer(GL_COPY_READ_BUFFER, ((VoxelModel*)data)->vbo[2]);
        glBindBuffer(GL_COPY_WRITE_BUFFER, newVoxelModel->vbo[2]);
        glBufferData(GL_COPY_WRITE_BUFFER, fullSize, NULL, GL_DYNAMIC_DRAW);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, dataSize);
        
        }else{
        newVoxelModel->model = NULL;
        newVoxelModel->numberOfVertices = 0;
    }

	return newVoxelModel;
}

cJSON* VoxelModelEncode(void** data, cJSON* currentData){
    if(!data) return NULL;

    VoxelModel *v = *data; 

    int hasChanged = 0;
    if(currentData){
        //Check if any data has changed
        cJSON *curCenter = cJSON_GetObjectItem(currentData,"center");
        if(v->center.x != (cJSON_GetArrayItem(curCenter,0))->valuedouble ||
           v->center.y != (cJSON_GetArrayItem(curCenter,1))->valuedouble || 
           v->center.z != (cJSON_GetArrayItem(curCenter,2))->valuedouble)
        {
            hasChanged = 1;
        }

        cJSON *curScale = cJSON_GetObjectItem(currentData,"smallScale");
        if(curScale && v->smallScale != cJSON_IsTrue(curScale)){
            hasChanged = 1;
        }

        cJSON *curObjName = cJSON_GetObjectItem(currentData, "objectName");
        if(curObjName && v->objectName[0] == '\0'){
            hasChanged = 1;
        }

        if(!StringCompareEqual(v->modelPath, cJSON_GetObjectItem(currentData, "modelPath")->valuestring) || 
           !StringCompareEqual(v->modelName, cJSON_GetObjectItem(currentData, "modelName")->valuestring))
        {
            hasChanged = 1;
        }
    }

    //Encode this component if its not from a prefab (who has currentData) or if it has changed
    if(!currentData || hasChanged){
        cJSON *obj = cJSON_CreateObject();

        cJSON_AddStringToObject(obj, "modelPath", v->modelPath);
        cJSON_AddStringToObject(obj, "modelName", v->modelName);
        if(v->objectName[0] != '\0')
            cJSON_AddStringToObject(obj, "objectName", v->objectName);

        cJSON_AddBoolToObject(obj,"smallScale",v->smallScale);

        cJSON *center = cJSON_AddArrayToObject(obj,"center");
        cJSON_AddItemToArray(center, cJSON_CreateNumber(v->center.x));
        cJSON_AddItemToArray(center, cJSON_CreateNumber(v->center.y));
        cJSON_AddItemToArray(center, cJSON_CreateNumber(v->center.z));

        return obj;
    }
    return NULL;
}

void* VoxelModelDecode(cJSON **data){
    VoxelModel *v;
    VoxelModelConstructor((void**)(&v));

    cJSON *objName = cJSON_GetObjectItem(*data, "objectName");
    if(objName){
        InternalLoadMultiVoxelModelObject(&v, cJSON_GetObjectItem(*data, "modelPath")->valuestring, cJSON_GetObjectItem(*data, "modelName")->valuestring, objName->valuestring);
    }else{
        InternalLoadVoxelModel(&v, cJSON_GetObjectItem(*data, "modelPath")->valuestring, cJSON_GetObjectItem(*data, "modelName")->valuestring);
    }

    cJSON *center = cJSON_GetObjectItem(*data,"center");
    v->center = (Vector3){(cJSON_GetArrayItem(center,0))->valuedouble,
                          (cJSON_GetArrayItem(center,1))->valuedouble,
                          (cJSON_GetArrayItem(center,2))->valuedouble};
    
    cJSON *small = cJSON_GetObjectItem(*data,"smallScale");
    if(small){
        v->smallScale = cJSON_IsTrue(small);
    }else{
        v->smallScale = 0;
    }
    return v;
}




VoxelModel* GetVoxelModelPointer(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        PrintLog(Warning,"GetVoxelModelPointer: Entity doesn't have a VoxelModel component. (%d)\n",entity);
        return NULL;
    }
    return (VoxelModel *) ECS.Components[ThisComponentID()][entity].data;
}

Vector3 GetVoxelModelCenter(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        PrintLog(Warning,"GetVoxelModelCenter: Entity doesn't have a VoxelModel component. (%d)\n",entity);
        return VECTOR3_ZERO;
    }
    VoxelModel *m = GetVoxelModelPointer(entity);
    return m->center;
}

void SetVoxelModelCenter(EntityID entity, Vector3 center){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        PrintLog(Warning,"SetVoxelModelCenter: Entity doesn't have a VoxelModel component. (%d)\n",entity);
        return;
    }
    VoxelModel *m = GetVoxelModelPointer(entity);
    m->center = center;
}

int IsVoxelModelEnabled(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        PrintLog(Warning,"IsVoxelModelEnabled: Entity doesn't have a VoxelModel component. (%d)\n",entity);
        return 0;
    }
    VoxelModel *m = GetVoxelModelPointer(entity);
    return m->enabled;
}

void SetVoxelModelEnabled(EntityID entity, int booleanVal){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        PrintLog(Warning,"SetVoxelModelEnabled: Entity doesn't have a VoxelModel component. (%d)\n",entity);
        return;
    }
    VoxelModel *m = GetVoxelModelPointer(entity);
    m->enabled = booleanVal? 1: 0;
}

int IsVoxelModelSmallScale(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        PrintLog(Warning,"IsVoxelModelSmallScale: Entity doesn't have a VoxelModel component. (%d)\n",entity);
        return 0;
    }
    VoxelModel *m = GetVoxelModelPointer(entity);
    return m->smallScale;
}

void SetVoxelModelSmallScale(EntityID entity, int booleanVal){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        PrintLog(Warning,"SetVoxelModelSmallScale: Entity doesn't have a VoxelModel component. (%d)\n",entity);
        return;
    }
    VoxelModel *m = GetVoxelModelPointer(entity);
    m->smallScale = booleanVal? 1: 0;
}

//MagicaVoxel Voxel structure, used only to load data
typedef struct Voxel
{
    char x;
    char y;
    char z;
    char color;
}Voxel;

//Load a single Voxel model from a .vox file.
//Receives the path to the file as a string (Ex: "path/model.vox")
void LoadVoxelModel(EntityID entity, char modelPath[], char modelName[])
{
    if(entity<0 || entity>=ECS.maxEntities){
		PrintLog(Warning,"LoadVoxelModel: Entity index out of range!(%d)\n",entity);
		return;
	}
    
    //Clear any VoxelModel component before loading
    if(EntityContainsMask(entity, CreateComponentMaskByID(1,ThisComponentID()))){
        RemoveComponentFromEntity(ThisComponentID(), entity);
    }
    AddComponentToEntity(ThisComponentID(), entity);
    
    //Add transform component if it doesnt have
    if(!EntityContainsMask(entity, CreateComponentMaskByName(1,"Transform"))){
        AddComponentToEntity(GetComponentID("Transform"), entity);
    }

    VoxelModel *obj = GetVoxelModelPointer(entity);
    InternalLoadVoxelModel(&obj, modelPath, modelName);
    
}

int IsMultiVoxelModelFile(char modelPath[], char modelName[]){
    char fullPath[512+256];
    strncpy(fullPath,modelPath,512);
    int modelPathLen = strlen(modelPath);
    if(modelPathLen > 0 && modelPath[modelPathLen-1] != '/'){
        strcat(fullPath,"/");
    }
    strcat(fullPath,modelName);
    FILE* file = fopen(fullPath,"rb");

    if(file == NULL){
	    perror("IsMultiVoxelModelFile");
        return 0;
    }

    fseek(file, 0L, SEEK_END);
    int fileLength = ftell(file);
    rewind(file);

    char *magic = (char *)calloc(5,sizeof(char));
    fread(magic,sizeof(char),4,file);

    int version;
    fread(&version,sizeof(int),1,file);

    int sizeCount = 0;
    int nTRNCount = 0;

    // a MagicaVoxel .vox file starts with a 'magic' 4 character 'VOX ' identifier
    if (strcmp("VOX ",magic) == 0)
    {
        char *chunkId = (char *)calloc(5,sizeof(char));

        while (ftell(file)<fileLength)
        {
            // each chunk has an ID, size and child chunks
            fread(chunkId,sizeof(char),4,file);

            int chunkSize;
            fread(&chunkSize,sizeof(int),1,file);

            int childChunks;
            fread(&childChunks,sizeof(int),1,file);

            if (strcmp(chunkId,"SIZE") == 0){        
                sizeCount++;
            }else if (strcmp(chunkId,"nTRN") == 0){      
                nTRNCount++;
            }
            fseek(file,chunkSize,SEEK_CUR);
        }
        free(chunkId);
    }else{
        PrintLog(Error,"IsMultiVoxelModelFile: Magic word is not \"VOX \" (%s)\n",magic);
        free(magic);
        fclose(file);
        return 0;
    }
    free(magic);
    fclose(file);

    //If there is more than one model in the file and it has per object properties (line nTRN), it is an multi model file
    return (sizeCount>1 && nTRNCount>0)? 1:0;
}

void InternalLoadMultiVoxelModelObject(VoxelModel **modelPointer, char modelPath[], char modelName[], char objectName[]){
    VoxelModel *obj = *modelPointer;

    char fullPath[512+256];
    strncpy(fullPath,modelPath,512);

    int modelPathLen = strlen(modelPath);
    if(modelPathLen > 0 && modelPath[modelPathLen-1] != '/'){
        strcat(fullPath,"/");
    }
    strcat(fullPath,modelName);
    FILE* file = fopen(fullPath,"rb");

    if(file == NULL){
	    perror("LoadMultiVoxelModelObject");
        return;
    }

    strncpy(obj->modelPath,modelPath,512);
    strncpy(obj->modelName,modelName,256);
    strncpy(obj->objectName,objectName,65);

    fseek(file, 0L, SEEK_END);
    int fileLength = ftell(file);
    rewind(file);

    char *magic = (char *)calloc(5,sizeof(char));
    fread(magic,sizeof(char),4,file);

    int version;
    fread(&version,sizeof(int),1,file);

    Voxel *voxelData = NULL;

    char curObjName[64] = "";
    
    int shapeIndex = 0;
    int objIndex = -1;
    

    // a MagicaVoxel .vox file starts with a 'magic' 4 character 'VOX ' identifier
    if (strcmp("VOX ",magic) == 0)
    {
        int found = 0;
        int isHidden = 0;
        char *chunkId = (char *)calloc(5,sizeof(char));

        //First, gather the nSHP index
        while (ftell(file)<fileLength)
        {
            // each chunk has an ID, size and child chunks
            fread(chunkId,sizeof(char),4,file);

            int chunkSize;
            fread(&chunkSize,sizeof(int),1,file);

            int childChunks;
            fread(&childChunks,sizeof(int),1,file);

            if(strcmp(chunkId,"nTRN") == 0)
            {
                int dataRead = 0;   
                int aux,editorProp;
                //0: Index in list
                fread(&aux,sizeof(int),1,file);
                dataRead+=sizeof(int);
                //1: Number of string with the editor properties to be followed
                fread(&editorProp,sizeof(int),1,file);
                dataRead+=sizeof(int);

                if(editorProp){
                    //Parse these strings and get their data
                    for(int pNum=0;pNum<editorProp;pNum++){
                        //Number of characters of the string
                        int PropertyLength1,intData;
                        fread(&PropertyLength1,sizeof(int),1,file);
                        dataRead+=sizeof(int);

                        //Allocate string and read chars
                        char *editorProperty = calloc(PropertyLength1+1 ,sizeof(char));
                        for(aux = 0; aux<PropertyLength1; aux ++){
                            fread(&editorProperty[aux],sizeof(char),1,file);
                            dataRead+=sizeof(char);
                        }
                        editorProperty[aux] = '\0';

                        //Read the following int
                        fread(&intData,sizeof(int),1,file);
                        dataRead+=sizeof(int);
                        
                        //If the string was "_name", the last int is the number of
                        //characters of the object name, so read the characters of the name
                        if(strcmp(editorProperty,"_name") == 0){                    
                            for(aux = 0; aux<intData; aux ++){
                                fread(&curObjName[aux],sizeof(char),1,file);
                                dataRead+=sizeof(char);
                            }
                            curObjName[aux] = '\0';

                            //If this object name isn't what we are looking for, jump to the next chunk
                            //If it is, set as found and continue gathering the needed data
                            if(strcmp(objectName,curObjName) != 0){
                                free(editorProperty);
                                fseek(file,chunkSize-dataRead,SEEK_CUR);
                                break;
                            }else{
                                found = 1;
                            }
                        }
                        //If the string was "_hidden", the last int is the number of
                        //characters of the object hidden status, so read the character '1' or '0'
                        else if(strcmp(editorProperty,"_hidden") == 0){
                            char hid;
                            fread(&hid,sizeof(char),1,file);
                            isHidden = (hid == '1'? 1:0);
                        }
                        free(editorProperty);
                    }
                    
                    if(found){
                        //2: Index of the nSHP corresponding to that object in the list
                        fread(&shapeIndex,sizeof(int),1,file);
                        //Stop searching
                        break;
                    }

                }else{
                    //nTRN with no data (first one)
                    fseek(file,chunkSize-dataRead,SEEK_CUR);
                }

            }else fseek(file,chunkSize,SEEK_CUR);
        }

        if(!found){
            free(chunkId);
            free(magic);
            fclose(file);
            PrintLog(Error,"LoadMultiVoxelModelObject: Object with name \"%s\" not found in file!\n",objectName);
            return;
        }

        //Go back to the start to get the model index;
        rewind(file);
        fseek(file, sizeof(char)*4 + sizeof(int) ,SEEK_CUR); // Magic word and file version
        while (ftell(file)<fileLength)
        {
            // each chunk has an ID, size and child chunks
            fread(chunkId,sizeof(char),4,file);

            int chunkSize;
            fread(&chunkSize,sizeof(int),1,file);

            int childChunks;
            fread(&childChunks,sizeof(int),1,file);

            if (strcmp(chunkId,"nSHP") == 0)
            {
                int indexInList;
                //0: Index in list
                fread(&indexInList,sizeof(int),1,file);

                if(indexInList == shapeIndex){
                    //Jump two unneeded properties
                    fseek(file, sizeof(int)*2 ,SEEK_CUR);
                    //3: Index of the model it refers to
                    fread(&objIndex,sizeof(int),1,file);

                    //Stop searching
                    break;
                }else{
                    fseek(file, chunkSize - sizeof(int),SEEK_CUR);
                }
            }else fseek(file,chunkSize,SEEK_CUR);
        }

        if(objIndex<0){
            free(chunkId);
            free(magic);
            fclose(file);
            PrintLog(Error,"LoadMultiVoxelModelObject: nSHP property not found in file!\n");
            return;
        }

        //Go back to the start to get the model;
        rewind(file);
        fseek(file, sizeof(char)*4 + sizeof(int) ,SEEK_CUR); // Magic word and file version
        int currentObj = 0;

        while (ftell(file)<fileLength)
        {
            // each chunk has an ID, size and child chunks
            fread(chunkId,sizeof(char),4,file);

            int chunkSize;
            fread(&chunkSize,sizeof(int),1,file);

            int childChunks;
            fread(&childChunks,sizeof(int),1,file);

            if (strcmp(chunkId,"SIZE") == 0)
            {        
                if(currentObj!=objIndex){ fseek(file,chunkSize,SEEK_CUR); continue; }

                fread(&obj->dimension[0],sizeof(int),1,file);
                fread(&obj->dimension[1],sizeof(int),1,file);
                fread(&obj->dimension[2],sizeof(int),1,file);
            }
            //XYZI contains the voxels position and color index
            else if (strcmp(chunkId,"XYZI") == 0)
            {
                if(currentObj!=objIndex){currentObj++; fseek(file,chunkSize,SEEK_CUR); continue;}
                

                // XYZI contains n voxels
                fread(&obj->voxelCount,sizeof(int),1,file);
                obj->voxelsRemaining = obj->voxelCount;

                voxelData = calloc(obj->voxelCount,sizeof(Voxel));
                int i;
                for(i = 0; i < obj->voxelCount; i++){
                    //Get MagicaVoxel voxel position and color index and insert in the array
                    fread(&voxelData[i].x,sizeof(char),1,file);
                    fread(&voxelData[i].y,sizeof(char),1,file);
                    fread(&voxelData[i].z,sizeof(char),1,file);
                    fread(&voxelData[i].color,sizeof(char),1,file);
                }

                //After loading all voxels into the array, allocate and transfer them to the new objects

                //Allocating memory and initializing structures
                obj->model = calloc(obj->dimension[0] * obj->dimension[1] * obj->dimension[2],sizeof(unsigned char));

                //Request VBO buffers
                glBindBuffer(GL_ARRAY_BUFFER, obj->vbo[0]);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glBufferData(GL_ARRAY_BUFFER, obj->dimension[0] * obj->dimension[1] * obj->dimension[2] * 3 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);

                glBindBuffer(GL_ARRAY_BUFFER, obj->vbo[1]);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glBufferData(GL_ARRAY_BUFFER, obj->dimension[0] * obj->dimension[1] * obj->dimension[2] * 3 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);

                glBindBuffer(GL_ARRAY_BUFFER, obj->vbo[2]);
                glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glBufferData(GL_ARRAY_BUFFER, obj->dimension[0] * obj->dimension[1] * obj->dimension[2] * 3 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);

                obj->numberOfVertices = 0;
                
                //Inserting voxels into the VoxelObject structure
                for(i = 0; i < obj->voxelCount; i++){
                    // do not store this voxel if it lies out of range of the voxel chunk (126x126x126)
                    if (voxelData[i].x > 126 || voxelData[i].y > 126 || voxelData[i].z > 126) continue;

                    // Insert data based on the index = (x + z*sizeX + y*sizeX*sizeZ)
                    int voxel = (voxelData[i].x + voxelData[i].z * obj->dimension[0] + voxelData[i].y * obj->dimension[0] * obj->dimension[2]);
                    obj->model[voxel] = voxelData[i].color;
                }

                obj->modificationStartX = 0;
                obj->modificationEndX = obj->dimension[0]-1;

                obj->modificationStartY = 0;
                obj->modificationEndY = obj->dimension[1]-1;

                obj->modificationStartZ = 0;
                obj->modificationEndZ = obj->dimension[2]-1;

                obj->enabled = !isHidden;

                //Pass to the next model
                free(voxelData);

                //After loaded the data, stop the file navigation loop
                break;
            }else fseek(file,chunkSize,SEEK_CUR);
        }
        free(chunkId);
    }else{
        PrintLog(Error,"LoadMultiVoxelModelObject: Magic word is not \"VOX \" (%s)\n",magic);
    }
    free(magic);
    fclose(file);
}

void InternalLoadVoxelModel(VoxelModel **modelPointer, char modelPath[], char modelName[]){
    VoxelModel *obj = *modelPointer;
    char fullPath[512+256];
    strncpy(fullPath,modelPath,512);
    if(modelPath[strlen(modelPath)-1] != '/'){
        strcat(fullPath,"/");
    }
    strcat(fullPath,modelName);
    PrintLog(Info,"\nLoading model: (%s)\n",fullPath);
    FILE* file = fopen(fullPath,"rb");

    if(file == NULL){
	    perror("LoadVoxelModel");
        //Return an empty VoxelObject in case of failure
        return;
    }
    strncpy(obj->modelPath,modelPath,512);
    strncpy(obj->modelName,modelName,256);
    obj->objectName[0] = '\0';

    obj->enabled = 1;

    Voxel *voxelData = NULL;

    int numVoxels = 0;
    

    //Get file length and return to start
    fseek(file, 0L, SEEK_END);
    int fileLength = ftell(file);
    PrintLog(Info,"File length: %d\n",fileLength);
    rewind(file);

    //Get magic word to see if this is a magicavoxel .vox file
    char *magic = calloc(5,sizeof(char));
    fread(magic,sizeof(char),4,file);

    //Get file version
    int version;
    fread(&version,sizeof(int),1,file);
    PrintLog(Info,"Version: %d\n",version);

    // All MagicaVoxel .vox file starts with a 'magic' 4 character 'VOX ' identifier
    if (strcmp("VOX ",magic) == 0)
    {
        int modelLoaded = 0;
        char *chunkId = calloc(5,sizeof(char));

        //While we hasnt reached the end of the file, read data
        while (ftell(file)<fileLength)
        {
            // each chunk has an ID, size and child chunks
            fread(chunkId,sizeof(char),4,file);

            int chunkSize;
            fread(&chunkSize,sizeof(int),1,file);

            int childChunks;
            fread(&childChunks,sizeof(int),1,file);

            //PrintLog(Info,"Chunk: %s\n",chunkId);

            //There are only 2 chunks we only care about, and they are SIZE and XYZI
            //SIZE chunk contains the dimensions of the model
            if (strcmp(chunkId,"SIZE") == 0)
            {        
                //Only get the model size if it hasnt loaded any
                //If it already has, just ignore the new ones
                if(!modelLoaded){

                    fread(&obj->dimension[0],sizeof(int),1,file);
                    fread(&obj->dimension[1],sizeof(int),1,file);
                    fread(&obj->dimension[2],sizeof(int),1,file);

                    PrintLog(Info,"Dimension: %d %d %d\n",obj->dimension[0],obj->dimension[1],obj->dimension[2]);

                }else{
                    PrintLog(Warning,"InternalLoadVoxelModel: Multiple voxel models found on this object, ignoring new size\n");
                    fseek(file,chunkSize,SEEK_CUR);
                }
            }
            //XYZI contains the voxels position and color index
            else if (strcmp(chunkId,"XYZI") == 0)
            {
                //Only get a voxel model if it hasnt loaded any
                //If it already has, just ignore the new ones
                if(!modelLoaded){
                    modelLoaded = 1;

                    // XYZI contains n voxels
                    fread(&numVoxels,sizeof(int),1,file);
                    PrintLog(Info,"Number of Voxels: %d\n",numVoxels);
                    
                    voxelData = calloc(numVoxels,sizeof(Voxel));

                    //Free strings and return empty Object if it fails to allocate memory
                    if(!voxelData){
                        PrintLog(Error,"InternalLoadVoxelModel: Failed to allocate voxel array!\n");
                        free(magic);
                        free(chunkId);
                        fclose(file);
                        return;
                    }
                    
                    for(int i = 0; i < numVoxels; i++){
                        //Get MagicaVoxel voxel position and color index and insert in the array
                        fread(&voxelData[i].x,sizeof(char),1,file);
                        fread(&voxelData[i].y,sizeof(char),1,file);
                        fread(&voxelData[i].z,sizeof(char),1,file);
                        fread(&voxelData[i].color,sizeof(char),1,file);
                    }

                }else{
                    PrintLog(Warning,"InternalLoadVoxelModel: Multiple voxel models found on this object, ignoring new model\n");
                    fseek(file,chunkSize,SEEK_CUR);
                }
            }
            else {fseek(file,chunkSize,SEEK_CUR);}   // read any excess bytes
        }

        //Allocating memory and initializing structures
        obj->model = calloc(obj->dimension[0] * obj->dimension[1] * obj->dimension[2],sizeof(unsigned char));

        //Request VBO buffers
        glBindBuffer(GL_ARRAY_BUFFER, obj->vbo[0]);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glBufferData(GL_ARRAY_BUFFER, obj->dimension[0] * obj->dimension[1] * obj->dimension[2] * 3 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, obj->vbo[1]);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glBufferData(GL_ARRAY_BUFFER, obj->dimension[0] * obj->dimension[1] * obj->dimension[2] * 3 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, obj->vbo[2]);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glBufferData(GL_ARRAY_BUFFER, obj->dimension[0] * obj->dimension[1] * obj->dimension[2] * 3 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);

        
        //Inserting voxels into the VoxelObject structure
        for(int i = 0; i < numVoxels; i++){
            
            // do not store this voxel if it lies out of range of the voxel chunk (126x126x126)
            if (voxelData[i].x > 126 || voxelData[i].y > 126 || voxelData[i].z > 126) continue;

            // Insert data based on the index = (x + z*sizeX + y*sizeX*sizeZ)
            int voxel = (voxelData[i].x + voxelData[i].z * obj->dimension[0] + voxelData[i].y * obj->dimension[0] * obj->dimension[2]);
            obj->model[voxel] = voxelData[i].color;
        }

        free(chunkId);
    }else{
        PrintLog(Error,"InternalLoadVoxelModel: Magic word is not 'VOX ', but '%s'\n",magic);
        free(magic);
        fclose(file);
        return;
    }
    free(magic);
    free(voxelData);

    //Final settings
    obj->numberOfVertices = 0;
    obj->voxelCount = numVoxels;
    obj->voxelsRemaining = numVoxels;

    obj->center = (Vector3){obj->dimension[0]/2,obj->dimension[1]/2,obj->dimension[2]/2};

    obj->modificationStartX = 0;
    obj->modificationEndX = obj->dimension[0]-1;

    obj->modificationStartY = 0;
    obj->modificationEndY = obj->dimension[1]-1;

    obj->modificationStartZ = 0;
    obj->modificationEndZ = obj->dimension[2]-1;

    obj->enabled = 1;

    fclose(file);

    PrintLog(Info,">DONE!\n\n");
}

//Calculate the surface voxels to be shown and put into the VBO inside the component to be rendered
int avaliableVertices[128*128*128];
Vector3 requestedVertices[128*128*128];
Vector3 requestedNormals[128*128*128];
Vector3 requestedColors[128*128*128];
void CalculateRendered(EntityID entity){
    VoxelModel *obj = GetVoxelModelPointer(entity);


    if(obj->modificationStartZ <0 || obj->modificationEndZ <0 || !obj->model){
        return;
    }

    
    glBindBuffer(GL_ARRAY_BUFFER, obj->vbo[0]);
    Vector3* vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
    glBindBuffer(GL_ARRAY_BUFFER, obj->vbo[1]);
    Vector3* colors = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    glBindBuffer(GL_ARRAY_BUFFER, obj->vbo[2]);
    Vector3* normal = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

    int avaliableCount = 0;
    int requestedCount = 0;

    int xLimitS = clamp(obj->modificationStartX-1 ,0,obj->dimension[0]-1);
    int xLimitE = clamp(obj->modificationEndX+1   ,0,obj->dimension[0]-1);
    int yLimitS = clamp(obj->modificationStartY-1 ,0,obj->dimension[1]-1);
    int yLimitE = clamp(obj->modificationEndY+1   ,0,obj->dimension[1]-1);
    int zLimitS = clamp(obj->modificationStartZ-1 ,0,obj->dimension[2]-1);
    int zLimitE = clamp(obj->modificationEndZ+1   ,0,obj->dimension[2]-1);


    int i;
    for(i=0;i<obj->numberOfVertices;i++){
        if(vertices[i].x<0 || vertices[i].y<0 || vertices[i].z<0){
            avaliableVertices[avaliableCount++] = i;
            continue;
        }
        if((vertices[i].x<xLimitS || vertices[i].x>xLimitE || vertices[i].y<yLimitS || vertices[i].y>yLimitE || vertices[i].z<zLimitS || vertices[i].z>zLimitE))
            continue;
        else
            avaliableVertices[avaliableCount++] = i;        
    }
    
    int x,y,z;
        for(y = yLimitS; y<=yLimitE; y++){
        for(z = zLimitS; z<=zLimitE; z++){
            for(x = xLimitS; x<=xLimitE; x++){
                
                int index = (x + z * obj->dimension[0] + y * obj->dimension[0] * obj->dimension[2]);
                if(obj->model[index]!=0){
                        int occ = 0;
                        int neighIndex;
                        Vector3 vNormal = VECTOR3_ZERO;

                        //Calculate normal vector as the average direction without neighbors
                        if(x-1 < 0){
                            vNormal.x -= 1;
                        }else{
                            neighIndex = (x-1) + (z) * obj->dimension[0] + (y) * obj->dimension[0] * obj->dimension[2];
                            if(obj->model[neighIndex]==0){
                                vNormal.x -= 1;
                            }else{
                                occ++;
                            }
                        }

                        if(x+1 >= obj->dimension[0]){
                                vNormal.x += 1;
                        }else{
                            neighIndex = (x+1) + (z) * obj->dimension[0] + (y) * obj->dimension[0] * obj->dimension[2];
                            if(obj->model[neighIndex]==0){
                                vNormal.x += 1;
                            }else{
                                occ++;
                            }
                        }

                        if(z-1 < 0){
                            vNormal.z -= 1;
                        }else{
                            neighIndex = (x) + (z-1) * obj->dimension[0] + (y) * obj->dimension[0] * obj->dimension[2];
                            if(obj->model[neighIndex]==0){
                                vNormal.z -= 1;
                            }else{
                                occ++;
                            }
                        }

                        if(z+1 >= obj->dimension[2]){
                            vNormal.z += 1;
                        }else{
                            neighIndex = (x) + (z+1) * obj->dimension[0] + (y) * obj->dimension[0] * obj->dimension[2];
                            if(obj->model[neighIndex]==0){
                                vNormal.z += 1;
                            }else{
                                occ++;
                            }
                        }

                        if(y-1 < 0){
                            vNormal.y -= 1;
                        }else{
                            neighIndex = (x) + (z) * obj->dimension[0] + (y-1) * obj->dimension[0] * obj->dimension[2];
                            if(obj->model[neighIndex]==0){
                                vNormal.y -= 1;
                            }else{
                                occ++;
                            }
                        }

                        if(y+1 >= obj->dimension[1]){
                            vNormal.y += 1;
                        }else{
                            neighIndex = (x) + (z) * obj->dimension[0] + (y+1) * obj->dimension[0] * obj->dimension[2];
                            if(obj->model[neighIndex]==0){
                                vNormal.y += 1;
                            }else{
                                occ++;
                            }
                        }

                    if(occ!=6){
                        requestedVertices[requestedCount] = (Vector3){x,y,z};
                        Pixel vColor = Rendering.voxelColors[obj->model[index]];
                        requestedColors[requestedCount] = (Vector3){vColor.r/256.0,
                                                                    vColor.g/256.0,
                                                                    vColor.b/256.0};

                        //Voxels occluded by 5 neighbors are identified by its normals with length 1 in shader. 
                        //Others are rescaled to diferenciate
                        if(occ!=5) vNormal = ScalarMult(vNormal,1.5);
                        if(vNormal.x == 0 && vNormal.y == 0 && vNormal.z == 0) vNormal = (Vector3){0,0,0.5};
                        requestedNormals[requestedCount] = vNormal;

                        requestedCount++;                        
                    }
                }
            }
        }
    }
    
    //If there are no vertices inside the VBOs, just insert them
    if(obj->numberOfVertices == 0){
        obj->numberOfVertices = requestedCount;
        memcpy(vertices,requestedVertices,requestedCount * sizeof(Vector3));
        memcpy(normal,requestedNormals,requestedCount * sizeof(Vector3));
        memcpy(colors,requestedColors,requestedCount * sizeof(Vector3));
        
    }else{
        
        int av = 0;
        int req = 0;
        while(av<avaliableCount && req<requestedCount){
            int indx = avaliableVertices[av];
            vertices[indx] = requestedVertices[req];
            normal[indx] = requestedNormals[req];
            colors[indx] = requestedColors[req];

            av++;
            req++;
        }
        if(requestedCount > avaliableCount){
            //Have more voxels to be added than removed, keep inserting after numberOfVertices items
            int voxelsRemaining = requestedCount-avaliableCount;

            memcpy(&vertices[obj->numberOfVertices], &requestedVertices[req], voxelsRemaining*sizeof(Vector3));
            memcpy(&normal[obj->numberOfVertices], &requestedNormals[req], voxelsRemaining*sizeof(Vector3));
            memcpy(&colors[obj->numberOfVertices], &requestedColors[req], voxelsRemaining*sizeof(Vector3));

            obj->numberOfVertices += voxelsRemaining;

        }else if (avaliableCount > requestedCount){
            //Have more removed than added, mark the empty positions as invalid
            while(av<avaliableCount){
                int indx = avaliableVertices[av];

                vertices[indx] = (Vector3){-1,-1,-1};
                normal[indx] = VECTOR3_ZERO;
                colors[indx] = VECTOR3_ZERO;

                av++;
            }
        }
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, obj->vbo[0]);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, obj->vbo[1]);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, obj->vbo[2]);
    glUnmapBuffer(GL_ARRAY_BUFFER);
}

//Multi Object helper structures
typedef enum {nTRN, nSHP, nGRP}mpTypes;
typedef struct MagicaProperties *mpNode;

typedef struct MagicaProperties{
    mpTypes Type;
    int data[7];
    int* groupChildIds;
    int hidden;
    char *name;
    Vector3 position;
    int rotation;
    mpNode next;
}MagicaProperties;

void GenerateModelStructure(int nodeID, EntityID parentID,char* modelPath, char* modelName, mpNode sceneGraphStart, List modelsList);

//Load a voxel model with various parts into multiple entities parented to a main object
void LoadMultiVoxelModel(EntityID entity, char modelPath[], char modelName[])
{
    if(!IsMultiVoxelModelFile(modelPath, modelName)){
        PrintLog(Warning,"LoadMultiVoxelModel: File doesn't contain more than one object, use LoadVoxelModel\n");
        return;
    }

    char fullPath[512+256];
    strncpy(fullPath,modelPath,512);
    int modelPathLen = strlen(modelPath);
    if(modelPathLen > 0 && modelPath[modelPathLen-1] != '/'){
        strcat(fullPath,"/");
    }
    strcat(fullPath,modelName);
    FILE* file = fopen(fullPath,"rb");

    if(file == NULL){
	    perror("LoadMultiVoxelModel");
        return;
    }

    List modelsList = InitList(sizeof(VoxelModel));
    mpNode propertiesListStart = NULL;
    mpNode propertiesListEnd = NULL;

    Voxel *voxelData = NULL;
    int i;

    //Get file length and return to start
    fseek(file, 0L, SEEK_END);
    int fileLength = ftell(file);
    PrintLog(Info,"File length: %d\n",fileLength);
    rewind(file);

    //Get magic word to see if this is a magicavoxel .vox file
    char *magic = calloc(5,sizeof(char));
    fread(magic,sizeof(char),4,file);

    //Get file version
    int version;
    fread(&version,sizeof(int),1,file);
    PrintLog(Info,"Version: %d\n",version);

    // All MagicaVoxel .vox file starts with a 'magic' 4 character 'VOX ' identifier
    if (strcmp("VOX ",magic) == 0)
    {
        char *chunkId = calloc(5,sizeof(char));

        //While we hasnt reached the end of the file, read data
        while (ftell(file)<fileLength)
        {
            // each chunk has an ID, size and child chunks
            fread(chunkId,sizeof(char),4,file);

            int chunkSize;
            fread(&chunkSize,sizeof(int),1,file);

            int childChunks;
            fread(&childChunks,sizeof(int),1,file);

            //PrintLog(Info,"Chunk: %s\n",chunkId);

            //There are 5 chunks we only care about: SIZE, XYZI, nTRN, nSHP and nGRP
            //SIZE chunk contains the dimensions of the model
            if (strcmp(chunkId,"SIZE") == 0)
            {        
                //Only create a new object if the last one has been sucessfully loaded
                //If not, flag an error
                if(!voxelData){

                    //Create the new object
                    VoxelModel *newModel;
                    VoxelModelConstructor((void**)(&newModel));
                    
                    fread(&newModel->dimension[0],sizeof(int),1,file);
                    fread(&newModel->dimension[1],sizeof(int),1,file);
                    fread(&newModel->dimension[2],sizeof(int),1,file);

                    //Copy to the models list
                    InsertListEnd(&modelsList,newModel);
                    //free(newModel);

                }else{
                    PrintLog(Error,"LoadMultiVoxelModel: An error loading has ocurred!\n");
                    free(magic);
                    fclose(file);
                    return;
                }
            }
            //XYZI contains the voxels position and color index
            else if (strcmp(chunkId,"XYZI") == 0)
            {
                VoxelModel *objPointer = GetLastCell(modelsList)->element;
                // XYZI contains n voxels
                fread(&objPointer->voxelCount,sizeof(int),1,file);
                objPointer->voxelsRemaining = objPointer->voxelCount;

                voxelData = calloc(objPointer->voxelCount,sizeof(Voxel));

                for(i = 0; i < objPointer->voxelCount; i++){
                    //Get MagicaVoxel voxel position and color index and insert in the array
                    fread(&voxelData[i].x,sizeof(char),1,file);
                    fread(&voxelData[i].y,sizeof(char),1,file);
                    fread(&voxelData[i].z,sizeof(char),1,file);
                    fread(&voxelData[i].color,sizeof(char),1,file);
                }

                //After loading all voxels into the array, allocate and transfer them to the new objects

                //Allocating memory and initializing structures
                objPointer->model = calloc(objPointer->dimension[0] * objPointer->dimension[1] * objPointer->dimension[2],sizeof(unsigned char));

                //Request VBO buffers
                glBindBuffer(GL_ARRAY_BUFFER, objPointer->vbo[0]);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glBufferData(GL_ARRAY_BUFFER, objPointer->dimension[0] * objPointer->dimension[1] * objPointer->dimension[2] * 3 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);

                glBindBuffer(GL_ARRAY_BUFFER, objPointer->vbo[1]);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glBufferData(GL_ARRAY_BUFFER, objPointer->dimension[0] * objPointer->dimension[1] * objPointer->dimension[2] * 3 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);

                glBindBuffer(GL_ARRAY_BUFFER, objPointer->vbo[2]);
                glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glBufferData(GL_ARRAY_BUFFER, objPointer->dimension[0] * objPointer->dimension[1] * objPointer->dimension[2] * 3 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);

                objPointer->numberOfVertices = 0;
                
                //Inserting voxels into the VoxelObject structure
                for(i = 0; i < objPointer->voxelCount; i++){
                    
                    // do not store this voxel if it lies out of range of the voxel chunk (126x126x126)
                    if (voxelData[i].x > 126 || voxelData[i].y > 126 || voxelData[i].z > 126) continue;

                    // Insert data based on the index = (x + z*sizeX + y*sizeX*sizeZ)
                    int voxel = (voxelData[i].x + voxelData[i].z * objPointer->dimension[0] + voxelData[i].y * objPointer->dimension[0] * objPointer->dimension[2]);
                    objPointer->model[voxel] = voxelData[i].color;
                }


                objPointer->center = (Vector3){objPointer->dimension[0]/2,objPointer->dimension[1]/2,objPointer->dimension[2]/2};

                objPointer->modificationStartX = 0;
                objPointer->modificationEndX = objPointer->dimension[0]-1;

                objPointer->modificationStartY = 0;
                objPointer->modificationEndY = objPointer->dimension[1]-1;

                objPointer->modificationStartZ = 0;
                objPointer->modificationEndZ = objPointer->dimension[2]-1;

                objPointer->enabled = 1;

                //Pass to the next model
                free(voxelData);
                voxelData = NULL;
            }
            //Chunk containing the name, show/hidden status, rotation and position of an object
            //Will be put into a properties list
            else if (strcmp(chunkId,"nTRN") == 0)
            {
                int aux,pNum;
                //Create a new property element
                if(!propertiesListStart){
                    propertiesListStart = malloc(sizeof(MagicaProperties));
                    propertiesListEnd = propertiesListStart;
                }else{
                    propertiesListEnd->next = malloc(sizeof(MagicaProperties));
                    propertiesListEnd = propertiesListEnd->next;
                }
                propertiesListEnd->name = NULL;
                propertiesListEnd->position = VECTOR3_ZERO;
                propertiesListEnd->rotation = 0;
                propertiesListEnd->hidden = 0;
                propertiesListEnd->next = NULL;
                propertiesListEnd->Type = nTRN;


                //0: Index in list
                fread(&propertiesListEnd->data[0],sizeof(int),1,file);
                //1: Number of string with the editor properties to be followed
                fread(&propertiesListEnd->data[1],sizeof(int),1,file);

                //Parse these strings and get their data
                for(pNum=0;pNum<propertiesListEnd->data[1];pNum++){
                    //Number of characters of the string
                    int PropertyLength1,intData;
                    fread(&PropertyLength1,sizeof(int),1,file);

                    //Allocate string and read chars
                    char *editorProperty = calloc(PropertyLength1+1 ,sizeof(char));
                    for(aux = 0; aux<PropertyLength1; aux ++){
                        fread(&editorProperty[aux],sizeof(char),1,file);
                    }
                    editorProperty[aux] = '\0';

                    //Read the following int
                    fread(&intData,sizeof(int),1,file);
                    
                    //If the string was "_name", the last int is the number of
                    //characters of the object name, so read the characters of the name
                    if(strcmp(editorProperty,"_name") == 0){
                        propertiesListEnd->name = calloc(intData + 1 ,sizeof(char));
                        
                        for(aux = 0; aux<intData; aux ++){
                            fread(&propertiesListEnd->name[aux],sizeof(char),1,file);
                        }
                        propertiesListEnd->name[aux] = '\0';
                    }
                    //If the string was "_hidden", the last int is the number of
                    //characters of the object hidden status, so read the character '1' or '0'
                    else if(strcmp(editorProperty,"_hidden") == 0){
                        char hid;
                        fread(&hid,sizeof(char),1,file);
                        propertiesListEnd->hidden = hid == '1'? 1:0;
                    }
                    
                    free(editorProperty);
                }

                //2: Index of the nSHP corresponding to that object in the list
                fread(&propertiesListEnd->data[2],sizeof(int),1,file);
                //3: Always -1, as I have observed for now
                fread(&propertiesListEnd->data[3],sizeof(int),1,file);
                //4: Object layer
                fread(&propertiesListEnd->data[4],sizeof(int),1,file);
                //5: Always 1
                fread(&propertiesListEnd->data[5],sizeof(int),1,file);

                //6: Number of transform properties
                fread(&propertiesListEnd->data[6],sizeof(int),1,file);
                //Parse transform properties strings
                for(pNum=0;pNum<propertiesListEnd->data[6];pNum++){
                    //Number of characters of the string
                    int PropertyLength2,intData;
                    fread(&PropertyLength2,sizeof(int),1,file);

                    //Allocate string and read chars
                    char *transformProperty = calloc(PropertyLength2+1 ,sizeof(char));
                    for(aux = 0; aux<PropertyLength2; aux ++){
                        fread(&transformProperty[aux],sizeof(char),1,file);
                    }
                    transformProperty[aux] = '\0';

                    //Read the following int
                    fread(&intData,sizeof(int),1,file);
                    
                    //If the string was "_r", the last int is the number of
                    //characters of the rotation property, so read them
                    if(strcmp(transformProperty,"_r") == 0){
                        char *rot = calloc(intData + 1 ,sizeof(char));
                        
                        for(aux = 0; aux<intData; aux ++){
                            fread(&rot[aux],sizeof(char),1,file);
                        }
                        rot[aux] = '\0';
                        sscanf(rot,"%d",&propertiesListEnd->rotation);
                        free(rot);
                    }
                    //If the string was "_t", the last int is the number of
                    //characters of the object position, so read them
                    else if(strcmp(transformProperty,"_t") == 0){
                        char *pos = calloc(intData + 1 ,sizeof(char));
                        
                        for(aux = 0; aux<intData; aux ++){
                            fread(&pos[aux],sizeof(char),1,file);
                        }
                        pos[aux] = '\0';
                        int px,py,pz;
                        sscanf(pos,"%d %d %d",&px,&py,&pz);
                        propertiesListEnd->position.x = px;
                        propertiesListEnd->position.y = py;
                        propertiesListEnd->position.z = pz;
                        free(pos);
                    }
                    
                    free(transformProperty);
                }
            }
            //Chunk containing the model which a certain transform is linked to
            //Will be put into a properties list
            else if (strcmp(chunkId,"nSHP") == 0)
            {
                //Create a new property element
                if(!propertiesListStart){
                    propertiesListStart = malloc(sizeof(MagicaProperties));
                    propertiesListEnd = propertiesListStart;
                }else{
                    propertiesListEnd->next = malloc(sizeof(MagicaProperties));
                    propertiesListEnd = propertiesListEnd->next;
                }
                propertiesListEnd->name = NULL;
                propertiesListEnd->position = VECTOR3_ZERO;
                propertiesListEnd->rotation = 0;
                propertiesListEnd->hidden = 0;
                propertiesListEnd->next = NULL;
                propertiesListEnd->Type = nSHP;


                //0: Index in list
                fread(&propertiesListEnd->data[0],sizeof(int),1,file);
                //1: Unknown
                fread(&propertiesListEnd->data[1],sizeof(int),1,file);
                //2: Maybe the number of objects, but its always 1
                fread(&propertiesListEnd->data[2],sizeof(int),1,file);
                //3: Index of the model it refers to
                fread(&propertiesListEnd->data[3],sizeof(int),1,file);
                //4: Unknown
                fread(&propertiesListEnd->data[4],sizeof(int),1,file);
            }
            //Chunk containing the models inside a group
            //Will be put into a properties list
            else if (strcmp(chunkId,"nGRP") == 0)
            {
                //Create a new property element
                if(!propertiesListStart){
                    propertiesListStart = malloc(sizeof(MagicaProperties));
                    propertiesListEnd = propertiesListStart;
                }else{
                    propertiesListEnd->next = malloc(sizeof(MagicaProperties));
                    propertiesListEnd = propertiesListEnd->next;
                }
                propertiesListEnd->name = NULL;
                propertiesListEnd->next = NULL;
                propertiesListEnd->Type = nGRP;

                //0: Index in list
                fread(&propertiesListEnd->data[0],sizeof(int),1,file);
                //1: Number of attributes (should be zero)
                fread(&propertiesListEnd->data[1],sizeof(int),1,file);
                //2: Number of childs
                fread(&propertiesListEnd->data[2],sizeof(int),1,file);

                propertiesListEnd->groupChildIds = propertiesListEnd->data[2]==0? NULL : malloc(propertiesListEnd->data[2] * sizeof(int));
                int i;
                for(i=0; i<propertiesListEnd->data[2]; i++){
                    fread(&propertiesListEnd->groupChildIds[i],sizeof(int),1,file);
                }

            }
            else {fseek(file,chunkSize,SEEK_CUR);}   //Read any excess bytes
        }
        free(chunkId);
    }else{
        //Magic word is not VOX
        PrintLog(Error,"LoadMultiVoxelModel: Magic word is not 'VOX ', but '%s'\n",magic);
    }
    free(magic);
    fclose(file);

    //Generate the entities models and group transforms
    GenerateModelStructure(0, entity, modelPath, modelName, propertiesListStart, modelsList);

    //Free properties list
    mpNode current = propertiesListStart;
    while(current){
        if(current->Type == nTRN){
            free(current->name);
        }
        if(current->Type == nGRP){
            free(current->groupChildIds);
        }
        mpNode aux = current;
        current = current->next;
        free(aux);
    }
    ListCellPointer modelsCell;
    ListForEach(modelsCell,modelsList){
        VoxelModelDestructor(&modelsCell->element);
    }
    FreeList(&modelsList);

    PrintLog(Info,">DONE!\n\n");
}

void GenerateModelStructure(int nodeID, EntityID parentID,char* modelPath, char* modelName, mpNode sceneGraphStart, List modelsList){

    //Get the current node by ID
    mpNode current = sceneGraphStart;
    if(nodeID != 0){
        while(nodeID-- > 0){
            current = current->next;
        }
    }

    int i;
    int enab = 1;
    Vector3 pos = VECTOR3_ZERO;
    Vector3 rot = VECTOR3_ZERO;
    switch (current->Type){
        case nTRN:
        //Set object transform
        //PrintLog(Info,"\n Type:%s Data: %d %d %d %d %d %d %d Name: [%s] Hidden: %d Rot: %d Pos: %.1f %.1f %.1f \n",current->Type? (current->Type == 2? "nGRP":"nSHP"):"nTRN", current->data[0], current->data[1], current->data[2], current->data[3], current->data[4], current->data[5], current->data[6], current->name? current->name:"", current->hidden,current->rotation, current->position.x, current->position.y, current->position.z);

        enab = !current->hidden;
        pos = current->position;
        rot = DecodeMagicaVoxelRotation(current->rotation);

        //Get node corresponding to the object
        mpNode childNode = sceneGraphStart;
        for(i=0;i<current->data[2];i++){
            childNode = childNode->next;
        }

        //In case this transform is from an object, create an entity with this model and parent with the entity
        if(childNode->Type == nSHP){
            //printf("SHP\n");
            //Allocate and copy data to the final object
            EntityID subModel = CreateEntity();
            SetEntityParent(subModel,parentID);
            AddComponentToEntity(ThisComponentID(),subModel);
            AddComponentToEntity(GetComponentID("Transform"),subModel);
            

            //Pass the data to the entity
            VoxelModel *curModel = GetCellAt(modelsList,childNode->data[3])->element;
            VoxelModelDestructor(&ECS.Components[ThisComponentID()][subModel].data);
            ECS.Components[ThisComponentID()][subModel].data = VoxelModelCopy(curModel);
            
            SetPosition(subModel,pos);
            SetRotation(subModel,rot);
            SetVoxelModelEnabled(subModel, enab);
            strncpy(GetVoxelModelPointer(subModel)->objectName,current->name,65);
            strncpy(GetVoxelModelPointer(subModel)->modelPath,modelPath,512);
            strncpy(GetVoxelModelPointer(subModel)->modelName,modelName,256);
        }
        else{
            //printf("GRP\n");
            //In case this is the transform of a group, generate an entity with a Transform component
            EntityID group;

            //Treat the initial nTRN and nGRP, as the origin entity is already created, use it instead of generating another
            if(nodeID == 0){
                group = parentID;

                if(!EntityContainsComponent(group, GetComponentID("Transform")))
                    AddComponentToEntity(GetComponentID("Transform"),group);

                if(EntityContainsComponent(group, ThisComponentID()))
                    RemoveComponentFromEntity(ThisComponentID(),group);

            }else{
                group = CreateEntity();
                SetEntityParent(group,parentID);
                AddComponentToEntity(GetComponentID("Transform"),group);

                SetPosition(group,pos);
                SetRotation(group,rot);
            }

            //printf("%d\n", group);
            //Generate the objects and groups that are inside of this group and parent to the 'group' Entity
            for(i=0; i<childNode->data[2]; i++){
                GenerateModelStructure(childNode->groupChildIds[i], group, modelPath, modelName, sceneGraphStart, modelsList);
            }
            
        }

        break;
        case nSHP:
        //Keeping this case for debug
        // PrintLog(Info,"\n Type:%s Data: %d %d %d %d %d\n",current->Type? (current->Type == 2? "nGRP":"nSHP"):"nTRN", current->data[0], current->data[1], current->data[2], current->data[3], current->data[4]);

        break;
        case nGRP:
        //Keeping this case for debug
        //PrintLog(Info,"\n Type:%s\n", current->Type? (current->Type == 2? "nGRP":"nSHP"):"nTRN");  
        break;
    }
}

Vector3 DecodeMagicaVoxelRotation(int rot){
    //Each number is a binary encodification of a rotation. As I haven't figured
    //how it works, i just enumerated each possible rotation and inversion (28 with identity)
    //for now, single axis inversion is not supported, and others coincide with common rotations
    switch(rot){
        case 40:
            return (Vector3){90,0,0};
        break;
        case 105:
            return (Vector3){90,90,0};
        break;
        case 100:
            return (Vector3){180,0,0};
        break;
        case 118:
            return (Vector3){180,90,0};
        break;
        case 65:
            return (Vector3){180,0,90};
        break;
        case 9:
            return (Vector3){270,270,0};
        break;
        case 72:
            return (Vector3){270,0,0};
        break;
        case 82:
            return (Vector3){270,0,90};
        break;
        case 70:
            return (Vector3){0,90,0};
        break;
        case 89:
            return (Vector3){0,90,90};
        break;
        case 84:
            return (Vector3){0,180,0};
        break;
        case 24:
            return (Vector3){270,180,0};
        break;
        case 113:
            return (Vector3){0,180,90};
        break;
        case 22:
            return (Vector3){0,270,0};
        break;
        case 50:
            return (Vector3){90,0,270};
        break;
        case 17:
            return (Vector3){0,0,90};
        break;
        case 57:
            return (Vector3){90,270,0};
        break;
        case 2:
            return (Vector3){90,0,90};
        break;
        case 52:
            return (Vector3){0,0,180};
        break;
        case 120:
            return (Vector3){90,180,0};
        break;
        case 38:
            return (Vector3){90,270,90};
        break;
        case 33:
            return (Vector3){0,0,270};
        break;
        case 98:
            return (Vector3){270,0,270};
        break;
        case 20:
            return (Vector3){0,0,0};
        break;
        case 36:
            return (Vector3){0,0,0};
        break;
        case 68:
            return (Vector3){0,0,0};
        break;
        default:
            return VECTOR3_ZERO;
        break;
    }

}
