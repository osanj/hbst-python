FROM quay.io/pypa/manylinux1_x86_64:2021-12-11-951973d

# from https://github.com/opencv/opencv-python/blob/fd3bf8810e1cb9a2d14ded39e799dbf0ff931371/docker/manylinux1/Dockerfile_x86_64#L22-L35
RUN cd /opt && \
    curl -O -L https://cmake.org/files/v3.9/cmake-3.9.0.tar.gz && \
    tar -xf cmake-3.9.0.tar.gz && \
    cd cmake-3.9.0 && \
    #manylinux1 provides curl-devel equivalent and libcurl statically linked
    # against the same newer OpenSSL as other source-built tools
    # (1.0.2s as of this writing)
    yum -y install zlib-devel && \
    #configure does a bootstrap make under the hood
    export MAKEFLAGS=-j$(getconf _NPROCESSORS_ONLN) && \
    ./configure --system-curl && \
    make && \
    make install && \
    cd .. && \
    rm -rf cmake-3.9.0*
