#include "host.h"
#include "constants.h"
#include <vector> 

int main(int argc, char** argv) {

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <XCLBIN File>" << std::endl;
        return EXIT_FAILURE;
    }
    
    clock_t htod, dtoh, comp; 

    /*====================================================CL===============================================================*/

    std::string binaryFile = argv[1];
    cl_int err;
    cl::Context context;
    cl::Kernel krnl1, krnl2;
    cl::CommandQueue q;
    
    auto devices = get_xil_devices();
    auto fileBuf = read_binary_file(binaryFile);
    cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};
    bool valid_device = false;
    for (unsigned int i = 0; i < devices.size(); i++) {
        auto device = devices[i];
        OCL_CHECK(err, context = cl::Context(device, nullptr, nullptr, nullptr, &err));
        OCL_CHECK(err, q = cl::CommandQueue(context, device, 0, &err));
        std::cout << "Trying to program device[" << i << "]: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
        cl::Program program(context, {device}, bins, nullptr, &err);
        if (err != CL_SUCCESS) {
            std::cout << "Failed to program device[" << i << "] with xclbin file!\n";
        } else {
            std::cout << "Device[" << i << "]: program successful!\n";
            std::cout << "Setting CU(s) up..." << std::endl; 
            OCL_CHECK(err, krnl1 = cl::Kernel(program, "krnl", &err));
            valid_device = true;
            break; // we break because we found a valid device
        }
    }
    if (!valid_device) {
        std::cout << "Failed to program any device found, exit!\n";
        exit(EXIT_FAILURE);
    }

    /*====================================================INIT INPUT/OUTPUT VECTORS===============================================================*/

    std::vector<data_t, aligned_allocator<data_t> > a(DATA_SIZE);
    std::vector<data_t, aligned_allocator<data_t> > b(DATA_SIZE);
    std::vector<data_t, aligned_allocator<data_t> > c_hw(DATA_SIZE);
    std::vector<data_t> c_sw(DATA_SIZE);

    int lb = 20, ub = 100;
    for (int i = 0; i < DATA_SIZE; i++) {
        a[i] = (rand() % (ub - lb + 1)) + lb; 
        b[i] = (rand() % (ub - lb + 1)) + lb;
    }

    /*====================================================SW VERIFICATION===============================================================*/

    for (int i = 0; i < DATA_SIZE; i++) {
        c_sw[i] = a[i] + b[i];
    }

    /*====================================================Setting up kernel I/O===============================================================*/

    /* INPUT BUFFERS */
    OCL_CHECK(err, cl::Buffer buffer_a(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(data_t) * DATA_SIZE, a.data(), &err)); 
    OCL_CHECK(err, cl::Buffer buffer_b(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(data_t) * DATA_SIZE, b.data(), &err)); 

    /* OUTPUT BUFFERS */
    OCL_CHECK(err, cl::Buffer buffer_c(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, sizeof(data_t) * DATA_SIZE, c_hw.data(), &err)); 

    /* SETTING INPUT PARAMETERS */
    OCL_CHECK(err, err = krnl1.setArg(0, buffer_a));
    OCL_CHECK(err, err = krnl1.setArg(1, buffer_b));
    OCL_CHECK(err, err = krnl1.setArg(2, buffer_c));


    /*====================================================KERNEL===============================================================*/
    /* HOST -> DEVICE DATA TRANSFER*/
    std::cout << "HOST -> DEVICE" << std::endl; 
    htod = clock(); 
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_a}, 0 /* 0 means from host*/));
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_b}, 0 /* 0 means from host*/));
    q.finish();
    htod = clock() - htod; 
    
    /*STARTING KERNEL(S)*/
    std::cout << "STARTING KERNEL(S)" << std::endl; 
    comp = clock(); 
	OCL_CHECK(err, err = q.enqueueTask(krnl1));
    q.finish(); 
    comp = clock() - comp;
    std::cout << "KERNEL(S) FINISHED" << std::endl; 

    /*DEVICE -> HOST DATA TRANSFER*/
    std::cout << "HOST <- DEVICE" << std::endl; 
    dtoh = clock();
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_c}, CL_MIGRATE_MEM_OBJECT_HOST));
    q.finish();
    dtoh = clock() - dtoh;

    /*====================================================VERIFICATION & TIMING===============================================================*/

    printf("Host -> Device : %lf ms\n", 1000.0 * htod/CLOCKS_PER_SEC);
    printf("Device -> Host : %lf ms\n", 1000.0 * dtoh/CLOCKS_PER_SEC);
    printf("Computation : %lf ms\n",  1000.0 * comp/CLOCKS_PER_SEC);
    
    bool match = true;
    for (int i = 0; i < DATA_SIZE; i++) {
        //std::cout << "HW: " << c_hw[i] << " vs " << "SW: " << c_sw[i] << std::endl; 
        if (c_hw[i] != c_sw[i]) match = false; 
    }

    std::cout << "TEST " << (match ? "PASSED" : "FAILED") << std::endl;

    return (match ? EXIT_SUCCESS : EXIT_FAILURE);
}