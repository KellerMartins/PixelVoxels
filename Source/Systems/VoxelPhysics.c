#include "VoxelPhysics.h"
//Each voxel is 10 centimeters

static System *ThisSystem;
static ComponentID VoxelModelID = -1;
static ComponentID RigidBodyID = -1;
static ComponentID TransformID = -1;

extern engineTime Time;
extern engineECS ECS;

int VoxelModelVsVoxelModelCollision(EntityID entityA, EntityID entityB,Vector3 *collisionPoint);

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


        if(IsStaticRigidbody(entity)) {
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

    
    VoxelModel *objA = GetVoxelModelPointer(entityA);
    VoxelModel *objB = GetVoxelModelPointer(entityB);

    if(!objA->model || !objB->model){
        return 0;
    }

    Vector3 posA = GetPosition(entityA);
    Vector3 posB = GetPosition(entityB);
    Vector3 rotA = GetRotation(entityA);
    Vector3 rotB = GetRotation(entityB);
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

    if(norm(movementA)>1){
        printf("Step needed!(%f)\n",norm(movementA));
    }

    //Simplificated rotation matrix terms
    //ObjA rotation matrix
    float sinx = sin(rotA.x * PI_OVER_180);
    float cosx = cos(rotA.x * PI_OVER_180);
    float siny = sin(rotA.y * PI_OVER_180);
    float cosy = cos(rotA.y * PI_OVER_180);
    float sinz = sin(rotA.z * PI_OVER_180);
    float cosz = cos(rotA.z * PI_OVER_180);

    float arxt1 = cosy*cosz, arxt2 = (cosz*sinx*siny - cosx*sinz), arxt3 = (cosx*cosz*siny + sinx*sinz);
    float aryt1 = cosy*sinz, aryt2 = (cosx*siny*sinz - cosz*sinx), aryt3 = (cosx*cosz + sinx*siny*sinz);
    float arzt1 = cosx*cosy, arzt2 = sinx*cosy,                    arzt3 = siny;

    //ObjB Transposed rotation matrix
    sinx = sin(rotB.x * PI_OVER_180);
    cosx = cos(rotB.x * PI_OVER_180);
    siny = sin(rotB.y * PI_OVER_180);
    cosy = cos(rotB.y * PI_OVER_180);
    sinz = sin(rotB.z * PI_OVER_180);
    cosz = cos(rotB.z * PI_OVER_180);

    float brxt1 = cosy*cosz, brxt2 = siny,                         brxt3 = cosy*sinz;
    float bryt1 = cosy*sinx, bryt2 = (cosz*sinx*siny - cosx*sinz), bryt3 = (cosx*cosz + sinx*siny*sinz);
    float brzt1 = cosx*cosy, brzt2 = (cosx*cosz*siny + sinx*sinz), brzt3 = (cosx*siny*sinz - cosz*sinx);

    for(i = 0; i <objA->numberOfVertices*3  ; i+=3){

        x = roundf(objA->vertices[i]);
        y = roundf(objA->vertices[i+1]);
        z = roundf(objA->vertices[i+2]);

        //Ignore invalid voxels
        if(x<0 || y<0 || z<0) continue;

        if(objA->smallScale){
            x /= 2;
            y /= 2;
            z /= 2;
        }

        //Apply A rotation matrix
        Vector3 p = {x - centerA.x, y - centerA.y, z - centerA.z};
        x = p.x*arxt1 + p.y*arxt2 + p.z*arxt3;
        y = p.x*aryt1 + p.z*aryt2 + p.y*aryt3;
        z = p.z*arzt1 + p.y*arzt2 - p.x*arzt3;

        //Apply A position and movement
        x += posA.x + movementA.x;
        y += posA.y + movementA.y;
        z += posA.z + movementA.z;

        //Set origin as B position
        Vector3 localPosAinB = { x - posB.x-movementB.x, y - posB.y-movementB.y, z - posB.z-movementB.z};

        //Apply B (Transposed) rotation matrix
        p = (Vector3){localPosAinB.x, localPosAinB.y, localPosAinB.z};
        localPosAinB.x = p.x*brxt1 - p.z*brxt2 + p.y*brxt3;
        localPosAinB.y = p.z*bryt1 + p.x*bryt2 + p.y*bryt3;
        localPosAinB.z = p.z*brzt1 + p.x*brzt2 + p.y*brzt3;

        localPosAinB = (Vector3){roundf(localPosAinB.x +centerB.x), roundf(localPosAinB.y +centerB.y), roundf(localPosAinB.z +centerB.z)};

        //If voxel of entityA is inside the entityB volume
        if(!objB->smallScale){
            if(localPosAinB.x < objB->dimension[0] && localPosAinB.x >= 0 && localPosAinB.z < objB->dimension[2] && localPosAinB.z >= 0 && localPosAinB.y <objB->dimension[1] && localPosAinB.y >= 0){
                index = localPosAinB.x + localPosAinB.z * objB->dimension[0] + localPosAinB.y * objB->dimension[0] * objB->dimension[2];
                if(objB->model[index]!=0){
                    collisionPoint->x += x;
                    collisionPoint->y += y;
                    collisionPoint->z += z;

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
                    return 1;
                }
            }
        }
    }
    return 0;
}