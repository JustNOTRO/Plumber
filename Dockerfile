# ---------- Build stage ----------
FROM alpine:latest as build

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

# ---------- Runtime stage ----------
FROM alpine:latest as runtime

RUN adduser --disabled-password --home /home/container container
RUN apk add --no-cache bash \
    libstdc++ \
    libgcc

USER container
ENV  USER=container HOME=/home/container
ENV CONFIG_PATH=/home/container/config.yaml

WORKDIR /home/container

COPY --from=build /usr/src/Plumber/build/Plumber /home/container/Plumber
COPY --from=build /usr/src/Plumber/config.yaml /home/container/config.yaml

COPY ./entrypoint.sh /entrypoint.sh

CMD ["/bin/bash", "/entrypoint.sh"]