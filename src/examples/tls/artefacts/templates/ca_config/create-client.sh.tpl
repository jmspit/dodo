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
CLIENT_PKEY_BITS="{{getv "/ca/client/pkey-bits"}}"
CLIENT_PEM_ENCRYPTION="{{getv "/ca/pem-encryption"}}"
CLIENT_HASHING="{{getv "/ca/hashing"}}"

askMasterPassword

heading "Create a client identity"

read -p 'countryName            : ' CLIENT_SUBJECT_COUNTRY
read -p 'stateOrProvinceName    : ' CLIENT_SUBJECT_STATE
read -p 'localityName           : ' CLIENT_SUBJECT_LOCALITY
read -p 'organizationName       : ' CLIENT_SUBJECT_ORG
read -p 'organizationalUnitName : ' CLIENT_SUBJECT_ORGUNIT
read -p 'emailAddress           : ' CLIENT_SUBJECT_EMAIL
read -p 'commonName             : ' CLIENT_SUBJECT_COMMON_NAME

CLIENT_SUBJECT="/CN=${CLIENT_SUBJECT_COMMON_NAME}"
CLIENT_SUBJECT+="/C=${CLIENT_SUBJECT_COUNTRY}"
CLIENT_SUBJECT+="/ST=${CLIENT_SUBJECT_STATE}"
CLIENT_SUBJECT+="/L=${CLIENT_SUBJECT_LOCALITY}"
CLIENT_SUBJECT+="/O=${CLIENT_SUBJECT_ORG}"
CLIENT_SUBJECT+="/OU=${CLIENT_SUBJECT_ORGUNIT}"
CLIENT_SUBJECT+="/emailAddress=${CLIENT_SUBJECT_EMAIL}"

PKEY_FILE_NAME="${CA_ROOT_DIR}/ext/clients/"$(stripName "${CLIENT_SUBJECT_COMMON_NAME}")".key.pem"
CSR_FILE_NAME="${CA_ROOT_DIR}/ext/clients/"$(stripName "${CLIENT_SUBJECT_COMMON_NAME}")".csr.pem"
PASSPHRASE_FILE_NAME="${CA_ROOT_DIR}/ext/clients/"$(stripName "${CLIENT_SUBJECT_COMMON_NAME}")".passphrase"

generatePassword | encrypt "${PASSPHRASE_FILE_NAME}"
CLIENT_PKEY_PASSPHRASE=$(decrypt "${PASSPHRASE_FILE_NAME}")
export CLIENT_PKEY_PASSPHRASE

openssl genrsa -${CLIENT_PEM_ENCRYPTION} \
      -out ${PKEY_FILE_NAME} \
      -passout env:CLIENT_PKEY_PASSPHRASE \
      ${CLIENT_PKEY_BITS} > /dev/null 2>&1 \
      || exitError "failed to create client private key"

chmod 400 ${PKEY_FILE_NAME} \
  || exitError "failed to secure client private key"

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
        -${CLIENT_HASHING} \
        -out ${CSR_FILE_NAME} \
        -subj "${CLIENT_SUBJECT}" \
        -passin env:CLIENT_PKEY_PASSPHRASE \
        -addext "${SUBJECT_ALTNAMES}" \
        || exitError "failed to create client CSR"
else
  openssl req \
        -key ${PKEY_FILE_NAME} \
        -new \
        -${CLIENT_HASHING} \
        -out ${CSR_FILE_NAME} \
        -subj "${CLIENT_SUBJECT}" \
        -passin env:CLIENT_PKEY_PASSPHRASE \
        || exitError "failed to create client CSR"
fi

chmod 400 ${CSR_FILE_NAME} \
  || exitError "failed to set permission on client CSR"

echo "produced CSR ${CSR_FILE_NAME}"

exitClean
echo "${0} finished ok"
