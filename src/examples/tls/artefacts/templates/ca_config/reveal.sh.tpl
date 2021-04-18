#/bin/bash

# $1 = servers | clients
# $2 = commonname

THIS_DIR=$(realpath $(dirname "${0}"))
source "${THIS_DIR}/library.sh" || exit 1

trap "exitError 'exit on signal'" 1 2 3 8 15

SWITCH="${1}"
COMMON_NAME="${2}"
if [ "${SWITCH}" != "server" -a "${SWITCH}" != "client" ]; then
  exitError "specify 'server' or 'client'"
fi
test -z "${COMMON_NAME}" && exitError "specify a common name"

CA_ROOT_DIR="$(realpath {{getv "/ca/root/directory"}})"
EXT_DIR="${CA_ROOT_DIR}/ext/${SWITCH}s"
test -d "${EXT_DIR}" || exitError "directory ${EXT_DIR} not found"
CN_PASSPHRASE_FILE="${EXT_DIR}/${COMMON_NAME}.passphrase"
test -r "${CN_PASSPHRASE_FILE}" || exitError "passphrase file ${CN_PASSPHRASE_FILE} not found"

askMasterPassword

PASSPHRASE="$(decrypt "${CN_PASSPHRASE_FILE}")"

#echo "passphrase for ${SWITCH} ${COMMON_NAME} is"

echo "${PASSPHRASE}"

exitClean