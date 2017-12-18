#include "voxelLoader.h"

extern VoxelObjectList Scene;

int LoadMap(char mapPath[]){
    printf("Loading Map: %s\n",mapPath);
    FILE *map = fopen(mapPath,"r");
    
    if(map == NULL){
        printf("> Failed to load!\n\n");
        return -1;
    }
    FreeScene();
    
    //Get the number of objects to load
    int numberOfObjs;
    fscanf(map,"%d",&numberOfObjs);
    printf("Number of objects: %d\n\n",numberOfObjs);
    Scene = InitializeObjectList();

    int x,y,z;
    char *modelString = NULL;

    for(int i=0; i<numberOfObjs; i++){
        
        VoxelObject *newObj = calloc(1,sizeof(VoxelObject));
        AddObjectInList(&Scene,newObj);

        fscanf(map,"%d",&x);
        fscanf(map,"%d",&y);
        fscanf(map,"%d",&z);
        printf("Position: %d %d %d\n",x,y,z);
        //Carregando modelo da cena
        char tempString[100];
        fgets(tempString,100,map);

        #ifndef __unix__
        modelString = strtok(tempString,"\n");
        #else
        modelString = strtok(tempString,"\r");
        #endif

        printf("Opening: (%s)\n",modelString);
        *Scene.list[i] = LoadVoxelModel(modelString);

        Scene.list[i]->position = (Vector3){x,y,z};

    }
    
    printf(">Map Loaded Sucessfully!\n\n");
    return 0;
}

void FreeScene(){
    FreeObjectList(&Scene);
}


//MagicaVoxel Voxel structure, used only to load data
typedef struct Voxel
{
    char x;
    char y;
    char z;
    char color;
}Voxel;

typedef enum {nTRN, nSHP, nGRP}mpTypes;
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

