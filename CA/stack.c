#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "cJSON.h"

/* Structure to store server response */
struct Response {
    char *data;
    size_t size;
};

/* Callback function called when data is received */
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t total_size = size * nmemb;

    struct Response *response = (struct Response *)userp;

    char *ptr = realloc(response->data,
                        response->size + total_size + 1);

    if (ptr == NULL)
    {
        printf("Memory allocation failed!\n");
        return 0;
    }

    response->data = ptr;

    memcpy(&(response->data[response->size]),
           contents,
           total_size);

    response->size += total_size;

    response->data[response->size] = '\0';

    return total_size;
}

int main()
{
    CURL *curl;
    CURLcode result;

    struct Response response;

    response.data = malloc(1);
    response.size = 0;

    /* Initialize CURL */
    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();

    if (curl == NULL)
    {
        printf("CURL initialization failed!\n");
        return 1;
    }

    /* API URL */
    curl_easy_setopt(
        curl,
        CURLOPT_URL,
        "https://jsonplaceholder.typicode.com/users/1"
    );

    /* Function to receive response */
    curl_easy_setopt(
        curl,
        CURLOPT_WRITEFUNCTION,
        write_callback
    );

    /* Pass response structure */
    curl_easy_setopt(
        curl,
        CURLOPT_WRITEDATA,
        &response
    );

    /* Send request */
    result = curl_easy_perform(curl);

    if (result != CURLE_OK)
    {
        printf("Request failed: %s\n",
               curl_easy_strerror(result));
    }
    else
    {
        printf("\n========== SERVER RESPONSE ==========\n");
        printf("%s\n", response.data);

        /* Parse JSON */
        cJSON *json = cJSON_Parse(response.data);

        if (json == NULL)
        {
            printf("\nInvalid JSON response!\n");
        }
        else
        {
            cJSON *id =
                cJSON_GetObjectItem(json, "id");

            cJSON *name =
                cJSON_GetObjectItem(json, "name");

            cJSON *username =
                cJSON_GetObjectItem(json, "username");

            cJSON *email =
                cJSON_GetObjectItem(json, "email");

            printf("\n========== PARSED DATA ==========\n");

            printf("ID       : %d\n", id->valueint);
            printf("Name     : %s\n", name->valuestring);
            printf("Username : %s\n", username->valuestring);
            printf("Email    : %s\n", email->valuestring);

            cJSON_Delete(json);
        }
    }

    /* Cleanup */
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    free(response.data);

    return 0;
}