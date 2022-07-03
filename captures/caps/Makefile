all: targets

TARGETS=record-chello record-shello record-encext record-cert record-cverify \
	record-sfin record-cfin record-sack record-cdata record-sdata record-sbye
targets: aes_128_gcm_decrypt sn_removal $(TARGETS)

OSSL_DIR=/usr/local/Cellar/openssl@3/3.0.3

aes_128_gcm_decrypt.c:
	curl -sO https://quic.xargs.org/files/aes_128_gcm_decrypt.c

aes_128_gcm_decrypt: aes_128_gcm_decrypt.c
	cc -Wall -I $(OSSL_DIR)/include -o $@ $^ -L $(OSSL_DIR)/lib -lssl -lcrypto

sn_removal: sn_removal.c
	cc -Wall -I $(OSSL_DIR)/include -o $@ $^ -L $(OSSL_DIR)/lib -lssl -lcrypto

S_HSN_KEY=7173fac51194e775001d625ef69d7c9f
C_HSN_KEY=beed6218676635c2cb46a45694144fec

S_HS_KEY=004e03e64ab6cba6b542775ec230e20a
S_HS_IV=6d9924be044ee97c624913f2
C_HS_KEY=6caa2633d5e48f10051e69dc45549c97
C_HS_IV=106dc6e393b7a9ea8ef29dd7

S_ASN_KEY=57ba02596c6a1352d7fe8416c7e17d5a
C_ASN_KEY=5cb5bd8bac29777c650c0dde22d16d47

S_AP_KEY=2b65fffbbc8189474aa2003c43c32d4d
S_AP_IV=582f5a11bdaf973fe3ffeb4e
C_AP_KEY=9ba90dbce8857bc1fcb81d41a0465cfe
C_AP_IV=682219974631fa0656ee4eff

%-unprot: %-prot
	@if echo $^ | grep -q -- -chs-; then ./sn_removal $(C_HSN_KEY) < $^ > $@; fi
	@if echo $^ | grep -q -- -shs-; then ./sn_removal $(S_HSN_KEY) < $^ > $@; fi
	@if echo $^ | grep -q -- -cap-; then ./sn_removal $(C_ASN_KEY) < $^ > $@; fi
	@if echo $^ | grep -q -- -sap-; then ./sn_removal $(S_ASN_KEY) < $^ > $@; fi

%-dec: %-unprot
	@if echo $^ | grep -q -- -chs-; then ./decrypt.sh $(C_HS_KEY) $(C_HS_IV) 5 $^ $@; fi
	@if echo $^ | grep -q -- -shs-; then ./decrypt.sh $(S_HS_KEY) $(S_HS_IV) 5 $^ $@; fi
	@if echo $^ | grep -q -- -cap-; then ./decrypt.sh $(C_AP_KEY) $(C_AP_IV) 5 $^ $@; fi
	@if echo $^ | grep -q -- -sap-; then ./decrypt.sh $(S_AP_KEY) $(S_AP_IV) 5 $^ $@; fi

record-chello: 01-hello-c
	cp $^ $@

record-shello: 02-hello-s
	cp $^ $@

record-encext: 03-encext-shs-dec
	cp $^ $@

record-cert: 04-cert-shs-dec
	cp $^ $@

record-cverify: 05-cverify-shs-dec
	cp $^ $@

record-sfin: 06-fin-shs-dec
	cp $^ $@

record-cfin: 07-fin-chs-dec
	cp $^ $@

record-sack: 08-ack-sap-dec
	cp $^ $@

record-cdata: 09-data-cap-dec
	cp $^ $@

record-sdata: 10-data-sap-dec
	cp $^ $@

record-sbye: 11-bye-sap-dec
	cp $^ $@

clean:
	rm -f *-unprot *-dec
	rm -f aes_128_gcm_decrypt aes_128_gcm_decrypt.c sn_removal
	rm -f record-*

.PHONY: clean targets all
