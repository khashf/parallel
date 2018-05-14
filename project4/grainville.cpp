#include <stdio.h>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <omp.h>
#include <time.h>

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
const float GRAIN_GROWS_PER_MONTH = 8.0;
const float ONE_DEER_EATS_PER_MONTH = 0.5;

// Conditions
const float AVG_PRECIP_PER_MONTH = 6.0;
const float AMP_PRECIP_PER_MONTH = 6.0;
const float RANDOM_PRECIP = 2.0;
const float AVG_TEMP = 50.0;
const float AMP_TEMP = 20.0;
const float RANDOM_TEMP = 10.0;
const float MIDTEMP = 40.0;
const float MIDPRECIP = 10.0;

// Program config
const int NUM_ARGS = 1;
const int NUM_AGENTS = 3;
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



float Ranf(float low, float high, unsigned int* seed) {
    float r = (float) rand_r(seed); // 0 - RAND_MAX
    return ((low + r * (high - low) / (float)RAND_MAX));
}

void UpdateNowTemperature() {
    float ang = (30. * (float)NowMonth + 15.) * (M_PI / 180.);
    float temp = AVG_TEMP - AMP_TEMP * cos(ang);
    unsigned int seed = time(NULL);
    NowTemperature = temp + Ranf(-RANDOM_TEMP, RANDOM_TEMP, &seed);
}

void UpdateNowPrecip() {
    float ang = (30. * (float)NowMonth + 15.) * (M_PI / 180.);
    float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin(ang);
    unsigned int seed = time(NULL);
    NowPrecip = precip + Ranf(-RANDOM_PRECIP, RANDOM_PRECIP, &seed);
    if( NowPrecip < 0.)
        NowPrecip = 0.;
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
    NowNumDeer = 1;
    NowGrainHeight = 1.;
    NowMonth = 1;
    NowYear = 2018;
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
    if (NowGrainHeight < 0)
        NowGrainHeight = 0;
}

int ComputeDeers() {
    float requiredGrain = NowNumDeer * ONE_DEER_EATS_PER_MONTH;
    float diffGrain = NowGrainHeight - requiredGrain;
    float diffDeers = diffGrain / ONE_DEER_EATS_PER_MONTH;
    return diffDeers;
}

void UpdateDeers(int tmpDeers) {
    NowNumDeer += tmpDeers;
    if (NowNumDeer < 0)
        NowNumDeer = 0;
}

void PrintTime() {
    std::cout << "NowMonth: " << NowMonth << endl;
    std::cout << "NowYear: " << NowYear << endl;
}

void PrintFactors() {
    std::cout << "NowTemp: " << NowTemperature << endl;
    std::cout << "NowPrecip: " << NowPrecip << endl;
}

void PrintAgents() {
    std::cout << "NowGrain: " << NowGrainHeight << endl;
    std::cout << "NowDeers: " << NowNumDeer << endl;
}

void PrintState() {
    PrintTime();
    PrintFactors();
    PrintAgents();
}

int main() {
    omp_set_num_threads(NUM_THREADS);

    InitData();
    UpdateFactors(); // init the factor for the 1st time
    //for (int iStep = 1; iStep <= NUM_STEPS; ++iStep) {
    omp_lock_t* lockDeer;
    omp_lock_t* lockGrain;
    omp_lock_t* lockWatcherByGrain;
    omp_lock_t* lockWatcherByDeer;
    omp_init_lock(lockDeer);
    omp_init_lock(lockGrain);
    omp_init_lock(lockWatcherByGrain);
    omp_init_lock(lockWatcherByDeer);
    #pragma omp parallel sections default(none) shared(std::cout, lockWatcherByDeer, lockWatcherByGrain, NowGrainHeight, NowNumDeer, NowMonth, NowYear, NowPrecip, NowTemperature, lockDeer, lockGrain)
    {
        #pragma omp section 
        {
            std::cout << "Section [Grain] Thread [" << omp_get_thread_num() << "]: locking [Deer] from updating" << endl;
            omp_set_lock(lockDeer);
            std::cout << "Section [Grain] Thread [" << omp_get_thread_num() << "]: locking [Watcher] from starting" << endl;
            omp_set_lock(lockWatcherByGrain);

            std::cout << "Section [Grain] Thread [" << omp_get_thread_num() << "]: computing grain" << endl;
            float tmpGrainHeight = ComputeGrain();

            std::cout << "Section [Grain] Thread [" << omp_get_thread_num() << "]: unlocking [Deer] from updating" << endl;
            omp_unset_lock(lockDeer);
            std::cout << "Section [Grain] Thread [" << omp_get_thread_num() << "]: waiting for [Deer] to unlock for update" << endl;
            omp_set_lock(lockGrain);
            std::cout << "Section [Grain] Thread [" << omp_get_thread_num() << "]: has been unlocked by [Deer]" << endl;
            std::cout << "Section [Grain] Thread [" << omp_get_thread_num() << "]: updating grain" << endl;
            UpdateGrain(tmpGrainHeight);
            omp_unset_lock(lockGrain);

            std::cout << "Section [Grain] Thread [" << omp_get_thread_num() << "]: unlocking [Watcher] from starting" << endl;
            omp_unset_lock(lockWatcherByGrain);

        }
        #pragma omp section 
        {
            std::cout << "Section [Deers] Thread [" << omp_get_thread_num() << "]: locking [Grain] from updating" << endl;
            omp_set_lock(lockGrain);
            std::cout << "Section [Deers] Thread [" << omp_get_thread_num() << "]: locking [Watcher] from starting" << endl;
            omp_set_lock(lockWatcherByDeer);

            std::cout << "Section [Deers] Thread [" << omp_get_thread_num() << "]: computing deers" << endl;
            float tmpDeers = ComputeDeers();
            
            std::cout << "Section [Deers] Thread [" << omp_get_thread_num() << "]: unlocking [Grain] from updating" << endl;
            omp_unset_lock(lockGrain);
            std::cout << "Section [Deers] Thread [" << omp_get_thread_num() << "]: waiting for [Grain] to unlock for update" << endl;
            omp_set_lock(lockDeer);
            std::cout << "Section [Deers] Thread [" << omp_get_thread_num() << "]: has been unlocked by [Grain]" << endl;
            std::cout << "Section [Deers] Thread [" << omp_get_thread_num() << "]: updating deers" << endl;
            UpdateDeers(tmpDeers);
            omp_unset_lock(lockDeer);

            std::cout << "Section [Deers] Thread [" << omp_get_thread_num() << "]: unlocking [Watcher] from starting" << endl;
            omp_unset_lock(lockWatcherByDeer);

        }
        #pragma omp section 
        {
            
            std::cout << "Section [Watcher] Thread [" << omp_get_thread_num() << "]: waiting for [Grain] to unlock from doing work" << endl;
            omp_set_lock(lockWatcherByGrain);
            omp_unset_lock(lockWatcherByGrain);
            std::cout << "Section [Watcher] Thread [" << omp_get_thread_num() << "]: waiting for [Deer] to unlock from doing work" << endl;
            omp_set_lock(lockWatcherByDeer);
            omp_unset_lock(lockWatcherByDeer);

            std::cout << "Section [Watcher] Thread [" << omp_get_thread_num() << "]: doing work" << endl;
            PrintState();
            UpdateTime();
            UpdateFactors();

        }
    } // omp parallel sections
    
    omp_destroy_lock(lockDeer);
    omp_destroy_lock(lockGrain);
    omp_destroy_lock(lockWatcherByDeer);
    omp_destroy_lock(lockWatcherByGrain);
    return 0;
}