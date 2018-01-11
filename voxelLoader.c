#include "voxelLoader.h"

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
    printf("\nLoading multi model: %s\n",modelPath);
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
        return (VoxelObject){0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    }

    unsigned char *data = NULL;
    unsigned char *lighting = NULL;
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
                        return (VoxelObject){0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
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
        return (VoxelObject){0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    }
    free(magic);
    free(voxelData);

    obj.voxelCount = numVoxels;
    obj.voxelsRemaining = numVoxels;
    obj.model = data;
    obj.lighting = lighting;
    obj.vertices = NULL;
    obj.vColors = NULL;
    obj.numberOfVertices = 0;
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