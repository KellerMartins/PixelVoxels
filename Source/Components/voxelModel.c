#include "VoxelModel.h"

//Function that returns the ID of this component
static ComponentID ThisComponentID(){
    static ComponentID CompID = -1;
    if(CompID<0)
        CompID = GetComponentID("VoxelModel");
    
    return CompID;
}

extern engineECS ECS;
extern engineRendering Rendering;

void InternalLoadVoxelModel(VoxelModel **modelPointer, char modelPath[], char modelName[]);

void VoxelModelConstructor(void** data){
    if(!data) return;
    *data = calloc(1,sizeof(VoxelModel));
    VoxelModel *obj = *data;

    *obj = (VoxelModel){0,0,0,0,0,0,0,NULL,NULL,NULL,NULL,NULL,0,0,0};
}
void VoxelModelDestructor(void** data){
    if(!data) return;
    VoxelModel *obj = *data;

    free(obj->model);
    free(obj->lighting);
    free(obj->vertices);
    free(obj->vColors);
    free(obj->normal);

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

    if(((VoxelModel*)data)->model){
        int sizeModel = newVoxelModel->dimension[0] * newVoxelModel->dimension[1] * newVoxelModel->dimension[2] * sizeof(unsigned char);
        newVoxelModel->model = malloc(sizeModel);
        newVoxelModel->lighting = malloc(sizeModel);
        
        int sizeVertices = newVoxelModel->numberOfVertices*3*sizeof(GLfloat);
        newVoxelModel->vertices = malloc(sizeVertices);
        newVoxelModel->vColors = malloc(sizeVertices);
        newVoxelModel->normal = malloc(sizeVertices);
        
        memcpy(newVoxelModel->model,((VoxelModel*)data)->model,sizeModel);
        memcpy(newVoxelModel->lighting,((VoxelModel*)data)->lighting,sizeModel);

        memcpy(newVoxelModel->vertices,((VoxelModel*)data)->vertices,sizeVertices);
        memcpy(newVoxelModel->vColors,((VoxelModel*)data)->vColors,sizeVertices);
        memcpy(newVoxelModel->normal,((VoxelModel*)data)->normal,sizeVertices);
    }else{
        newVoxelModel->model = NULL;
        newVoxelModel->lighting = NULL;
        newVoxelModel->vertices = NULL;
        newVoxelModel->vColors = NULL;
        newVoxelModel->normal = NULL;
    }

	return newVoxelModel;
}

cJSON* VoxelModelEncode(void** data){
    VoxelModel *v = *data; 
    cJSON *obj = cJSON_CreateObject();

    cJSON_AddStringToObject(obj, "modelPath", v->modelPath);
    cJSON_AddStringToObject(obj, "modelName", v->modelName);

    cJSON *center = cJSON_AddArrayToObject(obj,"center");
    cJSON_AddItemToArray(center, cJSON_CreateNumber(v->center.x));
    cJSON_AddItemToArray(center, cJSON_CreateNumber(v->center.y));
    cJSON_AddItemToArray(center, cJSON_CreateNumber(v->center.z));

    return obj;
}

void* VoxelModelDecode(cJSON **data){
    VoxelModel *v = malloc(sizeof(VoxelModel));

    InternalLoadVoxelModel(&v, cJSON_GetObjectItem(*data, "modelPath")->valuestring, cJSON_GetObjectItem(*data, "modelName")->valuestring);

    cJSON *center = cJSON_GetObjectItem(*data,"center");
    v->center = (Vector3){(cJSON_GetArrayItem(center,0))->valuedouble,
                          (cJSON_GetArrayItem(center,1))->valuedouble,
                          (cJSON_GetArrayItem(center,2))->valuedouble};
    return v;
}

VoxelModel* GetVoxelModelPointer(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("GetVoxelModelPointer: Entity doesn't have a VoxelModel component. (%d)\n",entity);
        return NULL;
    }
    return (VoxelModel *) ECS.Components[ThisComponentID()][entity].data;
}

Vector3 GetVoxelModelCenter(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("GetVoxelModelCenter: Entity doesn't have a VoxelModel component. (%d)\n",entity);
        return VECTOR3_ZERO;
    }
    VoxelModel *m = GetVoxelModelPointer(entity);
    return m->center;
}

void SetVoxelModelCenter(EntityID entity, Vector3 center){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("SetVoxelModelCenter: Entity doesn't have a VoxelModel component. (%d)\n",entity);
        return;
    }
    VoxelModel *m = GetVoxelModelPointer(entity);
    m->center = center;
}

int IsVoxelModelEnabled(EntityID entity){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("IsVoxelModelEnabled: Entity doesn't have a VoxelModel component. (%d)\n",entity);
        return 0;
    }
    VoxelModel *m = GetVoxelModelPointer(entity);
    return m->enabled;
}

