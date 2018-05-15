#include <stdio.h>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <omp.h>
#include <time.h>
#include <random>

using std::cerr;
using std::endl;
using std::cout;

/* 
 * To keep this simple, a year consists of 12 months of 30 days each.
 * The first day of winter is considered to be January 1
 * Units of grain growth are inches 
 * Units of temperature are degrees Farhrenheit (°F) 
 * Units of precipitation are inches
 * Basic time step will be one month
 */

/* 
 * Constant
 */

// Agents
const float GRAIN_GROWS_PER_MONTH = 8.0f;
const float ONE_DEER_EATS_PER_MONTH = 0.1f;

// Conditions
const float AVG_PRECIP_PER_MONTH = 6.0f;
const float AMP_PRECIP_PER_MONTH = 6.0f;
const float RANDOM_PRECIP = 2.0f;
const float AVG_TEMP = 50.0f;
const float AMP_TEMP = 20.0f;
const float RANDOM_TEMP = 10.0f;
const float MIDTEMP = 40.0f;
const float MIDPRECIP = 10.0f;

// Program config
const int NUM_ARGS = 1;
const int NUM_AGENTS = 4;
const int NUM_THREADS = NUM_AGENTS;
const int NUM_STEPS = 72; // Each thread should return when the year hits 2020 
                          // (giving us 6 years, or 72 months, of simulation).


/* 
 * Global
 */

// Time
int NowYear; // starting in 2018
int NowMonth; // 1 - 12

// Factors
float NowPrecip; // inches of rain per month
float NowTemperature; // temperature this month

// Agents
float NowGrainHeight; // grain height of this month
int NowNumDeer; // current deer population
int NowNumWolf; // current wolf population
int LatestNumDeersEaten;
float LatestGrainStomped;

// From: https://stackoverflow.com/questions/29709897/c-thread-safe-uniform-distribution-random-number-generation
float Ranf(float min, float max) {
    thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_real_distribution<float> distribution(min, max);
    float chance = distribution(generator);
    //cout << "chance = " << chance << endl;
    return chance;
}

// float Ranf(float low, float high) {
//     cout << "low=" << low << endl;
//     cout << "high=" << high << endl;
//     unsigned int seed = time(NULL);
//     cout << "seed = " << seed << endl;
//     float r = (float) rand_r(&seed); // 0 - RAND_MAX
//     cout << "r = " << r << endl;
//     float chance = ((low + r * (high - low) / (float)RAND_MAX));
//     //cout << "chance = " << chance << endl;
//     return chance;
// }

void UpdateNowTemperature() {
    float ang = (30.0f * (float)NowMonth + 15.0f) * (M_PI / 180.0f);
    float temp = AVG_TEMP - AMP_TEMP * cos(ang);
    NowTemperature = temp + Ranf(-RANDOM_TEMP, RANDOM_TEMP);
}

void UpdateNowPrecip() {
    float ang = (30.0f * (float)NowMonth + 15.0f) * (M_PI / 180.0f);
    float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin(ang);
    NowPrecip = precip + Ranf(-RANDOM_PRECIP, RANDOM_PRECIP);
    if( NowPrecip < 0.0f)
        NowPrecip = 0.0f;
}

void UpdateFactors() {
    UpdateNowTemperature();
    UpdateNowPrecip();
}

void UpdateTime() {
    NowMonth += 1;
    if (NowMonth > 12) {
        NowMonth = 1;
        NowYear += 1;
    }
}

void InitData() {
    NowMonth = 1;
    NowYear = 2018;
    NowNumDeer = 1;
    NowNumWolf = 20;
    NowGrainHeight = 1.0f;
    LatestNumDeersEaten = 0;
    LatestGrainStomped = 0.0f;
}

float ComputeTemperatureFactor() {
    float tf = exp((-1) * pow((float)((NowTemperature - MIDTEMP)/10), 2.0f));
    return tf;
}

float ComputePrecipFactor() {
    float pf = exp((-1) * pow((float)((NowPrecip - MIDPRECIP)/10), 2.0f));
    return pf;
}

float GrainGrow() {
    float tempFactor = ComputeTemperatureFactor();
    float precipFactor = ComputePrecipFactor();
    float grownGrain = tempFactor * precipFactor * GRAIN_GROWS_PER_MONTH;
    return grownGrain;
}

float DeersEatGrain() {
    float eatenGrain = (float)NowNumDeer * ONE_DEER_EATS_PER_MONTH;
    return eatenGrain;
}

float ComputeGrain() {
    float grownGrain = GrainGrow();
    float eatenGrain = DeersEatGrain();
    float finalGrain = grownGrain - eatenGrain;
    return finalGrain;
}

void UpdateGrain(float tmpGrainHeight) {
    NowGrainHeight += tmpGrainHeight;
    NowGrainHeight -= LatestGrainStomped;
    if (NowGrainHeight < 0.0f)
        NowGrainHeight = 0.0f;
}

