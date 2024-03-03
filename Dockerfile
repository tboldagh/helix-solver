FROM docker.io/intel/oneapi-basekit:2023.2.1-devel-ubuntu20.04

WORKDIR /helix

RUN wget -O- https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB | gpg --dearmor | tee /usr/share/keyrings/oneapi-archive-keyring.gpg > /dev/null
RUN echo "deb [signed-by=/usr/share/keyrings/oneapi-archive-keyring.gpg] https://apt.repos.intel.com/oneapi all main" | tee /etc/apt/sources.list.d/oneAPI.list

RUN apt-get update
RUN apt-get -y install cmake libgtest-dev nano build-essential nlohmann-json3-dev dpkg-dev g++ gcc binutils libx11-dev libxpm-dev libxft-dev libxext-dev python libssl-dev valgrind

RUN mkdir /helix/lib
WORKDIR /helix/lib

RUN wget https://root.cern/download/root_v6.25.01.Linux-ubuntu20-x86_64-gcc9.3.tar.gz
RUN tar -xzvf root_v6.25.01.Linux-ubuntu20-x86_64-gcc9.3.tar.gz
RUN rm root_v6.25.01.Linux-ubuntu20-x86_64-gcc9.3.tar.gz

WORKDIR /helix/repo

ENV PATH="${PATH}:/cuda/bin"

# docker run --gpus all --rm -it -v /usr/local/cuda-12.1:/cuda -v /etc/passwd:/etc/passwd:ro -v /etc/group:/etc/group:ro -v $(pwd):/helix/ helix-solver-docker
# docker run --gpus all --rm -it -v .:/helix/repo -v /usr/local/cuda-12.1:/cuda helix-solver-docker
