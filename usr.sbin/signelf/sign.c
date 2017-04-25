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

#define VPRINTF(...) (verbose ? fprintf(stderr, __VA_ARGS__) : 0)

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
static size_t first_resizable;

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
        pkcs7 = PKCS7_sign(sign_cert, sign_priv, NULL, bio,
            PKCS7_DETACHED | PKCS7_BINARY | PKCS7_NOCERTS |
            PKCS7_NOCHAIN | PKCS7_NOSMIMECAP);

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

static bool
prefer_type(GElf_Word old, GElf_Word new)
{
        /* Always prefer anything to NULL, and prefer a NOTE section
         * over anything.
         */
        if (new == SHT_NULL || old == SHT_NOTE) {
                return (false);
        }
        if (old == SHT_NULL || new == SHT_NOTE) {
                return (true);
        }

        /* Next, prefer static information over anything else. */
        if (new == SHT_SYMTAB || new == SHT_STRTAB) {
                return (true);
        }
        if (old == SHT_SYMTAB || old == SHT_STRTAB) {
                return (false);
        }

        /* Default: prefer newer over older */
        return (true);
}

/* Figure out if the given section and everything after can be resized. */
static void
find_first_resizable(Elf *elf)
{
        Elf_Scn *curr;
        GElf_Ehdr ehdr;
        size_t phnum;
        size_t curridx;

        gelf_getehdr(elf, &ehdr);
        check_elf_error();
        phnum = ehdr.e_phnum;

        /* Check the section and all sections that follow it. */
        for (curridx = ehdr.e_shnum - 1; curridx > 0; curridx--) {
                GElf_Shdr shdr;
                size_t idx;
                size_t sbegin;
                size_t send;

                curr = elf_getscn(elf, curridx);
                check_elf_error();
                gelf_getshdr(curr, &shdr);
                check_elf_error();
                sbegin = shdr.sh_offset;
                send = sbegin + shdr.sh_size;

                /* Check if the current section's file offsets are
                 * used in the program header at all.  If they are,
                 * then we can't move them.
                 */
                for (idx = 0; idx < phnum; idx++) {
                        GElf_Phdr phdr;
                        size_t pbegin;
                        size_t pend;

                        gelf_getphdr(elf, idx, &phdr);
                        check_elf_error();
                        pbegin = phdr.p_offset;
                        pend = pbegin + phdr.p_filesz;


                        if ((pbegin <= sbegin && sbegin < pend) ||
                            (pbegin <= send && send <= pend) ||
                            (sbegin <= pbegin && pbegin < send) ||
                            (sbegin <= pend && pend <= send)) {
                                first_resizable = curridx + 1;
                                return;
                        }
                }
        }

        first_resizable = 0;
}

static bool
section_resizable(Elf_Scn *scn)
{
        size_t idx;

        idx = elf_ndxscn(scn);
        check_elf_error();

        return (idx >= first_resizable);
}

static size_t
align(size_t offset, size_t align)
{
        if (offset == 0) {
                return (0);
        } else {
                size_t mask = align - 1;

                return (((offset - 1) & ~mask) + align);
        }
}

static void
fix_offsets(Elf *elf)
{
        GElf_Ehdr ehdr;
        GElf_Shdr shdr;
        Elf_Scn *curr;
        size_t offset;
        size_t shdr_start;
        size_t shdr_size;
        size_t shdr_end;
        size_t aligned;
        size_t idx;

        gelf_getehdr(elf, &ehdr);
        check_elf_error();
        shdr_start = ehdr.e_shoff;
        shdr_size = ehdr.e_shnum * ehdr.e_shentsize;
        idx = first_resizable;

        /* This can happen if the very last section is not resizable,
         * in which case we can't do anything at all here.
         */
        if (idx >= ehdr.e_shnum) {
                return;
        }

        curr = elf_getscn(elf, idx);
        check_elf_error();
        shdr_end = shdr_start + shdr_size;
        gelf_getshdr(curr, &shdr);
        check_elf_error();
        offset = shdr.sh_offset;

        /* Fix up the offsets of the section and all sections that follow it */
        while (curr != NULL) {
                /* Grab all the data elements, so that they get
                 * written back to the file when we update.
                 * Otherwise, they'll get corrupted.
                 */
                Elf_Data *data;

                for(data = elf_getdata(curr, NULL); data != NULL;
                    data = elf_getdata(curr, data));

                /* The section header might be between two offsets */
                if ((offset <= shdr_start && shdr_start <= shdr.sh_offset) ||
                    (offset <= shdr_end && shdr_end <= shdr.sh_offset) ||
                    (shdr_start <= offset && offset <= shdr_end) ||
                    (shdr_start <= shdr.sh_offset &&
                     shdr.sh_offset <= shdr_end)) {
                        aligned = align(offset, 8);
                        ehdr.e_shoff = aligned;
                        gelf_update_ehdr(elf, &ehdr);
                        check_elf_error();
                        offset = ehdr.e_shoff + shdr_size;
                }

                aligned = align(offset, shdr.sh_addralign);
                shdr.sh_offset = aligned;

                offset = shdr.sh_offset + shdr.sh_size;
                gelf_update_shdr(curr, &shdr);
                check_elf_error();
                curr = elf_nextscn(elf, curr);
                check_elf_error();

                if (curr != NULL) {
                        gelf_getshdr(curr, &shdr);
                        check_elf_error();
                }
        }

        /* The section header might have been at the very end */
        if ((shdr_start <= offset && offset <= shdr_end)) {
                aligned = align(offset, 8);
                ehdr.e_shoff = aligned;
                gelf_update_ehdr(elf, &ehdr);
                check_elf_error();
                offset += shdr_size;
        }
}

