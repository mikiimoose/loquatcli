# LoquatClient - C HTTP Client

A simple and efficient C HTTP client built with libcurl for making GET requests to servers.

## Features

- Simple GET request functionality
- Custom header support
- Automatic redirect following
- Timeout handling (10 seconds)
- Error handling and logging
- Clean C interface with proper memory management

## Prerequisites

You need to have libcurl development libraries installed on your system.

### Installing Dependencies

#### Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install libcurl4-openssl-dev
```

#### CentOS/RHEL/Fedora:
```bash
sudo yum install libcurl-devel
```

#### macOS:
```bash
brew install curl
```

## Building

1. Clone or download the source files
2. Run the build:
```bash
make
```

Or build and run in one command:
```bash
make run
```

## Usage

### Command Line Usage

The client can be used directly from the command line with three parameters:

```bash
./loquatcli <ip_address> <port> <request_path>
```

#### Examples:

```bash
# Connect to a local server
./loquatcli localhost 8080 /api/users

# Connect to a remote server
./loquatcli 192.168.1.100 3000 /status

# Connect to a web service
./loquatcli api.example.com 443 /v1/data
```

### Programmatic Usage

```c
#include "loquatcli.h"

int main() {
    // Create client with base URL
    LoquatClient *client = loquat_client_init("http://api.example.com");
    if (!client) {
        fprintf(stderr, "Failed to initialize client\n");
        return 1;
    }
    
    char *response = NULL;
    int http_code;
    
    // Make a simple GET request
    if (loquat_client_get(client, "/users", &response, &http_code)) {
        printf("HTTP Code: %d\n", http_code);
        printf("Response: %s\n", response);
        loquat_client_free_response(response);
    }
    
    // Clean up
    loquat_client_cleanup(client);
    return 0;
}
```

### With Custom Headers

```c
char *headers[] = {
    "Accept: application/json",
    "Authorization: Bearer your-token-here",
    "X-Custom-Header: value"
};
int header_count = 3;

if (loquat_client_get_with_headers(client, "/protected-endpoint", 
                                   headers, header_count, &response, &http_code)) {
    printf("Response: %s\n", response);
    loquat_client_free_response(response);
}
```

### Changing Base URL

```c
loquat_client_set_base_url(client, "https://new-api.example.com");
```

## API Reference

### Functions

#### `LoquatClient* loquat_client_init(const char *base_url)`
- Initializes a new HTTP client
- `base_url`: The base URL for the client (can be NULL for default)
- Returns: Pointer to LoquatClient structure, or NULL on failure

#### `void loquat_client_cleanup(LoquatClient *client)`
- Cleans up the HTTP client and frees memory
- `client`: Pointer to LoquatClient structure

#### `int loquat_client_get(LoquatClient *client, const char *endpoint, char **response, int *http_code)`
- Makes a GET request to `base_url + endpoint`
- `client`: Pointer to LoquatClient structure
- `endpoint`: The endpoint to request
- `response`: Pointer to store the response string (caller must free with `loquat_client_free_response`)
- `http_code`: Pointer to store the HTTP response code
- Returns: 1 on success, 0 on failure

#### `int loquat_client_get_with_headers(LoquatClient *client, const char *endpoint, char **headers, int header_count, char **response, int *http_code)`
- Same as `loquat_client_get()` but with custom headers
- `headers`: Array of header strings in format "Header-Name: value"
- `header_count`: Number of headers in the array

#### `void loquat_client_set_base_url(LoquatClient *client, const char *url)`
- Changes the base URL for subsequent requests

#### `const char* loquat_client_get_base_url(LoquatClient *client)`
- Returns the current base URL

#### `void loquat_client_free_response(char *response)`
- Frees response memory allocated by the client

## Example Output

When you run the client with command line parameters:

```bash
./loquatcli localhost 8080 /api/status
```

You'll see output like:

```
Connecting to: http://localhost:8080
Request path: /api/status
Full URL: http://localhost:8080/api/status

Making GET request...
HTTP Code: 200
Response:
{
  "status": "online",
  "timestamp": "2024-01-15T10:30:00Z",
  "version": "1.0.0"
}
```

Or if the server is not available:

```
Connecting to: http://192.168.1.100:3000
Request path: /api/users
Full URL: http://192.168.1.100:3000/api/users

Making GET request...
Request failed!
```

## Error Handling

The client includes comprehensive error handling:

- CURL initialization failures return NULL
- Network errors are logged to stderr
- HTTP status codes are returned for inspection
- Timeout is set to 10 seconds by default
- Proper memory management with cleanup functions

## Memory Management

The C client requires manual memory management:

- Always call `loquat_client_cleanup()` when done with the client
- Always call `loquat_client_free_response()` after using response data
- Check return values for error conditions

## Makefile Targets

- `make` or `make all` - Build the client
- `make clean` - Remove built files
- `make install-deps` - Install libcurl development package
- `make run` - Build and run the client
- `make help` - Show available targets

## Troubleshooting

### Compilation Errors

If you get compilation errors about missing curl headers:
```bash
make install-deps
```

### Runtime Errors

If you get runtime errors about missing libcurl:
```bash
# Ubuntu/Debian
sudo apt-get install libcurl4

# CentOS/RHEL/Fedora
sudo yum install libcurl

# macOS
brew install curl
```

## License

This code is provided as-is for educational and development purposes. 