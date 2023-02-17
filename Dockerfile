# Copyright (C) 2023  Christian Berger
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Part to generate /tmp/peak-can.hpp.
FROM chalmersrevere/dbc2odvd-amd64:v0.0.6 as generator
MAINTAINER Christian Berger "christian.berger@gu.se"

ADD src/peak-can.dbc /opt
ADD src/peak-can.odvd /opt
WORKDIR /opt
RUN generateHeaderOnly.sh peak-can.dbc peak-can.odvd && \
    cp peak_can.hpp /tmp


# Part to build ascii2peak against musl-libc.
FROM alpine:3.17.2 as alpine_builder
MAINTAINER Christian Berger "christian.berger@gu.se"

RUN apk update && \
    apk --no-cache add \
        cmake \
        g++ \
        linux-headers \
        make \
        upx

RUN mkdir -p /opt/sources/build
COPY --from=generator /tmp/peak_can.hpp /opt/sources/build

ADD . /opt/sources
WORKDIR /opt/sources

RUN cd /opt/sources/build && \
    cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/tmp/ascii2peak-dest .. && \
    make && \
    c++ -o ascii2peak -static CMakeFiles/ascii2peak.dir/src/ascii2peak.cpp.o && \
    mkdir -p /tmp/ascii2peak-dest/bin && \
    cp ascii2peak /tmp/ascii2peak-dest/bin && \
    strip /tmp/ascii2peak-dest/bin/ascii2peak && \
    upx -9 /tmp/ascii2peak-dest/bin/ascii2peak


# Part to deploy ascii2peak against musl-libc
FROM alpine:3.17.2 as alpine_deploy
MAINTAINER Christian Berger "christian.berger@gu.se"

WORKDIR /usr/bin
COPY --from=alpine_builder /tmp/ascii2peak-dest/bin/ascii2peak .
ENTRYPOINT ["/usr/bin/ascii2peak"]

# Part to build ascii2peak against GNU libc.
FROM ubuntu:22.04 as ubuntu_builder
MAINTAINER Christian Berger "christian.berger@gu.se"

RUN apt-get update -y && \
    apt-get upgrade -y && \
    apt-get dist-upgrade -y && \
    apt-get install -y --no-install-recommends \
        ca-certificates \
        cmake \
        build-essential \
        upx

RUN mkdir -p /opt/sources/build
COPY --from=generator /tmp/peak_can.hpp /opt/sources/build

ADD . /opt/sources
WORKDIR /opt/sources

RUN cd /opt/sources/build && \
    cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/tmp/ascii2peak-dest .. && \
    make && \
    c++ -o ascii2peak -static CMakeFiles/ascii2peak.dir/src/ascii2peak.cpp.o && \
    mkdir -p /tmp/ascii2peak-dest/bin && \
    cp ascii2peak /tmp/ascii2peak-dest/bin && \
    strip /tmp/ascii2peak-dest/bin/ascii2peak && \
    upx -9 /tmp/ascii2peak-dest/bin/ascii2peak


# Part to deploy ascii2peak against GNU libc.
FROM ubuntu:22.04 as ubuntu_deploy
MAINTAINER Christian Berger "christian.berger@gu.se"

WORKDIR /usr/bin
COPY --from=ubuntu_builder /tmp/ascii2peak-dest/bin/ascii2peak .
ENTRYPOINT ["/usr/bin/ascii2peak"]

