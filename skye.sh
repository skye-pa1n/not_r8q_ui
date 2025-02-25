#!/bin/bash
LLVM_PATH="/home/skye/bomb/clang/bin/"
TC_PATH="/home/skye/bomb/clang/bin/"
GCC_PATH="/usr/bin/"
LLD_PATH="/usr/bin/"
CLANGV="21"
KERNEL_NAME="not-interstellar+"
MAKE="./makeparallel"
BUILD_ENV="CC=${TC_PATH}clang-${CLANGV} CROSS_COMPILE=${TC_PATH}aarch64-linux-gnu- LLVM=1 LLVM_IAS=1 PATH=$LLVM_PATH:$LLD_PATH:$PATH"  
KERNEL_MAKE_ENV="DTC_EXT=$(pwd)/tools/dtc CONFIG_BUILD_ARM64_DT_OVERLAY=y"

rm -rf /home/skye/bomb/out/arch/arm64/boot/Image
rm -rf /home/skye/bomb/AnyKernel3/dtb
rm -rf .version
rm -rf .local
make O=/home/skye/bomb/out clean
make O=/home/skye/bomb/out ARCH=arm64 $BUILD_ENV dtb_defconfig

make -j12 O=/home/skye/bomb/out ARCH=arm64 $BUILD_ENV dtbs
DTB_OUT="/home/skye/bomb/out/arch/arm64/boot/dts/vendor/qcom"
cat $DTB_OUT/*.dtb > /home/skye/bomb/AnyKernel3/oneui.dtb

make O=/home/skye/bomb/out clean
make O=/home/skye/bomb/out ARCH=arm64 $BUILD_ENV not_defconfig
make -j12 O=/home/skye/bomb/out ARCH=arm64 $KERNEL_MAKE_ENV $BUILD_ENV Image
IMAGE="/home/skye/bomb/out/arch/arm64/boot/Image"
cp $IMAGE /home/skye/bomb/AnyKernel3/OneUI

cd /home/skye/bomb/AnyKernel3
rm *.zip
zip -r9 ${KERNEL_NAME}clang_${CLANGV}.0.0git-$(date +"%Y%m%d")-r8q.zip .
echo "The bomb has been planted."

