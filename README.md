# pb-RPC
This is an RPC framework implemented in C++, using Protobuf for serialization and Etcd as the registration center.

Install required packages:
  cd scripts/
  ./etcd-cpp-api_install.sh
  
To build:
  mkdir build
  cd build/
  cmake ..
  make

To run:
  cd deploy/bins/
  ./server
  ./client