# OpenSSL intermediate CA configuration file.

[ ca ]
default_ca = CA_default

[ CA_default ]
# Directory and file locations.
dir               = @@DIR@@
certs             = $dir/certs
crl_dir           = $dir/crl
new_certs_dir     = $dir/newcerts
database          = $dir/index.txt
serial            = $dir/serial
RANDFILE          = $dir/private/.rand

# The root key and root certificate.
private_key       = $dir/private/@@INTERMEDIATE_NAME@@.key.pem
certificate       = $dir/certs/@@INTERMEDIATE_NAME@@.cert.pem

# For certificate revocation lists.
crlnumber         = $dir/crlnumber
crl               = $dir/crl/@@INTERMEDIATE_NAME@@.crl.pem
crl_extensions    = crl_ext
copy_extensions   = copy
default_crl_days  = {{getv "/ca/intermediate/crl-days"}}

# SHA-1 is deprecated, so use SHA-2 instead.
default_md        = {{getv "/ca/hashing"}}

name_opt          = ca_default
cert_opt          = ca_default
default_days      = {{getv "/ca/intermediate/expiry"}}
preserve          = no
policy            = policy_loose

[ policy_loose ]
countryName             = supplied
stateOrProvinceName     = supplied
localityName            = supplied
organizationName        = supplied
organizationalUnitName  = optional
commonName              = supplied
emailAddress            = supplied

[ req ]
default_bits        = {{getv "/ca/intermediate/pkey-bits"}}
distinguished_name  = req_distinguished_name
string_mask         = utf8only

default_md          = {{getv "/ca/hashing"}}

x509_extensions     = v3_ca

[ req_distinguished_name ]
countryName                     = Country Name (2 letter code)
stateOrProvinceName             = State or Province Name
localityName                    = Locality Name
0.organizationName              = Organization Name
organizationalUnitName          = Organizational Unit Name
commonName                      = Common Name
emailAddress                    = Email Address

countryName_default             = {{getv "/ca/countryName"}}
stateOrProvinceName_default     = {{getv "/ca/stateOrProvinceName"}}
localityName_default            = {{getv "/ca/localityName"}}
0.organizationName_default      = {{getv "/ca/organizationName"}}
organizationalUnitName_default  = {{getv "/ca/organizationalUnitName"}}
emailAddress_default            = {{getv "/ca/emailAddress"}}

[ v3_ca ]
subjectKeyIdentifier = hash
authorityKeyIdentifier = keyid:always,issuer
basicConstraints = critical, CA:true
keyUsage = critical, digitalSignature, cRLSign, keyCertSign

[ v3_intermediate ]
subjectKeyIdentifier = hash
authorityKeyIdentifier = keyid:always,issuer
basicConstraints = critical, CA:true, pathlen:0
keyUsage = critical, digitalSignature, cRLSign, keyCertSign

[ usr_cert ]
basicConstraints = CA:FALSE
nsCertType = client, email
nsComment = "OpenSSL Generated Client Certificate"
subjectKeyIdentifier = hash
authorityKeyIdentifier = keyid,issuer
keyUsage = critical, nonRepudiation, digitalSignature, keyEncipherment
extendedKeyUsage = clientAuth, emailProtection

[ server_cert ]
basicConstraints = CA:FALSE
nsCertType = server
nsComment = "OpenSSL Generated Server Certificate"
subjectKeyIdentifier = hash
authorityKeyIdentifier = keyid,issuer:always
keyUsage = critical, digitalSignature, keyEncipherment
extendedKeyUsage = serverAuth

[ crl_ext ]
authorityKeyIdentifier=keyid:always

[ ocsp ]
basicConstraints = CA:FALSE
subjectKeyIdentifier = hash
authorityKeyIdentifier = keyid,issuer
keyUsage = critical, digitalSignature
extendedKeyUsage = critical, OCSPSigning
