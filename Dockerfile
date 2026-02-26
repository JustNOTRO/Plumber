# ---------- Build stage ----------
FROM alpine:latest as build

RUN apk add --no-cache \
    git \
    openssl-dev \
    cmake \
    zlib-dev \
    curl \
    curl-dev \
    build-base

COPY src /usr/src/Plumber/src
COPY CMakeLists.txt /usr/src/Plumber/CMakeLists.txt
COPY third_party /usr/src/Plumber/third_party

WORKDIR /usr/src/Plumber

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug \
    && cmake --build build

# ---------- Runtime stage ----------
FROM alpine:latest as runtime

RUN adduser --disabled-password --home /home/container container
RUN apk add --no-cache bash \
    libstdc++ \
    libgcc \
    brotli-libs \
    zstd-libs

USER container
ENV USER=container HOME=/home/container

WORKDIR /home/container

COPY --from=build /usr/src/Plumber/build/Plumber /build/Plumber

COPY ./entrypoint.sh /entrypoint.sh

CMD ["/bin/bash", "/entrypoint.sh"]