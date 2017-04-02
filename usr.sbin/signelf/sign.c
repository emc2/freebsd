/*-
 * Copyright (c) 2017 Eric McCorkle <emc2@metricspace.net>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Bill Paul AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Bill Paul OR THE VOICES IN HIS HEAD
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <pwd.h>
#include <unistd.h>
#include <time.h>

#include <sys/param.h>

#include <libelf.h>
#include <gelf.h>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pkcs7.h>
#include <openssl/pem.h>
#include <openssl/x509.h>

#include "signelf.h"

static char keypath[MAXPATHLEN + 1];
static char pubpath[MAXPATHLEN + 1];
static char **signpaths;
static size_t nsignpaths = 0;
static size_t max_signpaths = 16;
static bool verbose = false;
static bool ephemeral = false;
static char ephemeralpath[MAXPATHLEN + 1];
static EVP_PKEY *sign_priv;
static X509 *sign_cert;
static size_t sigsize;

static void
check_malloc(const void *ptr) {
        if (ptr == NULL) {
                perror("Could not allocate memory");
                abort();
        }
}

static void
check_file(const FILE *ptr, const char* str) {
        if (ptr == NULL) {
                perror(str);
                exit(errno);
        }
}

static void
check_ssl(const char *op) {
        unsigned long err = ERR_get_error();

        if (err != 0) {
                fprintf(stderr, "Error in %s (%s) while %s: %s\n",
                    ERR_lib_error_string(err), ERR_func_error_string(err), op,
                    ERR_reason_error_string(err));
                exit(err);
        }
}

static void
add_signpath(char *signpath)
{
        if (max_signpaths <= nsignpaths) {
                void *tmp;

                max_signpaths *= 2;
                tmp = realloc(signpaths, max_signpaths * sizeof(signpaths[0]));
                check_malloc(tmp);
                signpaths = tmp;
        }

        signpaths[nsignpaths] = strdup(signpath);
        nsignpaths++;
}

static void
set_keypath(const char *path)
{
        strncpy(keypath, path, MAXPATHLEN);
}

static void
set_pubpath(const char *path)
{
        strncpy(pubpath, path, MAXPATHLEN);
}

static void
set_ephemeralpath(const char *path)
{
        strncpy(ephemeralpath, path, MAXPATHLEN);
}

static void
load_keys(void)
{
        FILE *f;
        EVP_PKEY *priv;
        X509 *cert;

        /* Load the private key */
        f = fopen(keypath, "r");
        check_file(f, "Error opening private key");
        priv = PEM_read_PrivateKey(f, NULL, NULL, NULL);
        check_ssl("loading private key");
        fclose(f);

        /* Load the public key */
        f = fopen(pubpath, "r");
        check_file(f, "Error opening public key");
        cert = PEM_read_X509(f, NULL, NULL, NULL);
        check_ssl("loading public key");
        fclose(f);

        if (ephemeral) {
                ASN1_TIME *asntime;
                X509_NAME *name;
                EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(priv, NULL);
                ASN1_INTEGER timestamp;

                if (verbose) {
                        fprintf(stderr, "Generating ephemeral key...");
                }

                /* Generate the ephemeral private key */
                check_ssl("generating ephemeral private key");
                EVP_PKEY_keygen_init(ctx);
                check_ssl("generating ephemeral private key");
                EVP_PKEY_keygen(ctx, &sign_priv);
                check_ssl("generating ephemeral private key");
                EVP_PKEY_CTX_free(ctx);
                fprintf(stderr, "done\n");

                /* Create ephemeral public key cert */
                sign_cert = X509_new();
                check_ssl("creating ephemeral public key cert");
                X509_set_pubkey(sign_cert, sign_priv);
                check_ssl("creating ephemeral public key cert");
                X509_set_version(sign_cert, 2);
                check_ssl("creating ephemeral public key cert");
                ASN1_INTEGER_set(&timestamp, time(NULL));
                check_ssl("creating ephemeral public key cert");
                asntime = X509_get_notBefore(cert);
                check_ssl("creating ephemeral public key cert");
                X509_set_notBefore(sign_cert, asntime);
                check_ssl("creating ephemeral public key cert");
                asntime = X509_get_notAfter(cert);
                check_ssl("creating ephemeral public key cert");
                X509_set_notAfter(sign_cert, asntime);
                check_ssl("creating ephemeral public key cert");
                name = X509_get_subject_name(cert);
                check_ssl("creating ephemeral public key cert");
                X509_set_subject_name(sign_cert, name);
                check_ssl("creating ephemeral public key cert");
                name = X509_get_issuer_name(cert);
                check_ssl("creating ephemeral public key cert");
                X509_set_issuer_name(sign_cert, name);
                check_ssl("creating ephemeral public key cert");

                /* Sign ephemeral public key */
                X509_sign(sign_cert, priv, EVP_sha256());
                check_ssl("signing ephemeral public key");
        } else {
                sign_priv = priv;
                sign_cert = cert;
        }
}

