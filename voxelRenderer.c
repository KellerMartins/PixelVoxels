#include "voxelRenderer.h"

// this is the default palette of voxel colors (the RGBA chunk is only included if the palette is differe)
/*unsigned short int voxColors[256] = { 32767, 25599, 19455, 13311, 7167, 1023, 32543, 25375, 19231, 13087, 6943, 799, 32351, 25183, 
    19039, 12895, 6751, 607, 32159, 24991, 18847, 12703, 6559, 415, 31967, 24799, 18655, 12511, 6367, 223, 31775, 24607, 18463, 12319, 6175, 31, 
    32760, 25592, 19448, 13304, 7160, 1016, 32536, 25368, 19224, 13080, 6936, 792, 32344, 25176, 19032, 12888, 6744, 600, 32152, 24984, 18840, 
    12696, 6552, 408, 31960, 24792, 18648, 12504, 6360, 216, 31768, 24600, 18456, 12312, 6168, 24, 32754, 25586, 19442, 13298, 7154, 1010, 32530, 
    25362, 19218, 13074, 6930, 786, 32338, 25170, 19026, 12882, 6738, 594, 32146, 24978, 18834, 12690, 6546, 402, 31954, 24786, 18642, 12498, 6354, 
    210, 31762, 24594, 18450, 12306, 6162, 18, 32748, 25580, 19436, 13292, 7148, 1004, 32524, 25356, 19212, 13068, 6924, 780, 32332, 25164, 19020, 
    12876, 6732, 588, 32140, 24972, 18828, 12684, 6540, 396, 31948, 24780, 18636, 12492, 6348, 204, 31756, 24588, 18444, 12300, 6156, 12, 32742, 
    25574, 19430, 13286, 7142, 998, 32518, 25350, 19206, 13062, 6918, 774, 32326, 25158, 19014, 12870, 6726, 582, 32134, 24966, 18822, 12678, 6534, 
    390, 31942, 24774, 18630, 12486, 6342, 198, 31750, 24582, 18438, 12294, 6150, 6, 32736, 25568, 19424, 13280, 7136, 992, 32512, 25344, 19200, 
    13056, 6912, 768, 32320, 25152, 19008, 12864, 6720, 576, 32128, 24960, 18816, 12672, 6528, 384, 31936, 24768, 18624, 12480, 6336, 192, 31744, 
    24576, 18432, 12288, 6144, 28, 26, 22, 20, 16, 14, 10, 8, 4, 2, 896, 832, 704, 640, 512, 448, 320, 256, 128, 64, 28672, 26624, 22528, 20480, 
    16384, 14336, 10240, 8192, 4096, 2048, 29596, 27482, 23254, 21140, 16912, 14798, 10570, 8456, 4228, 2114, 1  };*/

