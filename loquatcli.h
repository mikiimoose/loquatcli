#ifndef LOQUATCLI_H
#define LOQUATCLI_H

#include <curl/curl.h>

// Structure to hold response data
typedef struct {
    char *data;
    size_t size;
} ResponseData;

// Structure for the HTTP client
typedef struct {
    CURL *curl;
    char base_url[256];
} LoquatClient;

// Function declarations

/**
 * Initialize the HTTP client
 * @param base_url The base URL for the client (can be NULL for default)
 * @return Pointer to LoquatClient structure, or NULL on failure
 */
LoquatClient* loquat_client_init(const char *base_url);

/**
 * Clean up the HTTP client and free memory
 * @param client Pointer to LoquatClient structure
 */
void loquat_client_cleanup(LoquatClient *client);

/**
 * Set a new base URL for the client
 * @param client Pointer to LoquatClient structure
 * @param url New base URL
 */
void loquat_client_set_base_url(LoquatClient *client, const char *url);

/**
 * Get the current base URL
 * @param client Pointer to LoquatClient structure
 * @return Current base URL string, or NULL if client is NULL
 */
const char* loquat_client_get_base_url(LoquatClient *client);

/**
 * Make a GET request
 * @param client Pointer to LoquatClient structure
 * @param endpoint The endpoint to request (will be appended to base_url)
 * @param response Pointer to store the response string (caller must free with loquat_client_free_response)
 * @param http_code Pointer to store the HTTP response code
 * @return 1 on success, 0 on failure
 */
int loquat_client_get(LoquatClient *client, const char *endpoint, char **response, int *http_code);

/**
 * Make a POST request
 * @param client Pointer to LoquatClient structure
 * @param endpoint The endpoint to request (will be appended to base_url)
 * @param post_data The data to send in the POST request (can be NULL for empty POST)
 * @param response Pointer to store the response string (caller must free with loquat_client_free_response)
 * @param http_code Pointer to store the HTTP response code
 * @return 1 on success, 0 on failure
 */
int loquat_client_post(LoquatClient *client, const char *endpoint, const char *post_data, char **response, int *http_code);

/**
 * Make a GET request with custom headers
 * @param client Pointer to LoquatClient structure
 * @param endpoint The endpoint to request (will be appended to base_url)
 * @param headers Array of header strings in format "Header-Name: value"
 * @param header_count Number of headers in the array
 * @param response Pointer to store the response string (caller must free with loquat_client_free_response)
 * @param http_code Pointer to store the HTTP response code
 * @return 1 on success, 0 on failure
 */
int loquat_client_get_with_headers(LoquatClient *client, const char *endpoint, 
                                   char **headers, int header_count, 
                                   char **response, int *http_code);

/**
 * Free response memory allocated by the client
 * @param response Pointer to response string to free
 */
void loquat_client_free_response(char *response);

#endif // LOQUATCLI_H 