# Lightweight DNS Stub Resolver in C (Linux)

A low-level, DNS stub resolver written in pure C from scratch. This project implements manual wire-format packet construction, sequential length-prefixed label parsing, and raw socket communication without relying on high-level networking abstractions.

> 💡 **Documentation Note:** Please check the `/docs` folder at the start of your review for supplementary hand written notes and diagrams.

## Example Output

When running the compiled binary against a live domain (e.g., `www.krunker.io`), the custom parser handles the binary response packet, strips out the headers and question sections, and extracts the resulting IP addresses from the resource records:

```text
Enter the Dns Name (Ex : www.example.com): www.krunker.io
Reciving..
rcode : 0
Success :)
1) IP : 104.20.38.18
2) IP : 172.66.149.224
```
## Features

- **Raw Wire-Format Parsing:** Manually decodes DNS response packets byte-by-byte, handling length-prefixed domain labels and null-terminators.
- **RFC 1035 Compliance:** Built around foundational DNS standards, managing packet headers, question sections, and resource records (RRs).
- **Memory Safety & Control:** Utilizes explicit `malloc`/`memcpy` management for dynamic buffer handling and packet traversal.

---

## Technical Overview

Unlike standard applications that rely on operating system wrappers like `getaddrinfo()`, this resolver operates directly at the transport layer:
1. **Query Construction:** Manages binary bit-shifting and struct packing to construct standard DNS query headers and question blocks.
2. **Transport Layer:** Communicates with upstream recursive resolvers over standard UDP sockets on port 53.
3. **Answer Section Extraction:** Navigates to the answer section of the response packet to extract the resolved IP addresses (`A` records).
---

## Getting Started

### Prerequisites
You will need a C compiler  Unix-like environment.

### Building the Project
Clone the repository and compile the source code using `gcc`:

```bash
 gcc DnsLookup.c -o DnsLookup.out
./DnsLookup.out
