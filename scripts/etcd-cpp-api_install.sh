sudo apt-get install libboost-all-dev libssl-dev 
sudo apt-get install libprotobuf-dev protobuf-compiler-grpc 
sudo apt-get install libgrpc-dev libgrpc++-dev  
sudo apt-get install libcpprest-dev

cd ../3rdparty
git clone https://gitcode.com/gh_mirrors/et/etcd-cpp-apiv3.git
cd etcd-cpp-apiv3 
mkdir build && cd build 
cmake .. -DCMAKE_INSTALL_PREFIX=/usr 
make && sudo make install