unsigned int voxColors[256] = {
    0x00000000, 0xffffffff, 0xffccffff, 0xff99ffff, 0xff66ffff, 0xff33ffff, 0xff00ffff, 0xffffccff, 0xffccccff, 0xff99ccff, 0xff66ccff, 0xff33ccff, 0xff00ccff, 0xffff99ff, 0xffcc99ff, 0xff9999ff,
	0xff6699ff, 0xff3399ff, 0xff0099ff, 0xffff66ff, 0xffcc66ff, 0xff9966ff, 0xff6666ff, 0xff3366ff, 0xff0066ff, 0xffff33ff, 0xffcc33ff, 0xff9933ff, 0xff6633ff, 0xff3333ff, 0xff0033ff, 0xffff00ff,
	0xffcc00ff, 0xff9900ff, 0xff6600ff, 0xff3300ff, 0xff0000ff, 0xffffffcc, 0xffccffcc, 0xff99ffcc, 0xff66ffcc, 0xff33ffcc, 0xff00ffcc, 0xffffcccc, 0xffcccccc, 0xff99cccc, 0xff66cccc, 0xff33cccc,
	0xff00cccc, 0xffff99cc, 0xffcc99cc, 0xff9999cc, 0xff6699cc, 0xff3399cc, 0xff0099cc, 0xffff66cc, 0xffcc66cc, 0xff9966cc, 0xff6666cc, 0xff3366cc, 0xff0066cc, 0xffff33cc, 0xffcc33cc, 0xff9933cc,
	0xff6633cc, 0xff3333cc, 0xff0033cc, 0xffff00cc, 0xffcc00cc, 0xff9900cc, 0xff6600cc, 0xff3300cc, 0xff0000cc, 0xffffff99, 0xffccff99, 0xff99ff99, 0xff66ff99, 0xff33ff99, 0xff00ff99, 0xffffcc99,
	0xffcccc99, 0xff99cc99, 0xff66cc99, 0xff33cc99, 0xff00cc99, 0xffff9999, 0xffcc9999, 0xff999999, 0xff669999, 0xff339999, 0xff009999, 0xffff6699, 0xffcc6699, 0xff996699, 0xff666699, 0xff336699,
	0xff006699, 0xffff3399, 0xffcc3399, 0xff993399, 0xff663399, 0xff333399, 0xff003399, 0xffff0099, 0xffcc0099, 0xff990099, 0xff660099, 0xff330099, 0xff000099, 0xffffff66, 0xffccff66, 0xff99ff66,
	0xff66ff66, 0xff33ff66, 0xff00ff66, 0xffffcc66, 0xffcccc66, 0xff99cc66, 0xff66cc66, 0xff33cc66, 0xff00cc66, 0xffff9966, 0xffcc9966, 0xff999966, 0xff669966, 0xff339966, 0xff009966, 0xffff6666,
	0xffcc6666, 0xff996666, 0xff666666, 0xff336666, 0xff006666, 0xffff3366, 0xffcc3366, 0xff993366, 0xff663366, 0xff333366, 0xff003366, 0xffff0066, 0xffcc0066, 0xff990066, 0xff660066, 0xff330066,
	0xff000066, 0xffffff33, 0xffccff33, 0xff99ff33, 0xff66ff33, 0xff33ff33, 0xff00ff33, 0xffffcc33, 0xffcccc33, 0xff99cc33, 0xff66cc33, 0xff33cc33, 0xff00cc33, 0xffff9933, 0xffcc9933, 0xff999933,
	0xff669933, 0xff339933, 0xff009933, 0xffff6633, 0xffcc6633, 0xff996633, 0xff666633, 0xff336633, 0xff006633, 0xffff3333, 0xffcc3333, 0xff993333, 0xff663333, 0xff333333, 0xff003333, 0xffff0033,
	0xffcc0033, 0xff990033, 0xff660033, 0xff330033, 0xff000033, 0xffffff00, 0xffccff00, 0xff99ff00, 0xff66ff00, 0xff33ff00, 0xff00ff00, 0xffffcc00, 0xffcccc00, 0xff99cc00, 0xff66cc00, 0xff33cc00,
	0xff00cc00, 0xffff9900, 0xffcc9900, 0xff999900, 0xff669900, 0xff339900, 0xff009900, 0xffff6600, 0xffcc6600, 0xff996600, 0xff666600, 0xff336600, 0xff006600, 0xffff3300, 0xffcc3300, 0xff993300,
	0xff663300, 0xff333300, 0xff003300, 0xffff0000, 0xffcc0000, 0xff990000, 0xff660000, 0xff330000, 0xff0000ee, 0xff0000dd, 0xff0000bb, 0xff0000aa, 0xff000088, 0xff000077, 0xff000055, 0xff000044,
	0xff000022, 0xff000011, 0xff00ee00, 0xff00dd00, 0xff00bb00, 0xff00aa00, 0xff008800, 0xff007700, 0xff005500, 0xff004400, 0xff002200, 0xff001100, 0xffee0000, 0xffdd0000, 0xffbb0000, 0xffaa0000,
	0xff880000, 0xff770000, 0xff550000, 0xff440000, 0xff220000, 0xff110000, 0xffeeeeee, 0xffdddddd, 0xffbbbbbb, 0xffaaaaaa, 0xff888888, 0xff777777, 0xff555555, 0xff444444, 0xff222222, 0xff111111
};
    

extern const int GAME_SCREEN_WIDTH;
extern const int GAME_SCREEN_HEIGHT;
extern double deltaTime;

Vector3 cameraPosition = {62,71,0};

