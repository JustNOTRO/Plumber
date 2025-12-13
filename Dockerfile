FROM alpine:latest

RUN apk add --no-cache \
    git \
    openssl \
    openssl-dev \
    cmake \
    build-base

COPY . /usr/src/Plumber
WORKDIR /usr/src/Plumber

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug \
    && cmake --build build

ENV CONFIG_PATH=/usr/src/Plumber/config.yaml

EXPOSE 8080

CMD ["./build/Plumber"]