void Grain() {
    float tmpGrainHeight = ComputeGrain();
    #pragma omp barrier // computing barrier
    UpdateGrain(tmpGrainHeight);
    #pragma omp barrier // updating barrier
}

int ComputeDeers() {
    float requiredGrain = NowNumDeer * ONE_DEER_EATS_PER_MONTH;
    float diffGrain = NowGrainHeight - requiredGrain;
    float diffDeers = diffGrain / ONE_DEER_EATS_PER_MONTH;
    return diffDeers;
}

void UpdateDeers(int tmpDeers) {
    NowNumDeer += tmpDeers;
    NowNumDeer -= LatestNumDeersEaten;
    if (NowNumDeer < 0)
        NowNumDeer = 0;
}

void Deer() {
    float tmpDeers = ComputeDeers();
    #pragma omp barrier // computing barrier
    UpdateDeers(tmpDeers);
    #pragma omp barrier // updating barrier
}

int WolfBorn() {
    int numNewWolfs = 0;
    if (NowNumWolf <= 1)
        return 0;
    if (Ranf(1.0f, 100.0f) > 50.0f) // 10% chance a new wolf is born
        return 0;
    numNewWolfs *= 2;
    cout << "Born " << numNewWolfs << " wolfs" << endl;
    return numNewWolfs;
}

bool hasWolfAttack() {
    float chance = Ranf(1.0f, 100.0f);
    if (NowNumDeer >= NowNumWolf || chance <= 50.0f)
        return true;
    else
        return false;
}

int WolfsEatDeers() {
    int deersEaten = NowNumWolf/2 - static_cast<int>(NowGrainHeight*2.0f);
    if (deersEaten < 0)
        return 0;
    return deersEaten;
}

int WolfsStompGrain() {
    return LatestNumDeersEaten * 0.05f;
}

int WolfStarved() {
    int numWolfStarved;
    if (!hasWolfAttack()) {
        LatestNumDeersEaten = 0;
        LatestGrainStomped = 0.0f;
        numWolfStarved = NowNumWolf/4;
        cout << numWolfStarved << " wolfs died from starving..." << endl;
        return (-1)*numWolfStarved; // a quarter of population will die
    }
    cout << NowNumWolf << " wolfs attack " << NowNumDeer << " deers!" << endl;
    LatestNumDeersEaten = WolfsEatDeers();
    LatestGrainStomped = WolfsStompGrain();
    numWolfStarved = (NowNumWolf - LatestNumDeersEaten * 2);
    if (numWolfStarved < 0)
        return 0;
    cout << numWolfStarved << " wolfs died from starving..." << endl;
    return (-1)*numWolfStarved;
}

int ComputeWolf() {
    int tmpWolf = 0;
    tmpWolf += WolfBorn();
    tmpWolf += WolfStarved();
    return tmpWolf;
}

void UpdateWolf(int tmpWolfs) {
    NowNumWolf += tmpWolfs;
    if (NowNumWolf < 0)
        NowNumWolf = 0;    
}

void Wolf() {
    int tmpWolfs = ComputeWolf();
    #pragma omp barrier // computing barrier 1
    UpdateWolf(tmpWolfs);
    #pragma omp barrier // updating barrier 1
}

void PrintTime() {
    cout << "NowMonth: " << NowMonth << endl;
    cout << "NowYear: " << NowYear << endl;
}

void PrintFactors() {
    cout << "NowTemp: " << NowTemperature << endl;
    cout << "NowPrecip: " << NowPrecip << endl;
}

void PrintAgents() {
    cout << "NowGrain: " << NowGrainHeight << endl;
    cout << "NowDeers: " << NowNumDeer << endl;
    cout << "NowWolfs: " << NowNumWolf << endl;
}

void PrintState() {
    PrintTime();
    PrintFactors();
    PrintAgents();
}

void Watcher() {
    #pragma omp barrier // computing barrier
    #pragma omp barrier // updating barrier
    PrintState();
    UpdateTime();
    UpdateFactors();
}

int main() {
    omp_set_num_threads(NUM_THREADS);

    InitData();
    UpdateFactors(); // init the factor for the 1st time
    for (int iStep = 1; iStep <= NUM_STEPS; ++iStep) {
        cout << endl << "=============================" << endl;
        cout << "         Step " << iStep << endl << endl;
        #pragma omp parallel sections default(none) shared(NowGrainHeight, NowNumDeer, NowNumWolf, LatestGrainStomped, LatestNumDeersEaten)
        {
            #pragma omp section
            {
                Grain();            
            }
            #pragma omp section
            {
                Deer();
            }
            #pragma omp section
            {
                Wolf();
            }
            #pragma omp section
            {
                Watcher();
            }
        } // omp parallel sections
    } // for NUM_STEPS
    
    return 0;
}