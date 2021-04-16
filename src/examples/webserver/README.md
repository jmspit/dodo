# Simple key-value store

This example yet functional Docker container is a key-to-JSON document store with the following traits

  - Provides a HTTP/1.1 REST interface (GET,POST,PUT,DELETE).
  - Use the dodo in-process dodo::persist::KVStore backed by SQLite. Keys are free format strings. Values are JSON documents.
  - Uses TLS1.3 and keypair-fingerprint client authtentication. Fingerprints must be whitelisted and can be assigned a mix read,write, create and delete access. Additionally, clients must present valid `client-id` and `client-secret` headers in each HTTP request. A client would have to lose its private key, the private key passphrase as well as the client-id and client-secret to lose control over its access.
  - HTTP connections are left open until idle-expiry or client hangup.

# REST API

## GET /keys/{key}
Retrieve the value of a key.

### Request
```JSON
client-id: foo
client-secret: bar
accept: application/json
```

### Response

Returns the key value, if found.


#### 200 OK
```
content-type: application/json
{
  update-count: 0
  last-modified: 2021-02-09T13:36:12Z
  "value": {
    ....
  }
}
```

#### 404 Not found

When the key does not exist.

## POST /keys/{key}

Create a new key-value pair.

### Request
```JSON
POST /path/${key}
client-id: foo
client-secret: bar
content-type: application/json
{
  ...
}
```

### Response

#### 201 Created

When the key-value pair was created.

```JSON
HTTP/1.1 201 CREATED
```

#### 409 Conflict

When the key already exists (and is left as is).

```JSON
HTTP/1.1 409 CONFLICT
```

## PUT /keys/{key}

Update an existing key. The value is the body of the PUT request.

### Request

```JSON
PUT /path/${key}
client-id: foo
client-secret: bar
content-type: application/json
{
  ...
}
```

### Response

#### 202 Accepted

When the key is updated.

```JSON
HTTP/1.1 202 ACCEPTED
```

#### 404 Not found

When the key does not exist.

## DELETE /keys/{key}

Delete a key-value pair.

#### 202 Accepted

When the key is deleted.

#### 404 Not found

When the key does not exist.

## Deployment configuration


## GET /filter/{wildcard}

Get keys (not the values) matching the wildcard.

```JSON
{
  keys: [
    "key1",
    ...
  ]
}
```

## GET /stats

Get store statistics.

### Response

#### 200 OK

```JSON
{
  "content": {
    "keys": 201,
    "size-mib": 12.32
  },
  "usage": {
    "get" : 3392,
    "post" : 20,
    "put": 3,
    "delete": 12,
    "filter": 4,
    "stats": 192
  }
}
```

```YAML
dodo:
  common:
    application:
      name: kvstore
      secret:
        file: /secrets/application.secret
    logger:
      console:
        level: info
kvstore:
  baseuri: /store
```