# Developer Manual {#developer}

[TOC]

# Introduction

Framework to create services with C++ efficiency for deployment in Docker containers.

- Target is GNU/Linux exclusively.
- The Application class handles red tape
  - Runtime configuration through yaml file
  - Logging to one or more destinations, console, (rotating) files, syslog.
  - Handling signals (to the container entrypoint, including stop requests)
- Frameworks for typical demands such as
  - Network and TLS network sockets
  - TCP servers both with and without TLS support.
  - HTTP servers both with and without TLS support.
  - REST servers both with and without TLS support.
  - Persistent Key-Value store based on SQLLite.
- Framework classes can be initialized against YAML nodes that define the runtime configuration, a YAML node which may be in the same file that defines the Application as well as other classes.

# Using dodo

Include all headers by including dodo.hpp. Before using any of the dodo functionality, call dodo::initLibrary(). Call dodo::closeLibrary() to clean up before program termination.

```C
#include <dodo.hpp>

using namespace dodo;

int main( int argc, char* argv[] ) {
  int return_code = 0;
  initLibrary();
  try {
    network::Address localhost = "127.0.0.1";
    ...
  }
  catch ( std::exception ) {
    cerr << e.what() << endl;
    return_code = 1;
  }
  closeLibrary();
  return return_code;
}
```

Or use the dodo::common::Application class, as that will take care of

  - initializing / unloading the dodo library
  - install signal handlers
  - parse command line arguments, read environment variables
  - read configuration data from the specified configuration file.
  - logging

Subclass Application and implement the run method

```C
#include <dodo.hpp>

using namespace dodo;

class MyApp : public common::Application {
  public:

    MyApp( const StartParameters &param ) : common::Application( param ) {}

    virtual int run() {
      while ( !hasStopRequest() ) {
        cout << "Hello world!" << endl;
        std::this_thread::sleep_for(2s);
      }
      return 0;
    }

};

int main( int argc, char* argv[], char** envp ) {
  try {
    MyApp app( { "conf/myapp.yaml", argc, argv, envp } );
    return app.run();
  }
  catch ( const std::exception &e ) {
    cerr << e.what() << endl;
    return 1;
  }
}
```


# Exception and error handling

Dodo throws dodo::common::Exception (or its descendant dodo::common::SystemException) only in exceptional circumstances.
Methods that might be expected to fail return a SystemError, a mapping of of system and internal dodo errors which are more convenient in writing program flows.

Among the exceptional error conditions that will *throw* are

  - specifying an non-existing configuration file
  - loading an invalid private key
  - unable to allocate memory

whilst dodo::common::SystemError is *returned* when, for example,

  - a fqdn fails to resolve
  - send / receive timeouts

As dodo::common::Exception itself descends from std::runtime_error, which in turn descends from std::exception, a try / catch block such as

```C
try {
  ...
}
catch ( const std::exception &e ) {
  cerr << e.what() << endl;
}
```

will also catch any dod exception. dodo::common::Exception instances include the file and line number where the Exception was thrown. Developers may use the throw_Exception() macro's that are used by dodo internally.

The dodo::common::SystemError is declared `[[nodiscard]]` so that the compiler issues a warning if any function returns a
dodo::common::SystemError that is ignored.

# Deployment configuration

The dodo::common::Config interface enforces the use of a YAML configuration file with mandatory keys that configure mandatory configuration items

```YAML
dodo:
  common:
    application:
      name: myapp
    logger:
      console:
        level: info
```

A dodo Application configuration requires at least a name (myapp) and a dodo::common::Logger::LogLevel (info) for logging to console or
standard out.

The dodo mandatory key tree can appear anywhere in the YAML as long as it is a root node. Configuration formats of other dodo components are described, below, but may be present in the same file.

# Logging

A logging framework is available to write log entries to one or more destinations. The available destinations are

  - standard output aka the console of the container (assuming the Application is the entrypoint)
  - a directory with a file rotation policy
  - the syslog

