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
            Vector3 collisionPoint = VECTOR3_ZERO;
            //Vector3 collisionNormal = VECTOR3_ZERO;
            if(VoxelModelVsVoxelModelCollision(entity, i,&collisionPoint)){
                Vector3 VelA = Subtract(GetVelocity(entity) , ScalarMult(Subtract(GetVelocity(entity),GetVelocity(i)), Lerp(GetBounciness(entity),0.3,1)*2*GetMass(i)/(GetMass(entity) + GetMass(i)) ) );
                Vector3 VelB = Add(GetVelocity(i) , ScalarMult(Subtract(GetVelocity(entity),GetVelocity(i)), Lerp(GetBounciness(i),0.5,1)*2*GetMass(entity)/(GetMass(entity) + GetMass(i)) ) );
                //v1f = v1i -2*m2(v1i-v2i)/(m1+m2)
                //v2f = v2i +2*m1(v1i-v2i)/(m1+m2)

                SetVelocity(entity,VelA);
                SetVelocity(i,VelB);
                collided = 1;

                ExplodeAtPoint(entity,collisionPoint,10);   
                ExplodeAtPoint(i,collisionPoint,10);   
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

    
    VoxelModel *objA = GetVoxelModelPointer(entityA);
    VoxelModel *objB = GetVoxelModelPointer(entityB);

    if(!objA->model || !objB->model){
        return 0;
    }

    Vector3 posA = GetPosition(entityA);
    Vector3 posB = GetPosition(entityB);
    Vector3 centerA = GetVoxelModelCenter(entityA);
    Vector3 centerB = GetVoxelModelCenter(entityB);

    if(IsVoxelModelSmallScale(entityA)) centerA = (Vector3){centerA.x/2,centerA.y/2,centerA.z/2};
    if(IsVoxelModelSmallScale(entityB)) centerB = (Vector3){centerB.x/2,centerB.y/2,centerB.z/2};

    Vector3 velA = GetVelocity(entityA);
    Vector3 velB = GetVelocity(entityB);
    Vector3 accA = GetAcceleration(entityA);
    Vector3 accB = GetAcceleration(entityB);

    int i,x,y,z,index = 0;
    //Sets an limit of how high the physics time step can be
    double moveDelta = Time.deltaTime>0.02? 0.02:Time.deltaTime;

    Vector3 movementA = {velA.x + accA.x*moveDelta,velA.y + accA.y*moveDelta,velA.z + accA.z*moveDelta};
    movementA = ScalarMult(movementA,moveDelta);

    Vector3 movementB = {velB.x + accB.x*moveDelta,velB.y + accB.y*moveDelta,velB.z + accB.z*moveDelta};
    movementB = ScalarMult(movementB,moveDelta);

    //ObjA rotation matrix
    Matrix3x3 mA = GetRotationMatrix(entityA);

    //ObjB Transposed rotation matrix
    Matrix3x3 mB = Transpose(GetRotationMatrix(entityB));


    glBindBuffer(GL_ARRAY_BUFFER, objA->vbo[0]);
    Vector3* vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);

    for(i=0; i<objA->numberOfVertices; i++){

        x = roundf(vertices[i].x);
        y = roundf(vertices[i].y);
        z = roundf(vertices[i].z);

        //Ignore invalid voxels
        if(x<0 || y<0 || z<0) continue;

        if(objA->smallScale){
            x /= 2;
            y /= 2;
            z /= 2;
        }

        //Apply A rotation matrix
        Vector3 p = {x - centerA.x, y - centerA.y, z - centerA.z};

        Vector3 pRotA = RotateVector(p,mA);
        x = pRotA.x;
        y = pRotA.y;
        z = pRotA.z;

        //Apply A position and movement
        x += posA.x + movementA.x;
        y += posA.y + movementA.y;
        z += posA.z + movementA.z;

        //Set origin as B position
        Vector3 localPosAinB = { x - posB.x-movementB.x, y - posB.y-movementB.y, z - posB.z-movementB.z};

        //Apply B (Transposed) rotation matrix
        p = (Vector3){localPosAinB.x, localPosAinB.y, localPosAinB.z};
        localPosAinB = RotateVector(p,mB);

        localPosAinB = (Vector3){roundf(localPosAinB.x +centerB.x), roundf(localPosAinB.y +centerB.y), roundf(localPosAinB.z +centerB.z)};

        //If voxel of entityA is inside the entityB volume
        if(!objB->smallScale){
            if(localPosAinB.x < objB->dimension[0] && localPosAinB.x >= 0 && localPosAinB.z < objB->dimension[2] && localPosAinB.z >= 0 && localPosAinB.y <objB->dimension[1] && localPosAinB.y >= 0){
                index = localPosAinB.x + localPosAinB.z * objB->dimension[0] + localPosAinB.y * objB->dimension[0] * objB->dimension[2];
                if(objB->model[index]!=0){
                    collisionPoint->x += x;
                    collisionPoint->y += y;
                    collisionPoint->z += z;

                    glBindBuffer(GL_ARRAY_BUFFER, objA->vbo[0]);
                    glUnmapBuffer(GL_ARRAY_BUFFER);
                    return 1;
                }
            }
        }else{
            if(localPosAinB.x < objB->dimension[0]/2 && localPosAinB.x >= 0 && localPosAinB.z < objB->dimension[2]/2 && localPosAinB.z >= 0 && localPosAinB.y <objB->dimension[1]/2 && localPosAinB.y >= 0){
                index = localPosAinB.x*2 + localPosAinB.z*2 * objB->dimension[0] + localPosAinB.y*2 * objB->dimension[0] * objB->dimension[2];
                if(objB->model[index]!=0){
                    collisionPoint->x += x;
                    collisionPoint->y += y;
                    collisionPoint->z += z;

                    glBindBuffer(GL_ARRAY_BUFFER, objA->vbo[0]);
                    glUnmapBuffer(GL_ARRAY_BUFFER);
                    return 1;
                }
            }
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, objA->vbo[0]);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    return 0;
}

//Test function
void ExplodeAtPoint(EntityID entity,Vector3 point,int radius){
    VoxelModel *obj = GetVoxelModelPointer(entity);
    
    Vector3 rotation;
    GetGlobalTransform(entity, NULL, &rotation, NULL);
    Vector3 p = RotatePoint(point, rotation, obj->center);
    p = (Vector3){round(p.x), round(p.y), round(p.z)};

    int startx,endx,starty,endy,startz,endz;
    int ix,iy,iz,index;
        startx = p.x-radius <0? 0:p.x-radius;
        starty = p.y-radius <0? 0:p.y-radius;
        startz = p.z-radius <0? 0:p.z-radius;

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