// ==========================================================
// Smart Fire Station - Protected Source Version
// Original project owner source protection copy.
// Core functionality preserved.
// ==========================================================



















#define _CRT_SECURE_NO_WARNINGS

#include <GL/freeglut.h>
#include <GL/glu.h>
#include <cmath>
#include <cstdio>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif




struct Vec3 { float x, y, z; };


int windowWidth  = 1000;
int windowHeight = 700;


int cameraMode = 1;


float freeOrbitYaw    = 45.0f;  
float freeOrbitPitch  = 25.0f;  
float freeOrbitRadius = 60.0f;  
float freeOrbitTargetX = 0.0f, freeOrbitTargetY = 6.0f, freeOrbitTargetZ = 0.0f;


bool isNight = false;


enum SimState {
    STATE_NORMAL,
    STATE_FIRE_DETECTED,
    STATE_TRUCK_DISPATCHED,
    STATE_RESPONDING,
    STATE_WATER_SPRAY_ACTIVE,
    STATE_MISSION_COMPLETED,
    STATE_RETURNING
};
SimState simState = STATE_NORMAL;


float truckStartX = -35.0f, truckStartZ = -20.0f;
float truckX = truckStartX, truckZ = truckStartZ;
float truckY = 0.0f;
float truckAngle   = 0.0f;   
float truckSpeed   = 14.0f;  
float wheelAngle   = 0.0f;
bool  truckAutoMoving = false;


Vec3 waypoints[3];
Vec3 returnWaypoints[3]; 
int  currentWaypoint = 0;


bool  returnPending = false;   
float missionCompleteTimer = 0.0f;


float buildingX = 19.0f, buildingZ = 15.0f;
bool  buildingBurning = false;
float fireLevel  = 0.0f;   
float smokeLevel = 0.0f;   


bool waterSprayOn  = false;  
bool manualWaterOn = false;  


bool  sirenRedBright = true;
float sirenTimer = 0.0f;


bool  dispatchPending = false;
float dispatchTimer = 0.0f;


int   lastElapsedMs   = 0;
float simulationTime  = 0.0f;
float responseTimer   = 0.0f;
bool  responseTimerRunning = false;


GLUquadric* quadric = NULL;


struct TrafficCar {
    float x, z;
    float speed;
    int   axis;  
    int   dir;   
    float r, g, b;
};
const int NUM_TRAFFIC_CARS = 6;
TrafficCar trafficCars[NUM_TRAFFIC_CARS];


struct Pedestrian {
    float x, z;
    float speed;
    int   axis;  
    int   dir;   
    float r, g, b; 
    float phase;   
    float rangeMin, rangeMax; 
};
const int NUM_PEDESTRIANS = 6;
Pedestrian pedestrians[NUM_PEDESTRIANS];


struct Commuter {
    float startX, startZ;   
    float targetX, targetZ; 
    float t;                
    float speed;            
    float r, g, b;          
};
const int NUM_COMMUTERS = 4;
Commuter commuters[NUM_COMMUTERS];


struct Bird {
    float cx, cz;        
    float y;             
    float radius;
    float angle;         
    float angularSpeed;  
    float wingPhase;
};
const int NUM_BIRDS = 7;
Bird birds[NUM_BIRDS];


struct Plane {
    float x, y, z;
    float speed;
    int   dir; 
    float blinkPhase;
};
Plane plane = { -140.0f, 19.0f, -22.0f, 14.0f, +1, 0.0f };


bool fogEnabled = true;




void initGL();
void resetSimulation();
void startEmergency();

void drawGround();
void drawRoads();
void drawTree(float x, float z);
void drawStreetLight(float x, float z);
void drawTrafficLight(float x, float z, bool emergencyGreen);
void drawFireStation();
void drawFireStationInterior(float fx, float fz);
void drawHospital();
void drawHospitalInterior(float hx, float hz);
void drawPlainBuilding(float x, float z, float w, float h, float d, float r, float g, float b);
void drawFireHydrant(float x, float z);
void drawBarrier(float x, float z);
void drawTruck();
void drawFireAnimation();
void drawSmoke();
void drawWaterSpray();
void drawStatusText();
void drawLabel3D(float x, float y, float z, void* font, const char* text);
void drawSchool();
void drawSchoolInterior(float sx, float sz);

void applyFogForTime();
void drawGradientSky();
void drawGroundShadow(float x, float z, float radiusX, float radiusZ);

void initCityLife();
void updateCityLife(float dt);
void drawTrafficCars();
void drawPedestrians();
void drawCommuters();
void drawPersonFigure(float phase, float r, float g, float b);
void drawStandingPerson(float x, float z, float facingDeg, float phaseOffset, float r, float g, float b);
void drawBirds();
void drawPlane();

void drawCylinderAt(float x, float y, float z, float radius, float height,
                     float rotX, float rotY, float rotZ,
                     float r, float g, float b, bool emissive);

void setCameraView();
void display();
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);
void specialKeyboard(int key, int x, int y);
void update(int value);

float distance2D(float x1, float z1, float x2, float z2) {
    float dx = x2 - x1;
    float dz = z2 - z1;
    return sqrtf(dx * dx + dz * dz);
}




int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Smart Fire Station - Emergency Response Simulation");

    initGL();
    resetSimulation();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeyboard);
    glutTimerFunc(16, update, 0);

    lastElapsedMs = glutGet(GLUT_ELAPSED_TIME);

    glutMainLoop();
    return 0;
}




void initGL() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_NORMALIZE);

    GLfloat lightDiff[] = { 1.0f, 1.0f, 0.95f, 1.0f };
    GLfloat lightAmb[]  = { 0.35f, 0.35f, 0.35f, 1.0f };
    GLfloat lightSpec[] = { 0.9f, 0.9f, 0.9f, 1.0f };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiff);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpec);

    
    GLfloat matteSpec[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matteSpec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);

    quadric = gluNewQuadric();
    gluQuadricNormals(quadric, GLU_SMOOTH);

    
    waypoints[0] = { 0.0f, 0.0f, truckStartZ };
    waypoints[1] = { 0.0f, 0.0f, buildingZ };
    waypoints[2] = { buildingX - 10.0f, 0.0f, buildingZ }; 

    
    returnWaypoints[0] = waypoints[2];
    returnWaypoints[1] = waypoints[1];
    returnWaypoints[2] = waypoints[0];

    
    glFogi(GL_FOG_MODE, GL_EXP2);
    glHint(GL_FOG_HINT, GL_NICEST);
    glDisable(GL_FOG); 

    initCityLife();
}


void applyFogForTime() {
    if (!fogEnabled || !isNight) {
        glDisable(GL_FOG); 
        return;
    }
    glEnable(GL_FOG);
    GLfloat fogNight[] = { 0.03f, 0.03f, 0.08f, 1.0f };
    glFogfv(GL_FOG_COLOR, fogNight);
    glFogf(GL_FOG_DENSITY, 0.010f);
}





