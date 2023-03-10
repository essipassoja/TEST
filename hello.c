#include <stdio.h>
#include <stdlib.h>
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#include <CL/cl.h>

#define MEM_SIZE (128)
#define MAX_SOURCE_SIZE (0x001000)

int main() {
    cl_device_id device_id = NULL;
    cl_context context = NULL;
    cl_command_queue command_queue = NULL;
    cl_mem memobj = NULL;
    cl_program program = NULL;
    cl_kernel kernel = NULL;
    cl_platform_id platform_id = NULL;
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret;

    char string[MEM_SIZE];

    FILE* fp;
    char fileName[] = "Hello.cl";
    char* source_str;
    size_t source_size;

    /* Load the source code containing the kernel */
    fp = fopen(fileName, "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }
    source_str = (char*)calloc(MAX_SOURCE_SIZE, 1);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);

    fclose(fp);

    /* Get platform and Device info */
    ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    printf("platfrom ret: %d\n", ret);
    ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &ret_num_devices);
    printf("platform id: %d, device id: %d\n", platform_id, device_id);

    /* Create OpenCL context */
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);

    /* Create Command Queue */
    cl_command_queue_properties props[] = {CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0};
    command_queue = clCreateCommandQueueWithProperties(context, device_id, props, &ret);

    /* Create Memory Buffer */
    memobj = clCreateBuffer(context, CL_MEM_READ_WRITE, MEM_SIZE * sizeof(char), NULL, &ret);
    
    printf("source str: %s\n", source_str);
    /* Create Kernel Program from the source */
    program = clCreateProgramWithSource(context, 1, (const char**)&source_str, (const size_t*)&source_size, &ret);

    /* Build Kernel Program */
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    printf("%d\n", ret);
    if (ret == CL_BUILD_PROGRAM_FAILURE) {
        // Determine the size of the log
        size_t log_size;
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

        // Allocate memory for the log
        char* log = (char*)malloc(log_size);

        // Get the log
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

        printf("%s\n", log);
    }
    /* Create OpenCL Kernel */
    kernel = clCreateKernel(program, "hello", &ret);

    /* Set OpenCL Kernel Params */
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&memobj);

    /* Execute OpenCL Kernel */
    size_t global_item_size = 1024; // total number of work-items to be executed
    size_t local_item_size = 64; // number of work-items in a work-group
    size_t num_groups = global_item_size/local_item_size; // number of work-groups

    ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);

    /* Copy results from the memory buffer */
    ret = clEnqueueReadBuffer(command_queue, memobj, CL_TRUE, 0, MEM_SIZE*sizeof(char), string, 0, NULL, NULL);

    /* Display Result */
    puts(string);
    printf("%s\n", string);

    /* Finalization */
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(memobj);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);

    free(source_str);
    
    return 0;
}