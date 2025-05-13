/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 Zhaofeng Li
 * All rights reserved.
 */
#include "archive.h"
#include "archive_entry.h"
#include "test.h"

#define __LIBARCHIVE_BUILD 1
#include "archive_getdate.h"

static time_t
read_mtime(struct archive *a, const char *path)
{
	struct archive_entry *entry;
	time_t t;

	entry = archive_entry_new();
	assert(entry != NULL);
	archive_entry_copy_pathname(entry, path);
	assertEqualIntA(a, ARCHIVE_OK,
	    archive_read_disk_entry_from_file(a, entry, -1, NULL));

	t = archive_entry_mtime(entry);
	archive_entry_free(entry);

	return t;
}

static void
test_forced_mtime(void)
{
	struct archive *a;

	assert((a = archive_read_disk_new()) != NULL);
	assertEqualInt(ARCHIVE_OK, archive_read_disk_set_forced_mtime(a,
				20000, 0));

	assertEqualIntA(a, 20000, read_mtime(a, "new_mtime"));
	assertEqualIntA(a, 20000, read_mtime(a, "mid_mtime"));
	assertEqualIntA(a, 20000, read_mtime(a, "old_mtime"));

	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}

static void
test_forced_clamped_mtime(void)
{
	struct archive *a;

	assert((a = archive_read_disk_new()) != NULL);
	assertEqualInt(ARCHIVE_OK, archive_read_disk_set_forced_mtime(a,
				10001, 1));

	assertEqualIntA(a, 10001, read_mtime(a, "new_mtime"));
	assertEqualIntA(a, 10001, read_mtime(a, "mid_mtime"));
	assertEqualIntA(a, 10000, read_mtime(a, "old_mtime"));

	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}

static void
test_forced_mtime_str(void)
{
	struct archive *a;
	time_t now, t;

	time(&now);
	t = __archive_get_date(now, "1980/2/1 0:0:1 UTC");

	assert((a = archive_read_disk_new()) != NULL);
	assertEqualInt(ARCHIVE_OK, archive_read_disk_set_forced_mtime(a,
				t, 0));

	assertEqualIntA(a, t, read_mtime(a, "new_mtime"));
	assertEqualIntA(a, t, read_mtime(a, "mid_mtime"));
	assertEqualIntA(a, t, read_mtime(a, "old_mtime"));

	assertEqualInt(ARCHIVE_OK, archive_read_free(a));
}

DEFINE_TEST(test_read_disk_forced_mtime)
{
	assertMakeFile("new_mtime", 0666, "new");
	assertUtimes("new_mtime", 10002, 0, 10002, 0);
	assertMakeFile("mid_mtime", 0666, "mid");
	assertUtimes("mid_mtime", 10001, 0, 10001, 0);
	assertMakeFile("old_mtime", 0666, "old");
	assertUtimes("old_mtime", 10000, 0, 10000, 0);

	test_forced_mtime();
	test_forced_clamped_mtime();
	test_forced_mtime_str();
}
