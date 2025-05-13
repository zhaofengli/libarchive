/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 Zhaofeng Li
 * All rights reserved.
 */
#include "archive.h"
#include "archive_entry.h"
#include "test.h"
#include "test_common.h"

#define __LIBARCHIVE_BUILD 1
#include "archive_write_private.h"
#include "archive_getdate.h"

#define assertArchiveMtime(a, file_mtime, archive_mtime) \
  assertion_archive_mtime(__FILE__, __LINE__, a, file_mtime, archive_mtime)

static time_t last_entry_mtime;

static int
archive_write_test_header(struct archive_write *a, struct archive_entry *entry)
{
	(void)a; /* UNUSED */
	last_entry_mtime = archive_entry_mtime(entry);
	return (ARCHIVE_OK);
}

static void
archive_write_set_format_test(struct archive *_a)
{
	struct archive_write *a = (struct archive_write *)_a;
	a->format_write_header = archive_write_test_header;
}

/* writing with file_mtime, expecting archive_mtime */
static void
assertion_archive_mtime(const char *file, int line,
	struct archive *a, __LA_TIME_T file_mtime, __LA_TIME_T archive_mtime)
{
	struct archive_entry *ae;
	assert((ae = archive_entry_new()) != NULL);
	archive_entry_copy_pathname(ae, "new");
	archive_entry_set_mode(ae, AE_IFREG | 00644);
	archive_entry_set_size(ae, 0);
	archive_entry_set_uid(ae, 0);
	archive_entry_set_gid(ae, 0);
	archive_entry_set_mtime(ae, file_mtime, 0);
	assertEqualIntA(a, ARCHIVE_OK, archive_write_header(a, ae));
	assertion_equal_int(file, line, archive_mtime, "expected mtime", last_entry_mtime, "actual mtime", NULL);
	archive_entry_free(ae);
}

DEFINE_TEST(test_archive_write_set_forced_mtime)
{
	struct archive *a;
	time_t now, t;
	unsigned char *buff;
	const char *datestr = "1980/2/1 0:0:1 UTC";
	size_t used, buffSize = 1000000;

	assert((a = archive_write_new()) != NULL);
	archive_write_set_format_test(a);

	buff = malloc(buffSize);
	assertEqualIntA(a, ARCHIVE_OK,
		archive_write_open_memory(a, buff, buffSize, &used));

	/* no clamping */
	assertEqualIntA(a, ARCHIVE_OK,
		archive_write_set_forced_mtime(a, 20000, 0));

	assertArchiveMtime(a, 10002, 20000);
	assertArchiveMtime(a, 10001, 20000);
	assertArchiveMtime(a, 10000, 20000);

	/* clamping */
	assertEqualIntA(a, ARCHIVE_OK,
		archive_write_set_forced_mtime(a, 10001, 1));

	assertArchiveMtime(a, 10002, 10001);
	assertArchiveMtime(a, 10001, 10001);
	assertArchiveMtime(a, 10000, 10000);

	/* str */
	time(&now);
	t = __archive_get_date(now, datestr);
	assertEqualIntA(a, ARCHIVE_OK,
		archive_write_set_forced_mtime_str(a, datestr, 0));

	assertArchiveMtime(a, 10002, t);
	assertArchiveMtime(a, 10001, t);
	assertArchiveMtime(a, 10000, t);

	assertEqualIntA(a, ARCHIVE_OK, archive_write_close(a));
	assertEqualInt(ARCHIVE_OK, archive_write_free(a));
}
