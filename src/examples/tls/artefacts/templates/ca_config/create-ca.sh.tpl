#/bin/bash

THIS_DIR=$(dirname "${0}")
source "${THIS_DIR}/library.sh" || exit 1

trap "exitError 'exit on signal'" 1 2 3 8 15

CA_ROOT_DIR="{{getv "/ca/root/directory"}}"
CA_PEM_ENCRYPTION="{{getv "/ca/pem-encryption"}}"
CA_ROOT_PKEY_BITS="{{getv "/ca/root/pkey-bits"}}"
CA_ROOT_EXPIRY="{{getv "/ca/root/expiry"}}"
CA_HASHING="{{getv "/ca/hashing"}}"

heading "Setup CA master installation password - not stored anywhere, must remember!"

createMasterPassword

heading "Setup root CA directory structure"

# root CA
mkdir -p "${CA_ROOT_DIR}" && \
  chmod 700 "${CA_ROOT_DIR}" \
  || exitError "failed to create directory '${CA_ROOT_DIR}'"
CA_ROOT_DIR=$(realpath "${CA_ROOT_DIR}")
test -d "${CA_ROOT_DIR}" || exitError "failed to setup CA root directory ${CA_ROOT_DIR}"

mkdir "${CA_ROOT_DIR}"/certs \
  "${CA_ROOT_DIR}"/crl \
  "${CA_ROOT_DIR}"/newcerts \
  "${CA_ROOT_DIR}"/private \
  "${CA_ROOT_DIR}"/bin \
  "${CA_ROOT_DIR}"/conf \
  "${CA_ROOT_DIR}"/intermediates \
  "${CA_ROOT_DIR}"/ext \
  "${CA_ROOT_DIR}"/ext/servers \
  "${CA_ROOT_DIR}"/ext/clients \
  || exitError "failed to create subdirs under ${CA_ROOT_DIR}"

chmod 700 "${CA_ROOT_DIR}"/certs \
  "${CA_ROOT_DIR}"/crl \
  "${CA_ROOT_DIR}"/newcerts \
  "${CA_ROOT_DIR}"/private \
  "${CA_ROOT_DIR}"/bin \
  "${CA_ROOT_DIR}"/conf \
  "${CA_ROOT_DIR}"/intermediates \
  "${CA_ROOT_DIR}"/ext/servers \
  "${CA_ROOT_DIR}"/ext/clients \
  || exitError "failed to set permissions under ${CA_ROOT_DIR}"

generatePassword | encrypt "${CA_ROOT_DIR}/private/verify"

touch "${CA_ROOT_DIR}"/index.txt && \
  chmod 600 "${CA_ROOT_DIR}"/index.txt \
  || exitError "failed to create ${CA_ROOT_DIR}/index.txt"

echo 1000 > "${CA_ROOT_DIR}"/serial && \
  chmod 600 "${CA_ROOT_DIR}"/serial \
  || { echo "failed to create ${CA_ROOT_DIR}/serial"; exit 1; }

cp "${THIS_DIR}/../conf/root_ca_openssl.cnf" "${CA_ROOT_DIR}" \
  || exitError "failed to copy ${THIS_DIR}/../conf/root_ca_openssl.cnf"

generatePassword | encrypt "${CA_ROOT_DIR}/private/passphrase"
CA_PKEY_PASSPHRASE=$(decrypt "${CA_ROOT_DIR}/private/passphrase")
export CA_PKEY_PASSPHRASE

echo "installed tree into ${CA_ROOT_DIR}"


heading "Create root CA private key"

openssl genrsa -"${CA_PEM_ENCRYPTION}" \
  -out "${CA_ROOT_DIR}/private/ca.key.pem" \
  -passout "env:CA_PKEY_PASSPHRASE" \
  ${CA_ROOT_PKEY_BITS} > /dev/null 2>&1 \
  || exitError "failed to create root CA private key"

chmod 400 "${CA_ROOT_DIR}"/private/ca.key.pem || exitError "failed to secure root CA private key"

echo "produced private key ${CA_ROOT_DIR}/private/ca.key.pem"

heading "Create root CA certificate"

ROOT_CA_SUBJECT="/CN={{getv "/ca/root/commonName"}}"
ROOT_CA_SUBJECT+="/C={{getv "/ca/countryName"}}"
ROOT_CA_SUBJECT+="/ST={{getv "/ca/stateOrProvinceName"}}"
ROOT_CA_SUBJECT+="/L={{getv "/ca/localityName"}}"
ROOT_CA_SUBJECT+="/O={{getv "/ca/organizationName"}}"
ROOT_CA_SUBJECT+="/OU={{getv "/ca/organizationalUnitName"}}"
ROOT_CA_SUBJECT+="/emailAddress={{getv "/ca/emailAddress"}}"