void MoveCamera(float x, float y, float z){
    cameraPosition.x +=x*deltaTime;
    cameraPosition.y +=y*deltaTime;
    cameraPosition.z +=z*deltaTime;
    printf("CamPos: |%2.1f|%2.1f|%2.1f|\n",cameraPosition.x,cameraPosition.y,cameraPosition.z);
}

void ClearScreen(Pixel* screen){
    int y,x,cp = 0;
    for(y=0;y<GAME_SCREEN_HEIGHT;y++){
        for(x=0;x<GAME_SCREEN_WIDTH;x++){
            screen[cp].r = 0;
            screen[cp].g = 0;
            screen[cp].b = 0;
            screen[cp].a = 0;
            cp++;
        }
    }
}
void FillBackground(Pixel* screen){
    int y,x,cp = 0;
    for(y=0;y<GAME_SCREEN_HEIGHT;y++){
        for(x=0;x<GAME_SCREEN_WIDTH;x++){
            if(screen[cp].a == 0){
                screen[cp].r = (y)>255? 255:y;
                screen[cp].g = 145;
                screen[cp].b = (255-y)>255? 255:(255-y<0? 0: 255-y);
            }
            cp++;
        }
    }
}

void PostProcess(Pixel* screen){
    int y,x,cp = 0;
    char outlineBrightness = 64;

    /*Vector3 sample_sphere[16] = { (Vector3){ 0.5381, 0.1856,-0.4319}, (Vector3){ 0.1379, 0.2486, 0.4430},
                                        (Vector3){ 0.3371, 0.5679,-0.0057}, (Vector3){-0.6999,-0.0451,-0.0019},
                                        (Vector3){ 0.0689,-0.1598,-0.8547}, (Vector3){ 0.0560, 0.0069,-0.1843},
                                        (Vector3){-0.0146, 0.1402, 0.0762}, (Vector3){ 0.0100,-0.1924,-0.0344},
                                        (Vector3){-0.3577,-0.5301,-0.4358}, (Vector3){-0.3169, 0.1063, 0.0158},
                                        (Vector3){ 0.0103,-0.5869, 0.0046}, (Vector3){-0.0897,-0.4940, 0.3287},
                                        (Vector3){ 0.7119,-0.0154,-0.0918}, (Vector3){-0.0533, 0.0596,-0.5411},
                                        (Vector3){ 0.0352,-0.0631, 0.5460}, (Vector3){-0.4776, 0.2847,-0.0271}
                                };*/

    for(y=0;y<GAME_SCREEN_HEIGHT;y++){
        for(x=0;x<GAME_SCREEN_WIDTH;x++){
            if(screen[cp].a!=0 && cp%GAME_SCREEN_WIDTH !=0 && cp%GAME_SCREEN_WIDTH !=GAME_SCREEN_WIDTH-1){
                if(cp-1>0){
                    if((screen[cp-1].a-screen[cp].a)<-10 || screen[cp-1].a == 0){
                        screen[cp-1].r = outlineBrightness;
                        screen[cp-1].g = outlineBrightness;
                        screen[cp-1].b = outlineBrightness;
                    }
                }
                if(cp+1<GAME_SCREEN_HEIGHT*GAME_SCREEN_WIDTH){
                    if((screen[cp+1].a-screen[cp].a)<-10 || screen[cp+1].a == 0){
                        screen[cp+1].r = outlineBrightness;
                        screen[cp+1].g = outlineBrightness;
                        screen[cp+1].b = outlineBrightness;
                    }
                }
                if(cp-GAME_SCREEN_WIDTH>0){
                    if((screen[cp-GAME_SCREEN_WIDTH].a-screen[cp].a)<-10 || screen[cp-GAME_SCREEN_WIDTH].a == 0){
                        screen[cp-GAME_SCREEN_WIDTH].r = outlineBrightness;
                        screen[cp-GAME_SCREEN_WIDTH].g = outlineBrightness;
                        screen[cp-GAME_SCREEN_WIDTH].b = outlineBrightness;
                    }
                }
                if(cp+GAME_SCREEN_WIDTH<GAME_SCREEN_HEIGHT*GAME_SCREEN_WIDTH){
                    if((screen[cp+GAME_SCREEN_WIDTH].a-screen[cp].a)<-10 || screen[cp+GAME_SCREEN_WIDTH].a == 0){
                        screen[cp+GAME_SCREEN_WIDTH].r = outlineBrightness;
                        screen[cp+GAME_SCREEN_WIDTH].g = outlineBrightness;
                        screen[cp+GAME_SCREEN_WIDTH].b = outlineBrightness;
                    }
                }
            }
            cp++;
        }
    }
}
void *RenderThread(void *arguments){
    RendererArguments *args = arguments;

    Pixel *screen = args->screen;
	VoxelObject **objs = args->objs;
	unsigned int numObjs = args->numObjs;
	VoxelObject **shadowCasters = args->shadowCasters;
	unsigned int numCasters = args->numCasters;
    int i,endLoop = 0;
    pthread_t tID;

    for(i=0; i<numObjs; i++){
        if(objs[i]->enabled){
            if(objs[i]->maxDimension >= 100 && numObjs-(i+1)>0){
				RendererArguments renderArguments = {screen,objs+(i+1),numObjs-(i+1),shadowCasters,numCasters};
				pthread_create(&tID, NULL, &RenderThread, (void *)&renderArguments);
                endLoop = 1;
            }
            if(objs[i]->modificationStartZ >=0){
                CalculateRendered(objs[i]);
                CalculateLighting(objs[i]);

                objs[i]->modificationStartX = -1;
                objs[i]->modificationEndX = -1;

                objs[i]->modificationStartY = -1;
                objs[i]->modificationEndY = -1;

                objs[i]->modificationStartZ = -1;
                objs[i]->modificationEndZ = -1;

            }
            RenderObject(screen,objs[i]);
            if(shadowCasters!=NULL){
                for(int j=0;j<numCasters;j++){
                    CalculateShadow(objs[i],shadowCasters[j]);
                }
            }
            if(endLoop){
                pthread_join(tID, NULL);
                break;
            }
        }
    }
    return NULL;
}

