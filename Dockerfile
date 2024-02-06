FROM docker.io/intel/oneapi-basekit:2023.2.1-devel-ubuntu20.04

WORKDIR /helix

RUN wget -O- https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB | gpg --dearmor | tee /usr/share/keyrings/oneapi-archive-keyring.gpg > /dev/null
RUN echo "deb [signed-by=/usr/share/keyrings/oneapi-archive-keyring.gpg] https://apt.repos.intel.com/oneapi all main" | tee /etc/apt/sources.list.d/oneAPI.list

RUN apt-get update
RUN apt-get -y install cmake libgtest-dev nano build-essential nlohmann-json3-dev dpkg-dev g++ gcc binutils libx11-dev libxpm-dev libxft-dev libxext-dev python libssl-dev

RUN wget https://root.cern/download/root_v6.25.01.Linux-ubuntu20-x86_64-gcc9.3.tar.gz
RUN mkdir lib
RUN tar -xzvf root_v6.25.01.Linux-ubuntu20-x86_64-gcc9.3.tar.gz --directory=lib
RUN rm root_v6.25.01.Linux-ubuntu20-x86_64-gcc9.3.tar.gz

COPY ../codeplay ./codeplay  

ENV PATH="${PATH}:/cuda/bin"
RUN sh codeplay/oneapi-for-nvidia-gpus-2023.2.1-cuda-12.0-linux.sh

# docker run --gpus all --rm -it -v /usr/local/cuda-12.1:/cuda -v /etc/passwd:/etc/passwd:ro -v /etc/group:/etc/group:ro helix-solver-docker
