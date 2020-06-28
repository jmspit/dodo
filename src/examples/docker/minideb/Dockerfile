FROM bitnami/minideb:latest

RUN echo 'debconf debconf/frontend select Noninteractive' | debconf-set-selections
RUN apt-get update && apt-get install -y apt-utils
RUN apt-get install -y libssl-dev git g++ cmake

RUN mkdir -p /opt/build
WORKDIR /opt/build
RUN git clone https://github.com/jmspit/dodo.git
RUN cd dodo && mkdir build && cd build && cmake .. && make

