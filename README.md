# WebCerver

A minimal HTTP/1.1 server written in C for learning purposes. Features raw socket programming, and a basic request parsing pipeline.

## Features
- TCP socket server with connection handling
- HTTP/1.1 request parsing (GET method, headers, body)
- Thread-per-connection model using pthreads
- Clean Makefile with dependency tracking

## Table of Contents
- [Project Structure](#project-structure)
- [Building](#building)
- [Running](#running)
- [Architecture](#architecture)
- [Dependencies](#dependencies)

## Project Structure
```
.
├── lib/
│   ├── sds/                      # SDS dynamic string library
│   │   ├── sds.h
│   │   ├── sds.c
│   │   └── sdsalloc.h
│   └── argtable3/
│       ├── argtable3.h
│       └── argtable3.c
├── src/
│   ├── main.c                    # Entry point, server loop
│   ├── http/                     # HTTP protocol handling
│   │   ├── http.c                # Request parsing (request line, headers, body)
│   │   └── http.h
│   ├── net/                      # Network layer
│   │   ├── network.c             # Socket abstraction
│   │   └── network.h
│   ├── cli/                      # CLI handling
│   │   ├── cli.c
│   │   └── cli.h
│   ├── resource/                 # Resource handling
│   │   ├── filesystem.c          # Filesystem operations
│   │   ├── filesystem.h
│   │   ├── resource_resolver.c   # Resource resolution (static files, routes)
│   │   └── resource_resolver.h
│   └── server/                   # Server core
│       ├── server.c              # Request dispatch, response building
│       └── server.h
├── Makefile                      # Build system
├── LICENSE
└── README.md
```

## Building

### Prerequisites
- C compiler (clang or gcc)
- make
- pthreads

### Build Commands
```bash
# Build debug binary (default)
make

# Clean build artifacts
make clean
```

Output binary location: `./output/main`

## Running
```bash
# After building
./output/WebCerver start --webroot <path> [--port <port>] 
```

Server listens on **port 8080 by default**. Test with:
```bash
curl -X GET http://localhost:[PORT]/[file-path]
```

## Architecture

### Request Flow
```
main.c (server loop)
    └── pthread_create → handle_connection()
            └── net/network.c → network_read_bytes()
            └── server/server.c → server_process_request()
                    ├── http/http.c → http_parse_request()
                    │       ├── split_request()
                    │       ├── parse_request_line()
                    │       ├── parse_headers()
                    │       └── parse_body()
                    ├── resource/resource_resolver.c → resolve_resource()
                    │       └── resource/filesystem.c → read
                    └── http/http.c → http_build_response_str()
                            ├── build_success_response() / build_error_response()
                            └── get_mime_type(), http_reason_phrase()
            └── net/network.c → network_send_bytes()
```

### Module Responsibilities
| Module | Responsibility |
|--------|---------------|
| `main.c` | Socket setup, accept loop, thread spawning |
| `net/network.c` | Socket I/O: bind, listen, accept, read, write, close |
| `server/server.c` | Request orchestration: parse → resolve → build response |
| `http/http.c` | HTTP/1.1 parsing + response building (headers, status, body) |
| `resource/resource_resolver.c` | Path validation, traversal protection, resource mapping |
| `resource/filesystem.c` | file reading |
| `cli/cli.c` | Handling Command-Line options |
| `sds` | Dynamic string buffer (growable, binary-safe) |
| `argtable3` | CLI argument parsing |

## Dependencies
- **[SDS](https://github.com/antirez/sds)** (bundled in `lib/sds/`) 
- **[Argtable3](https://github.com/argtable/argtable3)** (bundled in `lib/argtable3/`) 
- **pthread**
- **Standard C library**


## License
The Unlicense (see LICENSE)

---

⁽ᴵˢ ᶦᵗ ᵐᵉ ᵒʳ ᶦˢ ᵗʰᵉ ᵖʳᵒʲᵉᶜᵗ ⁿᵃᵐᵉ ᵏᶦⁿᵈᵃ ˡᵃᵐᵉ⁾