void initCityLife() {
    
    trafficCars[0] = { -50.0f, -2.5f, 9.0f, 0, +1, 0.80f, 0.80f, 0.85f };
    trafficCars[1] = {  20.0f, -2.5f, 7.5f, 0, +1, 0.15f, 0.35f, 0.75f };
    trafficCars[2] = {  40.0f,  2.5f, 8.0f, 0, -1, 0.85f, 0.75f, 0.15f };
    trafficCars[3] = {  -2.5f,-40.0f, 8.5f, 1, +1, 0.20f, 0.55f, 0.25f };
    trafficCars[4] = {   2.5f, 25.0f, 7.0f, 1, -1, 0.60f, 0.15f, 0.55f };
    trafficCars[5] = {  -2.5f, 50.0f, 9.5f, 1, +1, 0.85f, 0.45f, 0.10f };

    
    
    
    pedestrians[0] = {  20.0f,   8.0f, 1.6f, 0, +1, 0.75f, 0.20f, 0.20f, 0.0f,  10.0f,  50.0f };
    pedestrians[1] = { -20.0f,  -8.0f, 1.8f, 0, -1, 0.20f, 0.55f, 0.70f, 1.0f, -50.0f, -10.0f };
    pedestrians[2] = {  15.0f,  13.0f, 1.5f, 0, +1, 0.70f, 0.65f, 0.15f, 2.1f,   8.0f,  45.0f };
    pedestrians[3] = {   8.0f, -35.0f, 1.7f, 1, +1, 0.35f, 0.35f, 0.75f, 0.6f, -50.0f, -10.0f };
    pedestrians[4] = {  -8.0f,  20.0f, 1.6f, 1, -1, 0.85f, 0.45f, 0.60f, 1.8f,  10.0f,  50.0f };
    pedestrians[5] = {   8.0f,  45.0f, 1.9f, 1, +1, 0.25f, 0.70f, 0.35f, 2.9f,  10.0f,  50.0f };

    
    commuters[0] = { -37.0f,  42.0f, -36.0f, 29.3f, 0.00f, 0.10f, 0.65f, 0.25f, 0.20f }; 
    commuters[1] = { -33.0f,  45.0f, -34.0f, 29.3f, 0.35f, 0.09f, 0.25f, 0.55f, 0.75f }; 
    commuters[2] = { -35.0f,  34.0f, -35.0f, 20.4f, 0.60f, 0.11f, 0.85f, 0.85f, 0.90f }; 
    commuters[3] = { -35.0f, -42.0f, -35.0f,-24.2f, 0.15f, 0.12f, 0.70f, 0.15f, 0.10f }; 

    
    birds[0] = {   0,   0, 16.0f, 14.0f, 0.0f, 0.35f, 0.0f };
    birds[1] = {  15, -15, 18.0f, 10.0f, 1.5f, 0.45f, 1.0f };
    birds[2] = { -20,  10, 15.0f, 12.0f, 3.0f, 0.30f, 2.0f };
    birds[3] = {  10,  20, 20.0f,  9.0f, 4.5f, 0.40f, 0.5f };
    birds[4] = { -10, -25, 17.0f, 11.0f, 2.2f, 0.38f, 1.5f };
    birds[5] = {  25,   5, 19.0f, 13.0f, 0.8f, 0.42f, 2.5f };
    birds[6] = {  -5,  30, 14.0f,  8.0f, 5.0f, 0.33f, 3.2f };

    plane = { -140.0f, 19.0f, -22.0f, 14.0f, +1, 0.0f };
}

void updateCityLife(float dt) {
    
    for (int i = 0; i < NUM_TRAFFIC_CARS; i++) {
        TrafficCar &c = trafficCars[i];
        if (c.axis == 0) {
            c.x += c.dir * c.speed * dt;
            if (c.x >  60.0f) c.x = -60.0f;
            if (c.x < -60.0f) c.x =  60.0f;
        } else {
            c.z += c.dir * c.speed * dt;
            if (c.z >  60.0f) c.z = -60.0f;
            if (c.z < -60.0f) c.z =  60.0f;
        }
    }

    
    for (int i = 0; i < NUM_BIRDS; i++) {
        birds[i].angle     += birds[i].angularSpeed * dt;
        birds[i].wingPhase += dt * 10.0f;
    }

    
    for (int i = 0; i < NUM_PEDESTRIANS; i++) {
        Pedestrian &p = pedestrians[i];
        p.phase += dt * 4.5f;
        if (p.axis == 0) {
            p.x += p.dir * p.speed * dt;
            if (p.x > p.rangeMax) { p.x = p.rangeMax; p.dir = -1; }
            if (p.x < p.rangeMin) { p.x = p.rangeMin; p.dir = +1; }
        } else {
            p.z += p.dir * p.speed * dt;
            if (p.z > p.rangeMax) { p.z = p.rangeMax; p.dir = -1; }
            if (p.z < p.rangeMin) { p.z = p.rangeMin; p.dir = +1; }
        }
    }

    
    for (int i = 0; i < NUM_COMMUTERS; i++) {
        Commuter &c = commuters[i];
        c.t += c.speed * dt;
        if (c.t > 1.0f) c.t = 0.0f;
    }

    
    plane.x += plane.dir * plane.speed * dt;
    plane.blinkPhase += dt;
    if (plane.x >  150.0f) plane.x = -150.0f;
    if (plane.x < -150.0f) plane.x =  150.0f;
}


void drawTrafficCars() {
    for (int i = 0; i < NUM_TRAFFIC_CARS; i++) {
        TrafficCar &c = trafficCars[i];
        glPushMatrix();
        glTranslatef(c.x, 0, c.z);
        glRotatef((c.axis == 0) ? ((c.dir > 0) ? 0.0f : 180.0f)
                                 : ((c.dir > 0) ? -90.0f : 90.0f), 0, 1, 0);

        glPushMatrix();
        glTranslatef(0, 0.55f, 0);
        glScalef(2.4f, 0.9f, 1.1f);
        glColor3f(c.r, c.g, c.b);
        glutSolidCube(1.0);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0.1f, 1.05f, 0);
        glScalef(1.3f, 0.6f, 1.0f);
        glColor3f(c.r * 0.85f, c.g * 0.85f, c.b * 0.85f);
        glutSolidCube(1.0);
        glPopMatrix();

        float wx[2] = { -0.8f, 0.8f };
        float wz[2] = { -0.5f, 0.5f };
        for (int a = 0; a < 2; a++) {
            for (int bb = 0; bb < 2; bb++) {
                glPushMatrix();
                glTranslatef(wx[a], 0.28f, wz[bb]);
                glColor3f(0.08f, 0.08f, 0.08f);
                glutSolidTorus(0.10, 0.26, 6, 10);
                glPopMatrix();
            }
        }
        glPopMatrix();
    }
}


void drawPersonFigure(float phase, float r, float g, float b) {
    float swing = sinf(phase) * 18.0f;

    glPushMatrix();
    glTranslatef(0.07f, 0.28f, 0);
    glRotatef(swing, 1, 0, 0);
    glTranslatef(0, -0.28f, 0);
    glColor3f(0.15f, 0.15f, 0.2f);
    glScalef(0.12f, 0.56f, 0.14f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.07f, 0.28f, 0);
    glRotatef(-swing, 1, 0, 0);
    glTranslatef(0, -0.28f, 0);
    glColor3f(0.10f, 0.10f, 0.15f);
    glScalef(0.12f, 0.56f, 0.14f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, 0.85f, 0);
    glColor3f(r, g, b);
    glScalef(0.34f, 0.5f, 0.22f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, 1.25f, 0);
    glColor3f(0.85f, 0.68f, 0.55f);
    glutSolidSphere(0.15, 10, 10);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.22f, 1.0f, 0);
    glRotatef(-swing, 1, 0, 0);
    glColor3f(r * 0.8f, g * 0.8f, b * 0.8f);
    glScalef(0.09f, 0.42f, 0.12f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.22f, 1.0f, 0);
    glRotatef(swing, 1, 0, 0);
    glColor3f(r * 0.8f, g * 0.8f, b * 0.8f);
    glScalef(0.09f, 0.42f, 0.12f);
    glutSolidCube(1.0);
    glPopMatrix();
}



