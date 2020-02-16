# Based on opencv-alpine by Juliano Petronetto (MIT Licence)

FROM alpine:3.11
ENV LANG=C.UTF-8

RUN apk update && apk upgrade && apk --no-cache add \
  binutils \
  gcc \
  g++ \
  make \
  cmake \
  coreutils \
  freetype-dev \
  git \
  libc-dev \
  libffi-dev \
  libpng-dev \
  linux-headers \
  lapack-dev \
  openblas-dev \
  unzip \
  zlib-dev \
  freeglut-dev

RUN cd /opt && \
  wget https://github.com/opencv/opencv/archive/3.2.0.zip && \
  unzip 3.2.0.zip && rm 3.2.0.zip && \
  cd opencv-3.2.0 && mkdir build && cd build && \
  cmake -D CMAKE_BUILD_TYPE=RELEASE \
    -D CMAKE_INSTALL_PREFIX=/usr/local \
    -D INSTALL_C_EXAMPLES=OFF \
    .. && \
  make -j$(nproc) && \
  make install && \
  rm -rf /opt/opencv-3.2.0

RUN cd /opt && \
  wget https://downloads.sourceforge.net/project/arma/armadillo-9.850.1.tar.xz && \
  tar xvf armadillo-9.850.1.tar.xz && \
  rm armadillo-9.850.1.tar.xz && \
  cd armadillo-9.850.1 && \
  mkdir build && \
  cd build && \
  cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_INSTALL_LIBDIR=/usr/local/lib && \
  make -j$(nproc) && \
  make install && \
  rm -rf /opt/armadillo-9.850.1

RUN cd /opt && \
  wget https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.10/hdf5-1.10.6/src/hdf5-1.10.6.tar.gz && \
  tar xvf hdf5-1.10.6.tar.gz && rm hdf5-1.10.6.tar.gz && \
  cd hdf5-1.10.6 && \
  mkdir build && \
  cd build && \
  cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local && \
  make -j$(nproc) && \
  make install && \
  cp /usr/local/lib/pkgconfig/hdf5-1.10.6.pc /usr/local/lib/pkgconfig/hdf5.pc && \
  rm -rf /opt/hdf5-1.10.6

RUN cd /opt && \
  git clone https://github.com/open-source-parsers/jsoncpp.git && \
  cd jsoncpp && \
  git checkout 3beb37ea14aec1bdce1a6d542dc464d00f4a6cec && \
  mkdir build && \
  cd build && \
  cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_INSTALL_LIBDIR=/usr/local/lib && \
  make -j$(nproc) && \
  make install && \
  rm -rf /opt/jsoncpp

#RUN apk add --no-cache libxrandr-dev libxinerama-dev libxcursor-dev && cd /opt && \
#  git clone https://github.com/glfw/glfw.git && \
#  cd glfw && \
#  git checkout 6aca3e99f03617df1dd9081f20bd5408baf2a5d6 && \
#  mkdir build && \
#  cd build && \
#  cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_INSTALL_LIBDIR=/usr/local/lib && \
#  make && \
#  make install && \
#  rm -rf /opt/glfw

RUN cd /opt && \
  git clone https://github.com/ABRG-Models/morphologica.git && \
  cd morphologica && \
  mkdir build && \
  cd build && \
  cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local && \
  make -j$(nproc) && \
  #ctest && \
  make install && \
  cp /usr/lib/pkgconfig/openblas.pc /usr/lib/pkgconfig/blas.pc && \
  rm -rf /opt/morphologica
