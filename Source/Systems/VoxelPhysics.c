#include "VoxelPhysics.h"
//Each voxel is 10 centimeters

static System *ThisSystem;
static ComponentID VoxelModelID = -1;
static ComponentID RigidBodyID = -1;
static ComponentID TransformID = -1;

extern engineCore Core;
extern engineTime Time;
extern engineECS ECS;

int VoxelModelVsVoxelModelCollision(EntityID entityA, EntityID entityB,Vector3 *collisionPoint);
void ExplodeAtPoint(EntityID entity,Vector3 point,int radius);

static double gravity = 9.8;

double GetGravity(){
    return gravity;
}

void SetGravity(double g){
    gravity = g;
}

void VoxelPhysicsInit(){
    ThisSystem = (System*)GetElementAt(ECS.SystemList,GetSystemID("VoxelPhysics"));

    VoxelModelID = GetComponentID("VoxelModel");
    RigidBodyID = GetComponentID("RigidBody");
    TransformID = GetComponentID("Transform");
}

//Simple collision and kinematic movement
//Warning: Not even close to be physically or mathematically correct!
void VoxelPhysicsUpdate(){
    //Runs for all entities that use voxel physics
    EntityID entity;
	for(entity = 0; entity <= ECS.maxUsedIndex; entity++){

        //Check if entity contains needed components
        //If no component is required , run for all
        if(!IsEmptyComponentMask(ThisSystem->required) && !EntityContainsMask(entity,ThisSystem->required) ) continue;
        //Check if entity doesn't contains the excluded components
        //If there is no restriction, run all with the required components
        if(!IsEmptyComponentMask(ThisSystem->excluded) && EntityContainsMask(entity,ThisSystem->excluded) ) continue;


        if(IsStaticRigidBody(entity) || EntityIsChild(entity)) {
            SetVelocity(entity,VECTOR3_ZERO);
            SetAcceleration(entity,VECTOR3_ZERO);
            continue;
        }

        //Sets an limit of how high the physics time step can be
        double moveDelta = Time.deltaTime>0.02? 0.02:Time.deltaTime;
        ComponentMask needed = CreateComponentMaskByID(3,VoxelModelID, RigidBodyID, TransformID);

        //Checks for entities to collide with
        int i,collided = 0;
        for(i=0; i<=ECS.maxUsedIndex; i++){
            
            if(i==entity) continue;
            //Skip entities without the needed components
            if(!EntityContainsMask(i,needed)) continue;
            
            //VoxelModel *objB = GetVoxelModelPointer(i);
            Vector3 collisionPoint;
            //Vector3 collisionNormal = VECTOR3_ZERO;
            if(VoxelModelVsVoxelModelCollision(entity, i,&collisionPoint)){

                if(norm(GetVelocity(entity))>10 || norm(GetVelocity(i))>10){
                    ExplodeAtPoint(entity,collisionPoint,5);   
                    ExplodeAtPoint(i,collisionPoint,5);   
                }

                Vector3 VelA = Subtract(GetVelocity(entity) , ScalarMult(Subtract(GetVelocity(entity),GetVelocity(i)), Lerp(GetBounciness(entity),0.3,1)*2*GetMass(i)/(GetMass(entity) + GetMass(i)) ) );
                Vector3 VelB = Add(GetVelocity(i) , ScalarMult(Subtract(GetVelocity(entity),GetVelocity(i)), Lerp(GetBounciness(i),0.5,1)*2*GetMass(entity)/(GetMass(entity) + GetMass(i)) ) );
                //v1f = v1i -2*m2(v1i-v2i)/(m1+m2)
                //v2f = v2i +2*m1(v1i-v2i)/(m1+m2)

                SetVelocity(entity,VelA);
                SetVelocity(i,VelB);
                collided = 1;
            }
        }
        
        Vector3 posDelta;
        
        if(!collided){
            Vector3 accel = GetAcceleration(entity);
            if(UsesGravity(entity)){
                accel = Add(accel, (Vector3){0,0,-GetGravity()});   
            }
            posDelta = ScalarMult(Add(ScalarMult(GetVelocity(entity),moveDelta) , ScalarMult(accel,0.5 * moveDelta * moveDelta)),WORLD_SCALE);  
            SetVelocity(entity,Add(GetVelocity(entity),ScalarMult(accel,moveDelta)));
        }else{
            posDelta = ScalarMult(ScalarMult(GetVelocity(entity),moveDelta),WORLD_SCALE);
        }

        Vector3 pos = Add(GetPosition(entity),posDelta);
        SetPosition(entity,pos);
    }
}

