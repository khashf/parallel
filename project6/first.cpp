#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>
#include <iostream>

#define CL_TARGET_OPENCL_VERSION 110
#include "CL/cl.h"
#include "CL/cl_platform.h"

using std::cout;
using std::endl;
using std::cerr;

// #ifndef NMB
// #define NMB 64
// #endif
// #define NUM_ELEMENTS NMB * 1024 * 1024
//
// #ifndef LOCAL_SIZE
// #define LOCAL_SIZE 64
// #endif
//
// #define NUM_WORK_GROUPS NUM_ELEMENTS / LOCAL_SIZE

const char *CL_FILE_NAME = {"first.cl"};
const float TOL = 0.0001f;
//int gNMB = 64;
long unsigned int gNumElements = 64 * 1000 * 1000;
long unsigned int gLocalSize = 1024;
long unsigned int gNumWorkGroups;

int gNumProgramArgc = 2;
void Wait(cl_command_queue);
int LookAtTheBits(float);

int main(int argc, char* argv[]) {
    // -------------------------------------------------
    // 0. Precautionary error checking
    // -------------------------------------------------
    if (argc != gNumProgramArgc+1) {
        fprintf(stderr, "Number of arguments must be %d\n", gNumProgramArgc);
        fprintf(stderr, "Usage:\n%s <global work size> <local work size>\n", argv[0]);
        return 1;
    }
    gNumElements = strtoul(argv[1], NULL, 0);
    gLocalSize = strtoul(argv[2], NULL, 0);
    gNumWorkGroups = gNumElements / gLocalSize;
    cout << "Global Size = " << gNumElements << endl;
    cout << "Local Size = " << gLocalSize << endl;
    cout << "Number of WorkGroups = " << gNumWorkGroups << endl;

    // check if we can open the .cl code file
    // which will be use for step 7 below
	FILE *fp;
	fp = fopen(CL_FILE_NAME, "r");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open OpenCL source file '%s'\n", CL_FILE_NAME);
		return 1;
	}

    // -------------------------------------------------
    // 1. Get platform and device info
    // -------------------------------------------------

	cl_int status;

	// get the platform id:
	cl_platform_id platform;
	status = clGetPlatformIDs(1, &platform, NULL);
	if (status != CL_SUCCESS)
		fprintf(stderr, "clGetPlatformIDs failed (2)\n");

	// get the device id:
	cl_device_id device;
	status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
	if (status != CL_SUCCESS)
		fprintf(stderr, "clGetDeviceIDs failed (2)\n");

    // -------------------------------------------------
	// 2. allocate the host memory buffers:
    // -------------------------------------------------

	float *hA = new float[gNumElements];
	float *hB = new float[gNumElements];
	float *hC = new float[gNumElements];

	for (int i = 0; i < gNumElements; i++) {
		hA[i] = hB[i] = (float)sqrt((double)i);
	}
	size_t dataSize = gNumElements * sizeof(float);

    // -------------------------------------------------
	// 3. create an opencl context:
    // -------------------------------------------------

	cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &status);
	if (status != CL_SUCCESS)
		fprintf(stderr, "clCreateContext failed\n");

    // -------------------------------------------------
	// 4. create an opencl command queue:
    // -------------------------------------------------

	cl_command_queue cmdQueue = clCreateCommandQueue(context, device, 0, &status);
	if (status != CL_SUCCESS)
		fprintf(stderr, "clCreateCommandQueue failed\n");

    // -------------------------------------------------
	// 5. allocate the device memory buffers:
    // -------------------------------------------------

	cl_mem dA = clCreateBuffer(context, CL_MEM_READ_ONLY, dataSize, NULL, &status);
	if (status != CL_SUCCESS)
		fprintf(stderr, "clCreateBuffer failed (1)\n");

	cl_mem dB = clCreateBuffer(context, CL_MEM_READ_ONLY, dataSize, NULL, &status);
	if (status != CL_SUCCESS)
		fprintf(stderr, "clCreateBuffer failed (2)\n");

	cl_mem dC = clCreateBuffer(context, CL_MEM_WRITE_ONLY, dataSize, NULL, &status);
	if (status != CL_SUCCESS)
		fprintf(stderr, "clCreateBuffer failed (3)\n");

    // -------------------------------------------------
	// 6. enqueue the 2 commands to write the data 
    //    from the host buffers to the device buffers:
    // -------------------------------------------------

	status = clEnqueueWriteBuffer(cmdQueue, dA, CL_FALSE, 0, dataSize, hA, 0, NULL, NULL);
	if (status != CL_SUCCESS)
		fprintf(stderr, "clEnqueueWriteBuffer failed (1)\n");

	status = clEnqueueWriteBuffer(cmdQueue, dB, CL_FALSE, 0, dataSize, hB, 0, NULL, NULL);
	if (status != CL_SUCCESS)
		fprintf(stderr, "clEnqueueWriteBuffer failed (2)\n");

	Wait(cmdQueue);

    // -------------------------------------------------
	// 7. read the kernel code from a file:
    // -------------------------------------------------

	fseek(fp, 0, SEEK_END);
	size_t fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *clProgramText = new char[fileSize + 1]; // leave room for '\0'
	size_t readFileSize = fread(clProgramText, 1, fileSize, fp);
	clProgramText[fileSize] = '\0';
	fclose(fp);
	if (readFileSize != fileSize)
		fprintf(stderr, "Expected to read %lu bytes read from '%s' -- actually read %lu.\n", fileSize, CL_FILE_NAME, readFileSize);

	// create the text for the kernel program:
	char *strings[1];
	strings[0] = clProgramText;
	cl_program program = clCreateProgramWithSource(context, 1, (const char **)strings, NULL, &status);
	if (status != CL_SUCCESS)
		fprintf(stderr, "clCreateProgramWithSource failed\n");
	delete[] clProgramText;

    // -------------------------------------------------
	// 8. compile and link the kernel code:
    // -------------------------------------------------

	//char *options = {""};
	const char* options = static_cast<const char*>("");
	status = clBuildProgram(program, 1, &device, options, NULL, NULL);
	if (status != CL_SUCCESS) {
		size_t size;
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &size);
		cl_char *log = new cl_char[size];
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, size, log, NULL);
		fprintf(stderr, "clBuildProgram failed:\n%s\n", log);
		delete[] log;
	}

    // -------------------------------------------------
	// 9. create the kernel object:
    // -------------------------------------------------

	cl_kernel kernel = clCreateKernel(program, "ArrayMult", &status);
	if (status != CL_SUCCESS)
		fprintf(stderr, "clCreateKernel failed\n");

    // -------------------------------------------------
	// 10. setup the arguments to the kernel object:
    // -------------------------------------------------

	status = clSetKernelArg(kernel, 0, sizeof(cl_mem), &dA);
	if (status != CL_SUCCESS)
		fprintf(stderr, "clSetKernelArg failed (1)\n");

	status = clSetKernelArg(kernel, 1, sizeof(cl_mem), &dB);
	if (status != CL_SUCCESS)
		fprintf(stderr, "clSetKernelArg failed (2)\n");

	status = clSetKernelArg(kernel, 2, sizeof(cl_mem), &dC);
	if (status != CL_SUCCESS)
		fprintf(stderr, "clSetKernelArg failed (3)\n");

    // -------------------------------------------------
	// 11. enqueue the kernel object for execution:
    // -------------------------------------------------

	size_t globalWorkSize[3] = {gNumElements, 1, 1};
	size_t localWorkSize[3] = {gLocalSize, 1, 1};

	Wait(cmdQueue);
	double time0 = omp_get_wtime();
	time0 = omp_get_wtime();

	status = clEnqueueNDRangeKernel(cmdQueue, kernel, 1, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
	if (status != CL_SUCCESS)
		fprintf(stderr, "clEnqueueNDRangeKernel failed: %d\n", status);

	Wait(cmdQueue);
	double time1 = omp_get_wtime();

    // -------------------------------------------------
	// 12. read the results buffer back from the device to the host:
    // -------------------------------------------------

	status = clEnqueueReadBuffer(cmdQueue, dC, CL_TRUE, 0, dataSize, hC, 0, NULL, NULL);
	if (status != CL_SUCCESS)
		fprintf(stderr, "clEnqueueReadBuffer failed\n");

	// did it work?

	for (int i = 0; i < gNumElements; i++) {
		float expected = hA[i] * hB[i];
		if (fabs(hC[i] - expected) > TOL) {
			fprintf( stderr, "%4d: %13.6f * %13.6f wrongly produced %13.6f instead of %13.6f (%13.8f)\n",
			         i, hA[i], hB[i], hC[i], expected, fabs(hC[i]-expected) );
			fprintf( stderr, "%4d:    0x%08x *    0x%08x wrongly produced    0x%08x instead of    0x%08x\n",
			         i, LookAtTheBits(hA[i]), LookAtTheBits(hB[i]), LookAtTheBits(hC[i]), LookAtTheBits(expected) );
		}
	}

	fprintf(stderr, "GlobalSize=%8lu\tLocalSize=%4lu\tNumWorkGroups=%10lu\tPerformance=%10.3lf GigaMultsPerSecond\n",
			gNumElements, gLocalSize, gNumWorkGroups, (double)gNumElements / (time1 - time0) / 1000000000.);

    // -------------------------------------------------
	// 13. clean everything up:
    // -------------------------------------------------

	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(cmdQueue);
	clReleaseMemObject(dA);
	clReleaseMemObject(dB);
	clReleaseMemObject(dC);

	delete[] hA;
	delete[] hB;
	delete[] hC;

	return 0;
}

int LookAtTheBits(float fp) {
	int *ip = (int *)&fp;
	return *ip;
}

// wait until all queued tasks have taken place:

void Wait(cl_command_queue queue) {
	cl_event wait;
	cl_int status;

	status = clEnqueueMarker(queue, &wait);
	if (status != CL_SUCCESS)
		fprintf(stderr, "Wait: clEnqueueMarker failed\n");

	status = clWaitForEvents(1, &wait);
	if (status != CL_SUCCESS)
		fprintf(stderr, "Wait: clWaitForEvents failed\n");
}