static size_t
strtab_insert_sign(Elf_Scn *scn)
{
        GElf_Shdr shdr;
        Elf_Data *data;
        char *strtab;
        size_t idx;

        data = elf_getdata(scn, NULL);
        check_elf_error();
        strtab = data->d_buf;
        idx = data->d_size;
        data->d_buf = realloc(data->d_buf, data->d_size + sizeof (SIGN_NAME));
        check_malloc(data->d_buf);
        strncpy((char *)data->d_buf + data->d_size, SIGN_NAME,
            sizeof (SIGN_NAME));
        data->d_size += sizeof (SIGN_NAME);

        /* Update the section header */
        gelf_getshdr(scn, &shdr);
        check_elf_error();
        shdr.sh_size += sizeof (SIGN_NAME);
        gelf_update_shdr(scn, &shdr);
        check_elf_error();

        return (idx);
}

static size_t
strtab_find_sign(Elf_Scn *scn)
{
        Elf_Data *data;
        char *strtab;
        size_t offset;
        char *str;

        data = elf_getdata(scn, NULL);
        check_elf_error();
        strtab = data->d_buf;

        /* ELF conventions: offset 0 is the empty string, so we start at 1 */
        for(offset = 1; offset < data->d_size;
            offset += strlen(strtab + offset) + 1) {
                str = strstr(strtab + offset, SIGN_NAME);

                if (str != NULL) {
                        return (str - strtab);
                }
        }

        return (0);
}

/* Try to find ".sign" in the strtab, or insert it if it's not there. */
static size_t
get_sign_idx(Elf_Scn *strtab)
{
        size_t idx;

        if ((idx = strtab_find_sign(strtab)) == 0) {
                /* If we can't resize the strtab, give up */
                if (!section_resizable(strtab)) {
                        fprintf(stderr, "Cannot resize strtab");
                        exit(1);
                }

                /* Insert ".sign" into the strtab and fixup all the
                 * subsequent offsets.
                 */
                idx = strtab_insert_sign(strtab);
        }

        return (idx);
}

/* Set the section name.  To do this, we need to figure out what
 * strtab to use.
 */
static void
set_sign_name(Elf *elf, Elf_Scn *newscn)
{
        GElf_Ehdr ehdr;
        GElf_Shdr shdr;
        Elf_Scn *curr = NULL;
        GElf_Word type = SHT_NULL;
        size_t sym, idx = 0;
        Elf_Scn *strtab;

        /* First, figure out which strtab section to use. */
        gelf_getehdr(elf, &ehdr);
        check_elf_error();

        if (ehdr.e_shstrndx == SHN_UNDEF) {
                /* This shouldn't happen, but check for it anyway. */
                fprintf(stderr, "File contains no section header names\n");
                exit(1);
        } else if (ehdr.e_shstrndx != SHN_XINDEX) {
                /* We're not using extended section numbering */
               idx = ehdr.e_shstrndx;
        } else {
                /* Scan through the sections, looking for the best
                 * strtab to use.
                 */
                for(curr = elf_nextscn(elf, curr); curr != NULL;
                    curr = elf_nextscn(elf, curr)) {
                        GElf_Shdr currshdr;

                        check_elf_error();
                        gelf_getshdr(curr, &currshdr);
                        check_elf_error();

                        /* If this section has a better type, update the
                         * preferred index
                         */
                        if(prefer_type(currshdr.sh_type, type)) {
                                type = currshdr.sh_type;
                                idx = currshdr.sh_link;
                        }
                }

                /* Set the link */
                gelf_getshdr(newscn, &shdr);
                check_elf_error();
                shdr.sh_link = idx;
                gelf_update_shdr(newscn, &shdr);
                check_elf_error();
        }


        /* Now find or create the index of the ".sign" string. */
        strtab = elf_getscn(elf, idx);
        check_elf_error();
        sym = get_sign_idx(strtab);

        /* Set the name */
        gelf_getshdr(newscn, &shdr);
        check_elf_error();
        shdr.sh_name = sym;
        gelf_update_shdr(newscn, &shdr);
        check_elf_error();
}

