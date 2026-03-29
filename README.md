# Plumber

Plumber is a lightweight C++ service designed to automatically retry GitLab pipelines in order to evaluate unit test stability across multiple executions. It is particularly useful for identifying unstable tests and ensuring consistent CI behavior.

Plumber listens for incoming GitLab webhooks and triggers multiple retries of a pipeline when invoked. By analyzing the outcomes of repeated runs, it helps identify unstable or inconsistent unit tests.

## Features

* Automatically retries GitLab pipelines when triggered by tagging the bot with @bot_username
* Evaluates unit test stability across multiple executions
* Provides a simple HTTP server interface
* Supports deployment across various environments
* Implemented using modern C++

## Requirements

* Appropriate permissions are required (such as an access token)

## Running the Project

### 1. Run Locally

Build and start the HTTP server:

```bash
mkdir build
cd build
cmake ..
make
./Plumber
```

Refer to the **Configuration** section to ensure that all required environment variables (e.g., GitLab token, webhook secret) are properly set.

---

### 2. Run with Docker

Build the Docker image:

```bash
docker build -t Plumber .
```

Run the container:

```bash
docker run -p 8080:8080 Plumber
```

---

### 3. Deploy on Pterodactyl Panel

Plumber can be deployed using a custom egg in Pterodactyl:

* Create a new server using a generic Docker environment
* Build or pull the Docker image (see note below)
* Configure the startup command to execute the binary

** **
A public container registry (e.g., GHCR) is not currently available.
You will need to build and host the image yourself until one is provided.

---

## Configuration

Configuration is managed via environment variables:

* `BOT_USERNAME` – Required; the GitLab bot username
* `GITLAB_ACCESS_TOKEN` – Required; the bot's access token
* `GITLAB_INSTANCE` – Required; e.g., https://gitlab.com
* `JOB_NAME` – Required; the name of the unit test job
* `RETRY_AMOUNT` – Number of pipeline retries
* `SERVER_IP` – Required; the server IP address
* `SERVER_PORT` – Required; the server port
* `SSL_CERT_PATH` – Optional; path to SSL certificate for HTTPS
* `SSL_KEY_PATH` – Optional; path to SSL key for HTTPS

---

## Use Cases

* Detect unstable unit tests
* Validate CI reliability
* Stress-test pipelines
* Improve test consistency

---


## API Endpoints

| HTTP Request |    HTTP Endpoint      |                    Description                               |
| ------------------------- | -------- | ------------------------------------------------- |
| POST                      | /webhook | Receives GitLab webhook events to trigger retries |
