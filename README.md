[![Maintenance](https://img.shields.io/badge/Under%20construction-yes-red.svg)](https://bitbucket.org/lbesson/ansi-colors)
[![Maintenance](https://img.shields.io/badge/Pet%20project-yes-red.svg)](https://bitbucket.org/lbesson/ansi-colors)


[TOC]

# DODO - C++ framework for Docker applications
## About

Dodo is a C++ framework to integrate seamlessly with Docker containers and k8s ([kubernetes](https://kubernetes.io/)), hiding much of the red tape, as well as providing frameworks for typical demands such as DNS resolving, TLS-enabled TCP services, REST services, database accces and so on.

C++ offers high capability/requirement ratio, perfect for microservices.

Dodo applies c++17 features. It relies on and assumes the STL by using STL primitives such as std::string in interfaces. A dodo install comprises a bunch of header files and a shared library
to which applications or 'services' are linked.

This is a pet project. Do not take it too seriously.
### A skeleton for container services

The `dodo::common::Application` reads its run-time configuration from a YAML file, typically presented to the container as a [ConfigMap](https://kubernetes.io/docs/tasks/configure-pod-container/configure-pod-configmap/). Framework objects can be iniltialized (constructed) with a YAML document fragment, so the application can retrieve its runtime configuration from a single source.

An `dodo::common::Application` instance implicitly installs signal handlers that are triggered on container stop requests, allowing the application to shutdown cleanly.

Logging (`dodo::common::Logger`) is implicit and can be configured to write to one or more of these targets:

  -  A directory with a configured trail (size, history) of log files.
  -  A syslog call to [rsyslog](https://www.rsyslog.com/).
  -  Console aka standard out of the container entrypoint.

Docker healthchecks that run an in-container command (such as `pidof myservice`) are expensive on CPU, especially if the healthchecks need to be frequent or there are a lot of pods. The dodo::common::Application class starts a tiny TCP healthcheck listener (dodo::common::Application::HealthChecker) that responds to network-probes by the Docker host, which is much more efficient. The HealthChecker stops reponding when the Application exits.
### High level APIs to common functionality

Most services will require at least some of the functionality dodo provides as high-level C++ abstractions without compromising low-level C/Linux performance.

  - Binary data as the `dodo::common::Bytes` datatype used as primitive by dodo interfaces.
  - Encryption and compression.
  - Transparent ipv4 and ipv6 Address classes, name resolution.
  - TCPSocket (insecure) and TLSSocket classes (encryption and trust).
  - TCPServer and TLSServer classes ready for subclassing a custom protocol.
  - Standard protocols:
    - HTTP
    - STOMP
  - HTTP(S)Server and HTTP(S)Client handling HTTP server and client red tape.
  - REST(S)Listener as a specialization of HTTP(S)Server where a YAML section specifies the REST API.
  - C++ APIs to persistency solutions:
    - SQLite relational database
    - PostgreSQL relational database
    - MongoDB object store
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

See the [maintainer manual](MAINTAINER.md) for an overview.

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
If installed to a location outside the system library paths, set the `LD_LIBRARY_PATH` to `<CMAKE_INSTALL_PREFIX>/lib`.

### Build targets

In the configured build directory

| target | build command | What |
|--------|---------------|------|
| all    | `cmake --build .` | builds the dodo library, examples and tests |
| dodo    | `cmake --build . --target dodo` | Only builds `libdodo.so` |
| doc    | `cmake --build . --target doc` | Creates the doxygen documentation in `./doxygen/html`. |
| cppcheck | `cmake --build . --target cppcheck` | Run cppcheck code quality check |

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