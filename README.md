# The Illustrated DTLS Connection

Will be published at https://dtls.xargs.org

- `site/`: page source for the finished product
- `server/server.c`: server code
- `client/client.c`: client code
- `wolfssl/`: patch and build of wolfSSL that removes any random aspects of the documented connection
- `captures/`: PCAP and keylog files

See also https://github.com/syncsynchalt/illustrated-tls13 for a TLS 1.3 version of this project.

### Build instructions

If you'd like a working example that reproduces the exact handshake documented on the site:

```
git clone https://github.com/syncsynchalt/illustrated-dtls.git
cd illustrated-dtls/
cd wolfssl/
make
cd ../server/
make
cd ../client/
make
```

Then open two terminals and run `./server` in the server/ subdir and `./client` in the client/ subdir.

This has been shown to work on MacOS 12 and various Linuxes and
only has a few easy-to-find dependencies: gcc or clang, make, patch,
etc.
