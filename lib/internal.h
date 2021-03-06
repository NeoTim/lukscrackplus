/*
 * libcryptsetup - cryptsetup library internal
 *
 * Copyright (C) 2004, Christophe Saout <christophe@saout.de>
 * Copyright (C) 2004-2007, Clemens Fruhwirth <clemens@endorphin.org>
 * Copyright (C) 2009-2012, Red Hat, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef INTERNAL_H
#define INTERNAL_H

#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <inttypes.h>

#include "nls.h"
#include "bitops.h"
#include "utils_crypt.h"
#include "utils_loop.h"
#include "utils_dm.h"
#include "utils_fips.h"
#include "crypto_backend.h"

#include "libcryptsetup.h"

/* to silent gcc -Wcast-qual for const cast */
#define CONST_CAST(x) (x)(uintptr_t)

#define SECTOR_SHIFT		9
#define SECTOR_SIZE		(1 << SECTOR_SHIFT)
#define DEFAULT_DISK_ALIGNMENT	1048576 /* 1MiB */
#define DEFAULT_MEM_ALIGNMENT	4096
#define MAX_ERROR_LENGTH	512

#define at_least(a, b) ({ __typeof__(a) __at_least = (a); (__at_least >= (b))?__at_least:(b); })

struct crypt_device;

struct volume_key {
	size_t keylength;
	char key[];
};

struct volume_key *crypt_alloc_volume_key(unsigned keylength, const char *key);
struct volume_key *crypt_generate_volume_key(struct crypt_device *cd, unsigned keylength);
void crypt_free_volume_key(struct volume_key *vk);

int crypt_confirm(struct crypt_device *cd, const char *msg);

char *crypt_lookup_dev(const char *dev_id);
int crypt_sysfs_check_crypt_segment(const char *device, uint64_t offset, uint64_t size);
int crypt_sysfs_get_rotational(int major, int minor, int *rotational);

int sector_size_for_device(const char *device);
int device_read_ahead(const char *dev, uint32_t *read_ahead);
ssize_t write_blockwise(int fd, void *buf, size_t count);
ssize_t read_blockwise(int fd, void *_buf, size_t count);
ssize_t write_lseek_blockwise(int fd, char *buf, size_t count, off_t offset);
int device_ready(struct crypt_device *cd, const char *device, int mode);
int device_size(const char *device, uint64_t *size);

unsigned crypt_getpagesize(void);

enum devcheck { DEV_OK = 0, DEV_EXCL = 1, DEV_SHARED = 2 };
int device_check_and_adjust(struct crypt_device *cd,
			    const char *device,
			    enum devcheck device_check,
			    uint64_t *size,
			    uint64_t *offset,
			    uint32_t *flags);

void logger(struct crypt_device *cd, int class, const char *file, int line, const char *format, ...);
#define log_dbg(x...) logger(NULL, CRYPT_LOG_DEBUG, __FILE__, __LINE__, x)
#define log_std(c, x...) logger(c, CRYPT_LOG_NORMAL, __FILE__, __LINE__, x)
#define log_verbose(c, x...) logger(c, CRYPT_LOG_VERBOSE, __FILE__, __LINE__, x)
#define log_err(c, x...) logger(c, CRYPT_LOG_ERROR, __FILE__, __LINE__, x)

int crypt_get_debug_level(void);

int crypt_memlock_inc(struct crypt_device *ctx);
int crypt_memlock_dec(struct crypt_device *ctx);

void get_topology_alignment(const char *device,
			    unsigned long *required_alignment, /* bytes */
			    unsigned long *alignment_offset,   /* bytes */
			    unsigned long default_alignment);

int crypt_random_init(struct crypt_device *ctx);
int crypt_random_get(struct crypt_device *ctx, char *buf, size_t len, int quality);
void crypt_random_exit(void);
int crypt_random_default_key_rng(void);

int crypt_plain_hash(struct crypt_device *ctx,
		     const char *hash_name,
		     char *key, size_t key_size,
		     const char *passphrase, size_t passphrase_size);
int PLAIN_activate(struct crypt_device *cd,
		     const char *name,
		     struct volume_key *vk,
		     uint64_t size,
		     uint32_t flags);

/**
 * Different methods used to erase sensitive data concerning
 * either encrypted payload area or master key inside keyslot
 * area
 */
typedef enum {
	CRYPT_WIPE_ZERO, /**< overwrite area using zero blocks */
	CRYPT_WIPE_DISK, /**< erase disk (using Gutmann method if it is rotational disk)*/
	CRYPT_WIPE_SSD, /**< erase solid state disk (random write) */
	CRYPT_WIPE_RANDOM /**< overwrite area using some up to now unspecified
			    * random algorithm */
} crypt_wipe_type;

int crypt_wipe(const char *device,
	       uint64_t offset,
	       uint64_t sectors,
	       crypt_wipe_type type,
	       int flags);

#endif /* INTERNAL_H */
