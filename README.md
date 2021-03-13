# DODO - C++ framework for Docker containers
## Design goals

A framework to facilitate C++ software development for the Linux platform with a focus on k8s ([kubernetes](https://kubernetes.io/)) deployments of [Docker](https://www.docker.com/) containers. By using C++ one produces Docker containers that are memory and CPU friendly, by using this framework a lot of red tape is automated and simplified.

  - The dodo::common::Application reads its run-time configuration from a single (or more) YAML file presented to the container by k8s. Many classes in the framework can be constructed by handing a YAML node as parameter. This integrates seamlessly with the k8s concept of a deployment and rolling deployments.
  - Implicit signal handling by the Application allowing k8s to reliably operate the container.
  - Standard logging that can be configured to write to one or more of these targets:
    -  A directory with a configured trail (size, history) of log files.
    -  A syslog call to [rsyslog](https://www.rsyslog.com/).
    -  Console aka standard out of the container entrypoint.
  - Binary data as the Bytes datatype used by a variety of interfaces.
  - Encryption and compression.
  - Transparent ipv4 and ipv6 Address classes, name resolution and networking.
  - TCPSocket (insecure) and TLSSocket classes (encryption and trust).
  - TCPServer and TLSServer classes ready for custom protocol implementation.
  - HTTP(S)Server and HTTP(S)Client.
  - REST(S)Listener is a specialization of HTTP(S)Server
  - Persistent data stores based on [SQLite](https://sqlite.org/index.html)
    - Key-value store.
    - Persistent FIFO queue.
    - Persistent Priority queue.

## Dependencies

  - [OpenSSL](https://www.openssl.org/) For TLS, hashing and encryption.
  - [SQLite](https://sqlite.org/index.html) Persistent storage.
  - [yaml-cpp](https://github.com/jbeder/yaml-cpp) Read and write YAML.
  - [rapidjson]()
  - [crc32](https://github.com/stbrumme/crc32) CRC32 verfication codes.

## Developers

See the [developer manual](DEVELOPER.md) for an overview.

The source code is documented for Doxygen and automatically generated to github pages.

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