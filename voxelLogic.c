#include "voxelLogic.h"

extern int ExitGame;
extern double deltaTime;

extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;
extern const int GAME_SCREEN_WIDTH;
extern const int GAME_SCREEN_HEIGHT;

extern List Rooms;
extern PoolObject Pool[POOLSIZE];
extern Vector3 cameraPosition;

//Array com o estado do teclado (atual e do frame anterior)
const Uint8 *keyboard_current = NULL;
Uint8 *keyboard_last;
SDL_Event event;
VoxelObject model;

void GameStart(){

    //Carrega modelo da nave do player
	
	model = LoadVoxelModel("Models/spaceship.vox");
	model.position = (Vector3){0,30,20};

    //Carrega modelo da bala no pool
	Pool[0].baseObj = LoadVoxelModel("Models/Bullet.vox");
	//Define o número de instâncias disponíveis
	Pool[0].objs.numberOfObjects = 60;
    Pool[0].type = BULLET;
    
	Pool[1].baseObj = LoadVoxelModel("Models/Bullet.vox");
	//Define o número de instâncias disponíveis
	Pool[1].objs.numberOfObjects = 1;
	Pool[1].type = BULLET;
	
	//Inicializa o pool
	InitializePool(Pool);
}
float lastRot = 0;
void GameUpdate(){

    if (GetKeyDown(SDL_SCANCODE_R))
    {
        ReloadShaders();
    }

    Vector3 moveDir = VECTOR3_ZERO;
    Vector3 rotVal = VECTOR3_ZERO;
    int moved = 0;

    //Cria um vetor do mouse centrado no centro do player, para definir o angulo a rodar o objeto para olhar para o mouse
    float screenPosX, screenPosY;
    screenPosY = (int)((model.position.x+model.position.y+model.center.x+model.center.y) +roundf(model.position.z+model.center.z+cameraPosition.z)*2 + roundf(-cameraPosition.y));
    screenPosX = (int)(((model.position.x+model.center.x)-(model.position.y+model.center.y))*2 + roundf(-cameraPosition.x));

    screenPosY = ((screenPosY/(float)GAME_SCREEN_HEIGHT)-0.5f)*2;
    screenPosX = ((screenPosX/(float)GAME_SCREEN_WIDTH)-0.5f)*2;

    int mx,my;
    SDL_GetMouseState(&mx,&my);
    Vector3 mouseVec = {((mx/(float)SCREEN_WIDTH)-0.5f)*2 - screenPosX,((1-(my/(float)SCREEN_HEIGHT))-0.5f)*2 - screenPosY,0};
    
    mouseVec = NormalizeVector(mouseVec);

    Vector3 diag = {1/sqrt(2), 1/sqrt(2), 0};
    float dp = dot(mouseVec,diag);
    diag.x = -diag.x;
    float angle = acos(dp)*180/PI;
    float dir = dot(mouseVec,diag);
    if (dir < 0) angle = -angle;
    //Movimento da nave
    if (GetKey(SDL_SCANCODE_W))
    {
        moveDir.x += 1;
        moveDir.y += 1;
        moved=1;
    }
    else if (GetKey(SDL_SCANCODE_S))
    {
        moveDir.x += -1;
        moveDir.y += -1;
        moved=1;
    }

    if (GetKey(SDL_SCANCODE_D))
    {
        moveDir.x += 1;
        moveDir.y += -1;
        moved=1;
    }
    else if (GetKey(SDL_SCANCODE_A))
    {
        moveDir.x += -1;
        moveDir.y += 1;
        moved=1;
    }

    if (GetKey(SDL_SCANCODE_E))
    {
        moveDir.z += 1;
        moved=1;
    }
    else if (GetKey(SDL_SCANCODE_Q))
    {
        moveDir.z += -1;
        moved=1;
    }

    if (GetKey(SDL_SCANCODE_KP_7))
    {
        rotVal.x+= 100;
        moved=1;
    }
    else if (GetKey(SDL_SCANCODE_KP_8))
    {
        rotVal.x+= -100;
        moved=1;
    }
    if (GetKey(SDL_SCANCODE_KP_4))
    {
        rotVal.y+= 100;
        moved=1;
    }
    else if (GetKey(SDL_SCANCODE_KP_5))
    {
        rotVal.y+= -100;
        moved=1;
    }
    if (GetKey(SDL_SCANCODE_KP_1))
    {
        rotVal.z+= 100;
        moved=1;
    }
    else if (GetKey(SDL_SCANCODE_KP_2))
    {
        rotVal.z+= -100;
        moved=1;
    }
    MoveObjectTo(&model,model.position,(Vector3){model.rotation.x,model.rotation.y,angle},((MultiVoxelObject*) GetFirst(Rooms))->objects.list,((MultiVoxelObject*) GetFirst(Rooms))->objects.numberOfObjects,0,0);
    if(moved){
        if(moveDir.x!=0.0f || moveDir.y!=0.0f || moveDir.z!=0.0f){
            moveDir = NormalizeVector(moveDir);
            moveDir.x *= 80;
            moveDir.y *= 80;
            moveDir.z *= 20;
        }

        MoveObject(&model,moveDir,rotVal,((MultiVoxelObject*) GetFirst(Rooms))->objects.list,((MultiVoxelObject*) GetFirst(Rooms))->objects.numberOfObjects,5,2);
    }
    //Tiro da nave
    if (GetKeyDown(SDL_SCANCODE_SPACE))
    {
        if(model.numberOfPoints !=0){
            for(int i=0;i<model.numberOfPoints;i++){
                Vector3 rotatedPoint = {model.points[i].x,model.points[i].y,model.points[i].z};
                rotatedPoint = RotatePoint(rotatedPoint,
                                            model.rotation.x, 
                                            model.rotation.y, 
                                            model.rotation.z,
                                            model.dimension[0]/2,
                                            model.dimension[1]/2,
                                            model.dimension[2]/2);

                if(model.points[i].type == 0){
                    Spawn(0,rotatedPoint.x+model.position.x,
                            rotatedPoint.y+model.position.y,
                            rotatedPoint.z+model.position.z,
                            model.rotation.x,
                            model.rotation.y,
                            model.rotation.z);
                }
            }
        }
    }
    
    //Movimento da camera
    if (GetKey(SDL_SCANCODE_UP))
    {
        MoveCamera(0,-150,0);
    }
    else if (GetKey(SDL_SCANCODE_DOWN))
    {
        MoveCamera(0,150,0);
    }
    if (GetKey(SDL_SCANCODE_RIGHT))
    {
        MoveCamera(150,0,0);
    }
    else if (GetKey(SDL_SCANCODE_LEFT))
    {
        MoveCamera(-150,0,0);
    }
    if (GetKey(SDL_SCANCODE_RSHIFT))
    {
        MoveCamera(0,0,50);
    }
    else if (GetKey(SDL_SCANCODE_RCTRL))
    {
        MoveCamera(0,0,-50);
    }
    if (GetKey(SDL_SCANCODE_ESCAPE))
    {
        ExitGame = 1;
    }
}