void SetVoxelModelEnabled(EntityID entity, int booleanVal){
    if(!EntityContainsComponent(entity, ThisComponentID())){
        printf("SetVoxelModelEnabled: Entity doesn't have a VoxelModel component. (%d)\n",entity);
        return;
    }
    VoxelModel *m = GetVoxelModelPointer(entity);
    m->enabled = booleanVal? 1: 0;
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
		printf("LoadVoxelModel: Entity index out of range!(%d)\n",entity);
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

void InternalLoadVoxelModel(VoxelModel **modelPointer, char modelPath[], char modelName[]){
    VoxelModel *obj = *modelPointer;
    char fullPath[512+256];
    strncpy(fullPath,modelPath,512);
    if(modelPath[strlen(modelPath)-1] != '/'){
        strcat(fullPath,"/");
    }
    strcat(fullPath,modelName);
    printf("\nLoading model: (%s)(%s)(%s)\n",modelPath,modelName,fullPath);
    FILE* file = fopen(fullPath,"rb");

    if(file == NULL){
	    perror("LoadVoxelModel");
        //Return an empty VoxelObject in case of failure
        return;
    }
    strncpy(obj->modelPath,modelPath,512);
    strncpy(obj->modelName,modelName,256);
    obj->enabled = 1;

    Voxel *voxelData = NULL;

    int numVoxels = 0,modelLoaded = 0;
    int i;

    //Get file length and return to start
    fseek(file, 0L, SEEK_END);
    int fileLength = ftell(file);
    printf("File length: %d\n",fileLength);
    rewind(file);

    //Get magic word to see if this is a magicavoxel .vox file
    char *magic = calloc(5,sizeof(char));
    fread(magic,sizeof(char),4,file);

    //Get file version
    int version;
    fread(&version,sizeof(int),1,file);
    printf("Version: %d\n",version);

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

            //printf("Chunk: %s\n",chunkId);

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

                    printf("Dimension: %d %d %d\n",obj->dimension[0],obj->dimension[1],obj->dimension[2]);

                }else{
                    printf("Multiple voxel models found on this object, ignoring new size\n");
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
                    printf("Number of Voxels: %d\n",numVoxels);
                    
                    voxelData = calloc(numVoxels,sizeof(Voxel));

                    //Free strings and return empty Object if it fails to allocate memory
                    if(!voxelData){
                        printf("Failed to allocate voxel array!\n");
                        free(magic);
                        free(chunkId);
                        return;
                    }
                    
                    for(i = 0; i < numVoxels; i++){
                        //Get MagicaVoxel voxel position and color index and insert in the array
                        fread(&voxelData[i].x,sizeof(char),1,file);
                        fread(&voxelData[i].y,sizeof(char),1,file);
                        fread(&voxelData[i].z,sizeof(char),1,file);
                        fread(&voxelData[i].color,sizeof(char),1,file);
                    }

                }else{
                    printf("Multiple voxel models found on this object, ignoring new model\n");
                    fseek(file,chunkSize,SEEK_CUR);
                }
            }
            else {fseek(file,chunkSize,SEEK_CUR);}   // read any excess bytes
        }

        //Allocating memory and initializing structures
        obj->model = calloc(obj->dimension[0] * obj->dimension[1] * obj->dimension[2],sizeof(unsigned char));
        obj->lighting = calloc(obj->dimension[0] * obj->dimension[1] * obj->dimension[2],sizeof(unsigned char));
        
        //Inserting voxels into the VoxelObject structure
        for (i = 0; i < numVoxels; i++)
        {
            
            // do not store this voxel if it lies out of range of the voxel chunk (126x126x126)
            if (voxelData[i].x > 126 || voxelData[i].y > 126 || voxelData[i].z > 126) continue;

            // Insert data based on the index = (x + z*sizeX + y*sizeX*sizeZ)
            int voxel = (voxelData[i].x + voxelData[i].z * obj->dimension[0] + voxelData[i].y * obj->dimension[0] * obj->dimension[2]);
            obj->model[voxel] = voxelData[i].color;
        }

        //Initialize lighting array
        for(i = 0;i<obj->dimension[0]*obj->dimension[1]*obj->dimension[2]; i++){
            obj->lighting[i] = 1;
        }
        free(chunkId);
    }else{
        printf("Magic word is not 'VOX ', but '%s'\n",magic);
        return;
    }
    free(magic);
    free(voxelData);

    //Final settings
    obj->numberOfVertices = 0;
    obj->vertices = NULL;
    obj->vColors = NULL;
    obj->normal = NULL;
    obj->voxelCount = numVoxels;
    obj->voxelsRemaining = numVoxels;

    obj->center = VECTOR3_ZERO;

    obj->modificationStartX = 0;
    obj->modificationEndX = obj->dimension[0]-1;

    obj->modificationStartY = 0;
    obj->modificationEndY = obj->dimension[1]-1;

    obj->modificationStartZ = 0;
    obj->modificationEndZ = obj->dimension[2]-1;

    obj->enabled = 1;

    fclose(file);

    printf(">DONE!\n\n");
}

