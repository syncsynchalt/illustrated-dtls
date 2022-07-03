#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <openssl/evp.h>

void die(const char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

void die_usage(const char *msg)
{
    if (msg) {
        fprintf(stderr, "%s\n", msg);
    }
    fprintf(stderr, "Usage: sn_removal key < in > out\n");
    exit(1);
}

void debug_print(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "SN DEBUG: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}

void compute_mask(const uint8_t *key, size_t keylen, const uint8_t *sample, size_t samplelen, uint8_t *mask_out)
{
    // Using AES-128-ECB
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    uint8_t out[256];
    int outlen = 0;
    const EVP_CIPHER *cipher = EVP_get_cipherbyname("AES-128-ECB");
    EVP_EncryptInit_ex(ctx, cipher, NULL, key, NULL);
    EVP_EncryptUpdate(ctx, out, &outlen, sample, samplelen);
    if (outlen < 5) {
        die("Unexpected short write");
    }
    memcpy(mask_out, out, 16);
    EVP_CIPHER_CTX_free(ctx);
    return;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        die_usage(NULL);
    }

    // read the key from command line
    uint8_t key[16] = { 0 };
    const char *hexkey = argv[1];
    if (strlen(hexkey) != 32) {
        die_usage("Incorrect key length");
    }
    if (strspn(hexkey, "0123456789abcdefABCDEF") != 32) {
        die_usage("Key must be in hex");
    }
    for (size_t i = 0; i < 16; i++) {
        char bb[3] = { 0 };
        strncat(bb, hexkey + 2*i, 2);
        key[i] = (uint8_t)strtol(bb, NULL, 16);
    }

    // read in the beginning of the file for unmasking
    char buf[128];
    size_t nr = fread(buf, 1, sizeof(buf), stdin);

    uint8_t sample[16];
    size_t sample_offset = 5;
    size_t sample_len = nr < 16 ? nr : 16;
    if (sample_len < 16) {
        die("Unexpected short cipher text");
    }
    memcpy(sample, buf + sample_offset, sample_len);
    debug_print("sample [%02x%02x%02x%02x..%02x%02x%02x%02x]", sample[0], sample[1], sample[2], sample[3],
        sample[12], sample[13], sample[14], sample[15]);

    uint8_t mask[5];
    compute_mask(key, sizeof(key), sample, sizeof(sample), mask);
    debug_print("mask [%02x%02x%02x%02x%02x]", mask[0], mask[1], mask[2], mask[3], mask[4]);

    const int record_num_bytes = 2;
    for (size_t i = 0; i < record_num_bytes; i++) {
        buf[i+1] ^= mask[i];
        debug_print("unmasked byte[%d] to %02x", i+1, (uint8_t)buf[i+1]);
    }

    if (fwrite(buf, 1, nr, stdout) != nr) {
        die("short write");
    }

    // copy the remainder from stdin to stdout
    while (!feof(stdin)) {
        size_t nr = fread(buf, 1, sizeof(buf), stdin);
        if (fwrite(buf, 1, nr, stdout) != nr) {
            die("short write");
        }
    }

    exit(0);
}
