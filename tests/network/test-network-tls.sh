#!/bin/bash

set -x

# $1 binary dir
DIR="$1"
PROG="$DIR/tls-client"

test -x "$PROG" || exit 1

function exitError() {
  msg="$1"
  error_code="$2"
  echo "$msg"
  exit "$error_code"
}

export LD_LIBRARY_PATH=./lib

# must fail as server requires SNI and client has not enabled it
"$PROG" "badssl.com" "443" "pvVerifyPeer" "false" && exitError "test failed" 1

# must succeed as server requires SNI and client has enabled it
"$PROG" "badssl.com" "443" "pvVerifyPeer" "true" || exitError "test failed" 1

# must succeed as there is no verification, just encryption
"$PROG" "badssl.com" "443" "pvNone" "true" || exitError "test failed" 1

# must succeed as we connect to the proper SAN name of the server cert.
"$PROG" "badssl.com" "443" "pvVerifyFQDN" "true" || exitError "test failed" 1

# must fail as the server cert has expired
"$PROG" "expired.badssl.com" "443" "pvVerifyPeer" "true" && exitError "test failed" 1

# must succeed even though the server cert has expired
"$PROG" "expired.badssl.com" "443" "pvNone" "true" || exitError "test failed" 1

# must fail as the server presents a cert not matching this host
"$PROG" "wrong.host.badssl.com" "443" "pvVerifyFQDN" "true" && exitError "test failed" 1

# must fail as the remote cert is self-signed / not trusted by this client
"$PROG" "self-signed.badssl.com" "443" "pvVerifyPeer" "true" && exitError "test failed" 1

# must succeed as we do not verify the peer certificate
"$PROG" "self-signed.badssl.com" "443" "pvNone" "true" || exitError "test failed" 1

# must fail as the peer certificate is untrusted
"$PROG" "untrusted-root.badssl.com" "443" "pvVerifyPeer" "true" && exitError "test failed" 1

# must succeed as we do not check the server certificate
"$PROG" "untrusted-root.badssl.com" "443" "pvNone" "true" || exitError "test failed" 1

# must fail as an intermediate cert is too weak
"$PROG" "sha1-intermediate.badssl.com" "443" "pvVerifyPeer" "true" && exitError "test failed" 1

# must fail as we are using a weak cipher
"$PROG" "superfish.badssl.com" "443" "pvVerifyPeer" "true" && exitError "test failed" 1

exit 0