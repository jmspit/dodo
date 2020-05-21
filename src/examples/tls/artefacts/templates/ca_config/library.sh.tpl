#!/bin/bash

function heading() {
  echo
  echo -e ''$_{1..140}'\b='
  echo $1
  echo -e ''$_{1..140}'\b='
}

function exitClean() {
  unset CA_MASTER_PASSWORD
  unset CA_PKEY_PASSPHRASE
  unset CA_INTERMEDIATE_PKEY_PASSPHRASE
  unset INTERMEDIATE_CA_PKEY_PASSPHRASE
  unset SERVER_PKEY_PASSPHRASE
}

function exitError() {
  exitClean
  echo "$1"
  exit 1
}

function passPhrase() {
  #1  file
  #2  question
  touch "${1}"
  chmod 600 "${1}"
  read -s -p "${2} "
  echo "${REPLY}" > "${1}"
  chmod 400 "${1}"
  echo
}

function stripName() {
  echo $(echo "${1}" | tr -dc '[:alnum:]' | tr '[:upper:]' '[:lower:]')
}

function generatePassword() {
  openssl rand -base64 {{getv "/ca/passphrase-length"}}
}

function encrypt() {
  # $1 file name to encrypt to
  openssl enc -aes-256-cbc -md {{getv "/ca/hashing"}} \
    -pbkdf2 -iter {{getv "/ca/master-password/iterations"}} \
    -salt -out "${1}" -pass env:CA_MASTER_PASSWORD \
    || exitError "encryption failed"
  chmod 400 "${1}" || exitError "failed to set permission on ${1}"
}

function decrypt() {
  # $1 file name to decrypt from
  # result is echoed
  openssl enc -aes-256-cbc -md {{getv "/ca/hashing"}} \
    -pbkdf2 -iter {{getv "/ca/master-password/iterations"}} \
    -salt -in "${1}" -d -pass env:CA_MASTER_PASSWORD \
    || exitError "decryption failed"
}

function verifyMasterPassword() {
  openssl enc -aes-256-cbc -md {{getv "/ca/hashing"}} \
    -pbkdf2 -iter {{getv "/ca/master-password/iterations"}} \
    -salt -in "@@CA_ROOT_DIR@@/private/verify" -d -pass env:CA_MASTER_PASSWORD \
    > /dev/null 2>&1
  echo $?
}

function createMasterPassword() {
  PW1="A"
  PW2="B"
  echo "at least 10 characters, at least one digit, at least one lower and at least one uppercase"
  while [ "${PW1}" != "${PW2}" ]
  do
    read -s -p "enter the master password          : " PW1
    echo
    read -s -p "enter the master password (verify) : " PW2
    echo
    if [ "${PW1}" != "${PW2}" ]; then
      echo "passwords do not match. try again."
    else
      if [[ ${#PW1} -lt {{getv "/ca/master-password/min-length"}} \
            || "${PW1}" != *[A-Z]* \
            || "${PW1}" != *[a-z]* \
            || "${PW1}" != *[0-9]* ]]; then
        echo "that password does not meet the criteria, try again"
        PW1="A"
        PW2="B"
      fi
    fi
  done
  CA_MASTER_PASSWORD="${PW1}"
  export CA_MASTER_PASSWORD
}

function askMasterPassword() {
  export CA_MASTER_PASSWORD=""
  result=$(verifyMasterPassword)
  while [ ${result} -ne 0 ]
  do
    read -sp 'enter the master password : ' CA_MASTER_PASSWORD
    result=$(verifyMasterPassword)
    echo
  done
}