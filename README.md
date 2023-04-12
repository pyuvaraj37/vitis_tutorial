# Vitis Tutorial

This github is an example vector addition kernel.

## Host 

The host program is located in **src** and takes 1 arguments which is the bitstream to program the connected device. The helper functions for programming the FPGA are in **include/host.h**, which are taken from *Vitis_Accel_Examples*. 

## Kernel 

The Kernel reads from two buffers can be either connected to DDR or HBM, depending on the device used. The vectors are then added and written into another memory buffer. The ports are AXI interfactes. All the additions are done in parallel using a Vitis HLS pragam **UNROLL**. 

## Synthesizing and Compiling

The Makefile can be used to make both the host and kernel program. The host program is compiled with g++ with OpenCL. The kernel uses Vitits (v++) to compile and depending on software emulation, hardware emulation, and hardware the synthesis time will vary. Before you can run make, the Vitis and XRT versions need to be set in **.bashrc**.

``
source /tools/Xilinx/Vitis/20XX.X/settings64.sh
source /opt/xilinx/xrt/setup.sh
``


Inside the Makefile will have different configurations such as run, build, or host. When compiling the kernel a TARGET needs to be set, there are three options sw_emu, hw_emu and hw. sw_emu is software emulation and should be used to quickly verify software correctness (Compiles quickly and runs quickly), cannot be used to verify hardware correctness. hw_emu is hardware emulation, and simulates the hardware implmentation to verifty hardware correctness (compiles quickly but runs slow). hw generates a bitstream that programs the device (compiles very slowly, and runs fast). Using a combination of these different TARGETs, we can debug our design. 

The PLATFORM is the specific device that we are developing for. Generally **/opt/xilinx/platforms/** is the location of all the platforms that are installed on the machine. 

### make run <TARGET> <PLATFORM>

It builds both the host and kernel program. 

### make build <TARGET> <PLATFORM>

Builds only the kernel program. 

### make host 

Only builds the host program. 


## More Info

