# Vitis Tutorial

This github is an example vector addition kernel.

## Host 

The host program is located in **src** and takes 1 arguments which is the bitstream to program the connected device. The helper functions for programming the FPGA are in **include/host.h**, which are taken from *Vitis_Accel_Examples*. 

## Kernel 

The Kernel reads from two buffers can be either connected to DDR or HBM, depending on the device used. The vectors are then added and written into another memory buffer. The ports are AXI interfactes. All the additions are done in parallel using a Vitis HLS pragam **UNROLL**. 

## Synthesizing and Compiling

The Makefile can be used to make both the host and kernel program. The host program is compiled with g++ with OpenCL. The kernel uses Vitits (v++) to compile and depending on software emulation, hardware emulation, and hardware the synthesis time will vary. Before you can run make, the Vitis and XRT versions need to be set.

``
source /tools/Xilinx/Vitis/20XX.X/settings64.sh
source /opt/xilinx/xrt/setup.sh
``

Inside the Makefile will have different configurations such as run, build, or host. When compiling the kernel a TARGET needs to be set, there are three options sw_emu, hw_emu and hw. sw_emu is software emulation and should be used to quickly verify software correctness (Compiles quickly and runs quickly), cannot be used to verify hardware correctness. hw_emu is hardware emulation, and simulates the hardware implmentation to verifty hardware correctness (compiles quickly but runs slow). hw generates a bitstream that programs the device (compiles very slowly, and runs fast). Using a combination of these different TARGETs, we can debug our design. 

The PLATFORM is the specific device that we are developing for. Generally **/opt/xilinx/platforms/** is the location of all the platforms that are installed on the machine. We then have to provide the make file a platform file (.xpfm). 

I would recommend adding the following lines to your **.bashrc**

``
source /tools/Xilinx/Vitis/20XX.X/settings64.sh
``
``
source /opt/xilinx/xrt/setup.sh
``
``
export PLATFORM=/opt/xilinx/platforms/<device>/<device>.xpfm
``

You could also export the TARGET but this will change more often. 

### make run TARGET=sw_emu/hw_emu/hw PLATFORM=/opt/xilinx/platforms/*device*/*device*.xpfm

It builds both the host and kernel program. 

### make build TARGET=sw_emu/hw_emu/hw PLATFORM=/opt/xilinx/platforms/*device*/*device*.xpfm

Builds only the kernel program. 

### make host 

Only builds the host program. 

## Config File 
The config file **config.cfg** is 1 of 2 ways to control how the Vitis compiler syntheisizes the kernel to hardware. For example the connectivity of the FPGA design can be specified. In this example, we show the vector memory buffers can be instaniated in HBM or DDR. Other configuration options can be found https://docs.xilinx.com/r/en-US/ug1393-vitis-application-acceleration/v-General-Options. There are many options for profiling, debugging, etc. 

## HLS PRAGMAS 
The other way to control how the kernel is synthesized to hardware is using HLS PRAGMAS (#pragma), these commands are placed within the kernel HLS code. In this example we use only a 2 different types of pragmas. One is an INTERFACE pragma that specify the a, b, and c ports as AXI master ports. The AXI protocol is a standard and can be read about more https://support.xilinx.com/s/article/1053914?language=en_US. The other pragma used is the UNROLL pragma. A common technique in software is to unroll loops to perform more computation per iteration. Thhe UNROLL pragma acheives something similar, by adding more hardware units to perform the computation in parallel. As you might expect this pragma is limited by loop dependencies. The UNROLL pragma can also take in parameters such as factor which specifies the amount of unrolling to perform. For example some extremely large loops cannot be unrolled fully since there will be a physical resource limitation. Try different unroll factors to see the changes in performance. Other HLS pragmas can be found https://docs.xilinx.com/r/en-US/ug1399-vitis-hls/HLS-Pragmas. 

## More Info & Tips

The build folder will have a report for the synthesized kernel. There you can see the latency and resource utilization of the kernel. 

If you're using git, clean the project before commiting the changes also look for hidden folders **.run** and **.ipcache** these folders will get very large and cause issues pushing your repo. Some bitstreams (.xclbin) files will get too large for Git. 

If you notice a hw compile is taking a long(er) time, try a sw_emu or hw_emu to see if there are any errors. Sometimes when synthezing for hw the software overlooks syntax errors. 

If integrating Verilog/VHDL designs with your HLS design, you can only perform hw_emu and hw runs. 

A powerful tool to use for HW synthesis is TMUX https://www.redhat.com/sysadmin/introduction-tmux-linux. Sometimes HW synthesis can take multiple hours, and maintaining a ssh command for an extended period of time can be difficult. So you can use tmux 
to allow the synthesis continue without an SSH connection. The job can be disowned as well, but TMUX is prefereable as you can reconnect to the tmux session to check the progress easily.  