//--------------------------------------------------------- Input ----------------------------------------------------------------------------

void InputStart(){
    //Inicializa as vars do teclado
	keyboard_last = (Uint8 *)calloc(284,sizeof(Uint8));
	keyboard_current = SDL_GetKeyboardState(NULL);
}

void InputUpdate(){
    //Atualiza o vetor de input e gerencia eventos
    if(keyboard_current!=NULL){
        memcpy(keyboard_last,keyboard_current,284*sizeof(Uint8));
    }
    while (SDL_PollEvent(&event)) {
        switch (event.type)
        {
            case SDL_QUIT:
                ExitGame = 1;
                break;
        }
    }
}

void FreeInput(){
    free(keyboard_last);
}

//--------------------------------------------------------- Pool de objetos ----------------------------------------------------------------------------

void PoolUpdate(){
    int maxSize,maxXY;
    for(int p=0;p<POOLSIZE;p++){

        maxSize = Pool[p].baseObj.dimension[0]*Pool[p].baseObj.dimension[1]*Pool[p].baseObj.dimension[2];
        maxXY = Pool[p].baseObj.dimension[0]*Pool[p].baseObj.dimension[1];

        for(int o=0;o<Pool[p].objs.numberOfObjects;o++){

            if(Pool[p].objs.list[o]->enabled){
                if(Pool[p].type == BULLET){
                    if(Pool[p].objs.list[o]->timeOfActivation+2000 <= SDL_GetTicks()){

                        Pool[p].objs.list[o]->enabled = 0;
                        
                        memcpy(Pool[p].objs.list[o]->model,Pool[p].baseObj.model,maxSize*sizeof(unsigned char));
                        memcpy(Pool[p].objs.list[o]->lighting,Pool[p].baseObj.lighting, maxSize*sizeof(unsigned char ));

                    for(int j=0;j<Pool[p].baseObj.dimension[2];j++){
                            memcpy(Pool[p].objs.list[o]->render[j],Pool[p].baseObj.render[j],maxXY*sizeof(unsigned short int ));
                    }

                    Pool[p].avaliableInstances++;  
                    }else{
                        Vector3 dir = {1,0,0};
                        Vector3 rot = Pool[p].objs.list[o]->rotation;
                        dir = RotatePoint(dir,rot.z,rot.y,rot.z,0,0,0);
                        dir.x *=250;
                        dir.y *=250;
                        dir.z *=250;
                        MoveObject(Pool[p].objs.list[o],dir,VECTOR3_ZERO,((MultiVoxelObject*) GetFirst(Rooms))->objects.list,((MultiVoxelObject*) GetFirst(Rooms))->objects.numberOfObjects,4,8);
                    }
                }
            }
        }
    }
}


