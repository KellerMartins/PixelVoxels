#include "voxelLogic.h"

extern int ExitGame;
extern double deltaTime;

extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;
extern const int GAME_SCREEN_WIDTH;
extern const int GAME_SCREEN_HEIGHT;

extern VoxelObject **scene;
extern int sceneObjectCount;
extern PoolObject Pool[POOLSIZE];
extern Vector3 cameraPosition;

//Array com o estado do teclado (atual e do frame anterior)
const Uint8 *keyboard_current = NULL;
Uint8 *keyboard_last;
SDL_Event event;
VoxelObject model;

void GameStart(){
    
    FILE *voxelFile;

    //Carrega modelo da nave do player
	
	voxelFile = fopen("Models/spaceship.vox","rb");
	model = FromMagica(voxelFile);
	model.position = (Vector3){0,30,20};
	fclose(voxelFile);

    //Carrega modelo da bala no pool
	voxelFile = fopen("Models/Bullet.vox","rb");
	Pool[0].baseObj = FromMagica(voxelFile);
	fclose(voxelFile);
	//Define o número de instâncias disponíveis
	Pool[0].numberOfInstances = 60;
    Pool[0].type = BULLET;
    
    voxelFile = fopen("Models/Bullet.vox","rb");
	Pool[1].baseObj = FromMagica(voxelFile);
	fclose(voxelFile);
	//Define o número de instâncias disponíveis
	Pool[1].numberOfInstances = 1;
	Pool[1].type = BULLET;
	
	//Inicializa o pool
	InitializePool(Pool);
}
float lastRot = 0;
void GameUpdate(){
    Vector3 moveDir = VECTOR3_ZERO;
    Vector3 rotVal = VECTOR3_ZERO;
    int moved = 0;

    //Cria um vetor do mouse centrado no centro do player, para definir o angulo a rodar o objeto para olhar para o mouse
    float screenPosX, screenPosY;
    screenPosY = (int)((model.position.x+model.position.y) -roundf(model.position.z+cameraPosition.z)*2 + roundf(-cameraPosition.y));
    screenPosX = (int)((model.position.x-model.position.y)*2 + roundf(-cameraPosition.x));

    screenPosY = ((screenPosY/(float)GAME_SCREEN_HEIGHT)-0.5f)*2;
    screenPosX = ((screenPosX/(float)GAME_SCREEN_WIDTH)-0.5f)*2;

    int mx,my;
    SDL_GetMouseState(&mx,&my);
    Vector3 mouseVec = {((mx/(float)SCREEN_WIDTH)-0.5f)*2 - screenPosX,((my/(float)SCREEN_HEIGHT)-0.5f)*2 - screenPosY,0};
    
    mouseVec = NormalizeVector(mouseVec);

    Vector3 diag = {1/sqrt(2), 1/sqrt(2), 0};
    float dp = dot(mouseVec,diag);
    diag.x = -diag.x;
    float angle = acos(dp)*180/PI;
    float dir = dot(mouseVec,diag);
    if (dir < 0) angle = -angle;

    model.rotation.z = angle;
    //Movimento da nave
    if (GetKey(SDL_SCANCODE_W))
    {
        moveDir.x += -1;
        moveDir.y += -1;
        moved=1;
    }
    else if (GetKey(SDL_SCANCODE_S))
    {
        moveDir.x += 1;
        moveDir.y += 1;
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
    if(moved){
        if(moveDir.x!=0.0f || moveDir.y!=0.0f || moveDir.z!=0.0f){
            moveDir = NormalizeVector(moveDir);
            moveDir.x *= 80;
            moveDir.y *= 80;
            moveDir.z *= 20;
        }

        MoveObject(&model,moveDir,rotVal,&(*scene),sceneObjectCount,5,2);
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
                        Vector3 dir = {1,0,0};
                        Vector3 rot = Pool[p].objs[o]->rotation;
                        dir = RotatePoint(dir,rot.z,rot.y,rot.z,0,0,0);
                        dir.x *=250;
                        dir.y *=250;
                        dir.z *=250;
                        MoveObject(Pool[p].objs[o],dir,VECTOR3_ZERO,&(*scene),sceneObjectCount,4,8);
                    }
                }
            }
        }
    }
}

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

void Spawn(unsigned int index,float x, float y, float z, float rx, float ry, float rz){
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

            Pool[index].objs[i]->rotation.x = rx;
            Pool[index].objs[i]->rotation.y = ry;
            Pool[index].objs[i]->rotation.z = rz;

            return;
        }
    }
}

//------------------------------------------------------------------ Física e movimento -----------------------------------------------------

void MoveObject(VoxelObject *obj, Vector3 movement, Vector3 rotation,	VoxelObject **col,const int numCol,int damageColRadius,int damageObjRadius){
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
        sinx = sin((obj->rotation.x + rotation.x*moveDelta) * PI_OVER_180);
        cosx = cos((obj->rotation.x + rotation.x*moveDelta) * PI_OVER_180);

        siny = sin((obj->rotation.y + rotation.y*moveDelta) * PI_OVER_180);
        cosy = cos((obj->rotation.y + rotation.y*moveDelta) * PI_OVER_180);
        
        sinz = sin((obj->rotation.z + rotation.z*moveDelta) * PI_OVER_180);
        cosz = cos((obj->rotation.z + rotation.z*moveDelta) * PI_OVER_180);
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

                    x = rotx + (obj->position.x+(movement.x*moveDelta)) + halfDimX;
                    y = roty + (obj->position.y+(movement.y*moveDelta)) + halfDimY;
                    z = rotz + (obj->position.z+(movement.z*moveDelta)) + halfDimZ;

                }else{
                    x = ((obj->render[iz][i] & 127)+(obj->position.x+(movement.x*moveDelta)));
                    y = (((obj->render[iz][i]>>7) & 127)+(obj->position.y+(movement.y*moveDelta)));
                    z = iz+(obj->position.z+(movement.z*moveDelta));
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
        obj->position.x +=(movement.x*moveDelta);
        obj->position.y +=(movement.y*moveDelta);
        obj->position.z +=(movement.z*moveDelta);

        obj->rotation.x =fmod( obj->rotation.x +rotation.x*moveDelta,360);
        obj->rotation.y =fmod(obj->rotation.y +rotation.y*moveDelta,360);
        obj->rotation.z =fmod(obj->rotation.z +rotation.z*moveDelta,360);
    }
}

void ExplodeAtPoint(VoxelObject *obj,int x, int y, int z,int radius){
    int halfDimX = obj->dimension[0]/2.0, halfDimY = obj->dimension[1]/2.0;
    int px,py,pz;
    if(abs(-obj->rotation.z)> 0.1 ){
        float sinz = sin((-obj->rotation.z) * PI_OVER_180);
        float cosz = cos((-obj->rotation.z) * PI_OVER_180);

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


//VoxelPointerArrayUnion(int [tamanho final do ponteiro], int [número de ponteiros a unir], VoxelObject **[ponteiro1],int [tamanho do ponteiro1],...)
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