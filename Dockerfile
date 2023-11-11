FROM docker.io/intel/oneapi-basekit:latest

WORKDIR /helix

RUN wget -O- https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB | gpg --dearmor | tee /usr/share/keyrings/oneapi-archive-keyring.gpg > /dev/null
RUN echo "deb [signed-by=/usr/share/keyrings/oneapi-archive-keyring.gpg] https://apt.repos.intel.com/oneapi all main" | tee /etc/apt/sources.list.d/oneAPI.list

RUN apt-get update
RUN apt-get -y install cmake libgtest-dev nano build-essential

COPY ./ ./

ENV PATH="${PATH}:/cuda/bin"
RUN sh codeplay/oneapi-for-nvidia-gpus-2023.2.1-cuda-12.0-linux.sh

# docker run --gpus all --rm -it -v /usr/local/cuda-12.1:/cuda -v /etc/passwd:/etc/passwd:ro -v /etc/group:/etc/group:ro helix-solver-docker