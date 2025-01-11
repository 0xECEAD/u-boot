CROSS_COMPILE=powerpc-linux-gnu-
export CROSS_COMPILE

OUTPATH=~/mirari-uboot
IMGFILE=${OUTPATH}/mirari.img

mkdir -p ${OUTPATH}
make O=${OUTPATH} distclean
make O=${OUTPATH} mirari_SDCARD_defconfig
# make O=${OUTPATH} menuconfig
make O=${OUTPATH} all -j $(nproc)

cat board/freescale/mirari/default_env_strings | sort | ${OUTPATH}/tools/mkenvimage -b -p 0 -s 8192 -o ${OUTPATH}/mirari-env -

#
# Create the image file for writting to the SD card.
#
# u-boot.pbl is at 8 blocks (0x1000 bytes)
# default environment is at 2048 (0x100000 bytes)
# amigaboot is at 16384 blocks (0x800000 bytes)
# data image blob is at 18432 blocks (0x900000 bytes)
#
rm -f ${IMGFILE}
touch ${IMGFILE}
truncate -s 16M ${IMGFILE}
dd if=${OUTPATH}/u-boot-with-spl-pbl.bin of=${IMGFILE} bs=512 seek=8 conv=notrunc
dd if=${OUTPATH}/mirari-env of=${IMGFILE} bs=512 seek=2048 conv=notrunc
dd if=fsl_fman_ucode_t1040_r1.1_106_4_18.bin of=${IMGFILE} bs=512 seek=2080 conv=notrunc
#dd if=amigaboot of=${IMGFILE} bs=512 seek=16384 conv=notrunc
#dd if=dataimage of=${IMGFILE} bs=512 seek=18432 conv=notrunc
cp ${IMGFILE} /mnt/c/Users/Enceladus/Desktop/
