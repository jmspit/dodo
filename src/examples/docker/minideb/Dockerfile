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
#HEALTHCHECK --interval=5s --timeout=3s CMD grep ':07B0 00000000:0000 0A' /proc/net/tcp
HEALTHCHECK --interval=5s --timeout=3s CMD echo
ENTRYPOINT ["/opt/dodo/bin/service"]