MultiVoxelObject LoadMultiVoxelModel(char modelPath[])
{
    printf("\nLoading model: %s\n",modelPath);
    FILE* file = fopen(modelPath,"rb");

    if(file == NULL){
        printf("Failed to open file!\n");
	perror("Error");
        //Return an empty MultiVoxelObject in case of failure
        return (MultiVoxelObject){0,(VoxelObjectList){0,0},VECTOR3_ZERO,VECTOR3_ZERO,VECTOR3_ZERO,};
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

                modelsList.list[currentModel]->render = calloc(modelsList.list[currentModel]->dimension[2],sizeof(unsigned short int*));
                
                for(i=0;i<modelsList.list[currentModel]->dimension[2];i++){
                    //Alloc a list of voxels to render, with one extra position with the number of voxels in that z slice
                    modelsList.list[currentModel]->render[i] = calloc(1 + modelsList.list[currentModel]->dimension[0] * modelsList.list[currentModel]->dimension[1],sizeof(unsigned short int));
                }
                
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
                    //characters of the object hidden status, so read the character and ignore it for now
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

    //Create the models to be inserted on the MultiObject and copy the needed data

    VoxelObject *obj = NULL;
    Vector3 pos = VECTOR3_ZERO,rot;
    int enab = 1;

    mobj.objects = InitializeObjectList();
    mpNode current = propertiesListStart;
    while(current){
        switch (current->Type){
            case nTRN:
            printf("\n Type:%s Data: %d %d %d %d %d %d %d Name: [%s] Hidden: %d Rot: %d Pos: %.1f %.1f %.1f \n",current->Type? (current->Type == 2? "nGRP":"nSHP"):"nTRN", current->data[0], current->data[1], current->data[2], current->data[3], current->data[4], current->data[5], current->data[6], current->name? current->name:"", current->hidden,current->rotation, current->position.x, current->position.y, current->position.z);

            pos = current->position;
            rot = VECTOR3_ZERO;
            enab = !current->hidden;

            break;
            case nSHP:
            printf("\n Type:%s Data: %d %d %d %d %d\n",current->Type? (current->Type == 2? "nGRP":"nSHP"):"nTRN", current->data[0], current->data[1], current->data[2], current->data[3], current->data[4]);

            obj = malloc(sizeof(VoxelObject));

            obj->dimension[0] = modelsList.list[current->data[3]]->dimension[0];
            obj->dimension[1] = modelsList.list[current->data[3]]->dimension[1];
            obj->dimension[2] = modelsList.list[current->data[3]]->dimension[2];
            *obj = *modelsList.list[current->data[3]];
            printf("Copied initial data\n");

            obj->model = calloc(obj->dimension[0]*obj->dimension[1]*obj->dimension[2], sizeof(unsigned char));
            obj->lighting = calloc(obj->dimension[0]*obj->dimension[1]*obj->dimension[2], sizeof(unsigned char));
            obj->render = calloc(obj->dimension[2], sizeof(unsigned short int*));
            for(i=0;i<obj->dimension[2];i++){
                obj->render[i] = calloc(1+obj->dimension[1]*obj->dimension[0], sizeof(unsigned short int) );
            }
            printf("Allocated arrays\n");

            memcpy(obj->model, modelsList.list[current->data[3]]->model, obj->dimension[0]*obj->dimension[1]*obj->dimension[2] * sizeof(unsigned char) );
            memcpy(obj->lighting, modelsList.list[current->data[3]]->lighting, obj->dimension[0]*obj->dimension[1]*obj->dimension[2] * sizeof(unsigned char) );
            
            for(i=0;i<obj->dimension[2];i++){
                 memcpy(obj->render[i],modelsList.list[current->data[3]]->render[i],(1 + obj->dimension[0]*obj->dimension[1])* sizeof(unsigned short int) );
            }
            printf("Copied Array data\n");

            obj->position = (Vector3){pos.x - (obj->dimension[0]/2),pos.y - (obj->dimension[1]/2),pos.z - (obj->dimension[2]/2)};
            obj->rotation = rot;
            obj->enabled = enab;

            AddObjectInList(&mobj.objects,obj);
            break;
            case nGRP:
            printf("\n Type:%s\n", current->Type? (current->Type == 2? "nGRP":"nSHP"):"nTRN");
            //Ignore the group data, as it is not supported
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

    printf(">DOONE!\n\n");
    return mobj;
}





//Load a single Voxel model from a .vox file.
//Receives the path to the file as a string (Ex: "path/model.vox")
VoxelObject LoadVoxelModel(char modelPath[])
{
    printf("\nLoading model: %s\n",modelPath);
    FILE* file = fopen(modelPath,"rb");

    if(file == NULL){
        printf("Failed to open file!\n");
	perror("Error");
        //Return an empty VoxelObject in case of failure
        return (VoxelObject){0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,(Vector3){0,0,0}};
    }

    unsigned char *data = NULL;
    unsigned char *lighting = NULL;
    unsigned short int **render = NULL;
    Voxel *voxelData = NULL;
    AnchorPoint *points = NULL;

    VoxelObject obj;

    int numVoxels = 0,numPoints = 0,modelLoaded = 0;
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

                    fread(&obj.dimension[0],sizeof(int),1,file);
                    fread(&obj.dimension[1],sizeof(int),1,file);
                    fread(&obj.dimension[2],sizeof(int),1,file);

                    printf("Dimension: %d %d %d\n",obj.dimension[0],obj.dimension[1],obj.dimension[2]);

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
                        return (VoxelObject){0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,(Vector3){0,0,0}};
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
        data = calloc(obj.dimension[0] * obj.dimension[1] * obj.dimension[2],sizeof(unsigned char));
        lighting = calloc(obj.dimension[0] * obj.dimension[1] * obj.dimension[2],sizeof(unsigned char));

        render = calloc(obj.dimension[2],sizeof(unsigned short int*));
        
        for(i=0;i<obj.dimension[2];i++){
            //Alloc a list of voxels to render, with one extra position with the number of voxels in that z slice
            render[i] = calloc(1 + obj.dimension[0] * obj.dimension[1],sizeof(unsigned short int));
        }
        
        //Inserting voxels into the VoxelObject structure
        for (i = 0; i < numVoxels; i++)
        {
            
            // do not store this voxel if it lies out of range of the voxel chunk (126x126x126)
            if (voxelData[i].x > 126 || voxelData[i].y > 126 || voxelData[i].z > 126) continue;

            //Get voxel with special properties as points, and dont render them
            if(voxelData[i].color == 1){
                numPoints++;
                // Insert data based on the index = (x + z*sizeX + y*sizeX*sizeZ)
                int voxel = (voxelData[i].x + voxelData[i].z * obj.dimension[0] + voxelData[i].y * obj.dimension[0] * obj.dimension[2]);
                data[voxel] = 0;
            }else{
                // Insert data based on the index = (x + z*sizeX + y*sizeX*sizeZ)
                int voxel = (voxelData[i].x + voxelData[i].z * obj.dimension[0] + voxelData[i].y * obj.dimension[0] * obj.dimension[2]);
                data[voxel] = voxelData[i].color;
            }
        }

        printf("Points: %d\n",numPoints);
        //Get helper points
        if(numPoints>0){
            points = calloc(numPoints,sizeof(AnchorPoint));
            int aux = numPoints;
            for (i = 0; i < numVoxels; i++)
            {
                if (voxelData[i].x > obj.dimension[0] || voxelData[i].y > obj.dimension[1] || voxelData[i].z > obj.dimension[2]) continue;
                if(voxelData[i].color == 1){
                    points[--aux] = (AnchorPoint){0,voxelData[i].x,voxelData[i].y,voxelData[i].z};
                }
            }
        }

        //Initialize lighting array
        for(i = 0;i<obj.dimension[0]*obj.dimension[1]*obj.dimension[2]; i++){
            lighting[i] = 1;
        }
        free(chunkId);
    }else{
        printf("Magic word is not 'VOX ', but '%s'\n",magic);
        return (VoxelObject){0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,(Vector3){0,0,0}};
    }
    free(magic);
    free(voxelData);

    obj.voxelCount = numVoxels;
    obj.voxelsRemaining = numVoxels;
    obj.model = data;
    obj.lighting = lighting;
    obj.render = render;
    obj.position = (Vector3){0,0,0};
    obj.rotation = (Vector3){0,0,0};
    obj.center = (Vector3){obj.dimension[0]/2,obj.dimension[1]/2,obj.dimension[2]/2};
    obj.numberOfPoints = numPoints;
    obj.points = points;

    obj.modificationStartX = 0;
    obj.modificationEndX = obj.dimension[0]-1;

    obj.modificationStartY = 0;
    obj.modificationEndY = obj.dimension[1]-1;

    obj.modificationStartZ = 0;
    obj.modificationEndZ = obj.dimension[2]-1;
    
    CalculateRendered(&obj);
    CalculateLighting(&obj);

    obj.modificationStartX = -1;
    obj.modificationEndX = -1;

    obj.modificationStartY = -1;
    obj.modificationEndY = -1;

    obj.modificationStartZ = -1;
    obj.modificationEndZ = -1;

    obj.enabled = 1;

    fclose(file);

    printf(">DONE!\n\n");
    return obj;
}