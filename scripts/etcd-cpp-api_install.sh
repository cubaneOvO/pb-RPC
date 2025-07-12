mkdir -p ../deploy/bins
mkdir -p ../deploy/libs
mkdir -p ../deploy/include
mkdir -p ../deploy/log

sudo apt-get install libboost-all-dev libssl-dev 
sudo apt-get install libprotobuf-dev protobuf-compiler-grpc 
sudo apt-get install libgrpc-dev libgrpc++-dev  
sudo apt-get install libcpprest-dev

mkdir ../3rdparty
cd ../3rdparty
git clone https://gitcode.com/gh_mirrors/et/etcd-cpp-apiv3.git
cd etcd-cpp-apiv3 
mkdir build && cd build 
cmake .. -DCMAKE_INSTALL_PREFIX=/usr 
make && sudo make install
