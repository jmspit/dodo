# DODO - a C++ convenience library {#mainpage}

[TOC]

## Design goals

A library to facilitate development of C++ software for the Linux platform, with a strong focus on creation
of fast and lean services for containerized deployment - aka picoservices.

  - Cryptography
    - Encrypt and decrypt binary and string data.
    - Support algorithms backing authentication schemes.
  - Data stores
    - Relational
      - Postgresql
      - SQLite
    - Object
      - MongoDB
    - Streams
      - Apache AVRO
      - Apache Kafka
  - Runtime configuration
    - SQLite files, hierarchical keys
    - modifcation time allows the runtime to pick up changes without restarting
    - Cryptography allows to encrypt sensitive (or all) configuration data.
  - [(Secure) Socket programming](@ref developer_networking)
  - Logging (stdout, file, logstash, any plugin implementation)

  - Layer 4 assistence (HTTP,HTTPS)
  - Network layer 3 service interface classes ready for protocol implementation (TCP sockets and secure sockets).
  - Network layer 4 service interface classes ready for implementation (HTTP/HTTPS).

  - Serialization/Deserialization (JSON, CBOR)



## Design paradigms

  - **Preferably use the STL** - Where possible, use the STL. Avoid the use of `using namespace std` and
  `using std::string`, use fully qualified names.
  - **Use C++17 features** - Use modern C++17.
  - **Throw Exceptions only in exceptional conditions** - A connection failure is not an Execption but the type of
  error software should expect and handle. Exceptions should not be used for flow control.

## Dependencies

  - [OpenSSL for TLS](https://www.openssl.org/)

### Optional dependencies

  - DODO_PROVIDE_SQLITE3 depends on [SQLite](https://www.sqlite.org/index.html)