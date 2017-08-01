#include "voxelLoader.h"

extern VoxelObject **scene;
extern unsigned int sceneObjectCount;

int LoadMap(char mapPath[]){
    printf("Loading Map: %s\n",mapPath);
    FILE *map = fopen(mapPath,"r");

    if(map == NULL){
        printf("> Failed to load! Error %d\n\n",errno);
        return -1;
    }
    FreeScene();
    
    //Obtém o número de objetos a se carregar
    fscanf(map,"%d",&sceneObjectCount);
    printf("Number of objects: %d\n\n",sceneObjectCount);
    scene = (VoxelObject **)calloc(sceneObjectCount,sizeof(VoxelObject*));

    FILE *voxelFile;
    int x,y,z;
    char *modelString = (char *)calloc(100,sizeof(char));

    for(int i=0; i<sceneObjectCount; i++){
        
        scene[i] = (VoxelObject *)calloc(1,sizeof(VoxelObject));

        fscanf(map,"%d",&x);
        fscanf(map,"%d",&y);
        fscanf(map,"%d",&z);
        printf("Position: %d %d %d\n",x,y,z);
        //Carregando modelo da cena
        char tempString[100];
        fgets(tempString,100,map);
        modelString = strtok(tempString,"\n");

        printf("Opening: %s\n",modelString);
        voxelFile = fopen(modelString,"rb");
        *scene[i] = FromMagica(voxelFile);
        fclose(voxelFile);

        scene[i]->position = (Vector3){x,y,z};

    }
    free(modelString);
    printf(">Map Loaded Sucessfully!\n\n");
    return 0;
}

void FreeScene(){
    for(int i=0; i<sceneObjectCount; i++){
        FreeObject(scene[i]);
        free(scene[i]);
    }
    free(scene);
    sceneObjectCount = 0;
}

Voxel newVoxel(FILE * file,unsigned int sizey)
{
    Voxel data;
    //data.x = (byte)(subsample ? stream.ReadByte() / 2 : stream.ReadByte());
    fread(&data.x,sizeof(char),1,file);
    //data.x = (subsample? data.x/2:data.x);

    //data.y = (byte)(subsample ? stream.ReadByte() / 2 : stream.ReadByte());
    fread(&data.y,sizeof(char),1,file);
    //data.y = (subsample? data.y/2:data.y);
    data.y = sizey-1-data.y;
    //data.z = (byte)(subsample ? stream.ReadByte() / 2 : stream.ReadByte());
    fread(&data.z,sizeof(char),1,file);
    //data.z = (subsample? data.z/2:data.z);

    //data.color = stream.ReadByte();
    fread(&data.color,sizeof(char),1,file);
    return data;
}