//Calculate the surface voxels to be shown and put into a GLfloat vertex array inside the component to be rendered
void CalculateRendered(EntityID entity){
    VoxelModel *obj = GetVoxelModelPointer(entity);


    if(obj->modificationStartZ <0 || obj->modificationEndZ <0 || !obj->model){
        return;
    }

    List avaliableVoxels = InitList(sizeof(int));
    List requestedVoxels = InitList(sizeof(Vector3));

    List requestedNormals = InitList(sizeof(Vector3)); 

    int x,y,z,i,index,dir,occ;
    int xLimitS = clamp(obj->modificationStartX-1 ,0,obj->dimension[0]-1);
    int xLimitE = clamp(obj->modificationEndX+1   ,0,obj->dimension[0]-1);
    int yLimitS = clamp(obj->modificationStartY-1 ,0,obj->dimension[1]-1);
    int yLimitE = clamp(obj->modificationEndY+1   ,0,obj->dimension[1]-1);
    int zLimitS = clamp(obj->modificationStartZ-1 ,0,obj->dimension[2]-1);
    int zLimitE = clamp(obj->modificationEndZ+1   ,0,obj->dimension[2]-1);

    for(i=0;i<obj->numberOfVertices*3;i+=3){
        Vector3 voxelPos = {roundf(obj->vertices[i]),roundf(obj->vertices[i+1]),roundf(obj->vertices[i+2])};
        if(voxelPos.x<0 || voxelPos.y<0 || voxelPos.z<0){
            InsertListEnd(&avaliableVoxels,(void*)&i);
            continue;
        }
        if((voxelPos.x<xLimitS || voxelPos.x>xLimitE || voxelPos.y<yLimitS || voxelPos.y>yLimitE || voxelPos.z<zLimitS || voxelPos.z>zLimitE))
            continue;
        else
            InsertListEnd(&avaliableVoxels,(void*)&i);           
    }
    
    for(z = zLimitE; z>=zLimitS ;z--){
        for(y = yLimitS; y<=yLimitE; y++){
            for(x = xLimitS; x<=xLimitE; x++){
                occ = 0;

                index = (x + z * obj->dimension[0] + y * obj->dimension[0] * obj->dimension[2]);
                
                if(obj->model[index]!=0){
                    
                    if(x!=0 && x<obj->dimension[0]-1 && y!=0 && y<obj->dimension[1]-1 && z!=0 && z<obj->dimension[2]-1){
                        dir = (x + (z+1) * obj->dimension[0] + y * obj->dimension[0] * obj->dimension[2]);//0 0 1
                        if(obj->model[dir]!=0){
                            occ++; 
                        }
                        dir = (x + (z-1) * obj->dimension[0] + y * obj->dimension[0] * obj->dimension[2]);//0 0 -1
                        if(obj->model[dir]!=0){
                            occ++;
                        }
                        dir = (x + z * obj->dimension[0] + (y+1) * obj->dimension[0] * obj->dimension[2]);//0 1 0
                        if(obj->model[dir]!=0){
                            occ++;
                        }
                        dir = (x + z * obj->dimension[0] + (y-1) * obj->dimension[0] * obj->dimension[2]);//0 -1 0 
                        if(obj->model[dir]!=0){
                            occ++;
                        }
                        dir = ( (x+1) + z * obj->dimension[0] + y * obj->dimension[0] * obj->dimension[2]);//1 0 0
                        if(obj->model[dir]!=0){
                            occ++;
                        }
                        dir = ( (x-1) + z * obj->dimension[0] + y * obj->dimension[0] * obj->dimension[2]);//1 0 0 
                        if(obj->model[dir]!=0){
                            occ++;
                        }
                    }
                    if(occ!=6){
                        Vector3 vPos = {x,y,z};
                        Vector3 vNormal = VECTOR3_ZERO;
                        InsertListStart(&requestedVoxels,(void*)&vPos);

                        if(x-1 < 0){
                            vNormal.x -= 1;
                        }else{
                            index = (x-1) + (z) * obj->dimension[0] + (y) * obj->dimension[0] * obj->dimension[2];
                            if(obj->model[index]==0){
                                vNormal.x -= 1;
                            }
                        }

                        if(x+1 >= obj->dimension[0]){
                            vNormal.x += 1;
                        }else{
                            index = (x+1) + (z) * obj->dimension[0] + (y) * obj->dimension[0] * obj->dimension[2];
                            if(obj->model[index]==0){
                                vNormal.x += 1;
                            }
                        }

                        if(z-1 < 0){
                            vNormal.z -= 1;
                        }else{
                            index = (x) + (z-1) * obj->dimension[0] + (y) * obj->dimension[0] * obj->dimension[2];
                            if(obj->model[index]==0){
                                vNormal.z -= 1;
                            }
                        }

                        if(z+1 >= obj->dimension[2]){
                            vNormal.z += 1;
                        }else{
                            index = (x) + (z+1) * obj->dimension[0] + (y) * obj->dimension[0] * obj->dimension[2];
                            if(obj->model[index]==0){
                                vNormal.z += 1;
                            }
                        }

                        if(y-1 < 0){
                            vNormal.y -= 1;
                        }else{
                            index = (x) + (z) * obj->dimension[0] + (y-1) * obj->dimension[0] * obj->dimension[2];
                            if(obj->model[index]==0){
                                vNormal.y -= 1;
                            }
                        }

                        if(y+1 >= obj->dimension[1]){
                            vNormal.y += 1;
                        }else{
                            index = (x) + (z) * obj->dimension[0] + (y+1) * obj->dimension[0] * obj->dimension[2];
                            if(obj->model[index]==0){
                                vNormal.y += 1;
                            }
                        }

                        InsertListStart(&requestedNormals,(void*)&vNormal);
                    }
                }
            }
        }
    }
    
    //First array creation
    if(obj->numberOfVertices == 0){
        obj->vertices = malloc(GetLength(requestedVoxels) * 3 * sizeof(GLfloat));
        obj->normal = malloc(GetLength(requestedNormals) * 3 * sizeof(GLfloat));

        obj->numberOfVertices = GetLength(requestedVoxels);

        ListCellPointer currentV = requestedVoxels.first;
        ListCellPointer currentN = requestedNormals.first;
        i = 0;
        while(currentV){
            memcpy(&obj->vertices[i],GetElement(*currentV),sizeof(Vector3));
            memcpy(&obj->normal[i],GetElement(*currentN),sizeof(Vector3));
            i+=3;
            currentV = GetNextCell(currentV);
            currentN = GetNextCell(currentN);
        }

        FreeList(&requestedVoxels);
        FreeList(&requestedNormals);
    }else{
        
        ListCellPointer currentAv = avaliableVoxels.first;
        ListCellPointer currentReqV = requestedVoxels.first;
        ListCellPointer currentReqN = requestedNormals.first;
        while(currentAv && currentReqV){
            int indx = *((int*)GetElement(*currentAv));
            memcpy(&obj->vertices[indx],GetElement(*currentReqV),sizeof(Vector3));
            memcpy(&obj->normal[indx],GetElement(*currentReqN),sizeof(Vector3));

            RemoveListStart(&avaliableVoxels);
            RemoveListStart(&requestedVoxels);

            currentAv = avaliableVoxels.first;
            currentReqV = requestedVoxels.first;
            currentReqN = requestedVoxels.first;
        }
        if(!currentAv && currentReqV){
            //Have more voxels to be added than removed, realloc to insert
            int numReqRemaining = GetLength(requestedVoxels);
            obj->vertices = realloc(obj->vertices, (obj->numberOfVertices + numReqRemaining) * 3 * sizeof(GLfloat));
            obj->normal = realloc(obj->normal, (obj->numberOfVertices + numReqRemaining) * 3 * sizeof(GLfloat));
            
            int indx = (obj->numberOfVertices*3);
            while(currentReqV){ 
                memcpy(&obj->vertices[indx],GetElement(*currentReqV),sizeof(Vector3));
                memcpy(&obj->normal[indx],GetElement(*currentReqN),sizeof(Vector3));

                RemoveListStart(&requestedVoxels);
                RemoveListStart(&requestedNormals);
                currentReqV = requestedVoxels.first;
                currentReqN = requestedNormals.first;

                indx+=3;
            }

            obj->numberOfVertices += numReqRemaining;

        }else if (!currentReqV){
            //Have more removed than added, mark the empty positions as invalid
            while(currentAv){
                int indx = *((int*)GetElement(*currentAv));

                obj->vertices[indx] = -1;
                obj->vertices[indx+1] = -1;
                obj->vertices[indx+2] = -1;

                obj->normal[indx] = 0;
                obj->normal[indx+1] = 0;
                obj->normal[indx+2] = 0;

                RemoveListStart(&avaliableVoxels);
                currentAv = avaliableVoxels.first;
            }
        }
    }
}

