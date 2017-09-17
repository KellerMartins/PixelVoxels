#include "voxelLogic.h"

extern VoxelObject **scene;
extern int sceneObjectCount;
extern PoolObject Pool[POOLSIZE];
extern double deltaTime;

void InitializePool(){
    int maxSize,maxXY;
    for(int p=0; p<POOLSIZE; p++){

        Pool[p].avaliableInstances = Pool[p].numberOfInstances;

        //Alocando memória para o array de instâncias
        Pool[p].objs = (VoxelObject**)calloc(Pool[p].numberOfInstances,sizeof(VoxelObject*));

        maxSize = pow(Pool[p].baseObj.maxDimension,3);
        maxXY = pow(Pool[p].baseObj.maxDimension,2);

        for(int i=0; i<Pool[p].numberOfInstances; i++){
            //Alocando memória do ponteiro da instância
            Pool[p].objs[i] = (VoxelObject*)calloc(1,sizeof(VoxelObject));
            //Copiando os valores básicos do objeto base
            *Pool[p].objs[i] = Pool[p].baseObj;

            //Alocando memória para os ponteiros do objeto instanciado
            Pool[p].objs[i]->model = (unsigned char *)calloc(maxSize, sizeof(unsigned char));
            Pool[p].objs[i]->lighting = (unsigned char *)calloc(maxSize, sizeof(unsigned char));
            Pool[p].objs[i]->render = (unsigned short int **)calloc(Pool[p].baseObj.maxDimension, sizeof(unsigned short int*));
            
            //Termina a alocação dos ponteiros e inicia a cópia dos valores do objeto base para as instâncias
            for(int j=0;j<Pool[p].baseObj.maxDimension;j++){
                Pool[p].objs[i]->render[j] = (unsigned short int *)calloc(maxXY,sizeof(unsigned short int));
                for(int k=0;k<maxXY;k++){
                    Pool[p].objs[i]->render[j][k] = Pool[p].baseObj.render[j][k];
                }
            }
            memcpy(Pool[p].objs[i]->model, Pool[p].baseObj.model, maxSize*sizeof(unsigned char));
            memcpy(Pool[p].objs[i]->lighting, Pool[p].baseObj.lighting, maxSize*sizeof(unsigned char ));

            //Desabilita-as para serem spawnadas durante o jogo;
            Pool[p].objs[i]->enabled = 0;
        }
    }
    printf(">Pool Sucessfully initialized.\n\n");
}
void FreePool(){
    for(int p=0;p<POOLSIZE;p++){
        for(int i=0; i<Pool[p].numberOfInstances; i++){
            FreeObject(Pool[p].objs[i]);
            free(Pool[p].objs[i]);
        }
        free(Pool[p].objs);
        FreeObject(&Pool[p].baseObj);
    }
}

void Spawn(unsigned int index,int x, int y, int z){
    if(Pool[index].avaliableInstances==0){
        printf("Pool limit reached on object %d !\n",index);
        return;
    }
    for(int i=0;i<Pool[index].numberOfInstances;i++){
        if(!Pool[index].objs[i]->enabled){
            Pool[index].objs[i]->enabled = 1;
            Pool[index].objs[i]->timeOfActivation = SDL_GetTicks();
            Pool[index].avaliableInstances--;

            Pool[index].objs[i]->position.x = x;
            Pool[index].objs[i]->position.y = y;
            Pool[index].objs[i]->position.z = z;

            return;
        }
    }
}

void PoolUpdate(){
    int maxSize,maxXY;
    for(int p=0;p<POOLSIZE;p++){

        maxSize = pow(Pool[p].baseObj.maxDimension,3);
        maxXY = pow(Pool[p].baseObj.maxDimension,2);

        for(int o=0;o<Pool[p].numberOfInstances;o++){

            if(Pool[p].objs[o]->enabled){
                if(Pool[p].type == BULLET){
                    if(Pool[p].objs[o]->timeOfActivation+2000 <= SDL_GetTicks()){

                        Pool[p].objs[o]->enabled = 0;
                        
                        memcpy(Pool[p].objs[o]->model,Pool[p].baseObj.model,maxSize*sizeof(unsigned char));
                        memcpy(Pool[p].objs[o]->lighting,Pool[p].baseObj.lighting, maxSize*sizeof(unsigned char ));

                    for(int j=0;j<Pool[p].baseObj.maxDimension;j++){
                            memcpy(Pool[p].objs[o]->render[j],Pool[p].baseObj.render[j],maxXY*sizeof(unsigned short int ));
                        }

                        Pool[p].avaliableInstances++;  
                    }else{
                        MoveObject(Pool[p].objs[o],250,0,0,0,0,0,&(*scene),sceneObjectCount,4,8);
                    }
                }
            }
        }
    }
}