void VoxelPhysicsFree(){
    
}

int VoxelModelVsVoxelModelCollision(EntityID entityA, EntityID entityB,Vector3 *collisionPoint){

    *collisionPoint = VECTOR3_ZERO;    
    VoxelModel *objA = GetVoxelModelPointer(entityA);
    VoxelModel *objB = GetVoxelModelPointer(entityB);

    if(!objA->model || !objB->model){
        return 0;
    }

    Vector3 posA;
    Vector3 posB;
    Matrix3x3 rotA;
    Matrix3x3 rotB;
    GetGlobalTransform(entityA, &posA, NULL, &rotA);
    GetGlobalTransform(entityB, &posB, NULL, &rotB);
    rotB = Transpose(rotB);

    Vector3 centerA = GetVoxelModelCenter(entityA);
    Vector3 centerB = GetVoxelModelCenter(entityB);

    Vector3 velA = GetVelocity(entityA);
    Vector3 velB = GetVelocity(entityB);
    Vector3 accA = GetAcceleration(entityA);
    Vector3 accB = GetAcceleration(entityB);

    //Sets an limit of how high the physics time step can be
    double moveDelta = Time.deltaTime>0.02? 0.02:Time.deltaTime;

    Vector3 movementA = {velA.x + accA.x*moveDelta,velA.y + accA.y*moveDelta,velA.z + accA.z*moveDelta};
    movementA = ScalarMult(movementA,moveDelta);

    Vector3 movementB = {velB.x + accB.x*moveDelta,velB.y + accB.y*moveDelta,velB.z + accB.z*moveDelta};
    movementB = ScalarMult(movementB,moveDelta);

    glBindBuffer(GL_ARRAY_BUFFER, objA->vbo[0]);
    Vector3* vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);

    float x,y,z;
    int i,index = 0;
    int collisions = 0;
    for(i=0; i<objA->numberOfVertices; i++){

        x = vertices[i].x;
        y = vertices[i].y;
        z = vertices[i].z;

        //Ignore invalid voxels
        if(x<0 || y<0 || z<0) continue;

        //Apply A rotation matrix
        Vector3 p = {x - centerA.x, y - centerA.y, z - centerA.z};
        if(objA->smallScale){
            p = ScalarMult(p, 0.5);
        }

        Vector3 pRotA = RotateVector(p,rotA);

        //Apply A position and movement
        x = pRotA.x + posA.x + movementA.x;
        y = pRotA.y + posA.y + movementA.y;
        z = pRotA.z + posA.z + movementA.z;

        //Set origin as B position
        Vector3 localPosAinB = { x - posB.x-movementB.x, y - posB.y-movementB.y, z - posB.z-movementB.z};
        if(objB->smallScale){
            localPosAinB = ScalarMult(localPosAinB, 2.0);
        }

        //Apply B (Transposed) rotation matrix
        p = (Vector3){localPosAinB.x, localPosAinB.y, localPosAinB.z};
        localPosAinB = RotateVector(p,rotB);

        localPosAinB = (Vector3){roundf(localPosAinB.x + centerB.x), roundf(localPosAinB.y + centerB.y), roundf(localPosAinB.z + centerB.z)};

        //If voxel of entityA is inside the entityB volume
            if(localPosAinB.x < objB->dimension[0] && localPosAinB.x >= 0 && localPosAinB.z < objB->dimension[2] && localPosAinB.z >= 0 && localPosAinB.y <objB->dimension[1] && localPosAinB.y >= 0){
                index = localPosAinB.x + localPosAinB.z * objB->dimension[0] + localPosAinB.y * objB->dimension[0] * objB->dimension[2];
                if(objB->model[index]!=0){
                    collisionPoint->x += x;
                    collisionPoint->y += y;
                    collisionPoint->z += z;
                collisions++;
                }
            }
        }

    if(collisions){
        collisionPoint->x /= (float)collisions;
        collisionPoint->y /= (float)collisions;
        collisionPoint->z /= (float)collisions;
    }

    glBindBuffer(GL_ARRAY_BUFFER, objA->vbo[0]);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    return collisions>0 ;
}