void drawStandingPerson(float x, float z, float facingDeg, float phaseOffset, float r, float g, float b) {
    glPushMatrix();
    glTranslatef(x, 0, z);
    glRotatef(facingDeg, 0, 1, 0);
    float idlePhase = simulationTime * 0.6f + phaseOffset;
    drawPersonFigure(idlePhase, r, g, b);
    glPopMatrix();
}

void drawPedestrians() {
    for (int i = 0; i < NUM_PEDESTRIANS; i++) {
        Pedestrian &p = pedestrians[i];
        glPushMatrix();
        glTranslatef(p.x, 0, p.z);
        glRotatef((p.axis == 0) ? ((p.dir > 0) ? 0.0f : 180.0f)
                                 : ((p.dir > 0) ? -90.0f : 90.0f), 0, 1, 0);
        drawPersonFigure(p.phase, p.r, p.g, p.b);
        glPopMatrix();
    }
}




void drawCommuters() {
    for (int i = 0; i < NUM_COMMUTERS; i++) {
        Commuter &c = commuters[i];
        float x = c.startX + (c.targetX - c.startX) * c.t;
        float z = c.startZ + (c.targetZ - c.startZ) * c.t;

        float dx = c.targetX - c.startX, dz = c.targetZ - c.startZ;
        float headingDeg = atan2f(-dz, dx) * 180.0f / 3.14159265f;

        glPushMatrix();
        glTranslatef(x, 0, z);
        glRotatef(headingDeg, 0, 1, 0);
        drawPersonFigure(c.t * 20.0f, c.r, c.g, c.b);
        glPopMatrix();
    }
}


void drawBirds() {
    for (int i = 0; i < NUM_BIRDS; i++) {
        Bird &bd = birds[i];
        float x = bd.cx + bd.radius * cosf(bd.angle);
        float z = bd.cz + bd.radius * sinf(bd.angle);

        glPushMatrix();
        glTranslatef(x, bd.y, z);
        glRotatef(-90.0f - (bd.angle * 180.0f / 3.14159265f), 0, 1, 0);

        float flapY = sinf(bd.wingPhase) * 0.35f; 
        glColor3f(0.12f, 0.12f, 0.12f);
        glBegin(GL_TRIANGLES);
            glVertex3f(0, 0, 0); glVertex3f(-0.6f, flapY,  0.3f); glVertex3f(-0.6f, flapY, -0.3f);
            glVertex3f(0, 0, 0); glVertex3f( 0.6f, flapY,  0.3f); glVertex3f( 0.6f, flapY, -0.3f);
        glEnd();

        
        glBegin(GL_TRIANGLES);
            glVertex3f(-0.30f, 0, 0);
            glVertex3f(-0.62f, 0, 0.16f);
            glVertex3f(-0.62f, 0, -0.16f);
        glEnd();

        
        glPushMatrix();
        glScalef(0.38f, 0.20f, 0.20f);
        glutSolidSphere(1.0, 8, 8);
        glPopMatrix();

        
        glPushMatrix();
        glTranslatef(0.34f, 0.08f, 0);
        glutSolidSphere(0.14, 8, 8);
        glPopMatrix();

        
        glPushMatrix();
        glTranslatef(0.46f, 0.08f, 0);
        glColor3f(0.85f, 0.55f, 0.10f);
        glRotatef(90, 0, 1, 0);
        glutSolidCone(0.05, 0.14, 6, 2);
        glPopMatrix();

        glPopMatrix();
    }
}


void drawPlane() {
    glPushMatrix();
    glTranslatef(plane.x, plane.y, plane.z);
    glRotatef((plane.dir > 0) ? 0.0f : 180.0f, 0, 1, 0);

    glColor3f(0.85f, 0.85f, 0.88f);
    glPushMatrix(); glScalef(6.0f, 0.9f, 0.9f); glutSolidSphere(0.5, 10, 10); glPopMatrix();

    glColor3f(0.65f, 0.65f, 0.70f);
    glPushMatrix();
    glTranslatef(-0.3f, 0, 0);
    glScalef(1.6f, 0.15f, 6.5f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-2.6f, 0.6f, 0);
    glScalef(1.0f, 1.2f, 0.15f);
    glutSolidCube(1.0);
    glPopMatrix();

    if (isNight) {
        float blink = (sinf(plane.blinkPhase * 6.0f) > 0.5f) ? 1.0f : 0.15f;
        GLfloat redEmis[]   = { blink, 0.0f, 0.0f, 1.0f };
        GLfloat greenEmis[] = { 0.0f, blink, 0.0f, 1.0f };
        GLfloat noEmis[]    = { 0.0f, 0.0f, 0.0f, 1.0f };

        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, redEmis);
        glColor3f(blink, 0, 0);
        glPushMatrix(); glTranslatef(-0.3f, 0, 3.2f); glutSolidSphere(0.15, 6, 6); glPopMatrix();

        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, greenEmis);
        glColor3f(0, blink, 0);
        glPushMatrix(); glTranslatef(-0.3f, 0, -3.2f); glutSolidSphere(0.15, 6, 6); glPopMatrix();

        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, noEmis);
    }

    glPopMatrix();
}


void drawSchool() {
    float sx = -35.0f, sz = 25.0f;
    drawPlainBuilding(sx, sz, 9, 5, 8, 0.88f, 0.75f, 0.45f);

    glPushMatrix();
    glTranslatef(sx, 2.6f, sz + 4.05f);
    glColor3f(0.10f, 0.30f, 0.65f);
    glScalef(2.6f, 0.6f, 0.05f);
    glutSolidCube(1.0);
    glPopMatrix();

    drawCylinderAt(sx + 3.0f, 0, sz - 3.0f, 0.06f, 3.0f, -90, 0, 0, 0.4f, 0.3f, 0.2f, false);
    glPushMatrix();
    glTranslatef(sx + 3.0f, 3.0f, sz - 3.0f);
    glColor3f(0.85f, 0.15f, 0.15f);
    glScalef(0.8f, 0.5f, 0.02f);
    glutSolidCube(1.0);
    glPopMatrix();

    
    drawLabel3D(sx - 1.9f, 2.5f, sz + 4.08f, GLUT_BITMAP_HELVETICA_18, "SCHOOL");
    drawSchoolInterior(sx, sz);
}


void drawSchoolInterior(float sx, float sz) {
    
    glPushMatrix();
    glTranslatef(sx, 2.2f, sz - 3.9f);
    glColor3f(0.05f, 0.25f, 0.10f);
    glScalef(3.0f, 1.4f, 0.05f);
    glutSolidCube(1.0);
    glPopMatrix();

    
    glPushMatrix();
    glTranslatef(sx, 0.45f, sz - 3.0f);
    glColor3f(0.45f, 0.30f, 0.18f);
    glScalef(1.4f, 0.5f, 0.6f);
    glutSolidCube(1.0);
    glPopMatrix();

    
    float deskX[3] = { sx - 2.6f, sx, sx + 2.6f };
    float deskZ[2] = { sz + 0.5f, sz + 2.3f };
    for (int r = 0; r < 2; r++) {
        for (int c = 0; c < 3; c++) {
            glPushMatrix();
            glTranslatef(deskX[c], 0.35f, deskZ[r]);
            glColor3f(0.55f, 0.38f, 0.22f);
            glScalef(1.1f, 0.4f, 0.5f);
            glutSolidCube(1.0);
            glPopMatrix();
        }
    }

    
    drawStandingPerson(sx, sz - 2.2f, 0.0f,   0.0f, 0.30f, 0.35f, 0.55f);
    drawStandingPerson(sx - 2.6f, sz + 1.0f, 0.0f, 1.4f, 0.65f, 0.30f, 0.25f);
    drawStandingPerson(sx + 2.6f, sz + 1.0f, 0.0f, 2.7f, 0.30f, 0.55f, 0.45f);
}

