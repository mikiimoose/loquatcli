#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include "loquatcli.h"

#define MAX_URL_LENGTH 2048
#define MAX_RESPONSE_LENGTH 8192
#define DEFAULT_TIMEOUT 30

// Callback function to handle the response data
static size_t write_callback(void *contents, size_t size, size_t nmemb, ResponseData *resp) {
    size_t realsize = size * nmemb;
    char *ptr = realloc(resp->data, resp->size + realsize + 1);
    
    if (ptr == NULL) {
        printf("Memory allocation failed\n");
        return 0;
    }
    
    resp->data = ptr;
    memcpy(&(resp->data[resp->size]), contents, realsize);
    resp->size += realsize;
    resp->data[resp->size] = 0;
    
    return realsize;
}

// Initialize the HTTP client
LoquatClient* loquat_client_init(const char *base_url) {
    LoquatClient *client = malloc(sizeof(LoquatClient));
    if (!client) {
        fprintf(stderr, "Failed to allocate memory for client\n");
        return NULL;
    }
    
    // Initialize CURL
    curl_global_init(CURL_GLOBAL_ALL);
    client->curl = curl_easy_init();
    
    if (!client->curl) {
        fprintf(stderr, "Failed to initialize CURL\n");
        free(client);
        return NULL;
    }
    
    // Set base URL
    if (base_url) {
        strncpy(client->base_url, base_url, sizeof(client->base_url) - 1);
        client->base_url[sizeof(client->base_url) - 1] = '\0';
    } else {
        strcpy(client->base_url, "http://localhost:8080");
    }
    
    return client;
}

// Clean up the HTTP client
void loquat_client_cleanup(LoquatClient *client) {
    if (client) {
        if (client->curl) {
            curl_easy_cleanup(client->curl);
        }
        curl_global_cleanup();
        free(client);
    }
}

// Set a new base URL
void loquat_client_set_base_url(LoquatClient *client, const char *url) {
    if (client && url) {
        strncpy(client->base_url, url, sizeof(client->base_url) - 1);
        client->base_url[sizeof(client->base_url) - 1] = '\0';
    }
}

// Get the current base URL
const char* loquat_client_get_base_url(LoquatClient *client) {
    return client ? client->base_url : NULL;
}

// Make a GET request
int loquat_client_get(LoquatClient *client, const char *command, char **response, int *http_code) {
    if (!client || !client->curl || !command || !response || !http_code) {
        return 0;
    }
    
    char url[MAX_URL_LENGTH];
    snprintf(url, sizeof(url), "%s/%s", client->base_url, command);
    
    // Initialize response data
    ResponseData resp = {0};
    resp.data = malloc(1);
    resp.size = 0;
    
    if (!resp.data) {
        fprintf(stderr, "Failed to allocate memory for response\n");
        return 0;
    }
    
    // Reset curl handle for new request
    curl_easy_reset(client->curl);
    
    // Set the URL
    curl_easy_setopt(client->curl, CURLOPT_URL, url);
    
    // Set the callback function to receive data
    curl_easy_setopt(client->curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(client->curl, CURLOPT_WRITEDATA, &resp);
    
    // Set timeout
    curl_easy_setopt(client->curl, CURLOPT_TIMEOUT, DEFAULT_TIMEOUT);
    
    // Follow redirects
    curl_easy_setopt(client->curl, CURLOPT_FOLLOWLOCATION, 1L);
    
    // Set user agent
    curl_easy_setopt(client->curl, CURLOPT_USERAGENT, "LoquatClient/1.0");
    
    // Enable verbose output for debugging
    curl_easy_setopt(client->curl, CURLOPT_VERBOSE, 0L);
    
    // Perform the request
    CURLcode res = curl_easy_perform(client->curl);
    
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        free(resp.data);
        return 0;
    }
    
    // Get HTTP response code
    curl_easy_getinfo(client->curl, CURLINFO_RESPONSE_CODE, http_code);
    
    // Set response pointer
    *response = resp.data;
    
    return 1;
}

