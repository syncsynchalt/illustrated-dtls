TARGETS=serverextensions servercert servercertverify serverfinished \
	clientfinished clientdata serverticket1 serverticket2 serverdata
all: cleartargets targets
targets: aes_256_gcm_decrypt
    $(MAKE) $(TARGETS)

aes_256_gcm_decrypt.c:
    curl -sO https://tls13.xargs.org/files/aes_256_gcm_decrypt.c

aes_256_gcm_decrypt: aes_256_gcm_decrypt.c
    cc -I ../../openssl/openssl/include \
        -o aes_256_gcm_decrypt aes_256_gcm_decrypt.c \
        -L ../../openssl -lssl -lcrypto

S_HS_KEY=9f13575ce3f8cfc1df64a77ceaffe89700b492ad31b4fab01c4792be1b266b7f
S_HS_IV=9563bc8b590f671f488d2da3
C_HS_KEY=1135b4826a9a70257e5a391ad93093dfd7c4214812f493b3e3daae1eb2b1ac69
C_HS_IV=4256d2e0e88babdd05eb2f27

S_AP_KEY=01f78623f17e3edcc09e944027ba3218d57c8e0db93cd3ac419309274700ac27
S_AP_IV=196a750b0c5049c0cc51a541
C_AP_KEY=de2f4c7672723a692319873e5c227606691a32d1c59d8b9f51dbb9352e9ca9cc
C_AP_IV=bb007956f474b25de902432f

cleartargets:
    rm -f $(TARGETS)




clean:
    rm -f aes_256_gcm_decrypt aes_256_gcm_decrypt.c
    rm -f $(TARGETS)

.PHONY: clean cleartargets targets all