void resetSimulation() {
    simState = STATE_NORMAL;

    truckX = truckStartX;
    truckZ = truckStartZ;
    truckAngle = 0.0f;
    truckAutoMoving = false;
    currentWaypoint = 0;

    buildingBurning = false;
    fireLevel = 0.0f;
    smokeLevel = 0.0f;

    waterSprayOn = false;
    manualWaterOn = false;

    dispatchPending = false;
    dispatchTimer = 0.0f;

    returnPending = false;
    missionCompleteTimer = 0.0f;

    responseTimer = 0.0f;
    responseTimerRunning = false;
}

void startEmergency() {
    if (simState != STATE_NORMAL) return;

    buildingBurning = true;
    fireLevel = 100.0f;
    smokeLevel = 100.0f;

    simState = STATE_FIRE_DETECTED;
    dispatchPending = true;
    dispatchTimer = 0.0f;

    responseTimer = 0.0f;
    responseTimerRunning = true;
}




void drawCylinderAt(float x, float y, float z, float radius, float height,
                     float rotX, float rotY, float rotZ,
                     float r, float g, float b, bool emissive) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(rotX, 1, 0, 0);
    glRotatef(rotY, 0, 1, 0);
    glRotatef(rotZ, 0, 0, 1);
    glColor3f(r, g, b);

    if (emissive) {
        GLfloat emis[] = { r, g, b, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emis);
    }

    gluCylinder(quadric, radius, radius, height, 12, 4);

    if (emissive) {
        GLfloat none[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, none);
    }
    glPopMatrix();
}




void drawGround() {
    glColor3f(0.20f, 0.45f, 0.22f);
    glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        glVertex3f(-60, 0, -60);
        glVertex3f( 60, 0, -60);
        glVertex3f( 60, 0,  60);
        glVertex3f(-60, 0,  60);
    glEnd();
}

void drawRoads() {
    glColor3f(0.15f, 0.15f, 0.17f);

    
    glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        glVertex3f(-55, 0.02f, -5);
        glVertex3f( 55, 0.02f, -5);
        glVertex3f( 55, 0.02f,  5);
        glVertex3f(-55, 0.02f,  5);
    glEnd();

    
    glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        glVertex3f(-5, 0.02f, -55);
        glVertex3f( 5, 0.02f, -55);
        glVertex3f( 5, 0.02f,  55);
        glVertex3f(-5, 0.02f,  55);
    glEnd();

    
    glColor3f(0.9f, 0.9f, 0.9f);
    for (int x = -50; x <= 50; x += 8) {
        glBegin(GL_QUADS);
            glNormal3f(0, 1, 0);
            glVertex3f((float)x - 1.5f, 0.03f, -0.15f);
            glVertex3f((float)x + 1.5f, 0.03f, -0.15f);
            glVertex3f((float)x + 1.5f, 0.03f,  0.15f);
            glVertex3f((float)x - 1.5f, 0.03f,  0.15f);
        glEnd();
    }
    for (int z = -50; z <= 50; z += 8) {
        glBegin(GL_QUADS);
            glNormal3f(0, 1, 0);
            glVertex3f(-0.15f, 0.03f, (float)z - 1.5f);
            glVertex3f( 0.15f, 0.03f, (float)z - 1.5f);
            glVertex3f( 0.15f, 0.03f, (float)z + 1.5f);
            glVertex3f(-0.15f, 0.03f, (float)z + 1.5f);
        glEnd();
    }
}

void drawTree(float x, float z) {
    drawGroundShadow(x, z, 1.3f, 1.3f);
    drawCylinderAt(x, 0, z, 0.3f, 2.2f, -90, 0, 0, 0.40f, 0.26f, 0.13f, false);

    glPushMatrix();
    glTranslatef(x, 2.2f, z);
    glRotatef(-90, 1, 0, 0);
    glColor3f(0.13f, 0.45f, 0.15f);
    glutSolidCone(1.3, 3.0, 10, 6);
    glPopMatrix();
}

void drawStreetLight(float x, float z) {
    drawCylinderAt(x, 0, z, 0.12f, 4.0f, -90, 0, 0, 0.3f, 0.3f, 0.3f, false);

    GLfloat none[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glPushMatrix();
    glTranslatef(x, 4.1f, z);
    if (isNight) {
        GLfloat emis[] = { 1.0f, 0.95f, 0.6f, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emis);
        glColor3f(1.0f, 0.95f, 0.6f);
    } else {
        glColor3f(0.85f, 0.8f, 0.55f);
    }
    glutSolidSphere(0.35, 10, 10);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, none);
    glPopMatrix();
}

void drawTrafficLight(float x, float z, bool emergencyGreen) {
    drawCylinderAt(x, 0, z, 0.15f, 3.2f, -90, 0, 0, 0.25f, 0.25f, 0.25f, false);

    GLfloat none[] = { 0.0f, 0.0f, 0.0f, 1.0f };

    glPushMatrix();
    glTranslatef(x, 3.7f, z);

    
    glPushMatrix();
    glTranslatef(0, 0.4f, 0);
    if (!emergencyGreen) {
        GLfloat emis[] = { 1.0f, 0.1f, 0.1f, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emis);
        glColor3f(1.0f, 0.1f, 0.1f);
    } else {
        glColor3f(0.35f, 0.05f, 0.05f);
    }
    glutSolidSphere(0.28, 10, 10);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, none);
    glPopMatrix();

    
    glPushMatrix();
    glTranslatef(0, -0.4f, 0);
    if (emergencyGreen) {
        GLfloat emis[] = { 0.1f, 1.0f, 0.15f, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emis);
        glColor3f(0.1f, 1.0f, 0.15f);
    } else {
        glColor3f(0.05f, 0.25f, 0.05f);
    }
    glutSolidSphere(0.28, 10, 10);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, none);
    glPopMatrix();

    glPopMatrix();
}

