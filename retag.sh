#/bin/bash

tag="$1"
git push origin :refs/tags/"${1}"
git tag --delete "${1}"
git tag "$1"
git push origin "${1}"