//Calculate the basic lighting and self shadowing and put into a GLint color array inside the component
void CalculateLighting(EntityID entity){
    VoxelModel *obj = GetVoxelModelPointer(entity);
    if(!obj->model){
        return;
    }

    int y,x,z,index,dir;
    int occlusion,lightAir,lightBlock;

    int zstart = obj->dimension[2]-1;
    int lightFinal;
    for(y=obj->modificationStartY; y<=obj->modificationEndY; y++){
        for(x=obj->modificationStartX; x<=obj->modificationEndX; x++){

            //Sets light at the top of the object, that is "transported" at each z iteration
            lightAir = 1;
            lightBlock = 1;

            for(z=zstart; z>=0; z--){
                occlusion = 0;
                index = (x + z * obj->dimension[0] + y * obj->dimension[0] * obj->dimension[2]);
                
                if(obj->model[index]!=0){
                    if(z<obj->dimension[2]-1){ //Up
                        dir = (x + (z+1) * obj->dimension[0] + y * obj->dimension[0] * obj->dimension[2]);
                        occlusion += obj->model[dir]==0? 0:1;
                        //Lights the block if the top is empty (with light or shadow), else, keep the color
                        lightBlock = obj->model[dir]==0? lightAir*2:1;
                    }else{
                        lightBlock = 2;
                        occlusion = 3;
                    }
                    if(z>0){ //Down
                        dir = (x + (z-1) * obj->dimension[0] + y * obj->dimension[0] * obj->dimension[2]);
                        occlusion += obj->model[dir]==0?  0:1;
                    }else{
                        occlusion = 3;
                    }
                    if(x>0){ //Left
                        dir = ((x-1) + z * obj->dimension[0] + y * obj->dimension[0] * obj->dimension[2]);
                        occlusion += obj->model[dir]==0? 0:1;
                    }else{
                        occlusion = 2;
                    }
                    if(x<obj->dimension[0]-1){ //Right
                        dir = ((x+1) + z * obj->dimension[0] + y * obj->dimension[0] * obj->dimension[2]);
                        occlusion += obj->model[dir]==0?  0:1;
                    }else{
                        occlusion = 3;
                    }
                    if(y<obj->dimension[1]-1){ //Front
                        dir = (x + z * obj->dimension[0] + (y+1) * obj->dimension[0] * obj->dimension[2]);
                        occlusion += obj->model[dir]==0?  0:1;
                    }else{
                        occlusion = 3;
                    }
                    if(y>0){ //Back
                        dir = (x + z * obj->dimension[0] + (y-1) * obj->dimension[0] * obj->dimension[2]);
                        occlusion += obj->model[dir]==0?  0:1;
                    }else{
                        occlusion = 3;
                    }
                    lightFinal = lightBlock;
                }else{
                    if(z+1<obj->dimension[2]){
                        dir = (x + (z+1) * obj->dimension[0] + y * obj->dimension[0] * obj->dimension[2]);
                        if(obj->model[dir]!=0){
                            lightAir = 0;
                        }
                    }
                    lightFinal = lightAir;
                }

                //lighting 8bits  [1-Empty] [3-Occlusion][2-Direct Light(2), Ambient(1) and self shadow(0)] [1-Shadow from caster]
                obj->lighting[index] = (unsigned char)( (((occlusion & 255)<<3)|((lightFinal & 3)<<1)) | (obj->lighting[index] & 1) );
            }
        }
    }

    const float edge = 0.80;
    const float base = 0.75;
    const float crease = 0.70;
    const float sunlight = 1.42;
    const float shadow = 0.79;

    free(obj->vColors);
    obj->vColors = malloc(obj->numberOfVertices * 3 * sizeof(GLfloat));

    int i;
    for(i = 0; i <obj->numberOfVertices*3; i+=3){
        x = roundf(obj->vertices[i]);
        y = roundf(obj->vertices[i+1]);
        z = roundf(obj->vertices[i+2]);

        if(x<0 || y<0 || z<0){
            obj->vColors[i] = 0;
            obj->vColors[i+1] = 0;
            obj->vColors[i+2] = 0;
            continue;
        }

        float heightVal = clamp(((1.0+(((z+ GetPosition(entity).z+Rendering.cameraPosition.z)*0.5))/128)),0,1.4);

        unsigned colorIndex = (x) + (z) * obj->dimension[0] + (y) * obj->dimension[0] * obj->dimension[2];
        Pixel color = Rendering.voxelColors[obj->model[colorIndex]];
        
        //Get voxel lighting level and multiply by the dynamic shadow (not used for now)
        int lightIndx = (obj->lighting[colorIndex] & 6)>>1;
        lightIndx *= obj->lighting[colorIndex] & 1;

        //Add some lightness on the edges
        int edgeIndx = obj->lighting[colorIndex]>>3;
        float lightVal = lightIndx == 1? 1:(lightIndx >= 2? sunlight:shadow);
        float edgeVal = (edgeIndx<5? edge:edgeIndx == 5? base:crease);

        //Multiply illuminations and convert from (0,256) to (0,1) range
        double illuminFrac = lightVal * edgeVal * heightVal * ONE_OVER_256;

        //Transform the color from a Int16 to each RGB component
        obj->vColors[i] = clamp(color.r * illuminFrac,0,1);
        obj->vColors[i+1] = clamp(color.g * illuminFrac,0,1);
        obj->vColors[i+2] = clamp(color.b * illuminFrac,0,1);
    }

}

