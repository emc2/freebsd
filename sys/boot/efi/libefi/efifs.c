/*-
 * Copyright (c) 2017 Eric McCorkle
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#include <efi.h>
#include <efilib.h>
#include <efiprot.h>
#include <stand.h>
#include <stdarg.h>
#include <bootstrap.h>

static EFI_GUID FileInfoGUID = EFI_FILE_INFO_ID;;
static EFI_GUID SimpleFileSystemProtocolGUID = SIMPLE_FILE_SYSTEM_PROTOCOL;
static EFI_GUID DevicePathGUID = DEVICE_PATH_PROTOCOL;

static int efifs_open(const char *path, struct open_file *f);
static int efifs_write(struct open_file *f, void *buf, size_t size, size_t *resid);
static int efifs_close(struct open_file *f);
static int efifs_read(struct open_file *f, void *buf, size_t size, size_t *resid);
static off_t efifs_seek(struct open_file *f, off_t offset, int where);
static int efifs_stat(struct open_file *f, struct stat *sb);
static int efifs_readdir(struct open_file *f, struct dirent *d);

static int efifs_dev_init(void);
static int efifs_dev_strategy(void *, int, daddr_t, size_t, char *, size_t *);
static int efifs_dev_open(struct open_file *, ...);
static int efifs_dev_close(struct open_file *);
static int efifs_dev_print(int);

struct devsw efifs_dev = {
	.dv_name = "EFI",
	.dv_type = DEVT_EFI,
	.dv_init = efifs_dev_init,
	.dv_strategy = efifs_dev_strategy,
	.dv_open = efifs_dev_open,
	.dv_close = efifs_dev_close,
	.dv_ioctl = noioctl,
	.dv_print = efifs_dev_print,
	.dv_cleanup = NULL
};

struct fs_ops efifs_fsops = {
	"EFI",
	efifs_open,
	efifs_close,
	efifs_read,
	efifs_write,
	efifs_seek,
	efifs_stat,
	efifs_readdir
};

static int
efifs_dev_init(void)
{
        EFI_HANDLE *hin, *handles;
	EFI_STATUS status;
	UINTN sz;
	u_int n, nin, unit;
	int err;

	sz = 0;
	hin = NULL;
	status = BS->LocateHandle(ByProtocol,
            &SimpleFileSystemProtocolGUID, 0, &sz, 0);
	if (status == EFI_BUFFER_TOO_SMALL) {
		hin = (EFI_HANDLE *)malloc(sz);
                handles = (EFI_HANDLE *)malloc(sz);
		status = BS->LocateHandle(ByProtocol,
                    &SimpleFileSystemProtocolGUID, 0, &sz, hin);
		if (EFI_ERROR(status))
			free(hin);
	}
	if (EFI_ERROR(status))
		return (efi_status_to_errno(status));

	/* Filter handles to only include FreeBSD partitions. */
	nin = sz / sizeof(EFI_HANDLE);
	unit = 0;

	for (n = 0; n < nin; n++) {
		status = BS->OpenProtocol(hin[n], &SimpleFileSystemProtocolGUID,
                    NULL, IH, NULL, EFI_OPEN_PROTOCOL_TEST_PROTOCOL);
		if (EFI_ERROR(status))
			continue;

                handles[unit++] = hin[n];
	}

        efi_register_handles(&efifs_dev, handles, NULL, unit);

        free(handles);
	free(hin);
	return (err);
}


static int
efifs_dev_print(int verbose)
{
	char line[80];
        CHAR16 *name16;
        EFI_DEVICE_PATH *devpath;
	EFI_HANDLE h;
	EFI_STATUS status;
	u_int unit;

	for (unit = 0, h = efi_find_handle(&efifs_dev, 0);
	    h != NULL; h = efi_find_handle(&efifs_dev, ++unit)) {
		sprintf(line, "    %s%d:", efifs_dev.dv_name, unit);
		pager_output(line);
                pager_output("    EFI_SIMPLE_FILE_SYSTEM");

                if (verbose) {
                        status = BS->HandleProtocol(h, &DevicePathGUID,
                            (void **)&devpath);
                        if (!EFI_ERROR(status)) {
                                name16 = efi_devpath_name(devpath);
                                char buf[wcslen(name16) + 1];
                                cpy16to8(name16, buf, wcslen(name16));

                                /* Print out the device path if we have one */
                                pager_output(", devpath = ");
                                pager_output(buf);
                        }
                }

		pager_output(")\n");
	}
        return (0);
}