// Make a POST request
int loquat_client_post(LoquatClient *client, const char *endpoint, const char *post_data, char **response, int *http_code) {
    if (!client || !client->curl || !endpoint || !response || !http_code) {
        return 0;
    }
    
    char url[MAX_URL_LENGTH];
    snprintf(url, sizeof(url), "%s/%s", client->base_url, endpoint);
    
    // Initialize response data
    ResponseData resp = {0};
    resp.data = malloc(1);
    resp.size = 0;
    
    if (!resp.data) {
        fprintf(stderr, "Failed to allocate memory for response\n");
        return 0;
    }
    
    // Reset curl handle for new request
    curl_easy_reset(client->curl);
    
    // Set the URL
    curl_easy_setopt(client->curl, CURLOPT_URL, url);
    
    // Set POST method
    curl_easy_setopt(client->curl, CURLOPT_POST, 1L);
    
    // Set POST data if provided
    if (post_data && strlen(post_data) > 0) {
        curl_easy_setopt(client->curl, CURLOPT_POSTFIELDS, post_data);
        curl_easy_setopt(client->curl, CURLOPT_POSTFIELDSIZE, strlen(post_data));
    }
    
    // Set the callback function to receive data
    curl_easy_setopt(client->curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(client->curl, CURLOPT_WRITEDATA, &resp);
    
    // Set timeout - longer for connect command
    long timeout = (strcmp(endpoint, "connect") == 0) ? 120L : DEFAULT_TIMEOUT;
    curl_easy_setopt(client->curl, CURLOPT_TIMEOUT, timeout);
    
    // Follow redirects
    curl_easy_setopt(client->curl, CURLOPT_FOLLOWLOCATION, 1L);
    
    // Set user agent
    curl_easy_setopt(client->curl, CURLOPT_USERAGENT, "LoquatClient/1.0");
    
    // Enable verbose output for debugging
    curl_easy_setopt(client->curl, CURLOPT_VERBOSE, 1L);
    
    // Perform the request
    CURLcode res = curl_easy_perform(client->curl);
    
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        free(resp.data);
        return 0;
    }
    
    // Get HTTP response code
    curl_easy_getinfo(client->curl, CURLINFO_RESPONSE_CODE, http_code);
    
    // Set response pointer
    *response = resp.data;
    
    return 1;
}

// Make a GET request with custom headers
int loquat_client_get_with_headers(LoquatClient *client, const char *endpoint, 
                                   char **headers, int header_count, 
                                   char **response, int *http_code) {
    if (!client || !client->curl || !endpoint || !response || !http_code) {
        return 0;
    }
    
    char url[MAX_URL_LENGTH];
    snprintf(url, sizeof(url), "%s%s", client->base_url, endpoint);
    
    // Initialize response data
    ResponseData resp = {0};
    resp.data = malloc(1);
    resp.size = 0;
    
    if (!resp.data) {
        fprintf(stderr, "Failed to allocate memory for response\n");
        return 0;
    }
    
    // Reset curl handle for new request
    curl_easy_reset(client->curl);
    
    // Set the URL
    curl_easy_setopt(client->curl, CURLOPT_URL, url);
    
    // Set the callback function to receive data
    curl_easy_setopt(client->curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(client->curl, CURLOPT_WRITEDATA, &resp);
    
    // Set timeout
    curl_easy_setopt(client->curl, CURLOPT_TIMEOUT, DEFAULT_TIMEOUT);
    
    // Follow redirects
    curl_easy_setopt(client->curl, CURLOPT_FOLLOWLOCATION, 1L);
    
    // Set user agent
    curl_easy_setopt(client->curl, CURLOPT_USERAGENT, "LoquatClient/1.0");
    
    // Set custom headers
    struct curl_slist *header_list = NULL;
    for (int i = 0; i < header_count; i++) {
        if (headers[i]) {
            header_list = curl_slist_append(header_list, headers[i]);
        }
    }
    curl_easy_setopt(client->curl, CURLOPT_HTTPHEADER, header_list);
    
    // Perform the request
    CURLcode res = curl_easy_perform(client->curl);
    
    // Clean up headers
    curl_slist_free_all(header_list);
    
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        free(resp.data);
        return 0;
    }
    
    // Get HTTP response code
    curl_easy_getinfo(client->curl, CURLINFO_RESPONSE_CODE, http_code);
    
    // Set response pointer
    *response = resp.data;
    
    return 1;
}

// Free response memory
void loquat_client_free_response(char *response) {
    if (response) {
        free(response);
    }
}

