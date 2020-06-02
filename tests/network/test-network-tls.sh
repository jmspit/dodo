#/bin/bash

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

# must succeed as server requires SNI and client has enabled it
"$PROG" "badssl.com" "443" "pvNone" "true" || exitError "test failed" 1

# must succeed
"$PROG" "badssl.com" "443" "pvVerifyFQDN" "true" || exitError "test failed" 1

# must fail
"$PROG" "expired.badssl.com" "443" "pvVerifyPeer" "true" && exitError "test failed" 1

# must succeed
"$PROG" "expired.badssl.com" "443" "pvNone" "true" || exitError "test failed" 1

# must fail
"$PROG" "wrong.host.badssl.com" "443" "pvVerifyFQDN" "true" && exitError "test failed" 1

# must fail
"$PROG" "self-signed.badssl.com" "443" "pvVerifyPeer" "true" && exitError "test failed" 1

# must succeed
"$PROG" "self-signed.badssl.com" "443" "pvNone" "true" || exitError "test failed" 1

# must fail
"$PROG" "untrusted-root.badssl.com" "443" "pvVerifyPeer" "true" && exitError "test failed" 1

# must succeed
"$PROG" "untrusted-root.badssl.com" "443" "pvNone" "true" || exitError "test failed" 1

# must fail
"$PROG" "sha1-intermediate.badssl.com" "443" "pvVerifyPeer" "true" && exitError "test failed" 1

# must fail
"$PROG" "superfish.badssl.com" "443" "pvVerifyPeer" "true" && exitError "test failed" 1

exit 0