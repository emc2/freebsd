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

#ifndef _SIGNELF_H_
#define _SIGNELF_H_

#include <stdio.h>
#include <stdbool.h>

#include <libelf.h>

#define DEFAULT_TRUSTDIR "/etc/trust/"
#define DEFAULT_PRIVDIR DEFAULT_TRUSTDIR "priv/"
#define DEFAULT_CERTDIR DEFAULT_TRUSTDIR "certs/"
#define DEFAULT_KEYPATH DEFAULT_PRIVDIR "local.pem"
#define DEFAULT_PUBKEYPATH DEFAULT_CERTDIR "local.pub.pem"

#define SIGN_NAME ".sign"

extern void usage(void);

extern void check_elf_error(void);
extern void check_fd_error(int fd);
extern void check_file_error(const FILE *ptr, const char* str);
extern void check_malloc_error(const void *ptr);
extern void check_ssl_error(const char *op);

extern size_t find_sig(Elf *elf);

extern int sign_main(int argc, char *argv[]);
extern int verify_main(int argc, char *argv[]);
extern int unsign_main(int argc, char *argv[]);
#endif