int is_valid_command(const char *command) {
    if (!command) return 0;
    // Add more commands as needed
    if (strcmp(command, "get_scan_result") == 0) return 1;
    if (strcmp(command, "status") == 0) return 1;
    if (strcmp(command, "get_status") == 0) return 1;
    if (strcmp(command, "connect") == 0) return 1;
    if (strcmp(command, "apikey") == 0) return 1;
    if (strcmp(command, "get_net_info") == 0) return 1;

    return 0;
}

// Returns 1 if the command should be sent as a GET, 0 otherwise
int is_get_command(const char *command) {
    if (!command) return 0;
    // Add more GET commands as needed
    if (strcmp(command, "get_scan_result") == 0) return 1;
    if (strcmp(command, "status") == 0) return 1;
    if (strcmp(command, "get_status") == 0) return 1;
    if (strcmp(command, "get_net_info") == 0) return 1;
    // Default: not a GET command (POST commands like "connect")
    return 0;
}

void print_scan_result(const char *response) {
    if (!response) {
        fprintf(stderr, "No response data\n");
        return;
    }
    
    fprintf(stderr, "\n=== WiFi Access Points ===\n");
    fprintf(stderr, "%-40s %-8s %-12s\n", "SSID", "Bars", "Security");
    fprintf(stderr, "%-40s %-8s %-12s\n", "--------------------", "--------", "------------");
    
    // Parse JSON using cJSON
    cJSON *json = cJSON_Parse(response);
    if (!json) {
        fprintf(stderr, "Failed to parse JSON response\n");
        return;
    }
    
    // Check if it's an array
    if (!cJSON_IsArray(json)) {
        fprintf(stderr, "Expected JSON array\n");
        cJSON_Delete(json);
        return;
    }
    
    // Iterate through the array
    int array_size = cJSON_GetArraySize(json);
    for (int i = 0; i < array_size; i++) {
        cJSON *ap = cJSON_GetArrayItem(json, i);
        if (!cJSON_IsObject(ap)) continue;
        
        // Get SSID
        cJSON *ssid_json = cJSON_GetObjectItem(ap, "ssid");
        const char *ssid = (ssid_json && cJSON_IsString(ssid_json)) ? ssid_json->valuestring : "Unknown";
        
        // Get bars
        cJSON *bars_json = cJSON_GetObjectItem(ap, "bars");
        int bars = (bars_json && cJSON_IsNumber(bars_json)) ? bars_json->valueint : 0;
        
        // Get security
        cJSON *security_json = cJSON_GetObjectItem(ap, "security");
        const char *security = (security_json && cJSON_IsString(security_json)) ? security_json->valuestring : "Unknown";
        
        // Print the AP information in a row
        fprintf(stderr, "%-40s %-8d %-12s\n", ssid, bars, security);
    }
    
    fprintf(stderr, "\n");
    
    // Clean up
    cJSON_Delete(json);
}

void print_net_info_response(const char *response) {
    if (!response) {
        fprintf(stderr, "No response data\n");
        return;
    }
    
    fprintf(stderr, "\n=== Network Information ===\n");
    
    // Parse JSON using cJSON
    cJSON *json = cJSON_Parse(response);
    if (!json) {
        fprintf(stderr, "Failed to parse JSON response\n");
        return;
    }
    
    // Get status
    cJSON *status_json = cJSON_GetObjectItem(json, "status");
    const char *status = (status_json && cJSON_IsString(status_json)) ? status_json->valuestring : "Unknown";
    fprintf(stderr, "Connection Status: %s\n", status);
    
    // Get IP address
    cJSON *ip_json = cJSON_GetObjectItem(json, "ip_address");
    const char *ip = (ip_json && cJSON_IsString(ip_json)) ? ip_json->valuestring : "Unknown";
    fprintf(stderr, "IP Address: %s\n", ip);
    
    // Get SSID
    cJSON *ssid_json = cJSON_GetObjectItem(json, "ssid");
    const char *ssid = (ssid_json && cJSON_IsString(ssid_json)) ? ssid_json->valuestring : "Unknown";
    fprintf(stderr, "Connected SSID: %s\n", ssid);
    
    fprintf(stderr, "\n");
    
    // Clean up
    cJSON_Delete(json);
}

void print_response(const char *command, const char *response) {
    if (strcmp(command, "get_scan_result") == 0) {
        print_scan_result(response);
    } else if (strcmp(command, "connect") == 0) {
        fprintf(stderr, "Response:\n%s\n", response);
    } else if (strcmp(command, "apikey") == 0) {
        fprintf(stderr, "Response:\n%s\n", response);
    } else if (strcmp(command, "get_net_info") == 0) {
        print_net_info_response(response);
    } else {
        fprintf(stderr, "Invalid print response: %s\n", command);
    }
}

