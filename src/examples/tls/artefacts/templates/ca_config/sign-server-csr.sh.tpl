#/bin/bash

# $1 = csr file

THIS_DIR=$(realpath $(dirname "${0}"))
source "${THIS_DIR}/library.sh" || exit 1

trap "exitError 'exit on signal'" 1 2 3 8 15

CSR_FILE="$(realpath ${1})"
CSR_DIR=$(dirname "${CSR_FILE}")

test -d "${CSR_DIR}" && test -r "${CSR_FILE}" || exitError "CSR file '${CSR_FILE}' not found"

askMasterPassword

CA_ROOT_DIR="$(realpath {{getv "/ca/root/directory"}})"
INTERMEDIATE_DIR="$(realpath @@INTERMEDIATE_DIR@@)"
CA_HASHING="{{getv "/ca/hashing"}}"
CA_SERVER_EXPIRY="{{getv "/ca/server/expiry"}}"

COMMON_NAME=$(openssl req -in $CSR_FILE \
              -subject -noout -nameopt multiline | sed -n 's/ *commonName *= //p')

test -z "${COMMON_NAME}" && exitError "failed to get CN from $CSR_FILE"

CERT_FILE="${INTERMEDIATE_DIR}/certs/${COMMON_NAME}.cert.pem"

INTERMEDIATE_CA_PKEY_PASSPHRASE=$(decrypt "${INTERMEDIATE_DIR}/private/passphrase")
export INTERMEDIATE_CA_PKEY_PASSPHRASE

openssl ca -config ${INTERMEDIATE_DIR}/intermediate_ca_openssl.cnf \
      -extensions server_cert -days ${CA_SERVER_EXPIRY} -notext -md "${CA_HASHING}" \
      -in "${CSR_FILE}" \
      -out "${CERT_FILE}" \
      -passin env:INTERMEDIATE_CA_PKEY_PASSPHRASE \
      || exitError "signing ${CSR_FILE} failed"

cp "${CERT_FILE}" "${CSR_DIR}" || exitError "failed to copy ${CERT_FILE} to ${CSR_DIR}"

echo "signed ${CSR_DIR}/$(basename ${CERT_FILE})"

PKCS_FILE="${CSR_DIR}/${COMMON_NAME}.pkcs12"
SERVER_PKEY_FILE="${CSR_DIR}/${COMMON_NAME}.key.pem"
PASSPHRASE_FILE_NAME="${CA_ROOT_DIR}/ext/servers/${COMMON_NAME}.passphrase"

SERVER_PKEY_PASSPHRASE=$(decrypt "${PASSPHRASE_FILE_NAME}")
export SERVER_PKEY_PASSPHRASE

if [ -r "${SERVER_PKEY_FILE}" ]; then
  openssl pkcs12 -export -out "${PKCS_FILE}" \
    -inkey "${SERVER_PKEY_FILE}"  \
    -in "${CSR_DIR}/${COMMON_NAME}.cert.pem" \
    -certfile "${INTERMEDIATE_DIR}/certs/ca-chain.cert.pem" \
    -passin env:SERVER_PKEY_PASSPHRASE \
    -passout env:SERVER_PKEY_PASSPHRASE \
    || exitError "failed to create PKCS12 file"
fi

echo "produced PKCS file ${PKCS_FILE}"

exitClean
echo "${0} finished SUCCESS"