openssl req -config "${CA_ROOT_DIR}"/root_ca_openssl.cnf \
      -key "${CA_ROOT_DIR}"/private/ca.key.pem \
      -new -x509 -days "${CA_ROOT_EXPIRY}" \
      -"${CA_HASHING}" \
      -extensions v3_ca \
      -subj "${ROOT_CA_SUBJECT}" \
      -out "${CA_ROOT_DIR}"/certs/ca.cert.pem \
      -passin "env:CA_PKEY_PASSPHRASE" \
      || exitError "failed to create root CA certificate"

echo "produced self-signed certificate ${CA_ROOT_DIR}/certs/ca.cert.pem"

heading "Copy scripts"

cp "${THIS_DIR}/library.sh" "${CA_ROOT_DIR}/bin" \
  || exitError "failed to copy ${THIS_DIR}/library.sh"

sed "s;@@CA_ROOT_DIR@@;${CA_ROOT_DIR};g" -i "${CA_ROOT_DIR}/bin/library.sh" \
  && chmod 400 "${CA_ROOT_DIR}/bin/library.sh" \
  || exitError "failed to set directory in ${CA_ROOT_DIR}/bin/library.sh"

chmod 400 "${CA_ROOT_DIR}/bin/library.sh" \
  || exitError "failed to set permissions on ${CA_ROOT_DIR}/bin/library.sh"

cp "${THIS_DIR}/create-intermediate.sh" "${CA_ROOT_DIR}/bin" \
  || exitError "failed to copy ${THIS_DIR}/create-intermediate.sh"
chmod 500 "${CA_ROOT_DIR}/bin/create-intermediate.sh" \
  || exitError "failed to set permissions on ${CA_ROOT_DIR}/bin/create-intermediate.sh"

cp "${THIS_DIR}/create-server.sh" "${CA_ROOT_DIR}/bin" \
  || exitError "failed to copy ${THIS_DIR}/create-server.sh"
chmod 500 "${CA_ROOT_DIR}/bin/create-server.sh" \
  || exitError "failed to set permissions on ${CA_ROOT_DIR}/bin/create-server.sh"

cp "${THIS_DIR}/create-client.sh" "${CA_ROOT_DIR}/bin" \
  || exitError "failed to copy ${THIS_DIR}/create-client.sh"
chmod 500 "${CA_ROOT_DIR}/bin/create-client.sh" \
  || exitError "failed to set permissions on ${CA_ROOT_DIR}/bin/create-client.sh"

cp "${THIS_DIR}/reveal.sh" "${CA_ROOT_DIR}/bin" \
  || exitError "failed to copy ${THIS_DIR}/reveal.sh"
chmod 500 "${CA_ROOT_DIR}/bin/reveal.sh" \
  || exitError "failed to set permissions on ${CA_ROOT_DIR}/bin/reveal.sh"

cp "${THIS_DIR}/../conf/intermediate_ca_openssl.cnf" "${CA_ROOT_DIR}/conf" \
  || exitError "failed to copy ${THIS_DIR}/../conf/intermediate_ca_openssl.cnf"
chmod 400 "${CA_ROOT_DIR}/conf/intermediate_ca_openssl.cnf" \
  || exitError "failed to set permissions on ${CA_ROOT_DIR}/conf/intermediate_ca_openssl.cnf"

cp "${THIS_DIR}/sign-server-csr.sh" "${CA_ROOT_DIR}/conf" \
  || exitError "failed to copy ${THIS_DIR}/create-server.sh"
chmod 500 "${CA_ROOT_DIR}/conf/sign-server-csr.sh" \
  || exitError "failed to set permissions on ${CA_ROOT_DIR}/conf/sign-server-csr.sh"

cp "${THIS_DIR}/sign-client-csr.sh" "${CA_ROOT_DIR}/conf" \
  || exitError "failed to copy ${THIS_DIR}/create-client.sh"
chmod 500 "${CA_ROOT_DIR}/conf/sign-client-csr.sh" \
  || exitError "failed to set permissions on ${CA_ROOT_DIR}/conf/sign-client-csr.sh"

cp "${THIS_DIR}/../conf/README.md" "${CA_ROOT_DIR}" \
  || exitError "failed to copy ${THIS_DIR}/../conf/README.md"
chmod 400 "${CA_ROOT_DIR}/README.md" \
  || exitError "failed to set permissions on ${CA_ROOT_DIR}/README.md"


exitClean
echo "${0} install into ${CA_ROOT_DIR} finished ok"
