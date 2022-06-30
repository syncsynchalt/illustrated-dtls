#include <wolfssl/options.h>
#include <wolfssl/ssl.h>
#include <string.h>
#include <errno.h>

void die(const char *str)
{
    fprintf(stderr, "%s: %s\n", str, strerror(errno));
    exit(1);
}

void die_ssl(WOLFSSL *ssl, const char *str)
{
    int err = wolfSSL_get_error(ssl, 0);
    fprintf(stderr, "%s: %s (err:%d)\n", str, wolfSSL_ERR_reason_error_string(err), err);
    exit(1);
}

int secret_callback(WOLFSSL *ssl, int id, const unsigned char *secret, int secret_sz, void *unused)
{
    unsigned char crand[32];
    int crand_sz = (int)wolfSSL_get_client_random(ssl, crand, sizeof(crand));
    if (crand_sz <= 0)
        die("error getting random size");

    const char *str = NULL;
    if (id == CLIENT_EARLY_TRAFFIC_SECRET) {
        str = "CLIENT_EARLY_TRAFFIC_SECRET";
    } else if (id == EARLY_EXPORTER_SECRET) {
        str = "EARLY_EXPORTER_SECRET";
    } else if (id == CLIENT_HANDSHAKE_TRAFFIC_SECRET) {
        str = "CLIENT_HANDSHAKE_TRAFFIC_SECRET";
    } else if (id == SERVER_HANDSHAKE_TRAFFIC_SECRET) {
        str = "SERVER_HANDSHAKE_TRAFFIC_SECRET";
    } else if (id == CLIENT_TRAFFIC_SECRET) {
        str = "CLIENT_TRAFFIC_SECRET";
    } else if (id == SERVER_TRAFFIC_SECRET) {
        str = "SERVER_TRAFFIC_SECRET";
    } else if (id == EXPORTER_SECRET) {
        str = "EXPORTER_SECRET";
    } else {
        printf("unknown secret %d\n", id);
        str = "UNKNOWN_SECRET";
    }

    FILE *fp = stdout;
    fprintf(fp, "%s ", str);
    for (size_t i = 0; i < crand_sz; i++) {
        fprintf(fp, "%02x", crand[i]);
    }
    fprintf(fp, " ");
    for (size_t i = 0; i < secret_sz; i++) {
        fprintf(fp, "%02x", secret[i]);
    }
    fprintf(fp, "\n");

    return 0;
}

void setup_keylog(WOLFSSL *ssl)
{
    wolfSSL_KeepArrays(ssl);
    wolfSSL_set_tls13_secret_cb(ssl, secret_callback, 0);
}

void setup_ctx(WOLFSSL_CTX *ctx)
{
    int groups[] = { WOLFSSL_ECC_X25519 };
    if (wolfSSL_CTX_set_groups(ctx, groups, 1) != SSL_SUCCESS)
        die("can't set groups list");
    wolfSSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, 0);
}

int main(int argc, char **argv)
{
    // set up ssl
    setenv("SERVER", "0", 1);
    wolfSSL_Init();
    // wolfSSL_Debugging_ON();
    WOLFSSL_CTX *ctx = wolfSSL_CTX_new(wolfDTLSv1_3_client_method());
    if (ctx == NULL)
        die("CTX_new error");
    setup_ctx(ctx);

    // create new session
    WOLFSSL *ssl = wolfSSL_new(ctx);
    if (ssl == NULL)
        die("can't create ssl object");
    setup_keylog(ssl);

    // connect to localhost:8400
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8400);
    if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) < 1)
        die("can't inet_pton");

    if (wolfSSL_dtls_set_peer(ssl, &addr, sizeof(addr)) != SSL_SUCCESS)
        die("can't set peer");

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
        die("can't create socket");

    fflush(stdout);
    wolfSSL_set_fd(ssl, fd);
    if (wolfSSL_connect(ssl) != SSL_SUCCESS)
        die_ssl(ssl, "connect failed");
    printf("Connected\n");

    const char *servername = "example.ulfheim.net";
    if (wolfSSL_UseSNI(ssl, WOLFSSL_SNI_HOST_NAME, servername, strlen(servername)) != SSL_SUCCESS)
        die_ssl(ssl, "SNI failed");

    const char wbuf[] = "ping";
    if (wolfSSL_write(ssl, wbuf, strlen(wbuf)) != strlen(wbuf))
        die_ssl(ssl, "write failed");
    printf("Wrote %s\n", wbuf);

    char rbuf[128] = { 0 };
    size_t n = wolfSSL_read(ssl, rbuf, sizeof(rbuf)-1);
    if (n < 0)
        die_ssl(ssl, "read failed");
    printf("Read [%s]\n", rbuf);

    wolfSSL_set_fd(ssl, 0);
    wolfSSL_shutdown(ssl);
    wolfSSL_free(ssl);
    close(fd);
    wolfSSL_CTX_free(ctx);
    wolfSSL_Cleanup();

    return 0;
}