char* get_post_connect_wifi_data(const char *ssid, const char *psk, const char *security) {
    // Create JSON object for WiFi connection
    cJSON *json = cJSON_CreateObject();
    if (!json) {
        fprintf(stderr, "Failed to create JSON object\n");
        return NULL;
    }
    
    // Add SSID (always required)
    if (ssid && strlen(ssid) > 0) {
        cJSON_AddStringToObject(json, "ssid", ssid);
    } else {
        fprintf(stderr, "Error: SSID is required for connect command\n");
        cJSON_Delete(json);
        return NULL;
    }
    
    // Handle security types
    const char *sec_type = (security && strlen(security) > 0) ? security : "WPA2";
    
    // Check if it's an open network
    if (strcmp(sec_type, "Open") == 0 || strcmp(sec_type, "open") == 0) {
        // Open network - no password required
        cJSON_AddStringToObject(json, "security", "Open");
        cJSON_AddStringToObject(json, "psk", ""); // Empty password for open networks
    } else {
        // Private network - password required
        if (!psk || strlen(psk) == 0) {
            fprintf(stderr, "Error: PSK (password) is required for private networks (security: %s)\n", sec_type);
            cJSON_Delete(json);
            return NULL;
        }
        
        cJSON_AddStringToObject(json, "psk", psk);
        cJSON_AddStringToObject(json, "security", sec_type);
    }
    
    // Convert JSON to string
    char *json_string = cJSON_Print(json);
    cJSON_Delete(json);
    
    if (!json_string) {
        fprintf(stderr, "Failed to create JSON string\n");
        return NULL;
    }
    
    return json_string;
}

char* get_post_apikey_data(const char *apikey, const char *aiserver) {
    if (!apikey || !aiserver) {
        fprintf(stderr, "Error: API key and AI server are required\n");
        return NULL;
    }

    // Create JSON object for API key
    cJSON *json = cJSON_CreateObject();
    if (!json) {
        fprintf(stderr, "Failed to create JSON object\n");
        return NULL;
    }
    
    // Add API key
    if (apikey && strlen(apikey) > 0) {
        cJSON_AddStringToObject(json, "apikey", apikey);
    } else {
        fprintf(stderr, "Error: API key is required\n");
        cJSON_Delete(json);
        return NULL;
    }
    
    // Add AI server
    if (aiserver && strlen(aiserver) > 0) {
        cJSON_AddStringToObject(json, "aiserver", aiserver);
    } else {
        fprintf(stderr, "Error: AI server is required\n");
        cJSON_Delete(json);
        return NULL;
    }

    // Convert JSON to string
    char *json_string = cJSON_Print(json);
    cJSON_Delete(json);
    
    if (!json_string) {
        fprintf(stderr, "Failed to create JSON string\n");
        return NULL;
    }
    
    return json_string;
}