The logging interface is thread-safe.

The dodo::common::Logger is configured through the dodo::common::Config configuration file. Minimally, the Logger
requires

```YAML
dodo:
  common:
    logger:
      console:
        level: info
```

which specifies that log entries should be sent to the console (standard out) if they are level
dodo::common::Logger::LogLevel::Info or worse.

The other destinations are file and syslog. The logger writes entries to all destinations specified (so console logging cannot be disabled). The file destination requires directory and trailing limits specifications as the log files are rotated automatically. `max-size-mib` (the maximum size of a file before rotate) and `max-file-trail` (the maximum number of already rotated log files to keep, oldest removed if exceeded) may be omitted, in which case
they have the values as specified below. The directory must be specified and may be relative to the current working
directory.

```YAML
dodo:
  common:
    logger:                       # mandatory
      console:                    # mandatory
        level: info               # mandatory
      file:                       # optional
        level: info               # mandatory
        directory: log            # mandatory
        max-size-mib: 10          # optional default 10
        max-file-trail: 4         # optional default 4
```

So the space needed for a logging config is `max-size-mib` * ( `max-file-trail` + 1 ) MiB.

The dodo::common::Logger::LogLevel levels can be specified in the configuration in either a short or longer
format, but the shorter format will be used in log entries written to console or file. So `level: fatal` and
`level: FAT` are equivalent.

|short|long|syslog mapping| Release build |
|-----|----|--------------|---------------|
| `FAT` | `fatal` | 2 CRITICAL | Included in code |
| `ERR` | `error` | 3 ERROR | Included in code |
| `WRN` | `warning` | 4 WARNING | Included in code |
| `INF` | `info` | 6 INFORMATIONAL | Included in code |
| `STA` | `statistics` | 6 INFORMATIONAL | Included in code |
| `DBG` | `debug` | not logged | Excluded from code |
| `TRC` | `trace` | not logged | Excluded from code |

So it is not possible to debug or trace to the syslog, and debug and trace loglevels have no effect in release builds.

To log messages in the code, use the log_Fatal through log_Trace macros (all include an implicit Puts() so streaming
syntax can be used to easily log different data-types). For example

```C
  log_Debug( "connection from " << address.AsString() );
  log_Error( "receive timeout on socket " << socket.getFD() );
```

In release builds (`-DNDEBUG`) the log_Debug and log_Trace macros reduce to void and are thus eliminated from the code, and the
above fragment would reduce to

```C
  log_Error( "receive timeout on socket " << socket.getFD() );
```


To configure the syslog destination

```YAML
dodo:
  common:
    logger:                       # mandatory
      console:                    # mandatory
        level: info               # mandatory
      syslog:                     # mandatory
        level: warning            # mandatory
        facility: 1               # optional default 1
```

The syslog facility should be either 1 for `user-level messages` or in the range `local0` to `local7` or 16 through 23
as the remaining values are reserved for other use.


# Networking

The networking framework is provided by the dodo::network namespace.

## Address

Sockets connect to or listen on Address objects. The Address class allow to program for both ipv4 and ipv6
transparently. For example, the following sequence resolves the name "httpbin.org" to either an ipv4 or ipv6 address,
whatever the server and client support.

```C
using namespace dodo;

network::SocketParams sock_params = network::SocketParams( network::SocketParams::afUNSPEC,
                                                           network::SocketParams::stSTREAM,
                                                           network::SocketParams::pnTCP );
std::string canonicalname;
std::string host = "httpbin.org";
network::Address address;
common::SystemError error = network::Address::getHostAddrInfo( host, sock_params, address, canonicalname );
```

# Sockets

All sockets descend from dodo::network::BaseSocket.

## Secure sockets {#developer_networking}

Dodo supports TLS through dodo::network::TLSContext and dodo::network::TLSSocket.


### Asymmetric cryptography