void InitializePool(){
    printf("Initializing Pool\n");
    int maxSize,maxXY;
    int p;
    for(p=0; p<POOLSIZE; p++){

        Pool[p].avaliableInstances = Pool[p].objs.numberOfObjects;

        //Allocate memory to the vector containing the objects
        //The way it is doing this breaks the list interface, but its faster
        Pool[p].objs.list = calloc(Pool[p].objs.numberOfObjects,sizeof(VoxelObject*));

        maxSize = Pool[p].baseObj.dimension[0]*Pool[p].baseObj.dimension[1]*Pool[p].baseObj.dimension[2];
        maxXY = Pool[p].baseObj.dimension[0]*Pool[p].baseObj.dimension[1];
        
        for(int i=0; i<Pool[p].objs.numberOfObjects; i++){
            //Alocando memória do ponteiro da instância
            Pool[p].objs.list[i] = malloc(sizeof(VoxelObject));
            //Copiando os valores básicos do objeto base
            *Pool[p].objs.list[i] = Pool[p].baseObj;

            //Alocando memória para os ponteiros do objeto instanciado
            Pool[p].objs.list[i]->model = calloc(maxSize, sizeof(unsigned char));
            Pool[p].objs.list[i]->lighting = calloc(maxSize, sizeof(unsigned char));
            Pool[p].objs.list[i]->render = calloc(Pool[p].baseObj.dimension[2], sizeof(unsigned short int*));
            
            //Termina a alocação dos ponteiros e inicia a cópia dos valores do objeto base para as instâncias
            
            for(int j=0;j<Pool[p].baseObj.dimension[2];j++){
                Pool[p].objs.list[i]->render[j] = calloc(1+maxXY,sizeof(unsigned short int));
                for(int k=0;k<maxXY;k++){
                    Pool[p].objs.list[i]->render[j][k] = Pool[p].baseObj.render[j][k];
                }
            }
            memcpy(Pool[p].objs.list[i]->model, Pool[p].baseObj.model, maxSize*sizeof(unsigned char));
            memcpy(Pool[p].objs.list[i]->lighting, Pool[p].baseObj.lighting, maxSize*sizeof(unsigned char ));
            
            //Desabilita-as para serem spawnadas durante o jogo;
            Pool[p].objs.list[i]->enabled = 0;
        }
    }
    printf(">Pool Sucessfully initialized.\n\n");
}

