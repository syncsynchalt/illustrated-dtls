CFLAGS += -I../wolfssl/include -Wall
LDFLAGS += -L../wolfssl/lib -lwolfssl

client: client.c ../wolfssl/lib/libwolfssl.a
	$(CC) $(CFLAGS) -o client client.c -Wall $(LDFLAGS)

clean:
	rm -f client
