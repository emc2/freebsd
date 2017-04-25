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
#include <unistd.h>
#include <fcntl.h>

#include <sys/param.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>

#include "signelf.h"

#define VPRINTF(...) (verbose ? fprintf(stderr, __VA_ARGS__) : 0)

static char pubpath[MAXPATHLEN + 1];
static char **signpaths;
static size_t nsignpaths = 0;
static size_t max_signpaths = 16;
static bool verbose = false;
static STACK_OF(X509) *verifycerts;

static void check_malloc(const void *ptr) {
        if (ptr == NULL) {
                perror("Could not allocate memory");
                abort();
        }
}

static void add_signpath(char *signpath)
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
load_cert(void)
{
        FILE *f;
        X509 *cert;

        /* Load the public key */
        f = fopen(pubpath, "r");
        check_file_error(f, "Error opening public key");
        cert = PEM_read_X509(f, NULL, NULL, NULL);
        check_ssl_error("loading public key");
        fclose(f);

        verifycerts = sk_X509_new_null();
        check_ssl_error("setting up public key");
        sk_X509_push(verifycerts, cert);
        check_ssl_error("setting up public key");
}

static void set_pubpath(const char *path)
{
        strncpy(pubpath, path, MAXPATHLEN);
}

static int parse_args(const int argc, char* argv[])
{
        int ch;
        int i;

        while ((ch = getopt(argc, argv, "p:v")) != -1) {
                switch (ch) {
                      default:
                              usage();

                              return (1);
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

        if (!strcmp(pubpath, "")) {
                set_pubpath(DEFAULT_PUBKEYPATH);
        }

        for(i = optind; i < argc; i++) {
                add_signpath(argv[i]);
        }

        return (0);
}

static bool
verify_elf(Elf *elf, const char path[])
{
        size_t idx;
        Elf_Scn *scn;
        Elf_Data *data;
        PKCS7 *pkcs7;
        const unsigned char *buf;
        void *ptr;
        size_t filesize;
        int out;
        unsigned long err;
        BIO *bio;

        /* Find the signature section */
        idx = find_sig(elf);

        if (idx == 0) {
                fprintf(stderr, "No signature in %s\n", path);
                return (false);
        }

        scn = elf_getscn(elf, idx);
        check_elf_error();
        data = elf_rawdata(scn, NULL);
        check_elf_error();

        /* Load the signature */
        buf = data->d_buf;

        pkcs7 = NULL;
        d2i_PKCS7(&pkcs7, &buf, data->d_size);
        check_ssl_error("parsing signature");

        /* Prepare the file data for verification */
        memset(data->d_buf, 0, data->d_size);
        ptr = elf_rawfile(elf, &filesize);
        bio = BIO_new_mem_buf(ptr, filesize);
        check_ssl_error("preparing data");

        /* Perform verification */
        out = PKCS7_verify(pkcs7, verifycerts, NULL, bio, NULL,
            PKCS7_NOINTERN | PKCS7_NOVERIFY);
        err = ERR_get_error();

        if (err != 0) {
                fprintf(stderr, "Signature verification for %s failed: %s\n",
                        path, ERR_reason_error_string(err));
                exit(err);
        } else if (out) {
                VPRINTF("Verification successful for %s\n", path);
        } else {
                fprintf(stderr, "Verification failed for %s\n", path);
        }

        return (out);
}

int
verify_main(int argc, char *argv[])
{
        int err;
        unsigned int i;
        bool ok = true;

        signpaths = malloc(max_signpaths * sizeof(signpaths[0]));
        err = parse_args(argc, argv);

        VPRINTF("Loading verification key from %s, verifying", pubpath);

        for(i = 0; i < nsignpaths; i++) {
                VPRINTF(" %s", signpaths[i]);
        }

        if (elf_version(EV_CURRENT) == EV_NONE) {
                fprintf(stderr, "ELF library cannot handle version %u\n",
                    EV_CURRENT);
        }

        load_cert();

        if (elf_version(EV_CURRENT) == EV_NONE) {
                fprintf(stderr, "ELF library cannot handle version %u\n",
                    EV_CURRENT);
        }

        for(i = 0; i < nsignpaths; i++) {
                int fd;
                Elf *elf;
                void *ptr;
                struct stat st;

                VPRINTF("Signing %s\n", signpaths[i]);
                fd = open(signpaths[i], O_RDONLY);
                check_fd_error(fd);

                if (fstat(fd, &st) != 0) {
                        perror("Error getting file size");
                        exit(errno);
                }

                ptr = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE, fd, 0);

                if (ptr == MAP_FAILED) {
                        perror("Error mapping file contents");
                        exit(errno);
                }

                elf = elf_memory(ptr, st.st_size);
                check_elf_error();
                ok &= verify_elf(elf, signpaths[i]);
                elf_end(elf);
        }

        return (ok ? 0 : 1);
}