void FreePool(){
    for(int p=0;p<POOLSIZE;p++){
        FreeObjectList(&Pool[p].objs);
        FreeObject(&Pool[p].baseObj);
    }
}

void Spawn(unsigned int index,float x, float y, float z, float rx, float ry, float rz){
    if(Pool[index].avaliableInstances==0){
        printf("Pool limit reached on object %d !\n",index);
        return;
    }
    for(int i=0;i<Pool[index].objs.numberOfObjects;i++){
        if(!Pool[index].objs.list[i]->enabled){
            Pool[index].objs.list[i]->enabled = 1;
            Pool[index].objs.list[i]->timeOfActivation = SDL_GetTicks();
            Pool[index].avaliableInstances--;

            Pool[index].objs.list[i]->position.x = x;
            Pool[index].objs.list[i]->position.y = y;
            Pool[index].objs.list[i]->position.z = z;

            Pool[index].objs.list[i]->rotation.x = rx;
            Pool[index].objs.list[i]->rotation.y = ry;
            Pool[index].objs.list[i]->rotation.z = rz;

            return;
        }
    }
}

//------------------------------------------------------------------ Física e movimento -----------------------------------------------------

void MoveObject(VoxelObject *obj, Vector3 movement, Vector3 rotation,	VoxelObject **col,const int numCol,int damageColRadius,int damageObjRadius){
    //printf("%0.0f Per cent\n",100*(obj->voxelsRemaining/(float)obj->voxelCount));
    int o,i,iz,nv,x,y,z,index = 0,allowMovement = 1,useRot = 0;
    double moveDelta = deltaTime>0.02? 0.02:deltaTime;

    float rotx,roty,rotz;
    float sinx = 1,cosx = 0;
    float siny = 1,cosy = 0;
    float sinz = 1,cosz = 0;

    if(obj->rotation.x != 0.0f || obj->rotation.y != 0.0f || obj->rotation.z != 0.0f){
        useRot = 1;
        sinx = sin((obj->rotation.x + rotation.x*moveDelta) * PI_OVER_180);
        cosx = cos((obj->rotation.x + rotation.x*moveDelta) * PI_OVER_180);

        siny = sin((obj->rotation.y + rotation.y*moveDelta) * PI_OVER_180);
        cosy = cos((obj->rotation.y + rotation.y*moveDelta) * PI_OVER_180);
        
        sinz = sin((obj->rotation.z + rotation.z*moveDelta) * PI_OVER_180);
        cosz = cos((obj->rotation.z + rotation.z*moveDelta) * PI_OVER_180);
    }

    if(col!=NULL){
        for(iz=obj->dimension[2]-1;iz>=0;iz--){
            nv = obj->render[iz][0];
            for(i = nv; i > 0  ; i--){
                if(allowMovement == 0){
                    break;
                }

                if(useRot){

                    x = (obj->render[iz][i] & 127) - obj->center.x;
                    y = ((obj->render[iz][i]>>7) & 127) - obj->center.y;
                    z = iz - obj->center.z;

                    rotx = x*cosy*cosz + y*(cosz*sinx*siny - cosx*sinz) + z*(cosx*cosz*siny + sinx*sinz);
                    roty = x*cosy*sinz + z*(cosx*siny*sinz - cosz*sinx) + y*(cosx*cosz + sinx*siny*sinz);
                    rotz = z*cosx*cosy + y*sinx*cosy - x*siny;

                    x = rotx + (obj->position.x+(movement.x*moveDelta)) + obj->center.x;
                    y = roty + (obj->position.y+(movement.y*moveDelta)) + obj->center.y;
                    z = rotz + (obj->position.z+(movement.z*moveDelta)) + obj->center.z;

                }else{
                    x = ((obj->render[iz][i] & 127)+(obj->position.x+(movement.x*moveDelta)));
                    y = (((obj->render[iz][i]>>7) & 127)+(obj->position.y+(movement.y*moveDelta)));
                    z = iz+(obj->position.z+(movement.z*moveDelta));
                }

                for(o=0; o<numCol; o++){
                    
                    if((x-col[o]->position.x)<col[o]->dimension[0] && (x-col[o]->position.x)>-1 && (z-col[o]->position.z)<col[o]->dimension[2] && (z-col[o]->position.z)>-1 && (y-col[o]->position.y)<col[o]->dimension[1] && (y-col[o]->position.y)>-1){
                        index = (x-col[o]->position.x) + (z-col[o]->position.z) * col[o]->dimension[0] + (y-col[o]->position.y) * col[o]->dimension[0] * col[o]->dimension[2];
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
        obj->position.x +=(movement.x*moveDelta);
        obj->position.y +=(movement.y*moveDelta);
        obj->position.z +=(movement.z*moveDelta);

        obj->rotation.x =fmod( obj->rotation.x +rotation.x*moveDelta,360);
        obj->rotation.y =fmod(obj->rotation.y +rotation.y*moveDelta,360);
        obj->rotation.z =fmod(obj->rotation.z +rotation.z*moveDelta,360);
    }
}

void MoveObjectTo(VoxelObject *obj, Vector3 movement, Vector3 rotation,	VoxelObject **col,const int numCol,int damageColRadius,int damageObjRadius){
    //printf("%0.0f Per cent\n",100*(obj->voxelsRemaining/(float)obj->voxelCount));
    int o,i,iz,nv,x,y,z,index = 0,allowMovement = 1,useRot = 0;

    float rotx,roty,rotz;
    float sinx = 1,cosx = 0;
    float siny = 1,cosy = 0;
    float sinz = 1,cosz = 0;

    if(rotation.x != 0.0f || rotation.y != 0.0f || rotation.z != 0.0f){
        useRot = 1;
        sinx = sin((rotation.x) * PI_OVER_180);
        cosx = cos((rotation.x) * PI_OVER_180);

        siny = sin((rotation.y) * PI_OVER_180);
        cosy = cos((rotation.y) * PI_OVER_180);
        
        sinz = sin((rotation.z) * PI_OVER_180);
        cosz = cos((rotation.z) * PI_OVER_180);
    }

    if(col!=NULL){
        for(iz=obj->dimension[2]-1;iz>=0;iz--){
            nv = obj->render[iz][0];
            for(i = nv; i > 0  ; i--){
                if(allowMovement == 0){
                    break;
                }

                if(useRot){

                    x = (obj->render[iz][i] & 127) - obj->center.x;
                    y = ((obj->render[iz][i]>>7) & 127) - obj->center.y;
                    z = iz - obj->center.z;

                    rotx = x*cosy*cosz + y*(cosz*sinx*siny - cosx*sinz) + z*(cosx*cosz*siny + sinx*sinz);
                    roty = x*cosy*sinz + z*(cosx*siny*sinz - cosz*sinx) + y*(cosx*cosz + sinx*siny*sinz);
                    rotz = z*cosx*cosy + y*sinx*cosy - x*siny;

                    x = rotx + (movement.x) + obj->center.x;
                    y = roty + (movement.y) + obj->center.y;
                    z = rotz + (movement.z) + obj->center.z;

                }else{
                    x = ((obj->render[iz][i] & 127)+(movement.x));
                    y = (((obj->render[iz][i]>>7) & 127)+(movement.y));
                    z = iz+(movement.z);
                }

                for(o=0; o<numCol; o++){
                    
                    if((x-col[o]->position.x)<col[o]->dimension[0] && (x-col[o]->position.x)>-1 && (z-col[o]->position.z)<col[o]->dimension[2] && (z-col[o]->position.z)>-1 && (y-col[o]->position.y)<col[o]->dimension[1] && (y-col[o]->position.y)>-1){
                        index = (x-col[o]->position.x) + (z-col[o]->position.z) * col[o]->dimension[0] + (y-col[o]->position.y) * col[o]->dimension[0] * col[o]->dimension[2];
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
        obj->position.x =(movement.x);
        obj->position.y =(movement.y);
        obj->position.z =(movement.z);

        obj->rotation.x =fmod(rotation.x,360);
        obj->rotation.y =fmod(rotation.y,360);
        obj->rotation.z =fmod(rotation.z,360);
    }
}

void ExplodeAtPoint(VoxelObject *obj,int x, int y, int z,int radius){
    int px,py,pz;
    if(abs(-obj->rotation.z)> 0.1 ){
        float sinz = sin((-obj->rotation.z) * PI_OVER_180);
        float cosz = cos((-obj->rotation.z) * PI_OVER_180);

        px = x - obj->position.x;
        py = y - obj->position.y;

        float rotx = ( ((px-obj->center.x) *cosz - (py-obj->center.y) *sinz) + obj->center.x);
        float roty = ( ((px-obj->center.x) *sinz + (py-obj->center.y) *cosz) + obj->center.y);

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

        endx = px+radius>obj->dimension[0]? obj->dimension[0] : px+radius;
        endy = py+radius>obj->dimension[1]? obj->dimension[1] : py+radius;
        endz = pz+radius>obj->dimension[2]? obj->dimension[2] : pz+radius;

        for(ix = startx;ix<endx;ix++){
            for(iy = starty;iy<endy;iy++){
                for(iz = startz;iz<endz;iz++){
                    int randRadius = radius+(rand() % 3);
                    if( ((ix-px)*(ix-px))+((iy-py)*(iy-py))+((iz-pz)*(iz-pz)) <= (randRadius*randRadius)){
                        index = (ix) + (iz) * obj->dimension[0] + (iy) * obj->dimension[0] * obj->dimension[2];
                        if(obj->model[index] != 0){
                            obj->model[index] = 0;
                            obj->voxelsRemaining--;
                        }
                    }
                }
            }   
        }
        startz = startz >= 0? startz:0;
        endz = endz+1<obj->dimension[2]? endz+1:obj->dimension[2];

        startx = startx >= 0? startx:0;
        endx = endx+1<obj->dimension[0]? endx+1:obj->dimension[0];

        starty = starty >= 0? starty:0;
        endy = endy+1<obj->dimension[1]? endy+1:obj->dimension[1];

        obj->modificationStartZ = obj->modificationStartZ <0? startz:obj->modificationStartZ<startz?obj->modificationStartZ:startz;
        obj->modificationEndZ = obj->modificationEndZ <0? endz-1:obj->modificationEndZ>endz-1?obj->modificationEndZ:endz-1;

        obj->modificationStartX = obj->modificationStartX <0? startx:obj->modificationStartX<startx?obj->modificationStartX:startx;
        obj->modificationEndX = obj->modificationEndX <0? endx-1:obj->modificationEndX>endx-1?obj->modificationEndX:endx-1;

        obj->modificationStartY = obj->modificationStartY <0? starty:obj->modificationStartY<starty?obj->modificationStartY:starty;
        obj->modificationEndY = obj->modificationEndY <0? endy-1:obj->modificationEndY>endy-1?obj->modificationEndY:endy-1;
}


//VoxelPointerArrayUnion(int [tamanho final do ponteiro], int [número de ponteiros a unir], VoxelObject **[ponteiro1],int [tamanho do ponteiro1],...)
VoxelObject **VoxelPointerArrayUnion(int totalPointerSize,int numberOfPointers,...){
	va_list args;
	//Inicializa os argumentos variáveis (total =  numberOfPointers)
	va_start(args,numberOfPointers);
    printf("Number of pointers to join: %d\n",numberOfPointers);
	VoxelObject **p = calloc(totalPointerSize,sizeof(VoxelObject *));
	
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