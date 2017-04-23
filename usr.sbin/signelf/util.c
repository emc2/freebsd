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

#include <libelf.h>
#include <gelf.h>

#include "signelf.h"

void
check_elf_error(void)
{
        int err;

        if ((err = elf_errno()) != 0) {
                fprintf(stderr, "Error handling ELF file: %s\n",
                    elf_errmsg(err));
                exit(1);
        }
}

void
check_fd_error(int fd) {
        if(fd < 0) {
                perror("Error opening file");
                exit(1);
        }
}

size_t
find_sig(Elf *elf)
{
        Elf_Scn *curr = NULL;
        GElf_Ehdr ehdr;
        bool link_strtab = false;
        size_t strtabidx;
        size_t out = 0;

        /* Try to get the strtab index */
        gelf_getehdr(elf, &ehdr);
        check_elf_error();

        /* See elf(5) man page for meaning of this. */
        if (ehdr.e_shstrndx == SHN_UNDEF) {
                fprintf(stderr, "File contains no section header names\n");
                exit(1);
        } else if (ehdr.e_shstrndx == SHN_XINDEX) {
                link_strtab = true;
        } else {
                strtabidx = ehdr.e_shstrndx;
        }

        for(curr = elf_nextscn(elf, curr); curr != NULL;
            curr = elf_nextscn(elf, curr)) {
                GElf_Shdr shdr;
                char *str;

                check_elf_error();
                gelf_getshdr(curr, &shdr);
                check_elf_error();

                if(shdr.sh_type != SHT_NOTE) {
                        break;
                }

                /* See elf(5) man page for meaning of this. */
                if (link_strtab) {
                        strtabidx = shdr.sh_link;
                }

                str = elf_strptr(elf, strtabidx, shdr.sh_name);
                check_elf_error();

                if(!strcmp(str, ".sign")) {
                        break;
                }
        }

        if (curr != NULL) {
                out = elf_ndxscn(curr);
                check_elf_error();
        }

        return (out);
}