void MoveObject(VoxelObject *obj,float mx, float my, float mz,float rx, float ry, float rz,	VoxelObject **col,const int numCol,int damageColRadius,int damageObjRadius){
    //printf("%0.0f Per cent\n",100*(obj->voxelsRemaining/(float)obj->voxelCount));
    int o,i,iz,nv,x,y,z,index = 0,allowMovement = 1,useRot = 0;
    int halfDimX = obj->dimension[0]/2.0, halfDimY = obj->dimension[1]/2.0,halfDimZ = obj->dimension[2]/2.0;
    double moveDelta = deltaTime>0.02? 0.02:deltaTime;

    float rotx,roty,rotz;
    float sinx = 1,cosx = 0;
    float siny = 1,cosy = 0;
    float sinz = 1,cosz = 0;

    if(obj->rotation.x != 0.0f || obj->rotation.y != 0.0f || obj->rotation.z != 0.0f){
        useRot = 1;
        sinx = sin(obj->rotation.x * 0.01745329251);
        cosx = cos(obj->rotation.x * 0.01745329251);

        siny = sin(obj->rotation.y * 0.01745329251);
        cosy = cos(obj->rotation.y * 0.01745329251);
        
        sinz = sin(obj->rotation.z * 0.01745329251);
        cosz = cos(obj->rotation.z * 0.01745329251);
    }

    if(col!=NULL){
        for(iz=obj->maxDimension-1;iz>=0;iz--){
            nv = obj->render[iz][0];
            for(i = nv; i > 0  ; i--){
                if(allowMovement == 0){
                    break;
                }

                if(useRot){

                    x = (obj->render[iz][i] & 127) - halfDimX;
                    y = ((obj->render[iz][i]>>7) & 127) - halfDimY;
                    z = iz - halfDimZ;

                    rotx = x*cosy*cosz + y*(cosz*sinx*siny - cosx*sinz) + z*(cosx*cosz*siny + sinx*sinz);
                    roty = x*cosy*sinz + z*(cosx*siny*sinz - cosz*sinx) + y*(cosx*cosz + sinx*siny*sinz);
                    rotz = z*cosx*cosy + y*sinx*cosy - x*siny;

                    x = rotx + (obj->position.x+(mx*moveDelta)) + halfDimX;
                    y = roty + (obj->position.y+(my*moveDelta)) + halfDimY;
                    z = rotz + (obj->position.z+(mz*moveDelta)) + halfDimZ;

                }else{
                    x = ((obj->render[iz][i] & 127)+(obj->position.x+(mx*moveDelta)));
                    y = (((obj->render[iz][i]>>7) & 127)+(obj->position.y+(my*moveDelta)));
                    z = iz+(obj->position.z+(mz*moveDelta));
                }

                for(o=0; o<numCol; o++){
                    
                    if((x-col[o]->position.x)<col[o]->maxDimension && (x-col[o]->position.x)>-1 && (z-col[o]->position.z)<col[o]->maxDimension && (z-col[o]->position.z)>-1 && (y+col[o]->position.y)<col[o]->maxDimension && (y+col[o]->position.y)>-1){
                        index = (x-col[o]->position.x) + (z-col[o]->position.z) * col[o]->maxDimension + (y-col[o]->position.y) * col[o]->maxDimension * col[o]->maxDimension;
                        if(col[o]->model[index]!=0){
                            ExplodeAtPoint(col[o],x,y,z,damageColRadius);
                            ExplodeAtPoint(obj,x,y,z,damageObjRadius);
                            allowMovement = 0;
                            break;
                        }
                    }
                }
            }
        }
    }
    if(allowMovement){
        obj->position.x +=(mx*moveDelta);
        obj->position.y +=(my*moveDelta);
        obj->position.z +=(mz*moveDelta);

        obj->rotation.x +=(rx*moveDelta);
        obj->rotation.y +=(ry*moveDelta);
        obj->rotation.z +=(rz*moveDelta);
    }
}

