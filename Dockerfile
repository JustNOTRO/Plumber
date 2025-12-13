FROM alpine:latest

# Install build tools
RUN apk add --no-cache \
    git \
    openssl \
    openssl-dev \
    cmake \
    build-base

# Copy source code
COPY . /usr/src/Plumber
WORKDIR /usr/src/Plumber

# Build
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug \
    && cmake --build build

ENV CONFIG_PATH=/usr/src/Plumber/config.yaml

EXPOSE 8080

# Run
CMD ["./build/Plumber"]