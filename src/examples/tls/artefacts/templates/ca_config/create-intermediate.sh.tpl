#/bin/bash

THIS_DIR=$(dirname "${0}")
source "${THIS_DIR}/library.sh" || exit 1

trap "exitError 'exit on signal'" 1 2 3 8 15

DIR=$(dirname $(dirname $0))

CA_ROOT_DIR="$(realpath {{getv "/ca/root/directory"}})"
CA_INTERMEDIATES_DIR="${CA_ROOT_DIR}/intermediates"
CA_PEM_ENCRYPTION="{{getv "/ca/pem-encryption"}}"
CA_INTERMEDIATE_PKEY_BITS="{{getv "/ca/intermediate/pkey-bits"}}"
CA_INTERMEDIATE_EXPIRY="{{getv "/ca/intermediate/expiry"}}"

askMasterPassword

read -p "new intermediate name : "
INTERMEDIATE_FULL_NAME="${REPLY}"
INTERMEDIATE_NAME=$(stripName "$INTERMEDIATE_FULL_NAME")

CA_INTERMEDIATE_DIR="${CA_INTERMEDIATES_DIR}/${INTERMEDIATE_NAME}"

mkdir "${CA_INTERMEDIATE_DIR}" \
  || exitError "failed to create directory ${CA_INTERMEDIATE_DIR}"
chmod 700 "${CA_INTERMEDIATE_DIR}" \
  || exitError "failed to secure ${CA_INTERMEDIATE_DIR}"

mkdir "${CA_INTERMEDIATE_DIR}"/certs \
  "${CA_INTERMEDIATE_DIR}"/crl \
  "${CA_INTERMEDIATE_DIR}"/csr \
  "${CA_INTERMEDIATE_DIR}"/newcerts \
  "${CA_INTERMEDIATE_DIR}"/private \
  "${CA_INTERMEDIATE_DIR}"/bin \
  || exitError "failed to create subdirs under ${CA_INTERMEDIATE_DIR}"

chmod 700 "${CA_INTERMEDIATE_DIR}"/certs \
  "${CA_INTERMEDIATE_DIR}"/crl \
  "${CA_INTERMEDIATE_DIR}"/csr \
  "${CA_INTERMEDIATE_DIR}"/newcerts \
  "${CA_INTERMEDIATE_DIR}"/private \
  "${CA_INTERMEDIATE_DIR}"/bin \
  || exitError "failed to set permissions under ${CA_INTERMEDIATE_DIR}"

touch "${CA_INTERMEDIATE_DIR}"/index.txt && \
  chmod 600 "${CA_INTERMEDIATE_DIR}"/index.txt \
  || exitError "failed to create ${CA_INTERMEDIATE_DIR}/index.txt"

echo 1000 > "${CA_INTERMEDIATE_DIR}"/serial && \
  chmod 600 "${CA_INTERMEDIATE_DIR}"/serial \
  || exitError "failed to create ${CA_INTERMEDIATE_DIR}/serial"

echo 1000 > "${CA_INTERMEDIATE_DIR}"/crlnumber \
  || exitError "failed to create ${CA_INTERMEDIATE_DIR}/crlnumber"

cp "${CA_ROOT_DIR}/conf/intermediate_ca_openssl.cnf" "${CA_INTERMEDIATE_DIR}" \
  || exitError "failed to copy to ${CA_INTERMEDIATE_DIR}"

sed "s;@@DIR@@;${CA_INTERMEDIATE_DIR};g" -i "${CA_INTERMEDIATE_DIR}/intermediate_ca_openssl.cnf" \
  || exitError "failed to set directory in intermediate_ca_openssl.cnf"

sed "s;@@INTERMEDIATE_NAME@@;${INTERMEDIATE_NAME};g" -i "${CA_INTERMEDIATE_DIR}/intermediate_ca_openssl.cnf" \
  || exitError "failed to set intermediate name in intermediate_ca_openssl.cnf"

cp "${CA_ROOT_DIR}/conf/sign-server-csr.sh" "${CA_INTERMEDIATE_DIR}/bin" \
  || exitError "failed to copy to ${CA_INTERMEDIATE_DIR}/bin"

sed "s;@@INTERMEDIATE_DIR@@;${CA_INTERMEDIATE_DIR};g" -i "${CA_INTERMEDIATE_DIR}/bin/sign-server-csr.sh" \
  && chmod 500 "${CA_INTERMEDIATE_DIR}/bin/sign-server-csr.sh" \
  || exitError "failed to set directory in sign-server-csr.sh"

cp "${CA_ROOT_DIR}/conf/sign-client-csr.sh" "${CA_INTERMEDIATE_DIR}/bin" \
  || exitError "failed to copy to ${CA_INTERMEDIATE_DIR}/bin"

sed "s;@@INTERMEDIATE_DIR@@;${CA_INTERMEDIATE_DIR};g" -i "${CA_INTERMEDIATE_DIR}/bin/sign-client-csr.sh" \
  && chmod 500 "${CA_INTERMEDIATE_DIR}/bin/sign-client-csr.sh" \
  || exitError "failed to set directory in sign-client-csr.sh"