void drawPlainBuilding(float x, float z, float w, float h, float d, float r, float g, float b) {
    glPushMatrix();
    glTranslatef(x, h / 2.0f, z);

    
    glPushMatrix();
    glScalef(w, h, d);
    glColor3f(r, g, b);
    glutSolidCube(1.0);
    glPopMatrix();

    
    glPushMatrix();
    glTranslatef(0, h * 0.5f + 0.08f, 0);
    glScalef(w * 1.04f, 0.16f, d * 1.04f);
    glColor3f(r * 0.55f, g * 0.55f, b * 0.55f);
    glutSolidCube(1.0);
    glPopMatrix();

    
    glPushMatrix();
    glTranslatef(0, -h * 0.5f + 0.18f, 0);
    glScalef(w * 1.02f, 0.36f, d * 1.02f);
    glColor3f(r * 0.35f, g * 0.35f, b * 0.35f);
    glutSolidCube(1.0);
    glPopMatrix();

    
    int colsZ = (int)(w / 1.8f); if (colsZ < 2) colsZ = 2; if (colsZ > 6) colsZ = 6;
    int colsX = (int)(d / 1.8f); if (colsX < 2) colsX = 2; if (colsX > 6) colsX = 6;
    int rows  = (int)(h / 1.6f); if (rows  < 1) rows  = 1; if (rows  > 3) rows  = 3;

    float winH   = (h * 0.55f) / rows;
    float startY = -h * 0.30f;

    float glowR, glowG, glowB;
    if (isNight) { glowR = 1.00f; glowG = 0.85f; glowB = 0.45f; } 
    else         { glowR = 0.55f; glowG = 0.70f; glowB = 0.82f; } 

    if (isNight) {
        GLfloat emis[] = { glowR * 0.55f, glowG * 0.55f, glowB * 0.55f, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emis);
    }
    glColor3f(glowR, glowG, glowB);

    
    for (int row = 0; row < rows; row++) {
        float wy = startY + row * (h * 0.55f / rows);
        for (int col = 0; col < colsZ; col++) {
            float wx = -w * 0.35f + (w * 0.7f / colsZ) * (col + 0.5f);
            bool lit = ((row + col) % 3 != 0);
            if (isNight && !lit) continue;
            glPushMatrix(); glTranslatef(wx, wy,  d * 0.501f);
            glScalef((w * 0.7f / colsZ) * 0.65f, winH * 0.6f, 0.03f); glutSolidCube(1.0); glPopMatrix();
            glPushMatrix(); glTranslatef(wx, wy, -d * 0.501f);
            glScalef((w * 0.7f / colsZ) * 0.65f, winH * 0.6f, 0.03f); glutSolidCube(1.0); glPopMatrix();
        }
    }
    
    for (int row = 0; row < rows; row++) {
        float wy = startY + row * (h * 0.55f / rows);
        for (int col = 0; col < colsX; col++) {
            float wzp = -d * 0.35f + (d * 0.7f / colsX) * (col + 0.5f);
            bool lit = ((row + col + 1) % 3 != 0);
            if (isNight && !lit) continue;
            glPushMatrix(); glTranslatef( w * 0.501f, wy, wzp);
            glScalef(0.03f, winH * 0.6f, (d * 0.7f / colsX) * 0.65f); glutSolidCube(1.0); glPopMatrix();
            glPushMatrix(); glTranslatef(-w * 0.501f, wy, wzp);
            glScalef(0.03f, winH * 0.6f, (d * 0.7f / colsX) * 0.65f); glutSolidCube(1.0); glPopMatrix();
        }
    }

    if (isNight) {
        GLfloat noEmis[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, noEmis);
    }

    
    glPushMatrix();
    glTranslatef(0, -h * 0.5f + 0.9f, d * 0.502f);
    glColor3f(0.25f, 0.16f, 0.10f);
    glScalef(1.1f, 1.8f, 0.05f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPopMatrix();
}


void drawLabel3D(float x, float y, float z, void* font, const char* text) {
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos3f(x, y, z);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
    glEnable(GL_LIGHTING);
}

void drawFireStation() {
    drawPlainBuilding(truckStartX, truckStartZ - 8.0f, 10, 6, 9, 0.82f, 0.83f, 0.86f);

    glPushMatrix();
    glTranslatef(truckStartX, 1.6f, truckStartZ - 8.0f + 4.51f);
    glScalef(6.0f, 3.2f, 0.1f);
    glColor3f(0.15f, 0.15f, 0.15f);
    glutSolidCube(1.0);
    glPopMatrix();

    drawFireStationInterior(truckStartX, truckStartZ - 8.0f);
}


void drawFireStationInterior(float fx, float fz) {
    
    glPushMatrix();
    glTranslatef(fx - 3.5f, 1.6f, fz - 3.9f);
    glColor3f(0.35f, 0.22f, 0.12f);
    glScalef(2.6f, 0.1f, 0.5f);
    glutSolidCube(1.0);
    glPopMatrix();
    float helmetX[3] = { fx - 4.3f, fx - 3.5f, fx - 2.7f };
    for (int i = 0; i < 3; i++) {
        glPushMatrix();
        glTranslatef(helmetX[i], 1.85f, fz - 3.9f);
        glColor3f(0.85f, 0.15f, 0.10f);
        glutSolidSphere(0.22, 10, 8);
        glPopMatrix();
    }

    
    glPushMatrix();
    glTranslatef(fx + 3.9f, 1.8f, fz - 3.85f);
    glColor3f(0.75f, 0.05f, 0.05f);
    glutSolidTorus(0.08, 0.5, 8, 16);
    glPopMatrix();

    
    for (int i = 0; i < 3; i++) {
        glPushMatrix();
        glTranslatef(fx - 4.4f + i * 1.5f, 0.9f, fz + 3.6f);
        glColor3f(0.20f, 0.30f, 0.50f);
        glScalef(1.0f, 1.8f, 0.5f);
        glutSolidCube(1.0);
        glPopMatrix();
    }

    
    drawStandingPerson(fx - 2.0f, fz + 2.6f, 180.0f, 0.8f, 0.70f, 0.15f, 0.10f);
    drawStandingPerson(fx + 2.0f, fz + 1.0f, 180.0f, 2.3f, 0.20f, 0.25f, 0.35f);
}

void drawHospital() {
    
    float hx = truckStartX, hz = 16.0f;
    drawPlainBuilding(hx, hz, 9, 5, 8, 0.95f, 0.95f, 0.95f);

    glPushMatrix();
    glTranslatef(hx, 3.0f, hz + 4.05f);
    glColor3f(0.8f, 0.05f, 0.05f);
    glScalef(1.6f, 0.4f, 0.05f);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(hx, 3.0f, hz + 4.05f);
    glColor3f(0.8f, 0.05f, 0.05f);
    glScalef(0.4f, 1.6f, 0.05f);
    glutSolidCube(1.0);
    glPopMatrix();

    
    glPushMatrix();
    glTranslatef(hx, 1.7f, hz + 4.05f);
    glColor3f(0.05f, 0.20f, 0.55f);
    glScalef(3.0f, 0.6f, 0.05f);
    glutSolidCube(1.0);
    glPopMatrix();
    drawLabel3D(hx - 2.1f, 1.6f, hz + 4.08f, GLUT_BITMAP_HELVETICA_18, "MEDICAL");
    drawHospitalInterior(hx, hz);
}


void drawHospitalInterior(float hx, float hz) {
    
    glPushMatrix();
    glTranslatef(hx, 0.45f, hz + 2.6f);
    glColor3f(0.85f, 0.85f, 0.90f);
    glScalef(2.2f, 0.9f, 0.6f);
    glutSolidCube(1.0);
    glPopMatrix();

    
    float bedX[2] = { hx - 2.6f, hx + 2.6f };
    for (int i = 0; i < 2; i++) {
        glPushMatrix();
        glTranslatef(bedX[i], 0.35f, hz - 1.5f);
        glColor3f(0.95f, 0.95f, 0.97f);
        glScalef(1.0f, 0.5f, 2.2f);
        glutSolidCube(1.0);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(bedX[i], 0.62f, hz - 2.4f);
        glColor3f(0.75f, 0.85f, 0.95f);
        glScalef(0.8f, 0.2f, 0.5f);
        glutSolidCube(1.0);
        glPopMatrix();
    }

    
    glPushMatrix();
    glTranslatef(hx, 1.6f, hz - 3.9f);
    glColor3f(0.90f, 0.90f, 0.92f);
    glScalef(1.4f, 1.6f, 0.1f);
    glutSolidCube(1.0);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(hx, 1.6f, hz - 3.83f);
    glColor3f(0.8f, 0.05f, 0.05f);
    glScalef(0.5f, 0.12f, 0.02f);
    glutSolidCube(1.0);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(hx, 1.6f, hz - 3.83f);
    glColor3f(0.8f, 0.05f, 0.05f);
    glScalef(0.12f, 0.5f, 0.02f);
    glutSolidCube(1.0);
    glPopMatrix();

    
    drawStandingPerson(hx, hz + 1.6f, 180.0f, 0.5f, 0.85f, 0.90f, 0.95f);
    drawStandingPerson(hx - 2.6f, hz - 0.4f, 0.0f, 2.0f, 0.55f, 0.45f, 0.35f);
}

void drawFireHydrant(float x, float z) {
    drawCylinderAt(x, 0, z, 0.25f, 1.0f, -90, 0, 0, 0.75f, 0.05f, 0.05f, false);
    glPushMatrix();
    glTranslatef(x, 1.05f, z);
    glColor3f(0.75f, 0.05f, 0.05f);
    glutSolidSphere(0.32, 10, 10);
    glPopMatrix();
}

void drawBarrier(float x, float z) {
    for (int i = 0; i < 4; i++) {
        glPushMatrix();
        glTranslatef(x + i * 1.2f, 0.5f, z);
        if (i % 2 == 0) glColor3f(0.9f, 0.15f, 0.15f);
        else            glColor3f(0.9f, 0.85f, 0.15f);
        glScalef(1.0f, 1.0f, 0.3f);
        glutSolidCube(1.0);
        glPopMatrix();
    }
}




void drawTruck() {
    GLfloat none[] = { 0.0f, 0.0f, 0.0f, 1.0f };

    drawGroundShadow(truckX, truckZ, 2.6f, 1.4f);

    glPushMatrix();
    glTranslatef(truckX, truckY, truckZ);
    glRotatef(truckAngle, 0, 1, 0);

    
    GLfloat paintSpec[] = { 0.6f, 0.6f, 0.6f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, paintSpec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 60.0f);

    glPushMatrix();
    glTranslatef(0, 1.0f, 0);
    glScalef(4.4f, 1.6f, 2.0f);
    glColor3f(0.85f, 0.08f, 0.08f);
    glutSolidCube(1.0);
    glPopMatrix();

    
    glPushMatrix();
    glTranslatef(1.9f, 1.55f, 0);
    glScalef(1.6f, 1.5f, 1.9f);
    glColor3f(0.75f, 0.05f, 0.05f);
    glutSolidCube(1.0);
    glPopMatrix();

    
    GLfloat glassSpec[] = { 0.9f, 0.9f, 0.9f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, glassSpec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 100.0f);

    glPushMatrix();
    glTranslatef(2.65f, 1.7f, 0);
    glScalef(0.1f, 0.7f, 1.5f);
    glColor3f(0.55f, 0.75f, 0.85f);
    glutSolidCube(1.0);
    glPopMatrix();

    
    GLfloat matteSpecLocal[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matteSpecLocal);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);

    
    float wheelX[2] = { -1.5f, 1.6f };
    float wheelZ[2] = { -1.05f, 1.05f };
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            glPushMatrix();
            glTranslatef(wheelX[i], 0.5f, wheelZ[j]);
            glRotatef(wheelAngle, 0, 0, 1);
            glColor3f(0.08f, 0.08f, 0.08f);
            glutSolidTorus(0.18, 0.42, 8, 14);
            glPopMatrix();
        }
    }

    
    glPushMatrix();
    glTranslatef(0.5f, 1.95f, -0.5f);
    if (sirenRedBright) {
        GLfloat emis[] = { 1.0f, 0.1f, 0.1f, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emis);
        glColor3f(1.0f, 0.1f, 0.1f);
    } else {
        glColor3f(0.35f, 0.05f, 0.05f);
    }
    glutSolidSphere(0.22, 10, 10);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, none);
    glPopMatrix();

    
    glPushMatrix();
    glTranslatef(0.5f, 1.95f, 0.5f);
    if (!sirenRedBright) {
        GLfloat emis[] = { 0.1f, 0.3f, 1.0f, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emis);
        glColor3f(0.1f, 0.3f, 1.0f);
    } else {
        glColor3f(0.05f, 0.1f, 0.35f);
    }
    glutSolidSphere(0.22, 10, 10);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, none);
    glPopMatrix();

    glPopMatrix(); 

    
    float rad = truckAngle * (float)M_PI / 180.0f;
    drawCylinderAt(truckX + cosf(rad) * 2.2f - sinf(rad) * 0.0f,
                   truckY + 1.9f,
                   truckZ - sinf(rad) * 2.2f,
                   0.10f, 1.6f,
                   0, 90 - truckAngle, 0,
                   0.6f, 0.6f, 0.65f, false);
}




