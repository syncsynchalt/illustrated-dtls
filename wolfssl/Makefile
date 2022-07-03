all: lib

wolfssl/autogen.sh:
	rm -rf wolfssl/
	mkdir wolfssl
	cd wolfssl && git init
	cd wolfssl && git remote add origin git@github.com:wolfSSL/wolfssl.git
	cd wolfssl && git fetch --depth 1 origin e8e35c9a9226b6952745a7609d12d0cacda03bbf
	cd wolfssl && git checkout FETCH_HEAD

wolfssl/stamp.patched: wolfssl/autogen.sh
	cd wolfssl && patch -p1 < ../instruments.patch
	touch $@

wolfssl/configure: wolfssl/stamp.patched
	cd wolfssl && ./autogen.sh

wolfssl/Makefile: wolfssl/configure
	cd wolfssl && ./configure --prefix=`cd .. && pwd` \
		--disable-shared --enable-static --enable-debug \
		--enable-sni --enable-dtls --enable-dtls13 --enable-curve25519 \
		CFLAGS="-DHAVE_SECRET_CALLBACK"

wolfssl/stamp.built: wolfssl/Makefile
	cd wolfssl && make
	touch $@

lib: wolfssl/stamp.built
	cd wolfssl && make install

clean:
	rm -rf wolfssl include lib bin share

rebuild:
	rm -f wolfssl/stamp.built
	$(MAKE) lib

build: wolfssl/stamp.built

checkout: wolfssl/autogen.sh

patch: wolfssl/stamp.patched

reconfig:
	rm -f wolfssl/Makefile
	$(MAKE) lib

.PHONY: all link clean
