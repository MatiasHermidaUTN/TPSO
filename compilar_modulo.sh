cd $1/Debug/
sudo make clean
sudo make all
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/tp-2023-1c-Los-Matias/shared/Debug/
cd ../../