void ExplodeAtPoint(VoxelObject *obj,int x, int y, int z,int radius){
    int halfDimX = obj->dimension[0]/2.0, halfDimY = obj->dimension[1]/2.0;
    int px,py,pz;
    if(abs(-obj->rotation.z)> 0.1 ){
        float sinz = sin((-obj->rotation.z) * 0.01745329251);
        float cosz = cos((-obj->rotation.z) * 0.01745329251);

        px = x - obj->position.x;
        py = y - obj->position.y;

        float rotx = ( ((px-halfDimX) *cosz - (py-halfDimY) *sinz) + halfDimX);
        float roty = ( ((px-halfDimX) *sinz + (py-halfDimY) *cosz) + halfDimY);

        px = rotx;
        py = roty;
    }else{
        px = x-obj->position.x;
        py = y-obj->position.y; 
    }
    pz = z-obj->position.z;


    int startx,endx,starty,endy,startz,endz;
    int ix,iy,iz,index;
        startx = px-radius <0? 0:px-radius;
        starty = py-radius <0? 0:py-radius;
        startz = pz-radius <0? 0:pz-radius;

        endx = px+radius>obj->maxDimension? obj->maxDimension : px+radius;
        endy = py+radius>obj->maxDimension? obj->maxDimension : py+radius;
        endz = pz+radius>obj->maxDimension? obj->maxDimension : pz+radius;

        for(ix = startx;ix<endx;ix++){
            for(iy = starty;iy<endy;iy++){
                for(iz = startz;iz<endz;iz++){
                    int randRadius = radius+(rand() % 3);
                    if( ((ix-px)*(ix-px))+((iy-py)*(iy-py))+((iz-pz)*(iz-pz)) <= (randRadius*randRadius)){
                        index = (ix) + (iz) * obj->maxDimension + (iy) * obj->maxDimension * obj->maxDimension;
                        if(obj->model[index] != 0){
                            obj->model[index] = 0;
                            obj->voxelsRemaining--;
                        }
                    }
                }
            }   
        }
        startz = startz-1 >= 0? startz-1:0;
        endz = endz+1<obj->maxDimension? endz+1:obj->maxDimension-1;

        startx = startx-1 >= 0? startx-1:0;
        endx = endx+1<obj->maxDimension? endx+1:obj->maxDimension-1;

        starty = starty-1 >= 0? starty-1:0;
        endy = endy+1<obj->maxDimension? endy+1:obj->maxDimension-1;

        obj->modificationStartZ = obj->modificationStartZ <0? startz:obj->modificationStartZ<startz?obj->modificationStartZ:startz;
        obj->modificationEndZ = obj->modificationEndZ <0? endz-1:obj->modificationEndZ>endz-1?obj->modificationEndZ:endz-1;

        obj->modificationStartX = obj->modificationStartX <0? startx:obj->modificationStartX<startx?obj->modificationStartX:startx;
        obj->modificationEndX = obj->modificationEndX <0? endx-1:obj->modificationEndX>endx-1?obj->modificationEndX:endx-1;

        obj->modificationStartY = obj->modificationStartY <0? starty:obj->modificationStartY<starty?obj->modificationStartY:starty;
        obj->modificationEndY = obj->modificationEndY <0? endy-1:obj->modificationEndY>endy-1?obj->modificationEndY:endy-1;
}


//VoxelPointerArrayUnion(int [total size of pointer], int [number of pointers to join], VoxelObject **[Pointers],int [pointerSize],...)
VoxelObject **VoxelPointerArrayUnion(int totalPointerSize,int numberOfPointers,...){
	va_list args;
	//Inicializa os argumentos variáveis (total =  numberOfPointers)
	va_start(args,numberOfPointers);
    printf("Number of pointers to join: %d\n",numberOfPointers);
	VoxelObject **p = (VoxelObject **)calloc(totalPointerSize,sizeof(VoxelObject *));
	
	int pos = 0;
	for(int i=0; i<numberOfPointers; i++){
		VoxelObject ** current = va_arg(args,VoxelObject **);
		int pointerSize = va_arg(args,int);
        printf("Pointer %d size: %d\n",i,pointerSize);
		for(int j=0; j<pointerSize; j++){
			p[pos] = current[j];
			pos++;
		}
	}
    va_end(args);
    printf(">Joined pointers successfully\n");
    
	return p;
}