void RenderObject(Pixel* screen,VoxelObject *obj){

    unsigned int color = 0;

    int x,y,z,i,j,px,py,zp,startz,aux,nv,cp = 0,colorIndex,sumx = 0,sumy = 0,edgeIndx,lightIndx;

    const float edge = 0.80;
    const float base = 0.75;
    const float crease = 0.70;
    const float sunlight = 1.42;
    const float shadow = 0.79;

    float illuminFrac = 1,nz,lightVal,edgeVal;
    Pixel p;

    //z = obj->position.z>125? 125:(obj->position.z<0? 0:(int)(obj->position.z));
    startz = (obj->dimension[2]-1);
    
    if( ( ((obj->dimension[1])+roundf(obj->position.y-cameraPosition.y)+125)*2 <0 || -125 + ((obj->dimension[1])+roundf(obj->position.y-cameraPosition.y))*2 >GAME_SCREEN_HEIGHT) ||
        ( ((obj->dimension[0])+roundf(obj->position.x-cameraPosition.x)+125)*2 <0 || -125 + ((obj->dimension[0])+roundf(obj->position.x-cameraPosition.x))*2 >GAME_SCREEN_WIDTH)){
       return;
    }
    
    for(z=startz;z>=0;z--){
        zp = z + roundf(obj->position.z+cameraPosition.z);
        nz = (zp*0.5);
        if(zp<0 || zp>128){
            continue;
        }

        nv = obj->render[z][0];
        for(i = 1; i <= nv ; i++){

            x = (obj->render[z][i] & 127);
            y = ((obj->render[z][i]>>7) & 127);

            colorIndex = (x) + ((z)) * obj->maxDimension + (y) * obj->maxDimension * obj->maxDimension;
            color = voxColors[obj->model[colorIndex]];

            if(color!=0){
                //illuminFrac = obj->lighting[colorIndex]/126.0f;
                lightIndx = (obj->lighting[colorIndex] & 6)>>1;
                lightIndx *=obj->lighting[colorIndex] & 1; 
                obj->lighting[colorIndex] |= 1;
                edgeIndx = obj->lighting[colorIndex]>>3;
                
                lightVal = lightIndx == 1? 1:(lightIndx == 2? sunlight:shadow);
                edgeVal = (edgeIndx<5? edge:edgeIndx == 5? base:crease);

                illuminFrac = lightVal * edgeVal;
                illuminFrac *=((1.0+((zp*0.5))/128));
                //Pega a cor do voxel e coloca no pixel
                //A cor é transformada do int16 para cada um dos componentes RGB
                p.a = zp+1;
                p.r = ((color & 255)*illuminFrac)>255? 255:((color & 255)*illuminFrac);
                color = (color>>8);
                p.g = ((color & 255)*illuminFrac)>255? 255:((color & 255)*illuminFrac);
                color = (color>>8);
                p.b = ((color & 255)*illuminFrac)>255? 255:((color & 255)*illuminFrac);
            }
            py = ((y)+roundf(obj->position.y-cameraPosition.y)+(125-nz))*2;
            px = ((x)+roundf(obj->position.x-cameraPosition.x)+(125-nz))*2;
            for(j=0;j<4;j++){
                switch (j){
                    case 0:
                        sumx = 0;
                        sumy = 0;
                    break;
                    case 1:
                        sumx = 1;
                        sumy = 1;
                    break;
                    case 2:
                        sumx = 1;
                        sumy = 0;
                    break;
                    case 3:
                        sumx = 0;
                        sumy = 1;
                    break;
                }
                cp = sumy+py;
                cp = cp>=GAME_SCREEN_HEIGHT? -1: (cp<0? -1:cp*GAME_SCREEN_WIDTH);
                if(cp <0){
                    continue;
                }
                aux = sumx+px;
                cp = (aux)>=GAME_SCREEN_WIDTH? -1: ((aux)<0? -1:cp + (aux));
                if(cp <0){
                    continue;
                }

                if(zp+1 >= screen[cp].a){
                    screen[cp] = p;
                }
            }
            
        }
    }
}

