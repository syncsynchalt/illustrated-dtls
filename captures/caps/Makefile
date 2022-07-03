PROT_PACKETS=$(wildcard *-prot)
UNPROT_PACKETS=$(patsubst %-prot, %-unprot, $(PROT_PACKETS))
DECRYPT_PACKETS=$(patsubst %-prot, %-dec, $(PROT_PACKETS))
TARGETS=encextensions cert cverify

OSSL_DIR=/usr/local/Cellar/openssl@3/3.0.3
all: cleartargets targets
targets: aes_128_gcm_decrypt sn_removal
	$(MAKE) $(TARGETS)

aes_128_gcm_decrypt.c:
	curl -sO https://quic.xargs.org/files/aes_128_gcm_decrypt.c

aes_128_gcm_decrypt: aes_128_gcm_decrypt.c
	cc -Wall -I $(OSSL_DIR)/include -o $@ $^ -L $(OSSL_DIR)/lib -lssl -lcrypto

sn_removal: sn_removal.c
	cc -Wall -I $(OSSL_DIR)/include -o $@ $^ -L $(OSSL_DIR)/lib -lssl -lcrypto

S_SN_KEY=7173fac51194e775001d625ef69d7c9f
C_SN_KEY=beed6218676635c2cb46a45694144fec

S_HS_KEY=004e03e64ab6cba6b542775ec230e20a
S_HS_IV=6d9924be044ee97c624913f2
C_HS_KEY=6caa2633d5e48f10051e69dc45549c97
C_HS_IV=106dc6e393b7a9ea8ef29dd7

S_AP_KEY=xxx
S_AP_IV=xxx
C_AP_KEY=xxx
C_AP_IV=xxx

cleartargets:
	rm -f $(TARGETS)

%-unprot: %-prot
	if echo $^ | grep -q -- -c-; then ./sn_removal $(C_SN_KEY) < $^ > $@; else ./sn_removal $(S_SN_KEY) < $^ > $@; fi

03-s-enc-ext-dec: 03-s-enc-ext-unprot
	./decrypt.sh $(S_HS_KEY) $(S_HS_IV) 5 $^ $@

04-s-cert-dec: 04-s-cert-unprot
	./decrypt.sh $(S_HS_KEY) $(S_HS_IV) 5 $^ $@

05-s-cverify-dec: 05-s-cverify-unprot
	./decrypt.sh $(S_HS_KEY) $(S_HS_IV) 5 $^ $@

06-s-fin-dec: 06-s-fin-unprot
	./decrypt.sh $(S_HS_KEY) $(S_HS_IV) 5 $^ $@

### consider getting rid of these
encextensions: 03-s-enc-ext-dec
	cp $^ $@

cert: 04-s-cert-dec
	cp $^ $@

cverify: 05-s-cverify-dec
	cp $^ $@

clean:
	rm -f *-unprot *-dec
	rm -f aes_128_gcm_decrypt aes_128_gcm_decrypt.c sn_removal
	rm -f $(TARGETS)

.PHONY: clean cleartargets targets all