static int
efifs_dev_open(struct open_file *f, ...)
{
	va_list args;
	struct devdesc *dev;
	EFI_FILE_IO_INTERFACE *fsiface;
	EFI_HANDLE h;
	EFI_STATUS status;

	va_start(args, f);
	dev = va_arg(args, struct devdesc*);
	va_end(args);

	h = efi_find_handle(&efifs_dev, dev->d_unit);

	if (h == NULL)
		return (EINVAL);

        status = BS->OpenProtocol(h, &SimpleFileSystemProtocolGUID,
            (void**)&fsiface, IH, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);

        if (EFI_ERROR(status)) {
		return (efi_status_to_errno(status));
        }

        dev->d_opendata = fsiface;
	return (0);
}

static int
efifs_dev_close(struct open_file *f)
{
	struct devdesc *dev;
	EFI_HANDLE h;
        EFI_STATUS status;

	dev = (struct devdesc *)(f->f_devdata);
	h = efi_find_handle(&efifs_dev, dev->d_unit);

	if (h == NULL)
		return (EINVAL);

	if (dev->d_opendata == NULL)
		return (EINVAL);

        status = BS->CloseProtocol(h, &SimpleFileSystemProtocolGUID, IH, NULL);

        if (EFI_ERROR(status))
		return (efi_status_to_errno(status));

	dev->d_opendata = NULL;
	return (0);
}


/* Raw I/O isn't supported on EFI FS devices, as they talk through
 * SIMPLE_FILE_SYSTEM_INTERFACE.
 */
static int
efifs_dev_strategy(void *devdata __unused, int rw __unused, daddr_t blk __unused,
                   size_t size __unused, char *buf __unused,
                   size_t *rsize __unused)
{
        printf("Raw I/O not supported on EFI FS interface\n");
	return ENOTSUP;
}

/*
 * Open a file.
 */
static int
efifs_open(const char *upath, struct open_file *f)
{
	struct devdesc *dev;
	EFI_FILE_IO_INTERFACE *fsiface;
	EFI_FILE_HANDLE root;
        EFI_STATUS status;
        CHAR16 path[strlen(upath) + 1];

	dev = (struct devdesc *)(f->f_devdata);
        fsiface = dev->d_opendata;

        if (!strcmp(upath, "") || !strcmp(upath, "/")) {
                return (fsiface->OpenVolume(fsiface,
                           (EFI_FILE_HANDLE*)&(f->f_fsdata)));
        } else {
                status = fsiface->OpenVolume(fsiface, &root);

                if (EFI_ERROR(status)) {
                        return (efi_status_to_errno(status));
                }

                cpy8to16(upath, path, sizeof(CHAR16) * strlen(upath));
                status = root->Open(root, (EFI_FILE_HANDLE*)&(f->f_fsdata),
                    path, EFI_FILE_MODE_READ, 0);
                root->Close(root);

                if (EFI_ERROR(status)) {
                        return (efi_status_to_errno(status));
                }

                return 0;
        }
}

static int
efifs_close(struct open_file *f)
{
        EFI_FILE_HANDLE file = (EFI_FILE_HANDLE)f->f_fsdata;
        EFI_STATUS status;

        status = file->Close(file);

        if (EFI_ERROR(status))
		return (efi_status_to_errno(status));

        return (0);
}

static int
efifs_read(struct open_file *f, void *start, size_t size, size_t *resid /* out */)
{
        EFI_FILE_HANDLE file = (EFI_FILE_HANDLE)f->f_fsdata;
        UINTN readsize = size;
        EFI_STATUS status;

        status = file->Read(file, &readsize, start);

        if (EFI_ERROR(status))
		return (efi_status_to_errno(status));

	if (resid)
		*resid = size - readsize;

	return (0);
}

static int
efifs_write(struct open_file *f, void *start, size_t size,
            size_t *resid /* out */)
{
        EFI_FILE_HANDLE file = (EFI_FILE_HANDLE)f->f_fsdata;
        UINTN writesize = size;
        EFI_STATUS status;

        status = file->Write(file, &writesize, start);

        if (EFI_ERROR(status))
		return (efi_status_to_errno(status));

	if (resid)
		*resid = size - writesize;

	return (0);
}

static off_t
efifs_seek(struct open_file *f, off_t offset, int where)
{
        EFI_FILE_HANDLE file = (EFI_FILE_HANDLE)f->f_fsdata;
        UINT64 pos;
        EFI_STATUS status;

	switch (where) {
	case SEEK_SET:
                status = file->SetPosition(file, offset);

                if (status != EFI_SUCCESS) {
                        errno = (efi_status_to_errno(status));
                        return -1;
                }

		break;
	case SEEK_CUR:
                status = file->GetPosition(file, &pos);

                if (status != EFI_SUCCESS) {
                        errno = (efi_status_to_errno(status));
                        return -1;
                }

                status = file->SetPosition(file, pos + offset);

                if (status != EFI_SUCCESS) {
                        errno = (efi_status_to_errno(status));
                        return -1;
                }

		break;
	case SEEK_END:
                status = file->SetPosition(file, 0xffffffffffffffff);

                if (status != EFI_SUCCESS) {
                        errno = (efi_status_to_errno(status));
                        return -1;
                }
	default:
		errno = EINVAL;
		return (-1);
	}

        status = file->GetPosition(file, &pos);

        if (status != EFI_SUCCESS) {
                errno = (efi_status_to_errno(status));
                return -1;
        }

	return (pos);
}

