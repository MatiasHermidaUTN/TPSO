git clone https://github.com/sisoputnfrba/so-commons-library.git
cd so-commons-library.git
make debug
make install
cd ..
sudo cp tp-2023-1c-Los-Matias/shared/src/*.h ../../usr/local/include/
cd tp-2023-1c-Los-Matias/shared/Debug
make clean
make all
sudo cp libshared.so ../../../../../usr/local/lib/
./compilar_modulo.sh memoria
./compilar_modulo.sh fileSystem
./compilar_modulo.sh cpu
./compilar_modulo.sh kernel
./compilar_modulo.sh consola
cd ../../
