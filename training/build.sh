#!/bin/sh

oldpwd=`pwd`
KERNEL_SRC=$HOME/beagle/kernel/kernel
SRC=$KERNEL_SRC/training

echo "building in $KERNEL_SRC\n"

cd $KERNEL_SRC
ret=$?
if [ $ret -ne 0 ]
then
	echo "Kernel src dir not found"
	exit 1;
fi


find $SRC -type f -name "*.o" -delete
find $SRC -type f -name "*.ko" -delete
find $SRC -type f -name "*.cmd" -delete
find $SRC -type f -name "*.mod.c" -delete
find $SRC -type f -name "*.builtin" -delete
find $SRC -type f -name "*.order" -delete

make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- uImage dtbs LOADADDR=0x80008000 modules -j4
sudo make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- INSTALL_MOD_PATH=$HOME/beagle/rootfs modules_install
cp $KERNEL_SRC/arch/arm/boot/uImage $KERNEL_SRC/../../binaries/
cp $KERNEL_SRC/arch/arm/boot/dts/am335x-boneblack.dtb $KERNEL_SRC/../../binaries/

cd $oldpwd

