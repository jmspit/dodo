# A simplistic certificate authority

Note that this install will store passphrase to private keys as files, readable to only the owner. This is not
very secure and should only be used for testing purposes or with expert care.

## Install

This install uses the confd tool.

Copy and customize `config.yaml`. The `ca::root::directory` entry specifies the installation directory.

```
$ cd src/examples/artefacts
$ ./configure.sh myconfig.yaml && ca_config/bin/create-ca.sh
```

## Commands