void CalculateRendered(VoxelObject *obj){
    if(obj->modificationStartZ <0 || obj->modificationEndZ <0 ){
        return;
    }
    int x,y,z,index,dir,occ;
    for(z = obj->modificationStartZ; z<=obj->modificationEndZ ;z++){
        obj->render[z][0]=0;
        for(y = 0; y<obj->dimension[1]; y++){
            for(x = 0; x<obj->dimension[0]; x++){
                occ = 0;
                index = (x + z * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);
                if(obj->model[index]!=0){
                    if(x!=0 && x<obj->maxDimension-1 && y!=0 && y<obj->maxDimension-1 && z!=0 && z<obj->maxDimension-1){
                        dir = (x + (z+1) * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);//0 0 1
                        if(obj->model[dir]!=0){
                            occ++;
                        }
                        dir = (x + (z-1) * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);//0 0 -1
                        if(obj->model[dir]!=0){
                            occ++;
                        }
                        dir = (x + z * obj->maxDimension + (y+1) * obj->maxDimension * obj->maxDimension);//0 1 0
                        if(obj->model[dir]!=0){
                            occ++;
                        }
                        dir = (x + z * obj->maxDimension + (y-1) * obj->maxDimension * obj->maxDimension);//0 -1 0 
                        if(obj->model[dir]!=0){
                            occ++;
                        }
                        dir = ( (x+1) + z * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);//1 0 0
                        if(obj->model[dir]!=0){
                            occ++;
                        }
                        dir = ( (x-1) + z * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);//1 0 0 
                        if(obj->model[dir]!=0){
                            occ++;
                        }
                        dir = ((x-1) + z * obj->maxDimension + (y-1) * obj->maxDimension * obj->maxDimension);//-1 -1 0
                        if(obj->model[dir]!=0){
                            occ++;
                        }
                        dir = ((x+1) + z * obj->maxDimension + (y-1) * obj->maxDimension * obj->maxDimension);//-1 1 0
                        if(obj->model[dir]!=0){
                            occ++;
                        }
                        dir = ((x+1) + (z+1) * obj->maxDimension + (y+1) * obj->maxDimension * obj->maxDimension);//1 1 1
                        if(obj->model[dir]!=0){
                            occ++;
                        }
                        dir = ((x-1) + (z+1) * obj->maxDimension + (y+1) * obj->maxDimension * obj->maxDimension);//-1 1 1
                        if(obj->model[dir]!=0){
                            occ++;
                        }
                        dir = ((x-1) + (z+1) * obj->maxDimension + (y-1) * obj->maxDimension * obj->maxDimension);//-1 -1 1
                        if(obj->model[dir]!=0){
                            occ++;
                        }
                        dir = ((x+1) + (z+1) * obj->maxDimension + (y-1) * obj->maxDimension * obj->maxDimension);//1 -1 1
                        if(obj->model[dir]!=0){
                            occ++;
                        }
                    }
                    if(x==obj->maxDimension-1 && y<obj->maxDimension-1 && z<obj->maxDimension-1){
                        
                        dir = (x + z * obj->maxDimension + (y+1) * obj->maxDimension * obj->maxDimension);//0 1 0
                        if(obj->model[dir]!=0){
                            occ = 7;
                        }
                        dir = ((x+1) + (z+1) * obj->maxDimension + (y+1) * obj->maxDimension * obj->maxDimension);//1 1 1
                        if(obj->model[dir]!=0){
                            occ += 5;
                        }
                    }
                    if(z==0 && (y!=0 && x<obj->maxDimension-1)){
                        dir = (x + z * obj->maxDimension + (y+1) * obj->maxDimension * obj->maxDimension);//0 -1 0
                        if(obj->model[dir]!=0){
                            occ = 7;
                        }
                        dir = ((x+1) + (z) * obj->maxDimension + (y) * obj->maxDimension * obj->maxDimension);//1 0 0
                        if(obj->model[dir]!=0){
                            occ += 5;
                        }
                    }
                    if(y==0 && (z+1< obj->maxDimension && x+1<obj->maxDimension)){
                        dir = (x + (z+1) * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);//0 -1 0
                        if(obj->model[dir]!=0){
                            occ = 7;
                        }
                        dir = ((x+1) + (z) * obj->maxDimension + (y) * obj->maxDimension * obj->maxDimension);//1 0 0
                        if(obj->model[dir]!=0){
                            occ += 5;
                        }
                    }
                    if(occ!=12){
                        obj->render[z][0]++;
                        obj->render[z][(int)obj->render[z][0]] = (unsigned short int)(( y << 7) | x);
                    }
                }
            }
        }
    }
}


