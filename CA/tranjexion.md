# C API Client Project

## Overview

If you want to practice server and API communication in C, a great beginner-to-intermediate project is a small HTTP client that can:

- connect to a server
- send an HTTP request
- receive a response
- store the data in memory
- parse JSON data
- display useful information to the user

This project uses two important libraries:

- libcurl: for HTTP communication
- cJSON: for parsing JSON responses

---

## What You Will Learn

By building this project, you will learn:

- how C programs communicate with web servers
- how to send HTTP requests using libcurl
- how to handle response data dynamically
- how to parse JSON objects in C
- how to display extracted data in a readable format

---

## Required Libraries

Before compiling this program, make sure the following libraries are installed:

- libcurl
- cJSON

On Linux, you may install them using:

```bash
sudo apt-get install libcurl4-openssl-dev libcjson-dev
```

On other systems, use the equivalent package manager or development environment tools.

---

## Example C Program: HTTP API Client

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cJSON.h"

/* Structure to store HTTP response data */
struct Response {
    char *data;
    size_t size;
};

/* Callback function called when data is received from the server */
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t total_size = size * nmemb;
    struct Response *response = (struct Response *)userp;

    char *ptr = realloc(response->data, response->size + total_size + 1);

    if (ptr == NULL)
    {
        printf("Memory allocation failed!\n");
        return 0;
    }

    response->data = ptr;

    memcpy(&(response->data[response->size]), contents, total_size);
    response->size += total_size;
    response->data[response->size] = '\0';

    return total_size;
}

int main(void)
{
    CURL *curl;
    CURLcode result;
    struct Response response;

    response.data = malloc(1);
    response.size = 0;

    if (response.data == NULL)
    {
        printf("Failed to allocate initial memory.\n");
        return 1;
    }

    /* Initialize libcurl */
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl == NULL)
    {
        printf("CURL initialization failed!\n");
        free(response.data);
        return 1;
    }

    /* Target API endpoint */
    curl_easy_setopt(curl, CURLOPT_URL, "https://jsonplaceholder.typicode.com/users/1");

    /* Set callback to receive response data */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    /* Perform the HTTP request */
    result = curl_easy_perform(curl);

    if (result != CURLE_OK)
    {
        printf("Request failed: %s\n", curl_easy_strerror(result));
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        free(response.data);
        return 1;
    }

    printf("\n========== SERVER RESPONSE ==========");
    printf("\n%s\n", response.data);

    /* Parse JSON response */
    cJSON *json = cJSON_Parse(response.data);

    if (json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            printf("JSON parse error before: %s\n", error_ptr);
        }
        printf("Invalid JSON response!\n");
    }
    else
    {
        cJSON *id = cJSON_GetObjectItemCaseSensitive(json, "id");
        cJSON *name = cJSON_GetObjectItemCaseSensitive(json, "name");
        cJSON *username = cJSON_GetObjectItemCaseSensitive(json, "username");
        cJSON *email = cJSON_GetObjectItemCaseSensitive(json, "email");

        printf("\n========== PARSED DATA ==========");

        if (cJSON_IsNumber(id))
            printf("\nID       : %d", id->valueint);

        if (cJSON_IsString(name) && (name->valuestring != NULL))
            printf("\nName     : %s", name->valuestring);

        if (cJSON_IsString(username) && (username->valuestring != NULL))
            printf("\nUsername : %s", username->valuestring);

        if (cJSON_IsString(email) && (email->valuestring != NULL))
            printf("\nEmail    : %s", email->valuestring);

        printf("\n");
        cJSON_Delete(json);
    }

    /* Cleanup */
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    free(response.data);

    return 0;
}
```

---

## Explanation of the Program

### 1. Including required headers

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cJSON.h"
```

These headers provide:

- `stdio.h`: input and output functions
- `stdlib.h`: memory allocation functions like `malloc` and `free`
- `string.h`: string and memory functions like `memcpy`
- `curl/curl.h`: libcurl functions
- `cJSON.h`: JSON parsing functions

### 2. Response storage structure

```c
struct Response {
    char *data;
    size_t size;
};
```

This structure stores the response received from the HTTP server. It keeps:

- the actual response string
- the total size of the received data

### 3. write_callback function

```c
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp)
```

This callback is called by libcurl whenever new data is received. It appends the received bytes to the `response.data` buffer. This is important because the server may send data in chunks, not all at once.

### 4. Sending the request

```c
curl_easy_setopt(curl, CURLOPT_URL, "https://jsonplaceholder.typicode.com/users/1");
```

This tells libcurl which URL to request.

### 5. Receiving the response

```c
curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
```

These lines connect the callback function and the response structure so that incoming HTTP data is saved properly.

### 6. Performing the request

```c
result = curl_easy_perform(curl);
```

This sends the HTTP request and waits for the response.

### 7. Parsing JSON

```c
cJSON *json = cJSON_Parse(response.data);
```

The returned response is usually JSON text. This function converts the JSON string into a C structure that can be read easily.

Then the code reads values such as:

```c
cJSON *id = cJSON_GetObjectItemCaseSensitive(json, "id");
cJSON *name = cJSON_GetObjectItemCaseSensitive(json, "name");
```

These commands extract specific keys from the JSON object.

---

## Example JSON Response

The API may return data like this:

```json
{
  "id": 1,
  "name": "Leanne Graham",
  "username": "Bret",
  "email": "Sincere@april.biz"
}
```

The program prints:

```text
========== SERVER RESPONSE ==========
{
  "id": 1,
  "name": "Leanne Graham",
  "username": "Bret",
  "email": "Sincere@april.biz"
}

========== PARSED DATA ==========
ID       : 1
Name     : Leanne Graham
Username : Bret
Email    : Sincere@april.biz
```

---

## How the Program Works

```text
C Program
   │
   │ HTTP GET Request
   ▼
API Server
   │
   │ JSON Response
   ▼
libcurl
   │
   ▼
write_callback()
   │
   ▼
Dynamic memory stores response
   │
   ▼
cJSON parser
   │
   ▼
Extract fields such as id, name, and email
   │
   ▼
Display result to the user
```

---

## Compile the Program

Use this command:

```bash
gcc api_client.c -o api_client -lcurl -lcjson
```

Then run it:

```bash
./api_client
```

---

## Libraries Used

| Library | Purpose |
| --- | --- |
| stdio.h | Input and output functions |
| stdlib.h | Memory allocation and standard utilities |
| string.h | String and memory functions |
| libcurl | Sending HTTP requests and receiving responses |
| cJSON | Parsing JSON data |

---

## Possible Improvements for a Bigger Project

A more advanced version of this project could include:

- GET requests
- POST requests
- PUT requests
- DELETE requests
- request headers
- authentication tokens
- JSON request body creation
- response status code handling
- timeout handling
- retry logic
- better error messages
- logging
- saving response into files
- database integration

A larger project idea could be:

```text
C API Client
├── GET
├── POST
├── PUT
├── DELETE
├── Headers
├── Authentication
├── JSON request support
├── JSON response parsing
├── Timeout handling
├── Error handling
├── Retry mechanism
├── Logging
├── File saving
└── Database integration
```

---

## Summary

This C project is a practical and useful introduction to working with APIs. It shows how to connect to a remote service, retrieve data, parse JSON, and present meaningful information to the user. It is a strong foundation for building larger networking applications in C.

This is a good starting point for students who want to learn real-world programming with web APIs and C.