//Test function
void ExplodeAtPoint(EntityID entity,Vector3 point,int radius){
    VoxelModel *obj = GetVoxelModelPointer(entity);
    
    Matrix3x3 rotation;
    Vector3 position;
    GetGlobalTransform(entity, &position, NULL, &rotation);
    Vector3 c = obj->center;
    float scale = obj->smallScale? 2:1;
    
    Vector3 p = Add(RotateVector(ScalarMult(Subtract(point, position), scale), Transpose(rotation)), c);
    p = (Vector3){round(p.x), round(p.y), round(p.z)};

    //Vector3 pW = Add(RotateVector(ScalarMult(Subtract(p, c), 1.0/scale), rotation), position);
    //Vector3 pS = PositionToGameScreenCoords(pW);
    //pS.x = clamp(pS.x, 0, Screen.windowWidth);
    //pS.y = clamp(pS.y, 0, Screen.windowHeight);
    //DrawPoint(pS, 5, 0, 1, 1, 0);

    int startx,endx,starty,endy,startz,endz;
    int ix,iy,iz,index;

    startx = p.x-radius<0? 0:p.x-radius;
    starty = p.y-radius<0? 0:p.y-radius;
    startz = p.z-radius<0? 0:p.z-radius;

        endx = p.x+radius>obj->dimension[0]? obj->dimension[0] : p.x+radius;
        endy = p.y+radius>obj->dimension[1]? obj->dimension[1] : p.y+radius;
        endz = p.z+radius>obj->dimension[2]? obj->dimension[2] : p.z+radius;

        for(ix = startx;ix<endx;ix++){
            for(iy = starty;iy<endy;iy++){
                for(iz = startz;iz<endz;iz++){
                    int randRadius = radius+(rand() % 3);
                    if( ((ix-p.x)*(ix-p.x))+((iy-p.y)*(iy-p.y))+((iz-p.z)*(iz-p.z)) <= (randRadius*randRadius)){
                        index = ix + iz * obj->dimension[0] + iy * obj->dimension[0] * obj->dimension[2];

                        if(obj->model[index] != 0){
                            obj->model[index] = 0;
                            obj->voxelsRemaining--;
                        }
                    }
                }
            }   
        }
        startz = startz-1 >= 0? startz-1:0;
        endz = endz+1<obj->dimension[2]? endz+1:obj->dimension[2]-1;

        startx = startx-1 >= 0? startx-1:0;
        endx = endx+1<obj->dimension[0]? endx+1:obj->dimension[0]-1;

        starty = starty-1 >= 0? starty-1:0;
        endy = endy+1<obj->dimension[1]? endy+1:obj->dimension[1]-1;

        obj->modificationStartZ = obj->modificationStartZ <0? startz:obj->modificationStartZ<startz?obj->modificationStartZ:startz;
        obj->modificationEndZ = obj->modificationEndZ <0? endz-1:obj->modificationEndZ>endz-1?obj->modificationEndZ:endz-1;

        obj->modificationStartX = obj->modificationStartX <0? startx:obj->modificationStartX<startx?obj->modificationStartX:startx;
        obj->modificationEndX = obj->modificationEndX <0? endx-1:obj->modificationEndX>endx-1?obj->modificationEndX:endx-1;

        obj->modificationStartY = obj->modificationStartY <0? starty:obj->modificationStartY<starty?obj->modificationStartY:starty;
        obj->modificationEndY = obj->modificationEndY <0? endy-1:obj->modificationEndY>endy-1?obj->modificationEndY:endy-1;
}


//Lua interface functions

static int l_GetGravity (lua_State *L) {
    lua_settop(L, 0);
    double gravity = GetGravity();
    lua_pushnumber(L, gravity); //Put the returned number on the stack
    return 1; //Return number of results
}

static int l_SetGravity (lua_State *L) {
    //Get the arguments
    double g = luaL_checknumber (L, 1);

    SetGravity(g);
    return 0; //Return number of results
}

void VoxelPhysicsRegisterLuaFunctions(){
    lua_pushcfunction(Core.lua, l_GetGravity);
    lua_setglobal(Core.lua, "GetGravity");

    lua_pushcfunction(Core.lua, l_SetGravity);
    lua_setglobal(Core.lua, "SetGravity");
}