void CalculateLighting(VoxelObject *obj){

    int y,x,z,index,dir;
    int occlusion,lightAir,lightBlock;

    int zstart = obj->dimension[2]-1;
    int lightFinal;
    for(y=obj->modificationStartY; y<obj->modificationEndY; y++){
        for(x=obj->modificationStartX; x<obj->modificationEndX; x++){

            //Define a luz no topo do objeto, que é transportado para baixo a cada iteração em z
            lightAir = 1;
            lightBlock = 1;

            for(z=zstart; z>=0; z--){
                occlusion = 0;
                index = (x + z * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);
                
                if(obj->model[index]!=0){
                    if(z<obj->dimension[2]-1){ //Up
                        dir = (x + (z+1) * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);
                        occlusion += obj->model[dir]==0? 0:1;
                        //Ilumina o bloco caso o bloco acima seja vazio (com luz ou sombra), se não, mantém a cor
                        lightBlock = obj->model[dir]==0? lightAir*2:1;
                    }else{
                        lightBlock = 2;
                        occlusion = 3;
                    }
                    if(z>0){ //Down
                        dir = (x + (z-1) * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);
                        occlusion += obj->model[dir]==0?  0:1;
                    }else{
                        occlusion = 3;
                    }
                    if(x>0){ //Left
                        dir = ((x-1) + z * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);
                        occlusion += obj->model[dir]==0? 0:1;
                    }else{
                        occlusion = 2;
                    }
                    if(x<obj->dimension[0]-1){ //Right
                        dir = ((x+1) + z * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);
                        occlusion += obj->model[dir]==0?  0:1;
                    }else{
                        occlusion = 3;
                    }
                    if(y<obj->dimension[1]-1){ //Front
                        dir = (x + z * obj->maxDimension + (y+1) * obj->maxDimension * obj->maxDimension);
                        occlusion += obj->model[dir]==0?  0:1;
                    }else{
                        occlusion = 3;
                    }
                    if(y>0){ //Back
                        dir = (x + z * obj->maxDimension + (y-1) * obj->maxDimension * obj->maxDimension);
                        occlusion += obj->model[dir]==0?  0:1;
                    }else{
                        occlusion = 3;
                    }
                    lightFinal = lightBlock;
                }else{
                    if(z+1<obj->maxDimension){
                        dir = (x + (z+1) * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);
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
}

void CalculateShadow(VoxelObject *obj,VoxelObject *shadowCaster){

    int y,x,z,o,index,dir,cx,cy,cz;

    int startx = shadowCaster->position.x-obj->position.x;
    int starty = shadowCaster->position.y-obj->position.y;
    int startz = (shadowCaster->position.z-obj->position.z)+shadowCaster->dimension[2];

    int endx = startx + shadowCaster->dimension[0]+1;
    int endy = starty + shadowCaster->dimension[1];
    if(endx<0 || endy<0 ){
        return;
    }

    startx = startx <0? 0:startx;
    starty = starty <0? 0:starty;
    startz = startz <0? 0:startz;

    startz = startz >obj->maxDimension? obj->maxDimension:startz;
    endx = endx>obj->dimension[0]? obj->dimension[0] : endx;
    endy = endy>obj->dimension[1]? obj->dimension[1] : endy;
        
    int shadowVal,finalShadow;
    for(y=starty; y<endy; y++){
        for(x=startx; x<endx; x++){

            shadowVal = 1;

            for(z=startz; z>=0; z--){
                index = (x + z * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);
                //if(x>=startx && x<endx && y>=starty && y<endy && z<=startz){
                    if(obj->model[index]==0){
                        if(shadowCaster->enabled == 0){
                            continue;
                        }

                        cx = x-shadowCaster->position.x+obj->position.x;
                        cy = y-shadowCaster->position.y+obj->position.y;
                        cz = z-shadowCaster->position.z+obj->position.z;

                        if(cx>-1 && cx<shadowCaster->maxDimension && cy>-1 && cy<shadowCaster->maxDimension && cz>-1 && cz<shadowCaster->maxDimension){
                            o = (cx + cz * shadowCaster->maxDimension + cy * shadowCaster->maxDimension * shadowCaster->maxDimension);
                            if(shadowCaster->model[o]!=0){
                                shadowVal = 0;
                            }
                        }
                        finalShadow = shadowVal;
                    }else{
                        if(z<obj->dimension[2]-1){ //Up
                            dir = (x + (z+1) * obj->maxDimension + y * obj->maxDimension * obj->maxDimension);
                            finalShadow = obj->model[dir]==0? shadowVal:1;
                        }
                    }
                //}else{
                //    finalShadow = 1;
                //}
                //lighting => 8bits  [1-Empty] [3-Occlusion][2-Direct Light(2), Ambient(1) and self shadow(0)] [1-Shadow from caster]
                obj->lighting[index] = (unsigned char)(((obj->lighting[index]>>1)<<1) | (finalShadow&1));
            }
        }
    }
}

unsigned short int intersection(int minx, int miny, int minz, int maxx, int maxy, int maxz,Ray ray) {
    int j;
    float bmin[3] = {minx-0.57735,miny-0.57735,minz-0.57735};
	float bmax[3] = {maxx+0.57735,maxy+0.57735,maxz+0.57735};

    float t1 = (bmin[0] - ray.origin[0])*ray.inverseDirection[0];
    float t2 = (bmax[0] - ray.origin[0])*ray.inverseDirection[0];
 
    float tmin = min(t1, t2);
    float tmax = max(t1, t2);
 
    for (j = 1;j < 3; ++j) {
        t1 = (bmin[j] - ray.origin[j])*ray.inverseDirection[j];
        t2 = (bmax[j] - ray.origin[j])*ray.inverseDirection[j];
 
        tmin = max(tmin, min(t1, t2));
        tmax = min(tmax, max(t1, t2));
    }
 
    return tmax > max(tmin, 0.0)? 1:0;
}