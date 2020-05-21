#/bin/bash

THIS_DIR=$(realpath $(dirname "${0}"))
source "${THIS_DIR}/library.sh" || exit 1

trap "exitError 'exit on signal'" 1 2 3 8 15

function getSubjectAltnames() {
  ALTNAME=""
  ALTNAME_TEMP="x"
  while [ ! -z "${ALTNAME_TEMP}" ]
  do
    ALTNAME_TEMP=""
    read -p 'subject altname : ' ALTNAME_TEMP
    if [ ! -z "${ALTNAME_TEMP}" ]; then
      if [ ! -z "${ALTNAME}" ]; then
        ALTNAME="${ALTNAME}, ${ALTNAME_TEMP}"
      else
        ALTNAME="${ALTNAME_TEMP}"
      fi
    fi
  done
  echo "${ALTNAME}"
}

CA_ROOT_DIR="$(realpath {{getv "/ca/root/directory"}})"
SERVER_PKEY_BITS="{{getv "/ca/server/pkey-bits"}}"
SERVER_PEM_ENCRYPTION="{{getv "/ca/pem-encryption"}}"
SERVER_HASHING="{{getv "/ca/hashing"}}"

askMasterPassword

heading "Create a server identity (common name = server FQDN)"

read -p 'countryName            : ' SERVER_SUBJECT_COUNTRY
read -p 'stateOrProvinceName    : ' SERVER_SUBJECT_STATE
read -p 'localityName           : ' SERVER_SUBJECT_LOCALITY
read -p 'organizationName       : ' SERVER_SUBJECT_ORG
read -p 'organizationalUnitName : ' SERVER_SUBJECT_ORGUNIT
read -p 'emailAddress           : ' SERVER_SUBJECT_EMAIL
read -p 'commonName             : ' SERVER_SUBJECT_COMMON_NAME

SERVER_SUBJECT="/CN=${SERVER_SUBJECT_COMMON_NAME}"
SERVER_SUBJECT+="/C=${SERVER_SUBJECT_COUNTRY}"
SERVER_SUBJECT+="/ST=${SERVER_SUBJECT_STATE}"
SERVER_SUBJECT+="/L=${SERVER_SUBJECT_LOCALITY}"
SERVER_SUBJECT+="/O=${SERVER_SUBJECT_ORG}"
SERVER_SUBJECT+="/OU=${SERVER_SUBJECT_ORGUNIT}"
SERVER_SUBJECT+="/emailAddress=${SERVER_SUBJECT_EMAIL}"

PKEY_FILE_NAME="${CA_ROOT_DIR}/ext/servers/"$(stripName "${SERVER_SUBJECT_COMMON_NAME}")".key.pem"
CSR_FILE_NAME="${CA_ROOT_DIR}/ext/servers/"$(stripName "${SERVER_SUBJECT_COMMON_NAME}")".csr.pem"
PASSPHRASE_FILE_NAME="${CA_ROOT_DIR}/ext/servers/"$(stripName "${SERVER_SUBJECT_COMMON_NAME}")".passphrase"

generatePassword | encrypt "${PASSPHRASE_FILE_NAME}"
SERVER_PKEY_PASSPHRASE=$(decrypt "${PASSPHRASE_FILE_NAME}")
export SERVER_PKEY_PASSPHRASE

openssl genrsa -${SERVER_PEM_ENCRYPTION} \
      -out ${PKEY_FILE_NAME} \
      -passout env:SERVER_PKEY_PASSPHRASE \
      ${SERVER_PKEY_BITS} > /dev/null 2>&1 \
      || exitError "failed to create server private key"

chmod 400 ${PKEY_FILE_NAME} \
  || exitError "failed to secure server private key"

echo "produced private key ${PKEY_FILE_NAME}"

heading "Specify alternative names"

echo "examples:"
echo "  DNS:host.domain.net"
echo "  IP:192.168.1.1"
echo "  email:you@domain.net"
echo "ENTER to end list"

SUBJECT_ALTNAMES="$(getSubjectAltnames)"
if [ ! -z "${SUBJECT_ALTNAMES}" ]; then
  SUBJECT_ALTNAMES="subjectAltName = ${SUBJECT_ALTNAMES}"
  openssl req \
        -key ${PKEY_FILE_NAME} \
        -new \
        -${SERVER_HASHING} \
        -out ${CSR_FILE_NAME} \
        -subj "${SERVER_SUBJECT}" \
        -passin env:SERVER_PKEY_PASSPHRASE \
        -addext "${SUBJECT_ALTNAMES}" \
        || exitError "failed to create server CSR"
else
  openssl req \
        -key ${PKEY_FILE_NAME} \
        -new \
        -${SERVER_HASHING} \
        -out ${CSR_FILE_NAME} \
        -subj "${SERVER_SUBJECT}" \
        -passin env:SERVER_PKEY_PASSPHRASE \
        || exitError "failed to create server CSR"
fi

chmod 400 ${CSR_FILE_NAME} \
  || exitError "failed to set permission on server CSR"

echo "produced CSR ${CSR_FILE_NAME}"

exitClean
echo "${0} finished ok"
