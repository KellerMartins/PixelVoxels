#include "VoxelPhysics.h"
//Each voxel is 10 centimeters

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
    VoxelModelID = GetComponentID("VoxelModel");
    RigidBodyID = GetComponentID("RigidBody");
    TransformID = GetComponentID("Transform");
}

//Simple collision and kinematic movement
//Warning: Not even close to be physically or mathematically correct!
void VoxelPhysicsUpdate(EntityID entity){
    if(IsStaticRigidbody(entity)) {
        SetVelocity(entity,VECTOR3_ZERO);
        SetAcceleration(entity,VECTOR3_ZERO);
        return;
    }

    //Sets an limit of how high the physics time step can be
    double moveDelta = Time.deltaTime>0.02? 0.02:Time.deltaTime;
    ComponentMask needed = CreateComponentMaskByID(3,VoxelModelID, RigidBodyID, TransformID);

    int i,collided = 0;
    for(i=0;i<=ECS.maxUsedIndex;i++){

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
    

    SetVelocity(entity,Add(GetVelocity(entity),ScalarMult(GetAcceleration(entity),moveDelta)));
    if(UsesGravity(entity)){
        if(!collided)
            SetAcceleration(entity,(Vector3){0,0,-GetGravity()});
        else{
            SetAcceleration(entity,VECTOR3_ZERO);
        }  
    }
    Vector3 posDelta = ScalarMult(Add(ScalarMult(GetVelocity(entity),moveDelta) , ScalarMult(GetAcceleration(entity),0.5 * moveDelta * moveDelta)),WORLD_SCALE);

    Vector3 pos = Add(GetPosition(entity),posDelta);
    SetPosition(entity,pos);
}

void VoxelPhysicsFree(){
    
}

int VoxelModelVsVoxelModelCollision(EntityID entityA, EntityID entityB,Vector3 *collisionPoint){
    
    VoxelModel *objA = GetVoxelModelPointer(entityA);
    VoxelModel *objB = GetVoxelModelPointer(entityB);

    Vector3 posA = GetPosition(entityA);
    Vector3 posB = GetPosition(entityB);
    //Vector3 rotA = GetRotation(entityA);
    //Vector3 rotB = GetRotation(entityB);

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
    
    //int collisions = 0;

    for(i = 0; i <objA->numberOfVertices*3  ; i+=3){

        x = roundf(objA->vertices[i]);
        y = roundf(objA->vertices[i+1]);
        z = roundf(objA->vertices[i+2]);

        //Ignore invalid voxels
        if(x<0 || y<0 || z<0) continue;

        x += posA.x + movementA.x;
        y += posA.y + movementA.y;
        z += posA.z + movementA.z;

        Vector3 localPosAinB = { roundf(x-posB.x-movementB.x), roundf(y-posB.y-movementB.y), roundf(z-posB.z-movementB.z)};

        //If voxel of entityA is inside the entityB volume
        if(localPosAinB.x < objB->dimension[0] && localPosAinB.x > -1 && localPosAinB.z < objB->dimension[2] && localPosAinB.z >-1 && localPosAinB.y <objB->dimension[1] && localPosAinB.y >-1){
            index = localPosAinB.x + localPosAinB.z * objB->dimension[0] + localPosAinB.y * objB->dimension[0] * objB->dimension[2];
            if(objB->model[index]!=0){
                collisionPoint->x += x;
                collisionPoint->y += y;
                collisionPoint->z += z;

                /*objA->vColors[i] = 1;
                objA->vColors[i+1] = 0;
                objA->vColors[i+2] = 0;
                x = roundf(objA->vertices[i]);
                y = roundf(objA->vertices[i+1]);
                z = roundf(objA->vertices[i+2]);

                if(x-1 < 0){
                    collisionNormal->x -= 1;
                }else{
                    index = (x-1) + (z) * objA->dimension[0] + (y) * objA->dimension[0] * objA->dimension[2];
                    if(objA->model[index]==0){
                        collisionNormal->x -= 1;
                    }
                }

                if(x+1 >= objA->dimension[0]){
                    collisionNormal->x += 1;
                }else{
                    index = (x+1) + (z) * objA->dimension[0] + (y) * objA->dimension[0] * objA->dimension[2];
                    if(objA->model[index]==0){
                        collisionNormal->x += 1;
                    }
                }

                if(z-1 < 0){
                    collisionNormal->z -= 1;
                }else{
                    index = (x) + (z-1) * objA->dimension[0] + (y) * objA->dimension[0] * objA->dimension[2];
                    if(objA->model[index]==0){
                        collisionNormal->z -= 1;
                    }
                }

                if(z+1 >= objA->dimension[2]){
                    collisionNormal->z += 1;
                }else{
                    index = (x) + (z+1) * objA->dimension[0] + (y) * objA->dimension[0] * objA->dimension[2];
                    if(objA->model[index]==0){
                        collisionNormal->z += 1;
                    }
                }

                if(y-1 < 0){
                    collisionNormal->y -= 1;
                }else{
                    index = (x) + (z) * objA->dimension[0] + (y-1) * objA->dimension[0] * objA->dimension[2];
                    if(objA->model[index]==0){
                        collisionNormal->y -= 1;
                    }
                }

                if(y+1 >= objA->dimension[1]){
                    collisionNormal->y += 1;
                }else{
                    index = (x) + (z) * objA->dimension[0] + (y+1) * objA->dimension[0] * objA->dimension[2];
                    if(objA->model[index]==0){
                        collisionNormal->y += 1;
                    }
                }*/
                //printf("%f %f %f\n",collisionPoint->x,collisionPoint->y,collisionPoint->z);
                
                //collisions++;
                return 1;
            }
        }
    }
    /*if(collisions){
        collisionPoint->x /= collisions;
        collisionPoint->y /= collisions;
        collisionPoint->z /= collisions;
        printf("%i %i \n",entityA,entityB);
        return 1;
    }*/
    return 0;
}