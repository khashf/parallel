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

#if MODE == 1
const char *CL_FILE_NAME = {"multiply.cl"};
#elif MODE == 2
const char *CL_FILE_NAME = {"multiply-add.cl"};
#else 
const char *CL_FILE_NAME = {"multiply-reduce.cl"};
#endif
const float TOL = 0.0001f;
//int gNMB = 64;
long unsigned int gNumElements = 64 * 1000 * 1000;
long unsigned int gLocalSize = 1024;
long unsigned int gNumWorkGroups;

int gNumProgramArgc = 2;
void Wait(cl_command_queue);
int LookAtTheBits(float);


void print_cl_error(cl_int e, const char* prefix);

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
    // cout << "Global Size = " << gNumElements;
    // cout << " Local Size = " << gLocalSize;
    // cout << " Number of WorkGroups = " << gNumWorkGroups << endl;

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
#if MODE == 3 // for multiply-reduce
	float *hD = new float[gNumWorkGroups];
#else // for multiply and multiply-add
	float *hD = new float[gNumElements];
#endif

	for (int i = 0; i < gNumElements; i++) {
		hA[i] = hB[i] = hC[i] = (float)sqrt((double)i);
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

	cl_mem dC = clCreateBuffer(context, CL_MEM_READ_ONLY, dataSize, NULL, &status);
	if (status != CL_SUCCESS)
		fprintf(stderr, "clCreateBuffer failed (2)\n");

	cl_mem dD = clCreateBuffer(context, CL_MEM_WRITE_ONLY, dataSize, NULL, &status);
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

	status = clEnqueueWriteBuffer(cmdQueue, dC, CL_FALSE, 0, dataSize, hC, 0, NULL, NULL);
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

#if MODE == 1
	cl_kernel kernel = clCreateKernel(program, "ArrayMult", &status);
#elif MODE == 2
	cl_kernel kernel = clCreateKernel(program, "ArrayMultAdd", &status);
#else 
	cl_kernel kernel = clCreateKernel(program, "ArrayMultReduce", &status);
#endif
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
		fprintf(stderr, "clSetKernelArg failed (2)\n");

#if MODE == 3
	status = clSetKernelArg(kernel, 3, gLocalSize * sizeof(float), NULL);
	if (status != CL_SUCCESS)
		fprintf(stderr, "clSetKernelArg failed (2)\n");
	status = clSetKernelArg(kernel, 4, sizeof(cl_mem), &dD);
	if (status != CL_SUCCESS)
		fprintf(stderr, "clSetKernelArg failed (3)\n");
#else 
	status = clSetKernelArg(kernel, 3, sizeof(cl_mem), &dD);
	if (status != CL_SUCCESS)
		fprintf(stderr, "clSetKernelArg failed (3)\n");
#endif


    // -------------------------------------------------
	// 11. enqueue the kernel object for execution:
    // -------------------------------------------------

	size_t globalWorkSize[3] = {gNumElements, 1, 1};
	size_t localWorkSize[3] = {gLocalSize, 1, 1};

	Wait(cmdQueue);
	double time0 = omp_get_wtime();
	//time0 = omp_get_wtime();

	status = clEnqueueNDRangeKernel(cmdQueue, kernel, 1, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
    print_cl_error(status, "clEnqueueNDRangeKernel");
    //PrintCLError(status, "", stderr);
	// if (status != CL_SUCCESS)
	//     fprintf(stderr, "clEnqueueNDRangeKernel failed: %d\n", status);

	Wait(cmdQueue);
	double time1 = omp_get_wtime();

    // -------------------------------------------------
	// 12. read the results buffer back from the device to the host:
    // -------------------------------------------------

    double sum = 0.0;
#if MODE == 3
	status = clEnqueueReadBuffer(cmdQueue, dD, CL_TRUE, 0, gNumWorkGroups*sizeof(float), hD, 0, NULL, NULL);
	if (status != CL_SUCCESS)
		fprintf(stderr, "clEnqueueReadBuffer failed\n");
#else
	status = clEnqueueReadBuffer(cmdQueue, dD, CL_TRUE, 0, dataSize, hD, 0, NULL, NULL);
	if (status != CL_SUCCESS)
		fprintf(stderr, "clEnqueueReadBuffer failed\n");
    Wait(cmdQueue);
    for (int i = 0; i < gNumWorkGroups; ++i) {
        sum += hD[i];
    }
#endif

	// did it work?
// #if MODE == 1
//     for (int i = 0; i < gNumElements; i++) {
//         float expected = hA[i] * hB[i];
//         if (fabs(hD[i] - expected) > TOL) {
//             fprintf(stderr, "%4d: %13.6f * %13.6f wrongly produced %13.6f instead of %13.6f (%13.8f)\n",
//                      i, hA[i], hB[i], hD[i], expected, fabs(hD[i]-expected));
//             fprintf(stderr, "%4d:    0x%08x *    0x%08x wrongly produced    0x%08x instead of    0x%08x\n",
//                      i, LookAtTheBits(hA[i]), LookAtTheBits(hB[i]), LookAtTheBits(hD[i]), LookAtTheBits(expected));
//         }
//     }
// #elif MODE == 2
//     for (int i = 0; i < gNumElements; i++) {
//         float expected = hA[i] * hB[i] + hC[i];
//         if (fabs(hD[i] - expected) > TOL) {
//             fprintf(stderr, "%4d: %13.6f * %13.6f + %13.6f wrongly produced %13.6f instead of %13.6f (%13.8f)\n",
//                      i, hA[i], hB[i], hC[i], hD[i], expected, fabs(hD[i]-expected));
//             fprintf(stderr, "%4d:    0x%08x *    0x%08x +    0x%08x wrongly produced    0x%08x instead of    0x%08x\n",
//                      i, LookAtTheBits(hA[i]), LookAtTheBits(hB[i]), LookAtTheBits(hC[i]), LookAtTheBits(hD[i]), LookAtTheBits(expected));
//         }
//     }
// #else
//     float* tmpExpected = new float[gNumElements];
//     double expected = 0.0;
//     for (int i = 0; i < gNumElements; ++i) {
//         tmpExpected[i] = hA[i] * hB[i];
//         expected += static_cast<double>(tmpExpected[i]);
//     }
//     if (fabs(sum - expected) > TOL) {
//         fprintf(stderr, "wrongly produced %13.6f instead of %13.6f (%13.8f)\n",
//                  sum, expected, fabs(sum-expected));
//     }
//     delete[] tmpExpected;
// #endif

	fprintf(stdout, "%8.3lf\n",
			(double)gNumElements / (time1 - time0) / 1000000000.);

    // -------------------------------------------------
	// 13. clean everything up:
    // -------------------------------------------------

	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(cmdQueue);
	clReleaseMemObject(dA);
	clReleaseMemObject(dB);
	clReleaseMemObject(dC);
	clReleaseMemObject(dD);

	delete[] hA;
	delete[] hB;
	delete[] hC;
	delete[] hD;

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

// Modified from a useful helper function
// from Professor Andreas Kloeckner at UIUC ([link](https://github.com/hpc12/tools))
void print_cl_error(cl_int e, const char* prefix) {
  if (e == CL_SUCCESS)
      return;
  cerr << prefix << ": " << "error code = " << e << " ";
  switch (e)   {
    //case CL_SUCCESS: break; // cerr << "success | ";
    case CL_DEVICE_NOT_FOUND: cerr << "device not found | ";
    case CL_DEVICE_NOT_AVAILABLE: cerr << "device not available | ";
#if !(defined(CL_PLATFORM_NVIDIA) && CL_PLATFORM_NVIDIA == 0x3001)
    case CL_COMPILER_NOT_AVAILABLE: cerr << "device compiler not available | ";
#endif
    case CL_MEM_OBJECT_ALLOCATION_FAILURE: cerr << "mem object allocation failure | ";
    case CL_OUT_OF_RESOURCES: cerr << "out of resources | ";
    case CL_OUT_OF_HOST_MEMORY: cerr << "out of host memory | ";
    case CL_PROFILING_INFO_NOT_AVAILABLE: cerr << "profiling info not available | ";
    case CL_MEM_COPY_OVERLAP: cerr << "mem copy overlap | ";
    case CL_IMAGE_FORMAT_MISMATCH: cerr << "image format mismatch | ";
    case CL_IMAGE_FORMAT_NOT_SUPPORTED: cerr << "image format not supported | ";
    case CL_BUILD_PROGRAM_FAILURE: cerr << "build program failure | ";
    case CL_MAP_FAILURE: cerr << "map failure | ";

    case CL_INVALID_VALUE: cerr << "invalid value | ";
    case CL_INVALID_DEVICE_TYPE: cerr << "invalid device type | ";
    case CL_INVALID_PLATFORM: cerr << "invalid platform | ";
    case CL_INVALID_DEVICE: cerr << "invalid device | ";
    case CL_INVALID_CONTEXT: cerr << "invalid context | ";
    case CL_INVALID_QUEUE_PROPERTIES: cerr << "invalid queue properties | ";
    case CL_INVALID_COMMAND_QUEUE: cerr << "invalid command queue | ";
    case CL_INVALID_HOST_PTR: cerr << "invalid host ptr | ";
    case CL_INVALID_MEM_OBJECT: cerr << "invalid mem object | ";
    case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: cerr << "invalid image format descriptor | ";
    case CL_INVALID_IMAGE_SIZE: cerr << "invalid image size | ";
    case CL_INVALID_SAMPLER: cerr << "invalid sampler | ";
    case CL_INVALID_BINARY: cerr << "invalid binary | ";
    case CL_INVALID_BUILD_OPTIONS: cerr << "invalid build options | ";
    case CL_INVALID_PROGRAM: cerr << "invalid program | ";
    case CL_INVALID_PROGRAM_EXECUTABLE: cerr << "invalid program executable | ";
    case CL_INVALID_KERNEL_NAME: cerr << "invalid kernel name | ";
    case CL_INVALID_KERNEL_DEFINITION: cerr << "invalid kernel definition | ";
    case CL_INVALID_KERNEL: cerr << "invalid kernel | ";
    case CL_INVALID_ARG_INDEX: cerr << "invalid arg index | ";
    case CL_INVALID_ARG_VALUE: cerr << "invalid arg value | ";
    case CL_INVALID_ARG_SIZE: cerr << "invalid arg size | ";
    case CL_INVALID_KERNEL_ARGS: cerr << "invalid kernel args | ";
    case CL_INVALID_WORK_DIMENSION: cerr << "invalid work dimension | ";
    case CL_INVALID_WORK_GROUP_SIZE: cerr << "invalid work group size | ";
    case CL_INVALID_WORK_ITEM_SIZE: cerr << "invalid work item size | ";
    case CL_INVALID_GLOBAL_OFFSET: cerr << "invalid global offset | ";
    case CL_INVALID_EVENT_WAIT_LIST: cerr << "invalid event wait list | ";
    case CL_INVALID_EVENT: cerr << "invalid event | ";
    case CL_INVALID_OPERATION: cerr << "invalid operation | ";
    case CL_INVALID_GL_OBJECT: cerr << "invalid gl object | ";
    case CL_INVALID_BUFFER_SIZE: cerr << "invalid buffer size | ";
    case CL_INVALID_MIP_LEVEL: cerr << "invalid mip level | ";

#if defined(cl_khr_gl_sharing) && (cl_khr_gl_sharing >= 1)
    case CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR: cerr << "invalid gl sharegroup reference number | ";
#endif

#ifdef CL_VERSION_1_1
    case CL_MISALIGNED_SUB_BUFFER_OFFSET: cerr << "misaligned sub-buffer offset | ";
    case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST: cerr << "exec status error for events in wait list | ";
    case CL_INVALID_GLOBAL_WORK_SIZE: cerr << "invalid global work size | ";
#endif

    default: cerr << "invalid/unknown error code | ";
  }
  //cerr << "| ";
}
