# DODO - C++ framework for Docker containers
## About

Dodo is a C++ framework to facilitate development for the Linux platform that aims to ingerate with Docker containers and k8s ([kubernetes](https://kubernetes.io/)) deployments. Dodo is a response to the forgotton values of efficiency and quality - when a C++ implementaion provided more and faster functionality in 20MiB RAM than a so called 'low code' application manages to do in 2GiB.

### A skeleton for services

The dodo::common::Application reads its run-time configuration from a YAML file, typically presented to the container by k8s. Many classes in the framework can be iniltialized (constructed) by passing a YAML node as parameter. This integrates nicely with rolling deployments.

Additionally, the dodo::common::Application implitly installs the signal handlers that are trigger on Docker stop request so that the container can shut down cleanly when requested to.

Logging that can be configured to write to one or more of these targets:

  -  A directory with a configured trail (size, history) of log files.
  -  A syslog call to [rsyslog](https://www.rsyslog.com/).
  -  Console aka standard out of the container entrypoint.

### High level APIs to common functionality

Most services will require at least some of the functionality dodo provides as high-level C++ abstactaions without comrpomising low-level C/Linux performance.

  - Binary data as the Bytes datatype used by a variety of the other interfaces.
  - Encryption and compression.
  - Transparent ipv4 and ipv6 Address classes, name resolution.
  - TCPSocket (insecure) and TLSSocket classes (encryption and trust).
  - TCPServer and TLSServer classes ready for subclassing a custom protocol.
  - Protocols:
    - HTTP
    - STOMP
  - HTTP(S)Server and HTTP(S)Client to do HTTP server and client.
  - REST(S)Listener as a specialization of HTTP(S)Server where a YAML section specifies the REST API.
  - C++ APIs to persistency solutions:
    - SQLite relational database
    - PostgreSQL relational database
    - MongoDB relational database
  - Explicit persistent data stores implemented against [SQLite](https://sqlite.org/index.html)
    - Persistent key-value store
    - Persistent fifo queue.
    - Persistent Priority queue.

## Dependencies

  - [OpenSSL](https://www.openssl.org/) For TLS, hashing and encryption.
  - [SQLite](https://sqlite.org/index.html) Persistent storage.
  - [yaml-cpp](https://github.com/jbeder/yaml-cpp) Read and write YAML.
  - [rapidjson]()
  - [crc32](https://github.com/stbrumme/crc32) CRC32 verfication codes.

## Using the dodo framework

See the [developer manual](DEVELOPER.md) for an overview.

The source code is documented for Doxygen and automatically generated to github pages.

## Maintaining the dodo framework

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

To generate Doxygen API documentation, `make doc` will generate doxygen documentation
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