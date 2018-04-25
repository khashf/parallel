#include <iostream>
#include <fstream>
#include <omp.h>
#include <stdio.h>
#include <cstdlib>
#include <cmath>

using std::cout;
using std::cerr;
using std::endl;

#define XMIN 0.
#define XMAX 3.
#define YMIN 0.
#define YMAX 3.

#define TOPZ00 0.
#define TOPZ10 1.
#define TOPZ20 0.
#define TOPZ30 0.

#define TOPZ01 1.
#define TOPZ11 6.
#define TOPZ21 1.
#define TOPZ31 0.

#define TOPZ02 0.
#define TOPZ12 1.
#define TOPZ22 0.
#define TOPZ32 4.

#define TOPZ03 3.
#define TOPZ13 2.
#define TOPZ23 3.
#define TOPZ33 3.

#define BOTZ00 0.
#define BOTZ10 -3.
#define BOTZ20 0.
#define BOTZ30 0.

#define BOTZ01 -2.
#define BOTZ11 10.
#define BOTZ21 -2.
#define BOTZ31 0.

#define BOTZ02 0.
#define BOTZ12 -5.
#define BOTZ22 0.
#define BOTZ32 -6.

#define BOTZ03 -3.
#define BOTZ13 2.
#define BOTZ23 -8.
#define BOTZ33 -3.

int g_num_threads;
int g_num_nodes;
int g_num_subdivisions;

float ComputeHeight(int iu, int iv) {
    //  Intention: 
    //      Calculate the height between top tile and bottom tile given coordinate iu and iv
    //  Arguments: 
    //      iu,iv = 0 .. g_num_nodes-1

    float u = (float)iu / (float)(g_num_nodes - 1);
    float v = (float)iv / (float)(g_num_nodes - 1);

    // the basis functions
    float bu0 = (1. - u) * (1. - u) * (1. - u);
    float bu1 = 3. * u * (1. - u) * (1. - u);
    float bu2 = 3. * u * u * (1. - u);
    float bu3 = u * u * u;
    float bv0 = (1. - v) * (1. - v) * (1. - v);
    float bv1 = 3. * v * (1. - v) * (1. - v);
    float bv2 = 3. * v * v * (1. - v);
    float bv3 = v * v * v;

    float top = bu0 * (bv0 * TOPZ00 + bv1 * TOPZ01 + bv2 * TOPZ02 + bv3 * TOPZ03) 
                + bu1 * (bv0 * TOPZ10 + bv1 * TOPZ11 + bv2 * TOPZ12 + bv3 * TOPZ13) 
                + bu2 * (bv0 * TOPZ20 + bv1 * TOPZ21 + bv2 * TOPZ22 + bv3 * TOPZ23) 
                + bu3 * (bv0 * TOPZ30 + bv1 * TOPZ31 + bv2 * TOPZ32 + bv3 * TOPZ33);
    float bot = bu0 * (bv0 * BOTZ00 + bv1 * BOTZ01 + bv2 * BOTZ02 + bv3 * BOTZ03) 
                + bu1 * (bv0 * BOTZ10 + bv1 * BOTZ11 + bv2 * BOTZ12 + bv3 * BOTZ13) 
                + bu2 * (bv0 * BOTZ20 + bv1 * BOTZ21 + bv2 * BOTZ22 + bv3 * BOTZ23) 
                + bu3 * (bv0 * BOTZ30 + bv1 * BOTZ31 + bv2 * BOTZ32 + bv3 * BOTZ33);

    return (top - bot); // if the bottom surface sticks out above the top surface
                    // then that contribution to the overall volume is negative
}

enum TileLocation {
    MIDDLE,
    EDGE,
    CORNER
};

TileLocation DetermineTileLocation(int iu, int iv) {
    if (iu == 0 || iu == g_num_nodes-1) {
        if (iv == 0 || iv == g_num_nodes-1)
            return CORNER;
        else
            return EDGE;
    } else {
        if (iv == 0 || iv == g_num_nodes-1)
            return EDGE;
        else
            return MIDDLE;
    } 
}