void drawFireAnimation() {
    if (!buildingBurning || fireLevel <= 0.0f) return;

    float scale = fireLevel / 100.0f;
    float flicker = 0.15f * sinf(simulationTime * 10.0f);

    glPushMatrix();
    glTranslatef(buildingX, 7.0f, buildingZ);
    glScalef(scale, scale, scale);

    glPushMatrix();
    glTranslatef(0, 0.2f, 0);
    glRotatef(-90, 1, 0, 0);
    glColor3f(1.0f, 0.85f, 0.2f);
    glutSolidCone(0.8 + flicker, 2.0, 10, 6);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.3f, -0.1f, 0.2f);
    glRotatef(-90, 1, 0, 0);
    glColor3f(1.0f, 0.5f, 0.05f);
    glutSolidCone(1.0 - flicker, 2.6, 10, 6);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.25f, -0.3f, -0.15f);
    glRotatef(-90, 1, 0, 0);
    glColor3f(0.85f, 0.15f, 0.05f);
    glutSolidCone(1.2 + flicker, 3.0, 10, 6);
    glPopMatrix();

    glPopMatrix();
}

void drawSmoke() {
    if (smokeLevel <= 0.0f) return;

    int count = (int)(6.0f * (smokeLevel / 100.0f)) + 1;
    for (int i = 0; i < count; i++) {
        float phase  = fmodf(simulationTime * 1.4f + i * 0.9f, 5.0f);
        float y      = 8.0f + phase * 1.8f;
        float wobble = sinf(simulationTime * 2.0f + i) * 0.4f;
        float size   = 0.5f + phase * 0.12f;

        glPushMatrix();
        glTranslatef(buildingX + wobble, y, buildingZ + wobble * 0.5f);
        glColor3f(0.45f, 0.45f, 0.47f);
        glutSolidSphere(size, 10, 10);
        glPopMatrix();
    }
}

void drawWaterSpray() {
    bool spraying = waterSprayOn || manualWaterOn;
    if (!spraying || !buildingBurning) return;

    float rad = truckAngle * (float)M_PI / 180.0f;
    float nozzleX = truckX + cosf(rad) * 3.4f;
    float nozzleZ = truckZ - sinf(rad) * 3.4f;
    float nozzleY = truckY + 1.9f;

    float targetX = buildingX;
    float targetY = 5.5f;
    float targetZ = buildingZ;

    int drops = 8;
    for (int i = 0; i < drops; i++) {
        float t = fmodf((float)i / drops + simulationTime * 1.5f, 1.0f);
        float x = nozzleX + (targetX - nozzleX) * t;
        float y = nozzleY + (targetY - nozzleY) * t + sinf(t * (float)M_PI) * 1.5f;
        float z = nozzleZ + (targetZ - nozzleZ) * t;

        glPushMatrix();
        glTranslatef(x, y, z);
        glColor3f(0.25f, 0.55f, 0.95f);
        glutSolidSphere(0.15, 8, 8);
        glPopMatrix();
    }
}




