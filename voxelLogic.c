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
                if(Pool[p].objs[o]->timeOfActivation+2000 <= SDL_GetTicks()){

                    Pool[p].objs[o]->enabled = 0;
                    
                    memcpy(Pool[p].objs[o]->model,Pool[p].baseObj.model,maxSize*sizeof(unsigned char));
                    memcpy(Pool[p].objs[o]->lighting,Pool[p].baseObj.lighting, maxSize*sizeof(unsigned char ));

                   for(int j=0;j<Pool[p].baseObj.maxDimension;j++){
                        memcpy(Pool[p].objs[o]->render[j],Pool[p].baseObj.render[j],maxXY*sizeof(unsigned short int ));
                    }

                    Pool[p].avaliableInstances++;  
                }else{
                    MoveObject(Pool[p].objs[o],250,0,0,&(*scene),sceneObjectCount,4,8);
                }
            }
        }
    }
}

void MoveObject(VoxelObject *obj,float x, float y, float z,	VoxelObject **col,const int numCol,int damageColRadius,int damageObjRadius){
    //printf("%0.0f Per cent\n",100*(obj->voxelsRemaining/(float)obj->voxelCount));
    int o,i,iz,nv,px,py,pz,index = 0,allowMovement = 1;
    double moveDelta = deltaTime>0.02? 0.02:deltaTime;
    if(col!=NULL){
        for(iz=obj->maxDimension-1;iz>=0;iz--){
            pz = iz+(obj->position.z+(z*moveDelta));
            nv = obj->render[iz][0];
            for(i = nv; i > 0  ; i--){
                if(allowMovement == 0){
                    break;
                }
                px = ((obj->render[iz][i] & 127)+(obj->position.x+(x*moveDelta)));
                py = (((obj->render[iz][i]>>7) & 127)+(obj->position.y+(y*moveDelta)));

                for(o=0; o<numCol; o++){
                    
                    if((px-col[o]->position.x)<col[o]->maxDimension && (px-col[o]->position.x)>-1 && (pz-col[o]->position.z)<col[o]->maxDimension && (pz-col[o]->position.z)>-1 && (py+col[o]->position.y)<col[o]->maxDimension && (py+col[o]->position.y)>-1){
                        index = (px-col[o]->position.x) + (pz-col[o]->position.z) * col[o]->maxDimension + (py-col[o]->position.y) * col[o]->maxDimension * col[o]->maxDimension;
                        if(col[o]->model[index]!=0){
                            ExplodeAtPoint(col[o],px,py,pz,damageColRadius);
                            ExplodeAtPoint(obj,px,py,pz,damageObjRadius);
                            allowMovement = 0;
                            break;
                        }
                    }
                }
            }
        }
    }
    if(allowMovement){
        obj->position.x +=(x*moveDelta);
        obj->position.y +=(y*moveDelta);
        obj->position.z +=(z*moveDelta);
    }
}

void ExplodeAtPoint(VoxelObject *obj,int x, int y, int z,int radius){
    int px = x-obj->position.x,py = y-obj->position.y,pz = z-obj->position.z;
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