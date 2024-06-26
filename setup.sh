# script for setting up all the tools needed by the project

export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

mkdir build

cd build

wget https://ftp.gnu.org/gnu/binutils/binutils-2.35.tar.gz
wget https://ftp.lip6.fr/pub/gcc/releases/gcc-10.1.0/gcc-10.1.0.tar.gz

tar -xvf binutils-2.35.tar.gz
tar -xvf gcc-10.1.0.tar.gz

mkdir build-binutils

cd build-binutils
../binutils-2.35/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install

cd ..


which -- $TARGET-as || echo $TARGET-as is not in the PATH
 
mkdir build-gcc

cd build-gcc

../gcc-10.1.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc

cd ..

cd ..
rm -rf build