FROM bitnami/minideb:latest

RUN echo 'debconf debconf/frontend select Noninteractive' | debconf-set-selections
RUN apt-get update && apt-get install -y apt-utils
RUN apt-get install -y libssl-dev git g++ cmake

RUN mkdir -p /opt/dodo/build && mkdir -p /opt/dodo/bin && mkdir -p /opt/dodo/lib
WORKDIR /opt/dodo/build
RUN git clone https://github.com/jmspit/dodo.git
RUN cd dodo && mkdir build && cd build && cmake ..  -DCMAKE_INSTALL_PREFIX=/opt/dodo && make && make install
ENV LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/opt/dodo/lib
WORKDIR /opt/dodo