ln -s "${CA_ROOT_DIR}/bin/library.sh" "${CA_INTERMEDIATE_DIR}/bin/library.sh" \
  || exitError "failed to link ${CA_INTERMEDIATE_DIR}/bin/library.sh"


heading "Create intermediate CA private key"

generatePassword | encrypt "${CA_INTERMEDIATE_DIR}/private/passphrase"
CA_INTERMEDIATE_PKEY_PASSPHRASE=$(decrypt "${CA_INTERMEDIATE_DIR}/private/passphrase")
export CA_INTERMEDIATE_PKEY_PASSPHRASE

INTERMEDIATE_PKEY_FILE="${CA_INTERMEDIATE_DIR}/private/${INTERMEDIATE_NAME}.key.pem"

openssl genrsa -"${CA_PEM_ENCRYPTION}" \
  -out ${INTERMEDIATE_PKEY_FILE} \
  -passout "env:CA_INTERMEDIATE_PKEY_PASSPHRASE" \
  ${CA_INTERMEDIATE_PKEY_BITS} > /dev/null 2>&1 \
  || exitError "failed to create intermediate CA private key"

chmod 400 "${INTERMEDIATE_PKEY_FILE}" \
  || exitError "failed to secure intermediate CA private key"

echo "produced ${INTERMEDIATE_PKEY_FILE}"


heading "Create intermediate CA CSR"

#read -p "intermediate common name  : " INTERMEDIATE_CA_SUBJECT_CN
INTERMEDIATE_CA_SUBJECT_CN="${INTERMEDIATE_NAME}"

INTERMEDIATE_CA_SUBJECT="/CN=${INTERMEDIATE_CA_SUBJECT_CN}"
INTERMEDIATE_CA_SUBJECT+="/C={{getv "/ca/countryName"}}"
INTERMEDIATE_CA_SUBJECT+="/ST={{getv "/ca/stateOrProvinceName"}}"
INTERMEDIATE_CA_SUBJECT+="/L={{getv "/ca/localityName"}}"
INTERMEDIATE_CA_SUBJECT+="/O={{getv "/ca/organizationName"}}"
INTERMEDIATE_CA_SUBJECT+="/OU={{getv "/ca/organizationalUnitName"}} - intermediate CA ${INTERMEDIATE_CA_SUBJECT_CN}"
INTERMEDIATE_CA_SUBJECT+="/emailAddress={{getv "/ca/emailAddress"}}"

INTERMEDIATE_CA_CSR_FILE="${CA_INTERMEDIATE_DIR}"/csr/${INTERMEDIATE_NAME}.csr.pem

openssl req -config "${CA_INTERMEDIATE_DIR}"/intermediate_ca_openssl.cnf \
  -new -sha256 \
  -key "${INTERMEDIATE_PKEY_FILE}" \
  -out "${INTERMEDIATE_CA_CSR_FILE}" \
  -subj "${INTERMEDIATE_CA_SUBJECT}" \
  -passin "env:CA_INTERMEDIATE_PKEY_PASSPHRASE" \
  || exitError "failed to create intermediate CSR"

chmod 400 "${INTERMEDIATE_CA_CSR_FILE}" || exitError "failed to set permissions on ${INTERMEDIATE_CA_CSR_FILE}"

echo "produced ${INTERMEDIATE_CA_CSR_FILE}"

heading "Signing intermediate CA CSR"

CA_PKEY_PASSPHRASE=$(decrypt "${CA_ROOT_DIR}/private/passphrase")
export CA_PKEY_PASSPHRASE

INTERMEDIATE_CA_CERT_FILE="${CA_INTERMEDIATE_DIR}/certs/${INTERMEDIATE_NAME}.cert.pem"

openssl ca -config "${CA_ROOT_DIR}"/root_ca_openssl.cnf -extensions v3_intermediate_ca \
      -batch \
      -days ${CA_INTERMEDIATE_EXPIRY} -notext -md sha256 \
      -in "${INTERMEDIATE_CA_CSR_FILE}" \
      -passin "env:CA_PKEY_PASSPHRASE" \
      -out "${INTERMEDIATE_CA_CERT_FILE}" \
      || exitError "signing intermediate CSR failed"

chmod 400 "${INTERMEDIATE_CA_CERT_FILE}" || exitError "failed to set permissions on ${INTERMEDIATE_CA_CERT_FILE}"

openssl verify -CAfile "${CA_ROOT_DIR}"/certs/ca.cert.pem "${CA_INTERMEDIATE_DIR}"/certs/${INTERMEDIATE_NAME}.cert.pem \
  || exitError "verification of intermediate cetificate failed"

echo "produced certificate ${INTERMEDIATE_CA_CERT_FILE}"

INTERMEDIATE_CA_CHAIN_FILE="${CA_INTERMEDIATE_DIR}/certs/ca-chain.cert.pem"

cat "${INTERMEDIATE_CA_CERT_FILE}" > "${INTERMEDIATE_CA_CHAIN_FILE}" \
    && chmod 400 "${INTERMEDIATE_CA_CHAIN_FILE}" \
    || exitError "failed to create intermediate chain file"

echo "produced CA chain file ${INTERMEDIATE_CA_CHAIN_FILE}"

exitClean
echo "${0} finished ok"