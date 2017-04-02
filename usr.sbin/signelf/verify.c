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

#include <sys/param.h>

#include "signelf.h"

static char keypath[MAXPATHLEN + 1];
static char **signpaths;
static size_t nsignpaths = 0;
static size_t max_signpaths = 16;
static bool verbose = false;
static bool check = true;
static bool appkey = true;

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

static void set_keypath(const char *path)
{
        strncpy(keypath, path, MAXPATHLEN);
}

static int parse_args(const int argc, char* argv[])
{
        int ch;
        int i;

        while ((ch = getopt(argc, argv, "dk:nv")) != -1) {
                switch (ch) {
                      default:
                              usage();

                              return (1);
                      case 'd':
                              check = false;
                              appkey = false;
                              break;
                      case 'k':
                              set_keypath(optarg);
                              break;
                      case 'n':
                              check = false;
                              break;
                      case 'v':
                              verbose = true;
                              break;
                      case '?':
                              usage();
                              break;
                }
        }

        if (!strcmp(keypath, "")) {
                if (appkey && check) {
                        set_keypath(DEFAULT_KEYDIR);
                } else {
                        set_keypath(DEFAULT_PUBKEYPATH);
                }
        }

        for(i = optind; i < argc; i++) {
                add_signpath(argv[i]);
        }

        return (0);
}

int
verify_main(int argc, char *argv[])
{
        int err;

        signpaths = malloc(max_signpaths * sizeof(signpaths[0]));
        err = parse_args(argc, argv);

        if (verbose) {
                if (!check && appkey) {
                        fprintf(stderr, "Verifying");
                } else {
                        fprintf(stderr, "Loading %s %s, verifying",
                            check ? "vendor key(s) from" :
                            "verification key from", keypath);
                }

                for(unsigned int i = 0; i < nsignpaths; i++) {
                        fprintf(stderr, " %s", signpaths[i]);
                }

                fprintf(stderr, ", %s\n",
                    appkey ? (check ? "with signature-chain checking" :
                            "without signature-chain checking") :
                        "in direct-signing mode");
        }


        return (err);
}
