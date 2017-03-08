FROM ubuntu:14.04

RUN apt-get update && apt-get install --auto-remove -y make g++ libboost-all-dev && apt-get clean autoclean && apt-get autoremove -y && rm -rf /tmp/* /var/lib/{apt,dpkg,cache,log}

WORKDIR /opt/webserver

COPY . /opt/webserver

RUN make clean && make Webserver

CMD ["tar", "-cf", "-", "Webserver"]

