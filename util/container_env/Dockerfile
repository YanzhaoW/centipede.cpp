FROM fedora:latest

ENV CMAKE_GENERATOR=Ninja
ENV CC=clang
ENV CXX="clang++"

RUN yes | dnf update && yes | dnf upgrade &&\
        dnf install -y cmake tar git tbb-devel wget ninja root gh zlib-devel hostname conan &&\
        dnf install -y clang clang-tools-extra lld libcxx libcxx-static libcxx-devel libcxxabi libcxxabi-static &&\
        dnf install -y gcc gcc-c++ libgcc libstdc++-devel libstdc++-static libasan libtsan libubsan liblsan libhwasan &&\
        dnf install -y boost-devel protobuf-devel protobuf-lite-devel protobuf-compiler&&\
        dnf clean all
