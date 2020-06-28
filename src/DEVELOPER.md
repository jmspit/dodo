# Developer Manual

[TOC]

Documentation for developers using the library.

# Introduction

- Target is Linux.
- API to write low level services with C++ traits as a low memory footprint and efficient binaries.
- Implicit APIs for deployment configuration, logging, threading, networking, SQLite, PostgreSQL, MongoDB, Oracle
  JSON, XML, Kafka, AVRO
- Relies on the GNU C++ STL

# Using dodo

Include all headers by including dodo.hpp. Before using any of the dodo content, call dodo::initLibrary(). Call
dodo::closeLibrary() to clean up at the end.

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
  catch ( std::runtime_error ) {
    cerr << e.what() << endl;
    return_code = 1;
  }
  closeLibrary();
  return return_code;
}
```

It is simpler to just use the Application class though, as that will initialize the library, install signal handlers,
parse command line arguments, read environment variables and so on. You only need to subclass Application and
implement the run method:

```C
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
    MyApp app( { "myapp", "myapp.cnf", argc, argv, envp } );
    return app.run();
  }
  catch ( const std::exception &e ) {
    cerr << e.what() << endl;
    return 1;
  }
}
```


# Exception and error handling

Dodo might throws dodo::common::Exception or its descendant dodo::common::SystemException in exceptional circumstances.
Otherwise methods may return a SystemError, an abstraction of various types of system and dodo errors that may occur
and must be acted upon in program flows. For example, loading an invalid certificate will thrown an Exception, whilst
a failure to resolve a hostname will return a SystemError.

As dodo::common::Exception itself descends from std::runtime_error, try / catch code can be a simple as

```C
try {
  ...
}
catch ( const std::runtime_error &e ) {
  cerr << e.what() << endl;
}
```

# Logging

A logging framework is available to write log entries to one or more destinations of types

  - standard output aka console
  - a file local to the process
  - the syslog call (man 3 syslog)


# Deployment configuration

A configuration framework can combine configuration data from a variety of sources, such as environment variables,
configuration files and OS-level constants of interest to developers.

# Threads

# Networking

The networking API is grouped in the dodo::network namespace.

## Address

Sockets connect to or listen on Address objects. The Address class allow to program for both ipv4 and ipv6
transparently. For example, the following sequence resolves the name "httpbin.org" to either an ipv4 or ipv6 address,
whatever the server supports.

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

## Secure sockets {#developer_networking}

Dodo supports TLS through dodo::network::TLSContext and dodo::network::TLSSocket.


### Asymmetric cryptography

The core principle to asymmetric cryptography is a mathematical mapping is very easy to
compute in one direction, and computationally infeasible in the other. Typically, the ease of muliplying primes
and the difficulty in finding primes roots of integers is used. In an asymmetric communication handshake,
each endpoint has a private key, a *secret* filled with as much randomness or entropy as possible, typically stored as
a file which is (can be) in turn encrypted and protected by a passphrase. The private key is and must not be shared with anyone.

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
