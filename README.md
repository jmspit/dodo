# DODO - a C++ convenience library {#mainpage}

## Design goals

Create a library with easy to use components to build software quickly whilst maintaing C++ efficiency.

Among the functionality provided:

  - Threading.
  - Networking, Secure Socket Layer
  - Configuration
  - Database
    - postgresql
    - mongodb

## Design paradigms

### Use the C++-17 standard

### Execeptions

Exceptions are to be raised only in exceptional circumstances. For example, when connection a Socket to a remote
server, and the connection fails, that should be excpected and handled by the software. On the other hand, a socket
that fails to create is an exception. Software flow control is best handled with appropriate return codes.