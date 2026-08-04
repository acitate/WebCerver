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
│   └── sds/              # Simple Dynamic Strings library
│       ├── sds.h
│       ├── sds.c
│       └── sdsalloc.h
├── src/
│   ├── main.c            # Entry point, server loop
│   ├── network.h/.c      # Socket abstraction (bind, listen, accept, read, close, send)
│   ├── server.h/.c       # Request dispatch (process_request)
│   └── http.h/.c         # HTTP parsing (request line, headers, body)
├── Makefile              # Build system
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

# Build and run in one step
make run

# Clean build artifacts
make clean
```

Output binary location: `./output/main`

## Running
```bash
# After building
./output/main

# Or use make target
make run
```

Server listens on **port 8080** (hardcoded in `src/main.c`). Test with:
```bash
curl http://localhost:8080/
curl -X GET http://localhost:8080/test
```

## Architecture

### Request Flow
```
main.c (server loop)
    └── pthread_create → handle_connection()
            └── network.c → read_request()
            └── server.c → process_request()
                    └── http.c → http_parse()
                            ├── split_request()  (request line / headers / body)
                            ├── parse_request_line()
                            ├── parse_headers()
                            └── parse_body()
```

### Module Responsibilities
| Module | Responsibility |
|--------|---------------|
| `main.c` | Socket setup, accept loop, thread spawning |
| `network.c` | Socket creation, bind, listen, accept, read, close |
| `server.c` | Request/response orchestration (stub) |
| `http.c` | HTTP/1.1 parsing into `HttpRequest` struct |
| `sds` | Dynamic string buffer (growable, binary-safe) |

## Dependencies
- **[SDS](https://github.com/antirez/sds)** (bundled in `lib/sds/`) 
- **pthread**
- **Standard C library**


## License
The Unlicense (see LICENSE)

---

⁽ᴵˢ ᶦᵗ ᵐᵉ ᵒʳ ᶦˢ ᵗʰᵉ ᵖʳᵒʲᵉᶜᵗ ⁿᵃᵐᵉ ᵏᶦⁿᵈᵃ ˡᵃᵐᵉ⁾