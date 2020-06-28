# DODO - a C++ convenience library {#mainpage}

## Design goals

A library to facilitate development of C++ software for the Linux platform, with a strong focus on creation
of fast and lean services for containerized deployment.

## Dependencies

  - [OpenSSL](https://www.openssl.org/)

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

See also src/examples/docker/minideb.