#ifndef __SATELITE_OPT_H__
#define __SATELITE_OPT_H__

static char *g_disenabled[] = { "Disabled", "Enabled" };
static char *g_endisabled[] = { "Enabled", "Disabled" };

static char *g_cpuspeeds[] = { "Default", "20/10", "75/37", "100/50", "133/66", "222/111", "266/133", "300/150", "333/166" };
static char *g_umdmodes[] = { "Inferno", "M33 Driver", "ME Driver", "Sony NP9660" };

static char *g_highmem[] = { "Default", "Stable", "Max" };
static char *g_iso_cache[] = {"LRU", "RR", "Off"};
static char *g_iso_cache_num[] = {"Auto", "1", "2", "4", "8", "16", "32", "64", "128"};
static char *g_iso_cache_size[] = {"Auto", "1KB", "2KB", "4KB", "8KB", "16KB", "32KB", "64KB"};
static char *g_umd_seek_read_delay[] = {"Auto", "1x", "2x", "3x", "4x"};
static char *g_umd_seek_read_strat[] = {"Auto", "Per FD", "Global"};
static char *g_fake_free_mem[] = {"Auto", "16MB", "24MB", "32MB", "40MB", "48MB", "52MB", "60MB", "64MB"};

#endif /* __SATELITE_OPT_H__ */