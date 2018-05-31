# Install OpenCL
Install `clinfo` and run it to know what you are missing and what need to be done.

## Install vendor driver
My case is NVIDIA. `clinfo` tells me I already got the neccessary NVIDIA driver. Otherwise, download and install [`CUDA toolkit`](https://developer.nvidia.com/cuda-downloads?target_os=Linux&target_arch=x86_64&target_distro=Ubuntu&target_version=1604&target_type=deblocal)
More instruction at [wangruohui's gist](https://gist.github.com/wangruohui/df039f0dc434d6486f5d4d098aa52d07) or at offical NVIDIA website.

## Install opencl header
Check if /usr/include/CL exists. If not, get [`OpenCL headers from Kronos`](https://github.com/KhronosGroup/OpenCL-Headers). I like their version because it allows me to change the version of OpenCL at compile time just by adding a `#define` statement before the `#include`. For example, to use OpenCL 1.2:
```
#define CL_TARGET_OPENCL_VERSION 120
#include <CL/opencl.h>
```
More info at Kronos's reposiotory README.md

## Symlink the library file
If linker cannot find -lOpenCL, we need to symlink the OpenCL library file from `/usr/lib/x86_64-linux-gnu/`
```
ls /usr/lib/x86_64-linux-gnu/ | grep "opencl"
ln -s /usr/lib/x86_64-linux-gnu/libOpenCL.so.1 /usr/lib/libOpenCL.so
```

## Check if OpenCL is now working
Useful tool from Professor Andreas Kloeckner at UIUC ([link](https://github.com/hpc12/tools))
```
curl https://codeload.github.com/hpc12/tools/tar.gz/master | tar xvfz -
cd tools-master
```
Add `#define CL_TARGET_OPENCL_VERSION 120` to the `cl-header.h`. Replace `120` with opencl version you want to use. Then:
```
make
./print-devices
./cl-demo 1000000 10
```
Example of a good result:
```
---------------------------------------------------------------------
*** Kernel compilation resulted in non-empty log message.
*** Set environment variable CL_HELPER_PRINT_COMPILER_OUTPUT=1 to see more.
*** NOTE: this may include compiler warnings and other important messages
***   about your code.
*** Set CL_HELPER_NO_COMPILER_OUTPUT_NAG=1 to disable this message.
0.000203 s
59.126670 GB/s
GOOD
```

## More info
* https://wiki.tiker.net/OpenCLHowTo#How_to_set_up_OpenCL_in_Linux
* 

# Compile

