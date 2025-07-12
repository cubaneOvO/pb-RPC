# pb-RPC
This is an RPC framework implemented in C++, using Protobuf for serialization and Etcd as the registration center.



Install required packages:
-
- sudo apt install g++ cmake make
- sudo apt install etcd （As for Ubuntu 24.04 install the package with following name: apt install etcd-server）
- systemctl enable etcd
- systemctl restart etcd
- cd scripts/
- ./etcd-cpp-api_install.sh
  
To build:
-
- mkdir build
- cd build/
- cmake ..
- make

To run:
-
- cd deploy/bins/
- ./server
- ./client