float DetermineVolumeContribution(TileLocation location) {
    float contribution;
    if (location == MIDDLE) {
        contribution = 1.0f;
    } else if (location == EDGE) {
        contribution = 0.5f;
    } else { // (location == CORNER) 
        contribution = 0.25f;
    }
    return contribution;
}

float ComputeActualArea(int iu, int iv) {
    //  Intention: 
    //      Calculate the weighted area at a given location (iu, iv). 
    //      - Tiles in the middle of the floor are full-sized tiles. 
    //      - Tiles along the edges are half-sized. 
    //      - Tiles in the corners are quarter-sized. 
    //      The volume contribution of each extruded height tile 
    //      needs to be weighted accordingly.
    //  Arguments: 
    //      iu,iv = 0 .. g_num_nodes-1

    float actualArea;
    TileLocation location;
    float volumeContribution;
    float fullTileArea;
    float u, v;
    
    u = (float)iu / (float)(g_num_nodes - 1);
    v = (float)iv / (float)(g_num_nodes - 1);
    location = DetermineTileLocation(iu, iv);
    volumeContribution = DetermineVolumeContribution(location);
    fullTileArea = (((XMAX - XMIN)/(float)(g_num_nodes-1))
                    * ((YMAX - YMIN)/(float)(g_num_nodes-1)));
    actualArea = volumeContribution * fullTileArea;
    return actualArea;
}


double ComputeVolume(int iu, int iv) {
    // sum up the weighted heights into the variable "volume"
	float volume;
    float height;
    double area;

    height = ComputeHeight(iu, iv);
    area = ComputeActualArea(iu, iv);
    volume = height * area;
    return volume;
}

const int NUM_ARGS = 3;
const int NUM_TRIES = 10;

int main(int argc, char *argv[]) {
#ifndef _OPENMP
    cerr << "OpenMP is not supported here -- sorry" << endl;
    return 1;
#endif
    if (argc != NUM_ARGS) {
        cerr << "Number of arguments must be " << NUM_ARGS << endl;
        cerr << "Usage" << endl;
        cerr << argv[0] << " <number of threads> <number of nodes (sqrt(subdivision))>" << endl;
        return 1;
    }

    //double max_mega_mults = 0.;
    double sum_volume = 0.0;
    double avg_volume;
    double sum_exec_time = 0.0;
    double avg_exec_time;
    
    g_num_threads = atoi(argv[1]);
    g_num_nodes = atoi(argv[1]);
    g_num_subdivisions = g_num_nodes * g_num_nodes;

    omp_set_num_threads(g_num_threads);
    cout << "Using " << g_num_threads << " threads" << endl;
    cout << "Dividing total area into " << g_num_nodes << " x " << g_num_nodes << " subdivisions" << endl;
    cout << "Trying " << NUM_TRIES << " times" << endl;

    for (int t = 0; t < NUM_TRIES; ++t) {
        cout << endl;
        cout << "Trial #" << t << endl;
        double time0 = omp_get_wtime(); // in seconds
        double volume = 0.0f;
        #pragma omp parallel for default(none), reduction(+:volume)
        for( int i = 0; i < g_num_subdivisions; i++ ) {
            int iu = i % g_num_nodes;
            int iv = i / g_num_nodes;
            volume += ComputeVolume(iu, iv);
        }
        double time1 = omp_get_wtime(); // in seconds
        double exec_time = (double)(time1 - time0) * pow(10.0, 9.0); // in nano seconds
        sum_volume += volume;
        sum_exec_time += exec_time;
        cout << "Execution time = " << sum_exec_time << endl;
        cout << "Volume = " << sum_volume << endl;
        cout << endl;
    }

    avg_exec_time = sum_exec_time / (double)NUM_TRIES;
    avg_volume = sum_volume / (double)NUM_TRIES;
    fprintf(stdout, "Average Execution Time = %10.2lf nano seconds\n", avg_exec_time);
    fprintf(stdout, "Average Volume = %10.2lf (volume unit)\n", avg_volume);
}