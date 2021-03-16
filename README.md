# DODO - C++ framework for Docker containers
## About

Dodo is a C++ framework to GNU/Linux development and aims to integrate seamlessly with Docker containers and k8s ([kubernetes](https://kubernetes.io/)). Capable by itself, projects can obviously add other dependenices to realize any type of service at C++ speed and resource requirements.
### A skeleton for services

The `dodo::common::Application` reads its run-time configuration from a YAML file, typically presented to the container as a [ConfigMap](https://kubernetes.io/docs/tasks/configure-pod-container/configure-pod-configmap/). Many classes in the framework can be iniltialized (constructed) by passing a YAML node as parameter.

Wether the `dodo::common::Application` is a service is up to the code - one-pass runs as well as listening servers are easily implemented.

Additionally, the `dodo::common::Application` implicitly installs signal handlers that are triggered on Docker stop requests, so that the container can shut down cleanly and quickly when requested.

Logging (`dodo::common::Logger`) can be configured to write to one or more of these targets:

  -  A directory with a configured trail (size, history) of log files.
  -  A syslog call to [rsyslog](https://www.rsyslog.com/).
  -  Console aka standard out of the container entrypoint.

Docker healthchecks that run an in-container command (aka native healthchecks, such as say `pidof myservice`) are pretty costly, especially if the healthchecks need to be frequent or there are a lot of pods to healthcheck. The dodo::common::Application class can be instructed to setup a healthceck listener (dodo::common::Application::HealthChecker) that can be network-probed by the Docker host, which is much more efficient. This HealthChecker can be declared sick or healthy by the code at runtime.
### High level APIs to common functionality

Most services will require at least some of the functionality dodo provides as high-level C++ abstractions without compromising low-level C/Linux performance.

  - Binary data as the `dodo::common::Bytes` datatype used by a variety of the other interfaces.
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
  - [rapidjson]() Read and write JSON.
  - [crc32](https://github.com/stbrumme/crc32) CRC32 verfication codes (dependency included in the source tree).

## Using the dodo framework

See the [developer manual](DEVELOPER.md) for an overview.

The source code is documented for Doxygen and [automatically generated to github pages](https://jmspit.github.io/dodo/).

## Maintaining the dodo framework

See the [maintainer manual](@ref maintainer) for an overview.

## Build

```
git clone https://github.com/jmspit/dodo.git
cd dodo && \
   mkdir build && \
   cd build && \
   cmake ..  -DCMAKE_INSTALL_PREFIX=/opt/dodo && \
   cmake --build . && \
   cmake --install .
```
If installed to a location outside the library paths the link-loader expects (eg usr/lib and so on), the `LD_LIBRARY_PATH` can be amended with `CMAKE_INSTALL_PREFIX/lib`.

To generate Doxygen API documentation, `make doc` will generate doxygen documentation in the build/doxygen/html directory, which contains an index.html.

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