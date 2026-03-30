FROM fedora:43

ENV CMAKE_GENERATOR=Ninja
ENV CC=clang
ENV CXX="clang++"

RUN yes | dnf update && yes | dnf upgrade &&\
        dnf install -y cmake which tar git tbb-devel wget ninja root gh zlib-devel hostname conan gcovr &&\
        dnf install -y clang clang-tools-extra lld libcxx libcxx-static libcxx-devel libcxxabi libcxxabi-static &&\
        dnf install -y gcc gcc-c++ libgcc libstdc++-devel libstdc++-static libasan libtsan libubsan liblsan libhwasan &&\
        dnf install -y boost-devel protobuf-devel protobuf-lite-devel protobuf-compiler&&\
        dnf clean all

LABEL org.opencontainers.image.source=https://github.com/YanzhaoW/centipede.cpp
LABEL org.opencontainers.image.description="Testing and CI environment for centipede.cpp program"
LABEL org.opencontainers.image.licenses=MIT