void drawBitmapString(float x, float y, void* font, const char* text) {
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

const char* stateName(SimState s) {
    switch (s) {
        case STATE_NORMAL:             return "NORMAL";
        case STATE_FIRE_DETECTED:      return "FIRE DETECTED";
        case STATE_TRUCK_DISPATCHED:   return "FIRE TRUCK DISPATCHED";
        case STATE_RESPONDING:         return "RESPONDING TO FIRE";
        case STATE_WATER_SPRAY_ACTIVE: return "WATER SPRAY ACTIVE";
        case STATE_MISSION_COMPLETED:  return "MISSION COMPLETED";
        case STATE_RETURNING:          return "RETURNING TO STATION";
    }
    return "";
}

void drawStatusText() {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    char line[160];

    glColor3f(1.0f, 1.0f, 0.3f);
    sprintf(line, "STATUS: %s", stateName(simState));
    drawBitmapString(20, windowHeight - 30, GLUT_BITMAP_HELVETICA_18, line);

    glColor3f(1.0f, 1.0f, 1.0f);
    sprintf(line, "Fire: %.0f%%   Smoke: %.0f%%   Response Time: %.1fs",
            fireLevel, smokeLevel, responseTimer);
    drawBitmapString(20, windowHeight - 55, GLUT_BITMAP_HELVETICA_18, line);

    glColor3f(0.85f, 0.85f, 0.85f);
    drawBitmapString(20, 75, GLUT_BITMAP_HELVETICA_12,
        "Controls: W/S Move  A/D Turn  SPACE Manual Water  E Start Emergency");
    drawBitmapString(20, 55, GLUT_BITMAP_HELVETICA_12,
        "Camera: 1-5 views | I School  H Hospital  F FireStation (Arrows look) | N Day/Night | R Reset");

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}




void setCameraView() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    switch (cameraMode) {
        case 1: 
            gluLookAt(0, 20, 55,   0, 5, 0,   0, 1, 0);
            break;
        case 2: 
            gluLookAt(0, 85, 0.01f,   0, 0, 0,   0, 0, -1);
            break;
        case 3: 
            gluLookAt(65, 20, 0,   0, 5, 0,   0, 1, 0);
            break;
        case 4: { 
            float rad = truckAngle * (float)M_PI / 180.0f;
            float eyeX = truckX - cosf(rad) * 14.0f;
            float eyeZ = truckZ + sinf(rad) * 14.0f;
            gluLookAt(eyeX, truckY + 9.0f, eyeZ,
                      truckX, truckY + 1.0f, truckZ,
                      0, 1, 0);
            break;
        }
        case 5: { 
            float yawRad   = freeOrbitYaw   * (float)M_PI / 180.0f;
            float pitchRad = freeOrbitPitch * (float)M_PI / 180.0f;
            float eyeX = freeOrbitTargetX + freeOrbitRadius * cosf(pitchRad) * cosf(yawRad);
            float eyeY = freeOrbitTargetY + freeOrbitRadius * sinf(pitchRad);
            float eyeZ = freeOrbitTargetZ + freeOrbitRadius * cosf(pitchRad) * sinf(yawRad);
            gluLookAt(eyeX, eyeY, eyeZ,
                      freeOrbitTargetX, freeOrbitTargetY, freeOrbitTargetZ,
                      0, 1, 0);
            break;
        }
        case 6: { 
            float yawRad   = freeOrbitYaw   * (float)M_PI / 180.0f;
            float pitchRad = freeOrbitPitch * (float)M_PI / 180.0f;
            float eyeX = -35.0f, eyeY = 2.0f, eyeZ = 25.0f; 
            float lookX = eyeX + cosf(pitchRad) * cosf(yawRad);
            float lookY = eyeY + sinf(pitchRad);
            float lookZ = eyeZ + cosf(pitchRad) * sinf(yawRad);
            gluLookAt(eyeX, eyeY, eyeZ, lookX, lookY, lookZ, 0, 1, 0);
            break;
        }
        case 7: { 
            float yawRad   = freeOrbitYaw   * (float)M_PI / 180.0f;
            float pitchRad = freeOrbitPitch * (float)M_PI / 180.0f;
            float eyeX = -35.0f, eyeY = 2.0f, eyeZ = 16.0f; 
            float lookX = eyeX + cosf(pitchRad) * cosf(yawRad);
            float lookY = eyeY + sinf(pitchRad);
            float lookZ = eyeZ + cosf(pitchRad) * sinf(yawRad);
            gluLookAt(eyeX, eyeY, eyeZ, lookX, lookY, lookZ, 0, 1, 0);
            break;
        }
        case 8: { 
            float yawRad   = freeOrbitYaw   * (float)M_PI / 180.0f;
            float pitchRad = freeOrbitPitch * (float)M_PI / 180.0f;
            float eyeX = truckStartX, eyeY = 2.0f, eyeZ = truckStartZ - 8.0f; 
            float lookX = eyeX + cosf(pitchRad) * cosf(yawRad);
            float lookY = eyeY + sinf(pitchRad);
            float lookZ = eyeZ + cosf(pitchRad) * sinf(yawRad);
            gluLookAt(eyeX, eyeY, eyeZ, lookX, lookY, lookZ, 0, 1, 0);
            break;
        }
        default:
            gluLookAt(0, 20, 55,   0, 5, 0,   0, 1, 0);
    }
}







void drawGradientSky() {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE); 

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    float topR, topG, topB, botR, botG, botB;
    if (isNight) {
        topR = 0.01f; topG = 0.01f; topB = 0.06f;
        botR = 0.08f; botG = 0.08f; botB = 0.22f;
    } else {
        topR = 0.25f; topG = 0.55f; topB = 0.85f;
        botR = 0.75f; botG = 0.88f; botB = 0.97f;
    }

    glBegin(GL_QUADS);
        glColor3f(topR, topG, topB); glVertex2f(0, (float)windowHeight);
        glColor3f(topR, topG, topB); glVertex2f((float)windowWidth, (float)windowHeight);
        glColor3f(botR, botG, botB); glVertex2f((float)windowWidth, 0);
        glColor3f(botR, botG, botB); glVertex2f(0, 0);
    glEnd();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}



