CROSS_COMPILE=powerpc-linux-gnu-
export CROSS_COMPILE

mkdir -p ~/mirari
make O=~/mirari distclean
make O=~/mirari Mirari_SDCARD_defconfig
make O=~/mirari all -j 6
