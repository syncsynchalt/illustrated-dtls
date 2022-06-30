CFLAGS += -I../wolfssl/include -Wall
LDFLAGS += -L../wolfssl/lib -lwolfssl

server: server.c ../wolfssl/lib/libwolfssl.a
	$(CC) $(CFLAGS) -o server server.c -Wall $(LDFLAGS)

clean:
	rm -f server