//Multi Object helper structures
/*typedef enum {nTRN, nSHP, nGRP}mpTypes;
typedef struct MagicaProperties *mpNode;

typedef struct MagicaProperties{
    mpTypes Type;
    int data[7];
    int hidden;
    char *name;
    Vector3 position;
    int rotation;
    mpNode next;
}MagicaProperties;

//Load a voxel model with various parts into multiple entities parented to a main object
void LoadMultiVoxelModel(EntityID entity, char modelPath[])
{
    printf("\nLoading multi model: %s\n",modelPath);
    FILE* file = fopen(modelPath,"rb");

    if(file == NULL){
        printf("Failed to open file!\n");
	    perror("Error");
        //Return an empty MultiVoxelObject in case of failure
        return;
    }

    VoxelObjectList modelsList = InitializeObjectList();
    mpNode propertiesListStart = NULL;
    mpNode propertiesListEnd = NULL;

    Voxel *voxelData = NULL;

    MultiVoxelObject mobj;

    int currentModel = 0;
    int i;

    //Get file length and return to start
    fseek(file, 0L, SEEK_END);
    int fileLength = ftell(file);
    printf("File length: %d\n",fileLength);
    rewind(file);

    //Get magic word to see if this is a magicavoxel .vox file
    char *magic = calloc(5,sizeof(char));
    fread(magic,sizeof(char),4,file);

    //Get file version
    int version;
    fread(&version,sizeof(int),1,file);
    printf("Version: %d\n",version);

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

            //printf("Chunk: %s\n",chunkId);

            //There are 5 chunks we only care about: SIZE, XYZI, nTRN, nSHP and nGRP
            //SIZE chunk contains the dimensions of the model
            if (strcmp(chunkId,"SIZE") == 0)
            {        
                //Only create a new object if the last one has been sucessfully loaded
                //If not, flag an error
                if(!voxelData){

                    //Create the new object and add to the models list
                    VoxelObject *newModel = malloc(sizeof(VoxelObject));
                    AddObjectInList(&modelsList,newModel);

                    fread(&modelsList.list[currentModel]->dimension[0],sizeof(int),1,file);
                    fread(&modelsList.list[currentModel]->dimension[1],sizeof(int),1,file);
                    fread(&modelsList.list[currentModel]->dimension[2],sizeof(int),1,file);

                }else{
                    printf("An error loading has ocurred!\n");
                    fclose(file);
                    return (MultiVoxelObject){0,(VoxelObjectList){0,0},VECTOR3_ZERO,VECTOR3_ZERO,VECTOR3_ZERO,};
                }
            }
            //XYZI contains the voxels position and color index
            else if (strcmp(chunkId,"XYZI") == 0)
            {
                // XYZI contains n voxels
                fread(&modelsList.list[currentModel]->voxelCount,sizeof(int),1,file);
                modelsList.list[currentModel]->voxelsRemaining = modelsList.list[currentModel]->voxelCount;

                voxelData = calloc(modelsList.list[currentModel]->voxelCount,sizeof(Voxel));

                //Free strings and return empty Object if it fails to allocate memory
                if(!voxelData){
                    printf("Failed to allocate voxel array!\n");
                    fclose(file);
                    free(magic);
                    free(chunkId);
                    return (MultiVoxelObject){0,(VoxelObjectList){0,0},VECTOR3_ZERO,VECTOR3_ZERO,VECTOR3_ZERO,};
                }
                
                for(i = 0; i < modelsList.list[currentModel]->voxelCount; i++){
                    //Get MagicaVoxel voxel position and color index and insert in the array
                    fread(&voxelData[i].x,sizeof(char),1,file);
                    fread(&voxelData[i].y,sizeof(char),1,file);
                    fread(&voxelData[i].z,sizeof(char),1,file);
                    fread(&voxelData[i].color,sizeof(char),1,file);
                }

                //After loading all voxels into the array, allocate and transfer them to the new objects

                //Allocating memory and initializing structures
                modelsList.list[currentModel]->model = calloc(modelsList.list[currentModel]->dimension[0] * modelsList.list[currentModel]->dimension[1] * modelsList.list[currentModel]->dimension[2],sizeof(unsigned char));
                modelsList.list[currentModel]->lighting = calloc(modelsList.list[currentModel]->dimension[0] * modelsList.list[currentModel]->dimension[1] * modelsList.list[currentModel]->dimension[2],sizeof(unsigned char));

                modelsList.list[currentModel]->vertices = NULL;
                modelsList.list[currentModel]->vColors = NULL;
                modelsList.list[currentModel]->numberOfVertices = 0;
                
                //Inserting voxels into the VoxelObject structure
                for (i = 0; i < modelsList.list[currentModel]->voxelCount; i++)
                {
                    
                    // do not store this voxel if it lies out of range of the voxel chunk (126x126x126)
                    if (voxelData[i].x > 126 || voxelData[i].y > 126 || voxelData[i].z > 126) continue;

                    // Insert data based on the index = (x + z*sizeX + y*sizeX*sizeZ)
                    int voxel = (voxelData[i].x + voxelData[i].z * modelsList.list[currentModel]->dimension[0] + voxelData[i].y * modelsList.list[currentModel]->dimension[0] * modelsList.list[currentModel]->dimension[2]);
                    modelsList.list[currentModel]->model[voxel] = voxelData[i].color;
                }

                //Initialize lighting array
                for(i = 0;i<modelsList.list[currentModel]->dimension[0]*modelsList.list[currentModel]->dimension[1]*modelsList.list[currentModel]->dimension[2]; i++){
                    modelsList.list[currentModel]->lighting[i] = 1;
                }

                modelsList.list[currentModel]->position = (Vector3){0,0,0};
                modelsList.list[currentModel]->rotation = (Vector3){0,0,0};
                modelsList.list[currentModel]->center = (Vector3){modelsList.list[currentModel]->dimension[0]/2,modelsList.list[currentModel]->dimension[1]/2,modelsList.list[currentModel]->dimension[2]/2};
                modelsList.list[currentModel]->numberOfPoints = 0;
                modelsList.list[currentModel]->points = NULL;

                modelsList.list[currentModel]->modificationStartX = 0;
                modelsList.list[currentModel]->modificationEndX = modelsList.list[currentModel]->dimension[0]-1;

                modelsList.list[currentModel]->modificationStartY = 0;
                modelsList.list[currentModel]->modificationEndY = modelsList.list[currentModel]->dimension[1]-1;

                modelsList.list[currentModel]->modificationStartZ = 0;
                modelsList.list[currentModel]->modificationEndZ = modelsList.list[currentModel]->dimension[2]-1;
                
                CalculateRendered(modelsList.list[currentModel]);
                CalculateLighting(modelsList.list[currentModel]);

                modelsList.list[currentModel]->modificationStartX = -1;
                modelsList.list[currentModel]->modificationEndX = -1;

                modelsList.list[currentModel]->modificationStartY = -1;
                modelsList.list[currentModel]->modificationEndY = -1;

                modelsList.list[currentModel]->modificationStartZ = -1;
                modelsList.list[currentModel]->modificationEndZ = -1;

                modelsList.list[currentModel]->enabled = 1;

                //Pass to the next model
                free(voxelData);
                voxelData = NULL;
                currentModel++;

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
            //Will be put into a properties list, but wont be used for now
            //In the future, read the data instead of just ignore it
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

                fseek(file,chunkSize,SEEK_CUR);
            }
            else {fseek(file,chunkSize,SEEK_CUR);}   //Read any excess bytes
        }
        free(chunkId);
    }else{
        //Magic word is not VOX
        printf("Magic word is not 'VOX ', but '%s'\n",magic);
        return (MultiVoxelObject){0,(VoxelObjectList){0,0},VECTOR3_ZERO,VECTOR3_ZERO,VECTOR3_ZERO,};
    }
    free(magic);
    fclose(file);

    //If the file being loaded doesn't contains any property (files from Magicavoxel ver <0.99), just put the objects loaded as is
    if(!propertiesListStart){
        mobj.objects = modelsList;
        printf("File doesn't contain any property, considering all objects active and with position zero\n");
        printf(">DONE!\n\n");
        return mobj;
    }

    //Create the models to be inserted on the MultiObject and copy the needed data

    VoxelObject *obj = NULL;
    Vector3 pos = VECTOR3_ZERO,rot;
    int enab = 1;

    mobj.objects = InitializeObjectList();
    mpNode current = propertiesListStart;
    while(current){
        switch (current->Type){
            case nTRN:
            //Set object transform
            //printf("\n Type:%s Data: %d %d %d %d %d %d %d Name: [%s] Hidden: %d Rot: %d Pos: %.1f %.1f %.1f \n",current->Type? (current->Type == 2? "nGRP":"nSHP"):"nTRN", current->data[0], current->data[1], current->data[2], current->data[3], current->data[4], current->data[5], current->data[6], current->name? current->name:"", current->hidden,current->rotation, current->position.x, current->position.y, current->position.z);

            enab = !current->hidden;
            pos = current->position;
            //Each number is a binary encodification of a rotation. As I haven't figured
            //how it works, i just enumerated each possible rotation and inversion (28 with identity)
            //for now, single axis inversion is not supported, and others coincide with common rotations
            switch(current->rotation){
                case 40:
                    rot = (Vector3){90,0,0};
                break;
                case 105:
                    rot = (Vector3){90,90,0};
                break;
                case 100:
                    rot = (Vector3){180,0,0};
                break;
                case 118:
                    rot = (Vector3){180,90,0};
                break;
                case 65:
                    rot = (Vector3){180,0,90};
                break;
                case 9:
                    rot = (Vector3){270,270,0};
                break;
                case 72:
                    rot = (Vector3){270,0,0};
                break;
                case 82:
                    rot = (Vector3){270,0,90};
                break;
                case 70:
                    rot = (Vector3){0,90,0};
                break;
                case 89:
                    rot = (Vector3){0,90,90};
                break;
                case 84:
                    rot = (Vector3){0,180,0};
                break;
                case 24:
                    rot = (Vector3){270,180,0};
                break;
                case 113:
                    rot = (Vector3){0,180,90};
                break;
                case 22:
                    rot = (Vector3){0,270,0};
                break;
                case 50:
                    rot = (Vector3){90,0,270};
                break;
                case 17:
                    rot = (Vector3){0,0,90};
                break;
                case 57:
                    rot = (Vector3){90,270,0};
                break;
                case 2:
                    rot = (Vector3){90,0,90};
                break;
                case 52:
                    rot = (Vector3){0,0,180};
                break;
                case 120:
                    rot = (Vector3){90,180,0};
                break;
                case 38:
                    rot = (Vector3){90,270,90};
                break;
                case 33:
                    rot = (Vector3){0,0,270};
                break;
                case 98:
                    rot = (Vector3){270,0,270};
                break;
                case 20:
                    rot = (Vector3){0,0,0};
                break;
                case 36:
                    rot = (Vector3){0,0,0};
                break;
                case 68:
                    rot = (Vector3){0,0,0};
                break;
                default:
                    rot = VECTOR3_ZERO;
                break;
            }

            //Get shape corresponding to the object
            mpNode shp = propertiesListStart;
            for(i=0;i<current->data[2];i++){
                shp = shp->next;
            }

            if(shp->Type != nSHP){current = current->next; continue;}
            
            //Allocate and copy data to the final object
            obj = malloc(sizeof(VoxelObject));

            obj->dimension[0] = modelsList.list[shp->data[3]]->dimension[0];
            obj->dimension[1] = modelsList.list[shp->data[3]]->dimension[1];
            obj->dimension[2] = modelsList.list[shp->data[3]]->dimension[2];
            *obj = *modelsList.list[shp->data[3]];

            obj->model = calloc(obj->dimension[0]*obj->dimension[1]*obj->dimension[2], sizeof(unsigned char));
            obj->lighting = calloc(obj->dimension[0]*obj->dimension[1]*obj->dimension[2], sizeof(unsigned char));
            obj->vertices = calloc(modelsList.list[shp->data[3]]->numberOfVertices * 3, sizeof(GLfloat));
            obj->vColors = calloc(modelsList.list[shp->data[3]]->numberOfVertices * 3, sizeof(GLfloat));
            obj->numberOfVertices = modelsList.list[shp->data[3]]->numberOfVertices;

            memcpy(obj->model, modelsList.list[shp->data[3]]->model, obj->dimension[0]*obj->dimension[1]*obj->dimension[2] * sizeof(unsigned char) );
            memcpy(obj->lighting, modelsList.list[shp->data[3]]->lighting, obj->dimension[0]*obj->dimension[1]*obj->dimension[2] * sizeof(unsigned char) );
            memcpy(obj->vertices,modelsList.list[shp->data[3]]->vertices,obj->numberOfVertices * 3 * sizeof(GLfloat) );
            memcpy(obj->vColors,modelsList.list[shp->data[3]]->vColors,obj->numberOfVertices * 3 * sizeof(GLfloat) );

            obj->position = (Vector3){pos.x - (obj->dimension[0]/2),pos.y - (obj->dimension[1]/2),pos.z - (obj->dimension[2]/2)};
            obj->rotation = rot;
            obj->enabled = enab;

            AddObjectInList(&mobj.objects,obj);

            break;
            case nSHP:
            //This data is used in nTRN type properties to get the shape in the list
           // printf("\n Type:%s Data: %d %d %d %d %d\n",current->Type? (current->Type == 2? "nGRP":"nSHP"):"nTRN", current->data[0], current->data[1], current->data[2], current->data[3], current->data[4]);

            break;
            case nGRP:
            //This group data is ignored, as it is not supported
            //printf("\n Type:%s\n", current->Type? (current->Type == 2? "nGRP":"nSHP"):"nTRN");  
            break;
        }
        current = current->next;
    }

    //Free properties list
    current = propertiesListStart;
    while(current){
        if(current->Type == nTRN){
            free(current->name);
        }
        mpNode aux = current;
        current = current->next;
        free(aux);
    }

    FreeObjectList(&modelsList);

    printf(">DONE!\n\n");
    return mobj;
}*/