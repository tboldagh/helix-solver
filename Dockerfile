#FROM intel/oneapi:2023.1.0-devel-ubuntu20.04
FROM ghcr.io/acts-project/ubuntu2004_cuda_oneapi:v42
WORKDIR /opt
# Update packages information
RUN apt-get update

# Install required and optional packages
RUN DEBIAN_FRONTEND=noninteractive apt-get -yq install gfortran libpcre3-dev \
    xlibmesa-glu-dev libglew1.5-dev libftgl-dev \
    libmariadb-dev libfftw3-dev libcfitsio-dev \
    graphviz-dev libavahi-compat-libdnssd-dev \
    libldap2-dev python-dev libxml2-dev libkrb5-dev \
    libgsl0-dev qtwebengine5-dev less
# Download and install root
# RUN wget https://root.cern    /download/root_v6.28.02.Linux-ubuntu20-x86_64-gcc9.4.tar.gz && \
#     tar -xzvf root_v6.28.02.Linux-ubuntu20-x86_64-gcc9.4.tar.gz && \
#     rm root_v6.28.02.Linux-ubuntu20-x86_64-gcc9.4.tar.gz && \
#     echo "source /opt/root/bin/thisroot.sh" >> ~/.bashrc

# RUN wget https://root.cern/download/root_v6.24.02.Linux-ubuntu20-x86_64-gcc9.3.tar.gz && \
#     tar -xzvf root_v6.24.02.Linux-ubuntu20-x86_64-gcc9.3.tar.gz && \
#     rm root_v6.24.02.Linux-ubuntu20-x86_64-gcc9.3.tar.gz && \
#     echo "source /opt/root/bin/thisroot.sh" >> ~/.bashrc
# Use ~ as a start catalogue
WORKDIR /root


# commands to build and run it:
#docker build --tag=helix-solver-oneapi:1.0 .
#docker run --gpus all --rm -it -v$PWD:/helix -w/helix -v$HOME:$HOME --user $UID:$UID -v /etc/passwd:/etc/passwd:ro -v /etc/group:/etc/group:ro helix-solver-oneapi:1.0
