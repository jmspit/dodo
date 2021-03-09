# Dodo Docker swarm

To demonstrate integration between dodo and Docker, lets create a swarm of TCPServers.

## Building the image

The Dockerfile performs a multistage build and COPYs only required artefacts. As we are creating
a swarm (with an implicit service), we can use Docker's config mechanism to specify the runtime-configuration
of the container, which is much more flexible than including the config file in the Docker image, which would
require an image re-build.

```Docker
FROM bitnami/minideb:latest AS builder

RUN echo 'debconf debconf/frontend select Noninteractive' | debconf-set-selections
RUN apt-get update && \
    apt-get install -y apt-utils && \
    apt-get install -y tzdata libssl-dev libyaml-cpp-dev git g++ cmake
RUN echo "Europe/Amsterdam" > /etc/timezone && dpkg-reconfigure -f noninteractive tzdata

RUN mkdir -p /opt/dodo/build && mkdir -p /opt/dodo/bin && mkdir -p /opt/dodo/lib
WORKDIR /opt/dodo/build
RUN git config --global advice.detachedHead false && git clone https://github.com/jmspit/dodo.git
RUN cd dodo && \
    mkdir build && \
    cd build && \
    cmake ..  -DCMAKE_INSTALL_PREFIX=/opt/dodo -DCMAKE_BUILD_TYPE=RELEASE && \
    make && \
    make install

FROM bitnami/minideb:latest
ENV LD_LIBRARY_PATH=${LD_LIBRARY_PATH}/opt/dodo/lib
ENV PATH=${PATH}:/opt/dodo/bin
RUN apt-get update && \
    apt-get install -y tzdata openssl libyaml-cpp0.6 net-tools
RUN echo "Amsterdam/Europe" > /etc/timezone && dpkg-reconfigure -f noninteractive tzdata
RUN groupadd -r dodo && \
    useradd -r -g dodo dodo && \
    mkdir -p /opt/dodo/conf && mkdir -p /opt/dodo/log && \
    chmod -R 700 /opt/dodo
COPY --from=builder /opt/dodo/lib /opt/dodo/lib
COPY --from=builder /opt/dodo/bin /opt/dodo/bin
RUN chown -R dodo:dodo /opt/dodo
USER dodo
WORKDIR /opt/dodo
EXPOSE 1968/tcp
HEALTHCHECK --interval=5s --timeout=3s CMD pidof service
ENTRYPOINT ["/opt/dodo/bin/service"]
```

Build the image from this Dockerfile, name it `dodo` and tag it `v1`.

```bash
$ cd src/examples/docker
$ ls minideb
Dockerfile  README.md  config.yaml  dodo-stack.yaml
$ docker build minideb -t dodo:v1
```

## Create the swarm

On this machine there are two NICs, so we pick one to attach the swarm to.

```bash
$ docker swarm init --advertise-addr 192.168.178.2
```

## Create the swarm stack file

```YAML
version: '3.7'

configs:
  dodo_v1:
    file: ./config.yaml

services:
  dodo:
    labels:
      org.dodo.example: "A swarm of TCP servers loadbalanced through a VIP"
    image: dodo:v1
    configs:
      - source: dodo_v1
        target: /opt/dodo/conf/config.yaml
    ports:
      - "1968:1968"
    deploy:
      endpoint_mode: vip
      replicas: 3
      update_config:
        parallelism: 1
        delay: 5s
      restart_policy:
        condition: on-failure
        max_attempts: 3
        window: '13s'
      resources:
        limits:
          cpus: '0.20'
          memory: 50M
        reservations:
          cpus: '0.10'
          memory: 20M
```

and deploy the stack

```bash
$ docker stack deploy -c dodo-stack.yaml dodo
```

```bash
$ docker config ls
ID                          NAME                CREATED             UPDATED
xj4gmsn7qj2mlhdiuaf0khvlz   dodo_dodo_v1        29 minutes ago      29 minutes ago
$ docker service ls
ID                  NAME                MODE                REPLICAS            IMAGE               PORTS
gm25c4e56j2l        dodo_dodo           replicated          3/3                 dodo:v1             *:1968->1968/tcp
$ docker ps
CONTAINER ID        IMAGE               COMMAND                  CREATED             STATUS                    PORTS               NAMES
9c1b2aeaaa67        dodo:v1             "/opt/dodo/bin/servi…"   30 minutes ago      Up 30 minutes (healthy)   1968/tcp            dodo_dodo.1.0e1wgjmxndih4kwf2yhgiuivi
160b058317a9        dodo:v1             "/opt/dodo/bin/servi…"   30 minutes ago      Up 30 minutes (healthy)   1968/tcp            dodo_dodo.2.ty0m61649acqur9uwl8nd0kgu
d8ad216a7ce5        dodo:v1             "/opt/dodo/bin/servi…"   30 minutes ago      Up 30 minutes (healthy)   1968/tcp            dodo_dodo.3.o5o2spha65up6fjptozukoop6
```

## show containers in service
```bash
$ docker service ps dodo
```

## show logs
```bash
docker service logs jxr6udrdjuf8 -f --tail 10
```