static void
write_ephemeral(void)
{
        if (ephemeral) {
                FILE *f;

                if (verbose) {
                        fprintf(stderr, "Writing ephemeral key to %s\n",
                            ephemeralpath);
                }

                f = fopen(ephemeralpath, "w");
                check_file(f, "Error writing ephemeral key");
                PEM_write_X509(f, sign_cert);
                check_ssl("writing out ephemeral key");
                fclose(f);

        }
}

static PKCS7 *
make_sig(void *buf, size_t len)
{
        BIO *bio;
        PKCS7 *pkcs7;
        bio = BIO_new_mem_buf(buf, len);
        check_ssl("computing signature size");
        pkcs7 = PKCS7_sign(sign_cert, sign_priv, NULL, bio,
                    PKCS7_DETACHED | PKCS7_BINARY | PKCS7_NOCERTS |
                    PKCS7_NOCHAIN | PKCS7_NOSMIMECAP);
        check_ssl("computing signature size");

        return pkcs7;
}

static void
compute_sigsize(void)
{
        char buf[0];
        int len;
        PKCS7 *pkcs7 = make_sig(buf, sizeof(buf));

        len = i2d_PKCS7(pkcs7, NULL);
        check_ssl("computing signature size");
        sigsize = len;
        PKCS7_free(pkcs7);
}

static int parse_args(const int argc, char* argv[])
{
        int ch;
        int i;

        while ((ch = getopt(argc, argv, "e:k:p:v")) != -1) {
                switch (ch) {
                      default:
                              usage();

                              return (1);
                      case 'e':
                              ephemeral = true;
                              set_ephemeralpath(optarg);
                              break;
                      case 'k':
                              set_keypath(optarg);
                              break;
                      case 'p':
                              set_pubpath(optarg);
                              break;
                      case 'v':
                              verbose = true;
                              break;
                      case '?':
                              usage();
                              break;
                }
        }

        for(i = optind; i < argc; i++) {
                add_signpath(argv[i]);
        }

        return (0);
}

static size_t
get_sig_strtab(Elf *elf)
{
}

static size_t
set_sign_name(Elf *elf, GElf_Shdr *shdr)
{
}

static void
sign_elf(Elf *elf)
{
        size_t idx = find_sig(elf);
        Elf_Scn *scn;
        Elf_Data *data;
        char buf[sigsize];
        size_t filesize;
        void *filedata;
        PKCS7 *pkcs7;
        size_t siglen;

        if (idx != 0) {
                scn = elf_getscn(elf, idx);
                check_elf_error();
                data = elf_getdata(scn, NULL);
                check_elf_error();
        } else {
                GElf_Shdr shdr;

                /* Create a new section */
                scn = elf_newscn(elf);
                check_elf_error();
                data = elf_newdata(scn);
                check_elf_error();
                gelf_getshdr(scn, &shdr);
                check_elf_error();

                /* Set the section name and type */
                shdr.sh_type = SHT_NOTE;
                set_sign_name(elf, &shdr);

                /* Update the section data */
                gelf_update_shdr(scn, &shdr);
                check_elf_error();
        }

        /* Set the signature section to all zeros  */
        memset(buf, 0, sigsize);
        data->d_buf = buf;
        data->d_size = sigsize;

        /* Update the file and get a pointer to the raw data */
        elf_update(elf, ELF_C_NULL);
        check_elf_error();
        filedata = elf_rawfile(elf, &filesize);
        check_elf_error();

        /* The section and data pointers aren't good anymore */
        scn = elf_getscn(elf, idx);
        check_elf_error();
        data = elf_getdata(scn, NULL);
        check_elf_error();

        /* Actually compute the signature */
        pkcs7 = make_sig(filedata, filesize);
        siglen = i2d_PKCS7(pkcs7, NULL);

        if(siglen != sigsize) {
                fprintf(stderr, "Signature %zu is not expected %zu\n",
                    siglen, sigsize);
                abort();
        }

        i2d_PKCS7(pkcs7, data->d_buf);

        elf_update(elf, ELF_C_WRITE);
}

int
sign_main(int argc, char *argv[])
{
        int err;
        unsigned int i;

        set_keypath(DEFAULT_KEYPATH);
        signpaths = malloc(max_signpaths * sizeof(signpaths[0]));
        err = parse_args(argc, argv);

        if (verbose) {
                fprintf(stderr, "Loading key from %s and cert from %s, signing",
                        keypath, pubpath);

                for(i = 0; i < nsignpaths; i++) {
                        fprintf(stderr, " %s", signpaths[i]);
                }

                fprintf(stderr, " %s\n",
                    ephemeral ? "with ephemeral key" : "directly");
        }

        load_keys();
        write_ephemeral();
        compute_sigsize();

        for(i = 0; i < nsignpaths; i++) {
                int fd = open(signpaths[i], O_RDWR);
                Elf *elf;

                check_fd_error(fd);
                fprintf(stderr, " %s", signpaths[i]);
                elf = elf_begin(fd, ELF_C_RDWR, NULL);
                check_elf_error();
                sign_elf(elf);
                elf_end(elf);
        }

        return (err);
}