/// <summary>
/// Load a MagicaVoxel .vox format file into the custom unsigned short int[] structure that we use for voxel chunks.
/// </summary>
/// <param name="stream">An open BinaryReader stream that is the .vox file.</param>
/// <param name="overrideColors">Optional color lookup table for converting RGB values into my internal engine color format.</param>
/// <returns>The voxel chunk data for the MagicaVoxel .vox file.</returns>
VoxelObject FromMagica(FILE* file)
{
    // check out http://voxel.codeplex.com/wikipage?title=VOX%20Format&referringTitle=Home for the file format used below
    // we're going to return a voxel chunk worth of data

    if(file == NULL){
        printf("Failed to open file!\n");
        return (VoxelObject){0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,(Vector3){0,0,0}};
    }

    unsigned char *data = NULL;
    unsigned char *lighting = NULL;
    unsigned short int **render = NULL;
    unsigned short int *colors = NULL;
    Voxel *voxelData = NULL;
    AnchorPoint *points = NULL;

    VoxelObject obj;

    int numVoxels,numPoints = 0;
    fseek(file, 0L, SEEK_END);
    int fileLength = ftell(file);
    printf("File length: %d\n",fileLength);
    rewind(file);

    char *magic = (char *)calloc(5,sizeof(char));
    fread(magic,sizeof(char),4,file);

    int version;
    fread(&version,sizeof(int),1,file);
    printf("Version: %d\n",version);

    // a MagicaVoxel .vox file starts with a 'magic' 4 character 'VOX ' identifier
    if (strcmp("VOX ",magic) == 0)
    {
        int sizex = 0, sizey = 0, sizez = 0;
        char *chunkId = (char *)calloc(5,sizeof(char));

        while (ftell(file)<fileLength)
        {
            // each chunk has an ID, size and child chunks
            fread(chunkId,sizeof(char),4,file);

            int chunkSize;
            fread(&chunkSize,sizeof(int),1,file);

            int childChunks;
            fread(&childChunks,sizeof(int),1,file);

 
            // there are only 2 chunks we only care about, and they are SIZE and XYZI
            if (strcmp(chunkId,"SIZE") == 0)
            {        
                fread(&sizex,sizeof(int),1,file);
                fread(&sizey,sizeof(int),1,file);
                fread(&sizez,sizeof(int),1,file);

                obj.dimension[0] = sizex;
                obj.dimension[1] = sizey;
                obj.dimension[2] = sizez;

                printf("Dimension: %d %d %d\n",obj.dimension[0],obj.dimension[1],obj.dimension[2]);

                fseek(file,(chunkSize - 4 * 3),SEEK_CUR);
            }
            else if (strcmp(chunkId,"XYZI") == 0)
            {
                // XYZI contains n voxels
                fread(&numVoxels,sizeof(int),1,file);
                printf("Number of Voxels: %d\n",numVoxels);
 
                // each voxel has x, y, z and color index values
                voxelData = (Voxel*) calloc(numVoxels, sizeof(Voxel));
                for (int i = 0; i < numVoxels; i++){
                    voxelData[i] = newVoxel(file,obj.dimension[1]);
                }
            }
            else if (strcmp(chunkId,"RGBA") == 0)
            {
                colors = (unsigned short int *)calloc(256,sizeof(unsigned short int));

                for (int i = 0; i < 256; i++)
                {
                    unsigned char r,g,b,a;
                    fread(&r,sizeof(unsigned char),1,file);
                    fread(&g,sizeof(unsigned char),1,file);
                    fread(&b,sizeof(unsigned char),1,file);
                    fread(&a,sizeof(unsigned char),1,file);

                    // convert RGBA to our custom voxel format (16 bits, 0RRR RRGG GGGB BBBB)
                    colors[i] = (unsigned short int)(((r & 0x1f) << 10) | ((g & 0x1f) << 5) | (b & 0x1f));
                }
            }
            else {fseek(file,chunkSize,SEEK_CUR);}   // read any excess bytes
        }
        //if (sizeof(voxelData)/sizeof(Voxel) == 0){printf("Failed to read voxels"); return data;} // failed to read any valid voxel data
        obj.maxDimension = max(max(obj.dimension[0],obj.dimension[1]),obj.dimension[2]);

        //Allocating memory and initializing structures
        data = (unsigned char *)calloc(obj.maxDimension * obj.maxDimension * obj.maxDimension,sizeof(unsigned char));
        lighting = (unsigned char *)calloc(obj.maxDimension * obj.maxDimension * obj.maxDimension,sizeof(unsigned char));

        
        render = (unsigned short int **)calloc(obj.maxDimension,sizeof(unsigned short int*));
        for(int i=0;i<obj.maxDimension;i++){
            render[i] = (unsigned short int *)calloc(obj.maxDimension*obj.maxDimension,sizeof(unsigned short int));
        }
        
        // now push the voxel data into our voxel chunk structure
        for (int i = 0; i < numVoxels; i++)
        {
            
            // do not store this voxel if it lies out of range of the voxel chunk (32x128x32)
            if (voxelData[i].x > obj.maxDimension || voxelData[i].y > obj.maxDimension || voxelData[i].z > obj.maxDimension) continue;
            if(voxelData[i].color != 1){
                // use the voxColors array by default, or overrideColor if it is available
                int voxel = (voxelData[i].x + voxelData[i].z * obj.maxDimension + voxelData[i].y * obj.maxDimension * obj.maxDimension);
                data[voxel] = voxelData[i].color;//(colors == NULL ? voxColors[voxelData[i].color - 1] : colors[voxelData[i].color - 1]);
            }else{
                numPoints++;
            }
        }
        printf("Points: %d\n",numPoints);
        if(numPoints>0){
            points = (AnchorPoint *)calloc(numPoints,sizeof(AnchorPoint));
            int aux = numPoints;
            for (int i = 0; i < numVoxels; i++)
            {
                if (voxelData[i].x > obj.maxDimension || voxelData[i].y > obj.maxDimension || voxelData[i].z > obj.maxDimension) continue;
                if(voxelData[i].color == 1){
                    points[--aux] = (AnchorPoint){0,voxelData[i].x,voxelData[i].y,voxelData[i].z};
                }
            }
        }
        for(int i = 0;i<obj.maxDimension*obj.maxDimension*obj.maxDimension; i++){
            lighting[i] = 1;
        }
        free(chunkId);
    }else{
        printf("Magic word: %s\n",magic);
    }
    free(magic);
    if(voxelData != NULL){
        free(voxelData);
    }
    if(colors != NULL){
        free(colors);
    }
    if(data == NULL){
        data = (unsigned char *)calloc(1,sizeof(unsigned char));
    }
    
    obj.voxelCount = numVoxels;
    obj.voxelsRemaining = numVoxels;
    obj.model = data;
    obj.lighting = lighting;
    obj.render = render;
    obj.position = (Vector3){0,0,0};
    obj.numberOfPoints = numPoints;
    obj.points = points;

    obj.modificationStartX = 0;
    obj.modificationEndX = obj.maxDimension-1;

    obj.modificationStartY = 0;
    obj.modificationEndY = obj.maxDimension-1;

    obj.modificationStartZ = 0;
    obj.modificationEndZ = obj.maxDimension-1;
    
    CalculateRendered(&obj);
    CalculateLighting(&obj);

    obj.modificationStartX = -1;
    obj.modificationEndX = -1;

    obj.modificationStartY = -1;
    obj.modificationEndY = -1;

    obj.modificationStartZ = -1;
    obj.modificationEndZ = -1;

    obj.enabled = 1;

    printf(">DONE!\n\n");
    return obj;
}

void FreeObject(VoxelObject *obj){
    if(obj->model!=NULL)
        free(obj->model);

    if(obj->lighting!=NULL)
        free(obj->lighting);

    for(int i=0;i<obj->maxDimension;i++){
        if(obj->render[i]!=NULL)
            free(obj->render[i]);
    }
    if(obj->render!=NULL)
        free(obj->render);
}