/* SIMPLE_FILE_SYSTEM_PROTOCOL is geared towards FAT, so we can't
 * reproduce stat with absolute fidelity.
 */
static int
efifs_stat(struct open_file *f, struct stat *sb)
{
        EFI_FILE_HANDLE file;
        UINTN size = 0;
        EFI_FILE_INFO *finfo;
        EFI_STATUS status;

        file = (EFI_FILE_HANDLE)f->f_fsdata;
        status = file->GetInfo(file, &FileInfoGUID, &size, NULL);

        if (status != EFI_BUFFER_TOO_SMALL) {
                errno = (efi_status_to_errno(status));
                return -1;
        }

        finfo = malloc(size);
        status = file->GetInfo(file, &FileInfoGUID, &size, finfo);

        if (status != EFI_SUCCESS) {
                errno = (efi_status_to_errno(status));
                return -1;
        }

        /* We can't properly fill these in... */
        sb->st_ino = 0;
        sb->st_nlink = 0;
        sb->st_uid = 0;
        sb->st_gid = 0;
        sb->st_blksize = 512;
        /* Build the mode field */
        if (finfo->Attribute & EFI_FILE_DIRECTORY) {
                sb->st_mode = S_IFDIR;
        } else {
                sb->st_mode = S_IFREG;
        }

        if (finfo->Attribute & EFI_FILE_MODE_READ) {
                sb->st_mode = S_IRUSR | S_IXUSR | S_IRGRP |
                    S_IXGRP | S_IROTH | S_IXOTH;
        }

        if (finfo->Attribute & EFI_FILE_MODE_READ) {
          sb->st_mode = S_IWUSR | S_IWGRP | S_IWOTH;
        }
        /* This may or may not be supported, depending on the FS driver */
        sb->st_blocks = finfo->PhysicalSize / 512;
        /* These fields we can get right */
        sb->st_size = finfo->FileSize;
        sb->st_atime = from_efi_time(&(finfo->LastAccessTime));
        sb->st_mtime = from_efi_time(&(finfo->ModificationTime));
        sb->st_ctime = from_efi_time(&(finfo->CreateTime));

        free(finfo);

        return (0);
}

static int
efifs_readdir(struct open_file *f, struct dirent *d)
{
        EFI_FILE_HANDLE dir;
        EFI_FILE_HANDLE entry;
        UINTN size = 0;
        EFI_FILE_INFO *finfo;
        EFI_STATUS status;

        dir = (EFI_FILE_HANDLE)f->f_fsdata;
        status = dir->Read(dir, &size, NULL);

        if (status != EFI_BUFFER_TOO_SMALL) {
                errno = (efi_status_to_errno(status));
                return -1;
        }

        if (size == 0) {
                return (ENOENT);
        }

        finfo = malloc(size);
        status = dir->Read(dir, &size, finfo);

        if (status != EFI_SUCCESS) {
                errno = (efi_status_to_errno(status));
                return -1;
        }

        cpy16to8(finfo->FileName, d->d_name, MAXNAMLEN);
        d->d_namlen = strlen(d->d_name);
        d->d_reclen = sizeof(struct dirent);
        /* We can't faithfully reproduce this due to the limitations
         * of the SIMPLE_FILE_SYSTEM interface */
        d->d_fileno = 0;

        /* The FAT-style interface here forces us to open the file to
         * get its attributes.
         */
        status = dir->Open(dir, &entry, finfo->FileName,
                           EFI_FILE_MODE_READ, 0);
        free(finfo);

        if (EFI_ERROR(status)) {
                errno = (efi_status_to_errno(status));
                return -1;
        }

        size = 0;
        status = entry->GetInfo(entry, &FileInfoGUID, &size, NULL);

        if (status != EFI_BUFFER_TOO_SMALL) {
                errno = (efi_status_to_errno(status));
                return -1;
        }

        finfo = malloc(size);
        status = entry->GetInfo(entry, &FileInfoGUID, &size, finfo);

        if (status != EFI_SUCCESS) {
                free(finfo);
                errno = (efi_status_to_errno(status));
                return -1;
        }

        /* There is some information loss here due to the FAT-based
         * EFI_SIMPLE_FILE_SYSTEM interface
         */
        if (finfo->Attribute & EFI_FILE_DIRECTORY) {
                d->d_type = DT_DIR;
        } else {
                d->d_type = DT_REG;
        }

        free(finfo);
        entry->Close(entry);

        return (0);
}
