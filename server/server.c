#include <wolfssl/options.h>
#include <wolfssl/ssl.h>
#include <sys/socket.h>
#include <poll.h>
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

int create_bind(int port)
{
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		die("Unable to create socket");
	}
	int enable = 1;
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		die("SO_REUSEADDR failed");
	}
	if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		die("Unable to bind to port");
	}
	printf("Bound to port %d\n", port);
	return s;
}

int wait_for_readable(int fd)
{
    struct pollfd fds = {fd, POLLIN, 0};
    if (poll(&fds, 1, -1) != 1)
        die("poll failed");
    return fd;
}

void setup_ctx(WOLFSSL_CTX *ctx)
{
    if (wolfSSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM) != SSL_SUCCESS)
        die("can't use server.crt");
    if (wolfSSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) != SSL_SUCCESS)
        die("can't use server.key");
    wolfSSL_CTX_no_ticket_TLSv13(ctx);
}

int main(int argc, char **argv)
{
	// set up ssl
	setenv("SERVER", "1", 1);
    wolfSSL_Init();
    // wolfSSL_Debugging_ON();
    WOLFSSL_CTX *ctx = wolfSSL_CTX_new(wolfDTLSv1_3_server_method());
    if (ctx == NULL)
        die("CTX_new error");
    setup_ctx(ctx);

	int sock = create_bind(8400);

    struct sockaddr_storage addr;
    socklen_t addrlen = sizeof(addr);
    char packet[5000];

    printf("Waiting for client to send\n");
    wait_for_readable(sock);
    ssize_t sret = recvfrom(sock, (char *)&packet, sizeof(packet), MSG_PEEK, (struct sockaddr *)&addr, &addrlen);
    if (sret <= 0)
        die("Recvfrom failed");

	char host[NI_MAXHOST];
	char port[NI_MAXSERV];
    getnameinfo((struct sockaddr *)&addr, addrlen, host, sizeof(host),
        port, sizeof(port), NI_NUMERICHOST|NI_NUMERICSERV);
    printf("Receiving data from %s:%s\n", host, port);

    if (connect(sock, (const struct sockaddr *)&addr, addrlen) != 0)
        die("connect failed");

	WOLFSSL *ssl = wolfSSL_new(ctx);
    if (!ssl)
        die_ssl(ssl, "can't make new ssl");
    setup_keylog(ssl);
	wolfSSL_set_fd(ssl, sock);

	if (wolfSSL_accept(ssl) <= 0)
        die_ssl(ssl, "can't accept ssl connection");

	char rbuf[128];
	int ret = wolfSSL_read(ssl, rbuf, sizeof(rbuf)-1);
	if (ret <= 0)
		die_ssl(ssl, "Unable to read from connection");

	rbuf[ret] = '\0';
	printf("Read [%s]\n", rbuf);

	const char reply[] = "pong";
	if (wolfSSL_write(ssl, reply, strlen(reply)) <= 0)
		die_ssl(ssl, "Unable to write to connection");
	printf("Wrote [%s]\n", reply);

    // wolfSSL_set_fd(ssl, 0);
    wolfSSL_shutdown(ssl);
    wolfSSL_free(ssl);
    close(sock);
    wolfSSL_CTX_free(ctx);
    wolfSSL_Cleanup();

    return 0;
}
