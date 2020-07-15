# DODO - C++ container service library {#mainpage}

## Design goals

A library to facilitate C++ software development for the Linux platform with a focus on containerized deployments.

## Dependencies

  - [OpenSSL](https://www.openssl.org/)
  - [yaml-cpp](https://github.com/jbeder/yaml-cpp)

## Developers

See the [developer manual](@ref developer) for an overview.

The source code is documented for Doxygen. To generate full library documentation, see `make doc` in
the build section below.

A shorter tour is available in

## Maintainers

See the [maintainer manual](@ref maintainer) for an overview.

## Build

```
git clone https://github.com/jmspit/dodo.git
cd dodo && \
   mkdir build && \
   cd build && \
   cmake ..  -DCMAKE_INSTALL_PREFIX=/opt/dodo && \
   make && \
   make install
```

To generate Docygen API documentation, `make doc` will generate doxygen documentation
in the build/doxygen/html directory, which contains index.html.

## Docker

A multistage build example (src/examples/docker/minideb/Dockerfile).

```
FROM bitnami/minideb:latest AS builder

RUN echo 'debconf debconf/frontend select Noninteractive' | debconf-set-selections
RUN apt-get update && \
    apt-get install -y apt-utils && \
    apt-get install -y libssl-dev git g++ cmake

RUN mkdir -p /opt/dodo/build && mkdir -p /opt/dodo/bin && mkdir -p /opt/dodo/lib
WORKDIR /opt/dodo/build
RUN git clone --branch latest https://github.com/jmspit/dodo.git
RUN cd dodo && \
    mkdir build && \
    cd build && \
    cmake ..  -DCMAKE_INSTALL_PREFIX=/opt/dodo && \
    make && \
    make install

FROM bitnami/minideb:latest
ENV LD_LIBRARY_PATH=${LD_LIBRARY_PATH}/opt/dodo/lib
ENV PATH=${PATH}:/opt/dodo/bin
RUN apt-get update && \
    apt-get install -y openssl
COPY --from=builder /opt/dodo/lib /opt/dodo/lib
COPY --from=builder /opt/dodo/bin /opt/dodo/bin
WORKDIR /opt/dodo
```