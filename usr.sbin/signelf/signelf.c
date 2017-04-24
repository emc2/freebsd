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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "signelf.h"

#include <openssl/err.h>
#include <openssl/evp.h>

static const char* cmd;

static const char * const usage_detail =
  "  Commands:\n"
  "\n"
  "    sign: add signatures to one or more executables or shared objects\n"
  "\n"
  "      Options:\n"
  "\n"
  "        -e <path>    Sign with ephemeral keys, save public key to file\n"
  "        -k <path>    Path to private (signing) key\n"
  "        -p <path>    Path to public (verification) key\n"
  "        -v           Verbose mode\n"
  "\n"
  "    verify: check signatures on one or more executables or shared objects\n"
  "\n"
  "      Options:\n"
  "\n"
  "        -d           Files are signed directly\n"
  "        -n           Don't check signature chains\n"
  "        -k <path>    Path to verification (public) key\n"
  "        -v           Verbose mode\n"
  "\n"
  "    unsign: delete signatures from one or more executables or shared objects\n"
  "\n"
  "      Options:\n"
  "\n"
  "        -q           Don't fail if a file doesn't contain a signature\n"
  "        -v           Verbose mode\n";

void
usage(void)
{
        fprintf(stderr, "Usage: %s <command> [option]* [filename]+\n\n%s\n",
                cmd, usage_detail);
}

int
main(const int argc, char* argv[])
{
        ERR_load_crypto_strings();
        OpenSSL_add_all_algorithms();

        cmd = argv[0];

        if (argc > 1) {
                if (!strcmp(argv[1], "sign")) {
                        return (sign_main(argc - 1, argv + 1));
                } else if (!strcmp(argv[1], "verify")) {
                        return (verify_main(argc - 1, argv + 1));
                } else {
                        fprintf(stderr, "Invalid command %s\n", argv[1]);
                }
        } else {
                fprintf(stderr, "Need a command\n");
        }

        usage();

        return (1);
}