![Test Image 4](https://raw.githubusercontent.com/jmspit/dodo/master/src/doc/img/asymmetric_key_encryption.png)

The core principle to asymmetric cryptography is a mathematical mapping that is cheap to
compute in one direction, and computationally infeasible in the other. Typically, the ease of muliplying primes
and the difficulty in finding prime roots of integers is used.

In an asymmetric communication handshake, each endpoint has a private key, a *secret* filled with as much entropy
(randomness) as possible, typically stored as a file which is (can be) in turn encrypted and protected by a passphrase.
The private key is and must not be shared with anyone.

The private key includes public bits that map uniquely to a public key, which can thus be extracted from the
private key. The private and its public key share the same *modulus*, so a public key can be matched to a private key
and other artifacts containing that public key, such as a certificate or certificate signing request.

In secure communication, the public key encrypts and the private key decrypts. So data encrypted with a public key,
can only be decrypted by the owner of the matching private key.

In digital signing, the signature is encrypted with the private key of the signer, and decrypted with
the public key of the signer.

### Transport Layer Security (TLS)

TLS is a secure communication protocol that use an asymmetric cryptographic handshake to negotiate and share a session
key, which will be used for both encryption and decryption.

In TLS configuration and deployment the following document types are used (see dodo::network::X509Common::X509Type)

  - The private key.
  - A *certificate signing request* (CSR) which is a X.509 document
    (dodo::network::X509CertificateSigningRequest), typically stored as a PEM file, and comprises
    - the public key
    - data describing the owner identity (often the FQDN)
    - defined limits to its validty (such as expiry by date)
  - A *certificate*, which is a X.509 document
    (dodo::network::X509Certificate) typically stored as a PEM file, and comprises
    - the public key as in the CSR.
    - data describing the owner as in the CSR.
    - The digital signature of a Certificate Authority (CA) that has verified the CSR to be correct (the FQDN belongs to the owner
      described in the data).

A CA is a chain of certificates anchored to a trust anchor, which is a self-signed certificate. Such trust anchors
typically create intermediate certificates which are used to sign CSR's. The trust entails that the CA has established
that the private key belongs to the identity specified in the certificate with matching public key.

Operating systems or software suites are often deployed with well known and widely trusted CA's that verify identity
against network names.

The context of a TLS connection is handled by the dodo::network::TLSContext class. It is configured with a private
key, a certificate and a truststore. The same TLSContext can be used by multiple dodo::network::TLSSocket objects for
multiple connections.

TLS evolved through protocol versions, version 1.0 is deprecated (as are its predecessors, SSLv2 and SSLv3).

  - TLS 1.0 : Deprecated and not supported
  - TLS 1.1 : Deprecated but still supported
  - TLS 1.2 : Widely used
  - TLS 1.3 : Best

Dodo allows to specify the lowest acceptable TLS version:

  - dodo::network::TLSContext::TLSVersion::tls1_1 Accept TLS 1.1 or better
  - dodo::network::TLSContext::TLSVersion::tls1_2 Accept TLS 1.2 or better
  - dodo::network::TLSContext::TLSVersion::tls1_3 Accept TLS 1.3 or better

In order to verify a peer certificate, there is dodo::network::TLSContext::PeerVerification

  - dodo::network::TLSContext::PeerVerification::pvVerifyNone - Use no peer verification. The connection is private
    (encrypted), but information is shared with an unknown peer identity (like a man in the middle).
  - dodo::network::TLSContext::PeerVerification::pvVerifyPeer - The client verifies that peer offers a certificate that
    is signed by an entity trusted by the client. Note that this merely proves the server has a copy of a trusted
    certificate - not the the server is the identity specified by the certificate.
  - dodo::network::TLSContext::PeerVerification::pvVerifyFQDN - In addition to pvVerifyPeer, the client verifies that
    the target FQDN of the server either matches its CN (common name) or one of its
    [SAN](https://en.wikipedia.org/wiki/Subject_Alternative_Name) entries.
  - dodo::network::TLSContext::PeerVerification::pvVerifyCustom - In addition to pvVerifyPeer, the developer provides
    custom verification of the peer certificate.

#### Trust stores

Operating systems are typically configured with a truststore, on this Linux box a directory
with certificates:

```
$ ls -1 /etc/ssl/certs/*Nederland*
/etc/ssl/certs/Staat_der_Nederlanden_EV_Root_CA.pem
/etc/ssl/certs/Staat_der_Nederlanden_Root_CA_-_G2.pem
/etc/ssl/certs/Staat_der_Nederlanden_Root_CA_-_G3.pem
```

EV is short for [Extended Validation Certificate](https://en.wikipedia.org/wiki/Extended_Validation_Certificate),
which certifies that the legal identity of the certificate's owner has been verified.

Certificates can be inspected with

```
$ openssl x509 -in /etc/ssl/certs/Staat_der_Nederlanden_EV_Root_CA.pem -text -noout
Certificate:
    Data:
        Version: 3 (0x2)
        Serial Number: 10000013 (0x98968d)
        Signature Algorithm: sha256WithRSAEncryption
        Issuer: C = NL, O = Staat der Nederlanden, CN = Staat der Nederlanden EV Root CA
        Validity
            Not Before: Dec  8 11:19:29 2010 GMT
            Not After : Dec  8 11:10:28 2022 GMT
        Subject: C = NL, O = Staat der Nederlanden, CN = Staat der Nederlanden EV Root CA
        Subject Public Key Info:
            Public Key Algorithm: rsaEncryption
                RSA Public-Key: (4096 bit)
                Modulus:
                  ...
```

The modulus is shared between the private and public key (which is included in the certificate) and allows to match
public keys and private keys as pairs (equality is inferred from matching sha256 hashes in this example)

```

# for a private key
$ openssl rsa -noout -modulus -in server_private.pem | openssl sha256
(stdin)= 6158bf2941f9f2bd10d35f254cfa035f2c33bd36412beaf8427e51c99290b451

# for a public key
$ openssl rsa -noout -modulus -pubin -in server_public.pem  | openssl sha256
(stdin)= 6158bf2941f9f2bd10d35f254cfa035f2c33bd36412beaf8427e51c99290b451

# for a CSR
$ openssl req -noout -modulus -in server_csr.pem | openssl sha256
(stdin)= 6158bf2941f9f2bd10d35f254cfa035f2c33bd36412beaf8427e51c99290b451

# for a public key certificate
$ openssl x509 -noout -modulus -in certificate.pem | openssl sha256
(stdin)= 6158bf2941f9f2bd10d35f254cfa035f2c33bd36412beaf8427e51c99290b451
```

the output of the above commands proves that these public key artifacts all originate in the same private key.

Instead of the system trust store, a custom truststore can be used to fine-tune the trust. This allows to either limit
the trust or trust one's own CA - bypassing the need to buy the certificate produced from a CSR.

Custom truststores are loaded from PCKS12 files through dodo::network::TLSContext::loadPKCS12TrustStore.

#### Keystore

A keystore comrpises the private key and the (signed) certificate of the identity. It can be loaded with either
dodo::network::TLSContext::loadPEMIdentity or dodo::network::TLSContext::loadPKCS12KeyStore.

#### Server Name Indication

[Server Name Indication](https://en.wikipedia.org/wiki/Server_Name_Indication) is a TLS extension that
sends the client-intended target FQDN to allow the server to present the correct certificate - a certificate to
which that target FQDN matches either the CN or one of the SANs.

Currently, SNI sends the target FQDN unencrypted, which means intermediates know the destination (and can act
upon it, such as censoring). SNI seems to be required only by a minority of HTTPS servers, and due to its downsides,

SNI is enabled only if explicitly specified in the dodo::network::TLSContext::TLSContext constructor.


### Setup Certification Authority

The source tree contains a simplistic CA install that allows to operate as a root CA. The installation
uses intermediate CA to do the actual signing. The installation can create signed server and client certificates
to be shared with their Subjects (including private key and passphrase). Alternatively, external identities
can provide a CSR that can be signed - ot nor.

The root CA install is protected by a single master passphrase. The root CA and intermediate CA private keys are
protected with a individual passphrases, which are stored as a (readonly) files, each protected by the master
passphrase. No readable secrets are stored. It is crucial that the master passphrase is strong and is remembered or
safely kept - once it is lost the install is lost as well.

The scripts to install are part of the source, under `src/examples/tls/artefacts`. This install uses the confd tool,
so that must be present in the PATH, as well as the openssl bianry.

Copy `config.yaml` and customize the copy. The `ca::root::directory` entry specifies the installation directory
for the root CA directory tree. The `create-ca.sh` script will ask for the master passphrase.

Passphrases are passed to the openssl tools through ENV vars, and ENV vars of any user are readable to the root user,
at least during the time they exist (only when installed scripts are running), so one must either be root or trust root
if any serious use is considered.


```
$ cp -p config.yaml myconfig.yaml
$ vi myconfig.yaml
$ ./configure.sh myconfig.yaml
$ ./ca_config/bin/create-ca.sh
```

The `ca::root::directory` from `myconfig.yaml` now contains the root CA with a self-signed certificate. Now create an
intermediate CA. Intermediate CA's are used to sign CSR's. Specify a descriptive name for the intermediate CA.

```
$ cd [ca::root::directory]
$ bin/create-intermediate.sh
```

the intermediate CA is now created, along with its certificate that is signed by the root CA created earlier.

Create a server identity (private key + passphrase, signing request). Note that the common name is expected to be the
FQDN of the server, but SAN (subject altnames) can be specified to widen the certificate's applicability. Examples
of SAN entries are

  - DNS:our.domain.org
  - IP: 127.0.0.1
  - IP: ::1

```
$ bin/create-server.sh
```

Alternatively, the subject can create its own certificate, and just send a CSR. In case a CSR is received, best put it
in `ca::root::directory/ext/servers` or `ca::root::directory/ext/servers` depending on the certificate's intended use.

Either way, the CSR can now be signed:

```
$ intermediates/<intermediatename>/bin/sign-server-csr.sh <CSR file>
```

which will produce two files:

  - `ca/root/ext/servers|clients/<commonname>.cert.pem`
  - `ca/root/ext/servers|clients/<commonname>.pkcs12`

the former is the signed certificate, and the latter a PKCS12 file containing the signed certificate and
the trust chain, both the intermediate and root certificates used to sign ().



`<server commonname>.cert.pem` to be returned to the server identity.

```
$ openssl s_server -accept 12345 -cert ca/root/ext/servers/localhost.cert.pem -CAfile ca/root/certs/ca.cert.pem -chainCAfile ca/root/intermediates/signer1/certs/ca-chain.cert.pem -key ca/root/ext/servers/localhost.key.pem
$ openssl s_client -showcerts -connect localhost:12345 -CAfile /etc/ssl/certs/dodo.pem
```

# TCPListener / TCPServer

Developers create their own network protocols on top of TCP by sub-classing dodo::network::TCPServer and implement a
request (reading) and response (handling + sending) mechanism of arbitrary complexity. The dodo::network::TCPListener
is a dodo::threads::Thread, and once started will

  - listen and accept TCP handshakes
  - listen for socket events (data to be read,hangup,error to handle) and produces it as work to the pool
    of TCPServer instances.
  - manage the pool of TCPServer worker threads, scale the number of TCPServer instances between a minimum and maximum
    depending on the amount of work queued for processing
  - throttle the production of work by delaying (throttling) the TCPListener reading socket events, if the queue depth
    exceed its maximum and the maximum number of TCPServer have already been started.

As the dodo::network::TCPServer and dodo::network::TCPListener interfaces use dodo::network::BaseSocket,
both do not depend on the socket being a regular or a TLS-secured socket.

The dodo::network::TCPListener uses the epoll_wait interface to wait for and read socket events in its run()
method, which is executed a dedicated thread. Only if there is socket activity (new connections, data to be read,
errors, shutdowns) will the dodo::network::TCPListener wake up, and read `pollbatch` socket events in one go (instead
of waking up on each individual socket event). The dodo::network::TCPListener does not read data, it merely signals
the pool of dodo::network::TCPServer there is data to be read through a std::condition_variable on which the TCPServer
pool is sleeping.

The request-response paradigm comprises these steps

  - A protocol handshake (not the TCP handshake, that will already have taken place), for example to verify clients
    speak the correct protocol or setup state-full data for the remainder of the connection. The handshake is implemented
    by overriding the virtual bool dodo::network::TCPServer::handShake(), which should be returning false if the handshake fails,
    in which case the dodo::network::TCPListener will clean up.
  - Zero or more request-response cyles by overriding dodo::network::TCPServer::readSocket(). Note that the
    request data sent must be fully read within a single readSocket cycle, but if required a protocol can be
    created that reads across several requests (which would only serve a point if local RAM needs to be protected against
    very large request by applying streaming).
  - A shutdown when the peer hangs up or the socket is/must be terminated due to errors. Override
    dodo::network::TCPServer::shutDown() when the dodo::network::TCPServer needs to clean things up.

Additionally, the devloper will need to override dodo::network::TCPServer::addServer() to provide a new instance
of a dodo::network::TCPServer descendant so that dodo::network::TCPListner can spawn new servers.

The dodo::network::TCPListener has some configuration parameters that define its runtime behavior, which
are bundled in the dodo::network::TCPListener::Params struct. These parameters can either be specified by C++ code
or handed to a TCPListner instance by specifying a Config YAML node (which may thus appear anywhere in the YAML file).
In the below example, the myapp.tcplistener node would be passed.

```YAML
myapp:
  tcplistener:
    listen-address: 0.0.0.0
    listen-port: 1968
    scaling:
      min-servers: 2
      max-servers: 16
    sizing:
      max-connections: 5000
      send-buffer: 16384
      receive-buffer: 32768
    timeouts:
      send: 10
      receive: 10
```

| name | dataype | default | purpose |
|------|---------|---------|---------|
| `listen-address` | string | `0.0.0.0` | The address to listen on, may be a local ipv4 or an ipv6 address. `0.0.0.0` (ipv4) and `::` (ipv6) imply IN_ADDRANY and will listen on all network interfaces. |
| `listen-port` | unsigned int | `1968` | The port to listen on. |
| `min-servers` | unsigned int | `2` | The minimum TCPServer threads in the pool. |
| `max-servers` | unsigned int | `16` | The maximum TCPServer threads in the pool. |
| `max-connections` | unsigned int | `1000` | The maximum number of connections (10-60000). |
| `max-queue-depth` | unsigned int | `128` | The maximum number of queued work items before throttling. |
| `send-buffer` | unsigned int | `16384` | The size of the send buffer for each socket. |
| `receive-buffer` | unsigned int | `32768` | The size of the receive buffer for each socket. |
| `server-idle-ttl-s` | unsigned int | `300` | The number of second a TCPServer is allowed to be idle before scaling down (if # TCPServers > min-severs). |
| `poll-batch` | unsigned int | `128` | The number of socket events transformed into TCPServer work in a single cycle. |
| `listener-sleep-ms` | unsigned int | `1000` | The number of milliseconds to wait for socket events per TCPListener iteration before re-looping, allowing to check if the TCPListener is requested to stop. |
| `throttle-sleep-us` | unsigned int | `4000` | The number of microseconds to stall the TCPListener in case max-queue-depth is reached. |
| `cycle-max-throttles` | unsigned int | `40` | The maximum number of throttles per listener-sleep-ms cycle. |
| `stat-trc-interval-s` | unsigned int | `300` | The number of seconds between writing status/performance messages to the log. |
| `tcp-keep-alive` | bool | `false` | If true, enable TCP keep alive on client sockets. |


To highlight the purpose and effect of the parameters, the TCPListener loop roughly iterates as

```BASH
while ( ! stopped ) {
  throttle if needed, at max cycle-max-throttles times throttle-sleep-us microseconds
  check if servers need to be added (when the queue size is not shrinking)
  wait-sleep listener-sleep-ms for 1 to poll-batch socket events
  if ( !timeout ) {
    for new-connection events, setup the sockets, queue the handshake
    for existing connections with events, queue the readSocket cycle
  }
```


The implicit throttling mechanism of the TCPListener protects the TCPServer pool against overloading, and will
queue client data in the hosts receive buffers initially, and if they are full, clients will start to experience
send latency up until their send timeout values. So the TCPListener will seek to maximize the sustained
arrival rate of work against the configured capacity of the TCPServer pool, although, as the receive buffers
and request queue size permit, it can handle intermediate burst that exceed it without additional latency.


# KVStore

The dodo::persist::KVStore class is a simple key-value store backed by SQLite. Under multithreading, if each thread uses its own KVStore
object - even though they all point to the same SQLite file - no explicit synchronization is required.

  - The SQLite database (file) is initialized in WAL mode for performance.
  - All key-value pairs are stored in a single table (kvstore).
  - The value column can hold any datatype that SQLite supports.
  - The modified column stores the (unix timestamp) of the last modification.
  - The updates column stores the total number of times the value was updated (after first insert this is 0).

Suppose we require the hostname of a proxy server.

```C
dodo::persist::KVStore store( "cm.db" );
std::string proxy_server = store.ensureWidthDefault( "proxy-server", "proxy.domain.nl" );
```

If the key `proxy-server` did not exist, it is now set to `"proxy.domain.nl"` in the store and that value is returned by dodo::persist::KVStore::ensureWithDefault. If it did exist, the ensureWidthDefault function retruns the value from the store (and ignores the default). If a default cannot be set by the code and the key is just expected to be there, one could do

```C
dodo::persist::KVStore store( "cm.db" );
std::string proxy_server = "";
if ( store.getValue( "proxy-server", proxy_server ) ) {
  ...
} else throw std::runtime_error( "key not found" )
```

For bulk inserts (dodo::persist::KVStore::insertKey) or updates (dodo::persist::KVStore::setKey), be sure to call dodo::persist::KVStore::startTransaction before the modifications and dodo::persist::KVStore::commitTransaction (or dodo::persist::KVStore::rollbackTransaction) to speeds things up, as otherwise each modification
will commit individually (this is seen to speed things up 1000x).

```C
store.startTransaction();
for ( const auto &k : keys ) {
  store.insertKey( k, random_string( rand() % DATA_MAX_LENGTH ) );
}
store.commitTransaction();
```

The KVStore can be run as an in-memory database by opening the special file `:memory:`

```C
dodo::persist::KVStore store( ":memory:" );
```
but its contents are lost when the store object closes (destructs). Refer to the [SQLite documentation](https://sqlite.org/inmemorydb.html) for more details.

# Performance

The insertKey operations are enclosed between startTransaction / commitTransaction - all insertKey is comitted in one go. The setKey calls are individual commits. As a commit on persistent storage requires a physical write that has completed, the setKey speed is dominated by the write latency of the backing storage, as the huge difference in setKey speed below examplifies.

**Intel Corei7 3.4GHz**
| storage | insertKey | getValue | setKey |
|---------|-----------|----------|--------|
| memory | `575,000/s` | `1,000,000/s`| `345,000/s` |
| Samsung SSD 860 | `660,000/s` | `420,000/s` | `414/s` |