static void
sign_elf(Elf *elf)
{
        size_t idx;
        Elf_Scn *scn;
        Elf_Data *data;
        unsigned char *buf;
        size_t filesize;
        void *filedata;
        PKCS7 *pkcs7;
        size_t siglen;

        find_first_resizable(elf);
        idx = find_sig(elf);
        elf_flagelf(elf, ELF_C_SET, ELF_F_LAYOUT);

        if (idx != 0) {
                /* Resize the section and fixup offsets */
                GElf_Shdr shdr;

                scn = elf_getscn(elf, idx);
                check_elf_error();
                data = elf_getdata(scn, NULL);
                check_elf_error();

                if (data->d_size != sigsize) {
                        if (!section_resizable(scn)) {
                                fprintf(stderr,
                                    "Cannot resize signature section\n");
                                exit(1);
                        }

                        /* Set the section size and fix up all the
                         * following sections
                         */
                        gelf_getshdr(scn, &shdr);
                        check_elf_error();
                        shdr.sh_size = sigsize;
                        gelf_update_shdr(scn, &shdr);
                        check_elf_error();
                        fix_offsets(elf);

                        data->d_size = sigsize;
                        data->d_buf = realloc(data->d_buf, sigsize);
                        check_malloc(data->d_buf);
                }
        } else {
                /* Create the .sign section */
                GElf_Shdr shdr;
                GElf_Ehdr ehdr;

                /* Create a new section */
                scn = elf_newscn(elf);
                idx = elf_ndxscn(scn);
                check_elf_error();
                gelf_getehdr(elf, &ehdr);
                check_elf_error();
                ehdr.e_shnum += 1;
                gelf_update_ehdr(elf, &ehdr);
                check_elf_error();

                /* Set up the data */
                data = elf_newdata(scn);
                check_elf_error();
                data->d_align = 1;
                data->d_off = 0;
                data->d_size = sigsize;
                data->d_buf = malloc(sigsize);
                check_malloc(data->d_buf);

                /* Set the section name and type */
                gelf_getshdr(scn, &shdr);
                check_elf_error();
                shdr.sh_type = SHT_PROGBITS;
                shdr.sh_size = sigsize;
                shdr.sh_addralign = 1;
                gelf_update_shdr(scn, &shdr);
                check_elf_error();

                set_sign_name(elf, scn);
                fix_offsets(elf);
        }

        /* Set the signature section to all zeros  */
        memset(data->d_buf, 0, sigsize);
        data->d_size = sigsize;

        /* Update the file and get a pointer to the raw data */
        elf_update(elf, ELF_C_WRITE);
        check_elf_error();
        filedata = elf_rawfile(elf, &filesize);
        check_elf_error();

        /* The section and data pointers aren't good after elf_update,
         * so refresh them.
         */
        elf_nextscn(elf, NULL);
        scn = elf_getscn(elf, idx);
        check_elf_error();
        data = elf_getdata(scn, NULL);
        check_elf_error();

        /* Actually compute the signature */
        pkcs7 = make_sig(filedata, filesize);

        siglen = i2d_PKCS7(pkcs7, NULL);

        if(siglen != sigsize) {
                fprintf(stderr, "Signature size %zu is not expected %zu\n",
                    siglen, sigsize);
                abort();
        }

        /* Write back all the data. */
        buf = data->d_buf;
        siglen = i2d_PKCS7(pkcs7, &buf);

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

        VPRINTF("Loading key from %s and cert from %s, signing",
            keypath, pubpath);

        for(i = 0; i < nsignpaths; i++) {
                VPRINTF(" %s", signpaths[i]);
        }

        VPRINTF(" %s\n", ephemeral ? "with ephemeral key" : "directly\n");
        load_keys();
        write_ephemeral();
        compute_sigsize();

        if (elf_version(EV_CURRENT) == EV_NONE) {
                fprintf(stderr, "ELF library cannot handle version %u\n",
                    EV_CURRENT);
        }

        for(i = 0; i < nsignpaths; i++) {
                int fd;
                Elf *elf;

                VPRINTF("Signing %s\n", signpaths[i]);
                fd = open(signpaths[i], O_RDWR);
                check_fd_error(fd);
                elf = elf_begin(fd, ELF_C_RDWR, NULL);
                check_elf_error();
                sign_elf(elf);
                elf_end(elf);
        }

        return (err);
}