// Example usage and main function
int main(int argc, char *argv[]) {
    char *server = NULL;
    char *port = NULL;
    char *command = NULL;
    char *ssid = NULL;
    char *psk = NULL;
    char *security = NULL;
    char *apikey = NULL;
    char *aiserver = NULL;
    
    int opt;
    const char *optstring = "s:p:c:w:k:e:a:i:";
    static struct option long_options[] = {
        {"server", required_argument, 0, 's'},
        {"port", required_argument, 0, 'p'},
        {"com", required_argument, 0, 'c'},
        {"ssid", required_argument, 0, 'w'},
        {"psk", required_argument, 0, 'k'},
        {"security", required_argument, 0, 'e'},
        {"apikey", required_argument, 0, 'a'},
        {"aiserver", required_argument, 0, 'i'},
        {0, 0, 0, 0}
    };
    
    // Parse command line arguments
    while ((opt = getopt_long(argc, argv, optstring, long_options, NULL)) != -1) {
        switch (opt) {
            case 's':
                server = optarg;
                break;
            case 'p':
                port = optarg;
                break;
            case 'c':
                command = optarg;
                break;
            case 'w':
                ssid = optarg;
                break;
            case 'k':
                psk = optarg;
                break;
            case 'e':
                security = optarg;
                break;
            case 'a':
                apikey = optarg;
                break;
            case 'i':
                aiserver = optarg;
                break;
            case '?':
                fprintf(stderr, "Usage: %s --server <ip> --port <port> --com <command> [--ssid <ssid>] [--psk <key>] [--security <type>] [--apikey <key>] [--aiserver <server>]\n", argv[0]);
                fprintf(stderr, "Example: %s --server 192.168.1.100 --port 8080 --com get_scan_result\n", argv[0]);
                fprintf(stderr, "Example: %s --server localhost --port 3000 --com status --ssid MyWiFi --psk password123 --security WPA2\n", argv[0]);
                fprintf(stderr, "Example: %s --server api.example.com --port 443 --com connect --ssid MyWiFi --psk password123 --apikey your-api-key\n", argv[0]);
                fprintf(stderr, "Example: %s --server 192.168.1.100 --port 8080 --com get_scan_result --aiserver ai.example.com\n", argv[0]);
                return 1;
            default:
                fprintf(stderr, "Unknown option: %c\n", opt);
                return 1;
        }
    }
    
    // Check required parameters
    if (!server || !port || !command) {
        fprintf(stderr, "Error: --server, --port, and --com are required parameters\n");
        fprintf(stderr, "Usage: %s --server <ip> --port <port> --com <command> [--ssid <ssid>] [--psk <key>] [--security <type>] [--apikey <key>] [--aiserver <server>]\n", argv[0]);
        return 1;
    }
    
    // Construct the base URL
    char base_url[512];
    snprintf(base_url, sizeof(base_url), "http://%s:%s", server, port);
    
    fprintf(stderr, "Connecting to: %s\n", base_url);
    fprintf(stderr, "Command: %s\n", command);
    if (ssid) fprintf(stderr, "SSID: %s\n", ssid);
    if (psk) fprintf(stderr, "PSK: %s\n", psk);
    if (security) fprintf(stderr, "Security: %s\n", security);
    if (apikey) fprintf(stderr, "API Key: %s\n", apikey);
    if (aiserver) fprintf(stderr, "AI Server: %s\n", aiserver);
    fprintf(stderr, "Full URL: %s/%s\n\n", base_url, command);
    
    // Create client instance
    LoquatClient *client = loquat_client_init(base_url);
    if (!client) {
        fprintf(stderr, "Failed to initialize client\n");
        return 1;
    }
    
    char *response = NULL;
    int http_code;

    if (!is_valid_command(command)) {
        fprintf(stderr, "Invalid command: %s\n", command);
        goto cleanup;
    }

    if (is_get_command(command)) {
        fprintf(stderr, "Making GET request...\n");
        if (loquat_client_get(client, command, &response, &http_code)) {
            if (http_code != 200) {
                fprintf(stderr, "Error: HTTP Code: %d\n", http_code);
            } else {
                print_response(command, response);
            }
            loquat_client_free_response(response);
        }
        else if (strcmp(command, "get_net_info") == 0) {
            fprintf(stderr, "Getting network information...\n");
            if (loquat_client_get(client, command, &response, &http_code)) {
                if (http_code != 200) {
                    fprintf(stderr, "Error: HTTP Code: %d\n", http_code);
                } else {
                    print_response(command, response);
                }
                loquat_client_free_response(response);
            }
        }
    } else {
        fprintf(stderr, "Making POST request...\n");
        
        // Get POST data for commands that need it
        char *post_data = NULL;
        if (strcmp(command, "connect") == 0) {
            post_data = get_post_connect_wifi_data(ssid, psk, security);
            printf("POST data: %s\n", post_data);
            fprintf(stderr, "Using 120-second timeout for WiFi connection...\n");
        } else if (strcmp(command, "apikey") == 0) {
            post_data = get_post_apikey_data(apikey, aiserver);
            printf("POST data: %s\n", post_data);
            fprintf(stderr, "Using 120-second timeout for API key...\n");
        }
        
        if (loquat_client_post(client, command, post_data, &response, &http_code)) {
            if (http_code != 200) {
                fprintf(stderr, "Error: HTTP Code: %d\n", http_code);
            } else {
                print_response(command, response);
            }
            loquat_client_free_response(response);
        }
        
        // Free POST data if it was allocated
        if (post_data) {
            free(post_data);
        }
    }

cleanup:    
    // Clean up
    loquat_client_cleanup(client);
    
    return 0;
} 
