#include <stdio.h>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <omp.h>

using std::cerr;
using std::endl;
using std::cout;

const double G = 6.67300e-11; // m^3 / ( kg s^2 )
const double EARTH_MASS = 5.9742e24; // kg
const double EARTH_DIAMETER = 12756000.32; // meters
const double TIMESTEP = 1.0; // secs

#define NUM_BODIES 100
#define NUM_STEPS 200
const int NUM_ARGS = 2;
const int NUM_TRIES = 10;

int g_num_threads;

struct body {
    float mass;
    float x, y, z; // position
    float vx, vy, vz; // velocity
    float fx, fy, fz; // forces
    float xnew, ynew, znew;
    float vxnew, vynew, vznew;
};
typedef struct body Body;
Body Bodies[NUM_BODIES];

float GetDistanceSquared(Body*, Body*);
float GetUnitVector(Body*, Body*, float*, float*, float*);
float Ranf(float, float);
int Ranf(int, int);

void InitBodies() {
    for (int i = 0; i < NUM_BODIES; i++) {
        Bodies[i].mass = EARTH_MASS * Ranf(0.5f, 10.f);
        Bodies[i].x = EARTH_DIAMETER * Ranf(-100.f, 100.f);
        Bodies[i].y = EARTH_DIAMETER * Ranf(-100.f, 100.f);
        Bodies[i].z = EARTH_DIAMETER * Ranf(-100.f, 100.f);
        Bodies[i].vx = Ranf(-100.f, 100.f);
        Bodies[i].vy = Ranf(-100.f, 100.f);
        Bodies[i].vz = Ranf(-100.f, 100.f);
    };
}

int main(int argc, char *argv[]) {
    #ifndef _OPENMP
        fprintf(stderr, "OpenMP is not available\n");
        return 1;
    #endif
    if (argc != NUM_ARGS) {
        cerr << "Number of arguments must be " << NUM_ARGS-1 << endl;
        cerr << "Usage" << endl;
        cerr << argv[0] << " <number of threads>" << endl;
        return 1;
    }
    
    g_num_threads = atoi(argv[1]);

    omp_set_num_threads(g_num_threads);
    int num_processors = omp_get_num_procs();
    cout << "Using " << g_num_threads << " threads" << endl;
    cout << "Using " << num_processors << " processors" << endl;

    InitBodies();

    double time0 = omp_get_wtime();
    for (int t = 0; t < NUM_STEPS; t++) {
        #if PMODE == 0 // coarse-grained parallelism
            #if SMODE == 0 // static scheduling
                #pragma omp parallel for schedule(static) 
            #elif SMODE == 1 // dynamic scheduling
                #pragma omp parallel for schedule(dynamic)
            #else
                cerr << "Undefined value for SMODE directive" << endl;
                return 1;
            #endif
        #endif
        for (int i = 0; i < NUM_BODIES; i++) {
            float fx = 0.;
            float fy = 0.;
            float fz = 0.;
            Body *bi = &Bodies[i];
            #if PMODE == 1 // coarse-grained parallelism
                #if SMODE == 0 // static scheduling
                    #pragma omp parallel for schedule(static), reduction(+:fx, fy, fz)
                #elif SMODE == 1 // dynamic scheduling
                    #pragma omp parallel for schedule(dynamic), reduction(+:fx, fy, fz)
                #else
                    cerr << "Undefined value for SMODE directive" << endl;
                    return 1;
                #endif
            #endif
            for (int j = 0; j < NUM_BODIES; j++) {
                if (j == i)
                    continue;
                Body *bj = &Bodies[j];
                float rsqd = GetDistanceSquared(bi, bj);
                if (rsqd > 0.) {
                    float f = G * bi->mass * bj->mass / rsqd;
                    float ux, uy, uz;
                    GetUnitVector(bi, bj, &ux, &uy, &uz);
                    fx += f * ux;
                    fy += f * uy;
                    fz += f * uz;
                }
            }

            float ax = fx / Bodies[i].mass;
            float ay = fy / Bodies[i].mass;
            float az = fz / Bodies[i].mass;
            Bodies[i].xnew = Bodies[i].x + Bodies[i].vx * TIMESTEP + 0.5 * ax * TIMESTEP * TIMESTEP;
            Bodies[i].ynew = Bodies[i].y + Bodies[i].vy * TIMESTEP + 0.5 * ay * TIMESTEP * TIMESTEP;
            Bodies[i].znew = Bodies[i].z + Bodies[i].vz * TIMESTEP + 0.5 * az * TIMESTEP * TIMESTEP;
            Bodies[i].vxnew = Bodies[i].vx + ax * TIMESTEP;
            Bodies[i].vynew = Bodies[i].vy + ay * TIMESTEP;
            Bodies[i].vznew = Bodies[i].vz + az * TIMESTEP;
        } // NUM_BODIES

        // setup the state for the next animation step:
        for (int i = 0; i < NUM_BODIES; i++) {
            Bodies[i].x = Bodies[i].xnew;
            Bodies[i].y = Bodies[i].ynew;
            Bodies[i].z = Bodies[i].znew;
            Bodies[i].vx = Bodies[i].vxnew;
            Bodies[i].vy = Bodies[i].vynew;
            Bodies[i].vz = Bodies[i].vznew;
        }

    } // NUM_STEPS
    double time1 = omp_get_wtime();
    double exec_time = (double)(time1 - time0) * pow(10.0, 9.0); // in nano seconds
    float megaBodiesComparedPerSec = ((float)(NUM_BODIES * NUM_BODIES * NUM_STEPS) / (time1 - time0) / 1000000.);
    
    cout << "Execution Time = " << exec_time << endl;
    cout << "Mega Bodies Compared Per Seconds = " << megaBodiesComparedPerSec << endl;

    return 0;
}

float GetDistanceSquared(Body *bi, Body *bj) {
    float dx = bi->x - bj->x;
    float dy = bi->y - bj->y;
    float dz = bi->z - bj->z;
    return dx * dx + dy * dy + dz * dz;
}

float GetUnitVector(Body *from, Body *to, float *ux, float *uy, float *uz) {
    float dx = to->x - from->x;
    float dy = to->y - from->y;
    float dz = to->z - from->z;
    float d = sqrt(dx * dx + dy * dy + dz * dz);

    if (d > 0.) {
        dx /= d;
        dy /= d;
        dz /= d;
    }

    *ux = dx;
    *uy = dy;
    *uz = dz;
    return d;
}

float Ranf(float low, float high) {
    float r = (float)rand(); // 0 - RAND_MAX
    return (low + r * (high - low) / (float)RAND_MAX);
}

int Ranf(int ilow, int ihigh) {
    float low = (float)ilow;
    float high = (float)ihigh + 0.9999f;
    return (int)(Ranf(low, high));
}