void drawGroundShadow(float x, float z, float radiusX, float radiusZ) {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.28f);

    glPushMatrix();
    glTranslatef(x, 0.03f, z);
    glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0, 0, 0);
        for (int i = 0; i <= 20; i++) {
            float a = (float)i / 20.0f * 2.0f * 3.14159265f;
            glVertex3f(cosf(a) * radiusX, 0, sinf(a) * radiusZ);
        }
    glEnd();
    glPopMatrix();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void display() {
    if (isNight) glClearColor(0.03f, 0.03f, 0.08f, 1.0f);
    else         glClearColor(0.55f, 0.75f, 0.92f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawGradientSky();

    applyFogForTime();

    if (isNight) {
        GLfloat amb[] = { 0.08f, 0.08f, 0.12f, 1.0f };
        glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
    } else {
        GLfloat amb[] = { 0.35f, 0.35f, 0.35f, 1.0f };
        glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
    }

    setCameraView();

    GLfloat lightPos[] = { 40.0f, 60.0f, 20.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    drawGround();
    drawRoads();

    drawFireStation();
    drawHospital();
    drawSchool();

    drawPlainBuilding(buildingX, buildingZ, 8, 7, 8, 0.62f, 0.42f, 0.32f); 
    drawPlainBuilding(35, -15, 7, 6, 7, 0.55f, 0.55f, 0.60f);
    drawPlainBuilding(-15, 30, 6, 5, 6, 0.60f, 0.50f, 0.35f);
    drawPlainBuilding(15, -30, 6, 8, 6, 0.45f, 0.55f, 0.60f);

    drawFireHydrant(buildingX - 4.0f, buildingZ - 4.0f);
    drawBarrier(truckStartX - 6.0f, truckStartZ - 4.0f);

    drawTree(-25, 5);  drawTree(-20, -8); drawTree(30, 25);
    drawTree(38, -25); drawTree(-30, -30); drawTree(20, 35);

    drawStreetLight(-10, -8); drawStreetLight(10, -8);
    drawStreetLight(-10, 8);  drawStreetLight(10, 8);

    bool emergencyGreen = (simState == STATE_TRUCK_DISPATCHED || simState == STATE_RESPONDING);
    drawTrafficLight(-4, -4, emergencyGreen);
    drawTrafficLight(4, 4, emergencyGreen);

    drawTrafficCars();
    drawPedestrians();
    drawCommuters();
    drawBirds();
    drawPlane();

    drawTruck();
    drawFireAnimation();
    drawSmoke();
    drawWaterSpray();

    drawStatusText();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    windowWidth = w;
    windowHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50.0, (double)w / (double)h, 1.0, 300.0);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'e': case 'E': startEmergency(); break;
        case 'r': case 'R': resetSimulation(); break;
        case ' ': manualWaterOn = !manualWaterOn; break;
        case 'n': case 'N': isNight = !isNight; break;

        case '1': cameraMode = 1; break;
        case '2': cameraMode = 2; break;
        case '3': cameraMode = 3; break;
        case '4': cameraMode = 4; break;
        case '5': cameraMode = 5; break;
        case 'i': case 'I':
            cameraMode = 6;
            freeOrbitYaw = 0.0f;
            freeOrbitPitch = 0.0f;
            break;
        case 'h': case 'H':
            cameraMode = 7;
            freeOrbitYaw = 0.0f;
            freeOrbitPitch = 0.0f;
            break;
        case 'f': case 'F':
            cameraMode = 8;
            freeOrbitYaw = 0.0f;
            freeOrbitPitch = 0.0f;
            break;

        case 'w': case 'W':
            if (!truckAutoMoving) {
                float rad = truckAngle * (float)M_PI / 180.0f;
                truckX += cosf(rad) * 1.2f;
                truckZ += -sinf(rad) * 1.2f;
            }
            break;
        case 's': case 'S':
            if (!truckAutoMoving) {
                float rad = truckAngle * (float)M_PI / 180.0f;
                truckX -= cosf(rad) * 1.2f;
                truckZ -= -sinf(rad) * 1.2f;
            }
            break;
        case 'a': case 'A':
            if (!truckAutoMoving) truckAngle += 6.0f;
            break;
        case 'd': case 'D':
            if (!truckAutoMoving) truckAngle -= 6.0f;
            break;
    }
    glutPostRedisplay();
}


void specialKeyboard(int key, int x, int y) {
    if (cameraMode == 5 || cameraMode == 6 || cameraMode == 7 || cameraMode == 8) {
        switch (key) {
            case GLUT_KEY_LEFT:  freeOrbitYaw -= 3.0f; break;
            case GLUT_KEY_RIGHT: freeOrbitYaw += 3.0f; break;
            case GLUT_KEY_UP:
                freeOrbitPitch += 2.0f;
                if (freeOrbitPitch > 89.0f) freeOrbitPitch = 89.0f;
                break;
            case GLUT_KEY_DOWN:
                freeOrbitPitch -= 2.0f;
                if (freeOrbitPitch < -89.0f) freeOrbitPitch = -89.0f;
                break;
            case GLUT_KEY_PAGE_UP:
                freeOrbitRadius -= 3.0f;
                if (freeOrbitRadius < 10.0f) freeOrbitRadius = 10.0f;
                break;
            case GLUT_KEY_PAGE_DOWN:
                freeOrbitRadius += 3.0f;
                if (freeOrbitRadius > 150.0f) freeOrbitRadius = 150.0f;
                break;
        }
        glutPostRedisplay();
    }
}




void update(int value) {
    int now = glutGet(GLUT_ELAPSED_TIME);
    float dt = (now - lastElapsedMs) / 1000.0f;
    lastElapsedMs = now;
    if (dt > 0.1f) dt = 0.1f; 
    simulationTime += dt;
    updateCityLife(dt);

    
    if (truckAutoMoving) wheelAngle += dt * 720.0f;

    
    sirenTimer += dt;
    if (sirenTimer > 0.3f) { sirenTimer = 0.0f; sirenRedBright = !sirenRedBright; }

    
    if (responseTimerRunning) responseTimer += dt;

    
    if (dispatchPending) {
        dispatchTimer += dt;
        if (dispatchTimer >= 2.0f) {
            dispatchPending = false;
            simState = STATE_TRUCK_DISPATCHED;
            truckAutoMoving = true;
            currentWaypoint = 0;
        }
    }

    
    if (truckAutoMoving) {
        if (simState == STATE_TRUCK_DISPATCHED && currentWaypoint >= 1) {
            simState = STATE_RESPONDING;
        }

        bool  goingHome = (simState == STATE_RETURNING);
        Vec3 target = goingHome ? returnWaypoints[currentWaypoint] : waypoints[currentWaypoint];
        float dx = target.x - truckX;
        float dz = target.z - truckZ;
        float dist = sqrtf(dx * dx + dz * dz);

        if (dist < 0.3f) {
            currentWaypoint++;
            if (currentWaypoint >= 3) {
                truckAutoMoving = false;
                if (goingHome) {
                    
                    truckX = truckStartX;
                    truckZ = truckStartZ;
                    truckAngle = 0.0f;
                    simState = STATE_NORMAL;
                } else {
                    simState = STATE_WATER_SPRAY_ACTIVE;
                    waterSprayOn = true;
                }
            }
        } else {
            float rad = atan2f(-dz, dx);
            truckAngle = rad * 180.0f / (float)M_PI;
            truckX += cosf(rad) * truckSpeed * dt;
            truckZ += -sinf(rad) * truckSpeed * dt;
        }
    }

    
    if (simState == STATE_WATER_SPRAY_ACTIVE) {
        fireLevel  -= dt * (100.0f / 6.0f);
        smokeLevel -= dt * (100.0f / 8.0f);
        if (fireLevel  <= 0.0f) { fireLevel  = 0.0f; buildingBurning = false; }
        if (smokeLevel <= 0.0f)   smokeLevel = 0.0f;

        if (fireLevel <= 0.0f && smokeLevel <= 0.0f) {
            waterSprayOn = false;
            simState = STATE_MISSION_COMPLETED;
            responseTimerRunning = false;
            returnPending = true;
            missionCompleteTimer = 0.0f;
        }
    }

    
    if (returnPending) {
        missionCompleteTimer += dt;
        if (missionCompleteTimer >= 2.5f) {
            returnPending = false;
            simState = STATE_RETURNING;
            truckAutoMoving = true;
            currentWaypoint = 0;
        }
    }

    
    if (manualWaterOn && buildingBurning && simState != STATE_WATER_SPRAY_ACTIVE) {
        float d = distance2D(truckX, truckZ, buildingX, buildingZ);
        if (d < 12.0f) {
            fireLevel  -= dt * (100.0f / 10.0f);
            smokeLevel -= dt * (100.0f / 12.0f);
            if (fireLevel <= 0.0f) {
                fireLevel = 0.0f;
                smokeLevel = 0.0f;
                buildingBurning = false;
                simState = STATE_MISSION_COMPLETED;
                responseTimerRunning = false;
                returnPending = true;
                missionCompleteTimer = 0.0f;
            }
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}
