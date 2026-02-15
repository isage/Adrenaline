

#ifndef __XMBCTRL_OPTIONS_H__
#define __XMBCTRL_OPTIONS_H__

static char *g_plugins_options[] = {"Off", "On", "Remove"};

static char *g_cpuspeeds_options[] = {
	"Default",
	"20/10",
	"75/37",
	"100/50",
	"133/66",
	"222/111",
	"266/133",
	"300/150",
	"333/166"
};

static char *g_umd_driver_options[] = {
	"Inferno",
	"M33 Driver",
	"Sony NP9660",
};

static char *g_region_options[] = {
	"Disabled",        "Japan",         "America",   "Europe",    "Korea",
	"United Kingdom", "Latin America", "Australia", "Hong Kong", "Taiwan",
	"Russia",         "China",         "Debug I",   "Debug II"
};

static char *g_highmem_options[] = {"Disabled", "Stable", "Max"};

static char *g_use_extended_color_option[] = {"Disabled", "02g", "03g"};

static char *g_boolean_options[] = {"Off", "On"};

static char *g_iso_cache_options[] = {"LRU", "RR", "Off"};
static char *g_iso_cache_num_options[] = {"Auto", "1", "2", "4", "8", "16", "32", "64", "128"};
static char *g_iso_cache_size_options[] = {"Auto", "1KB", "2KB", "4KB", "8KB", "16KB", "32KB", "64KB"};
static char *g_iso_umd_seek_read_options[] = {"Auto", "1x", "2x", "3x", "4x"};

static char *g_hide_pics_options[] = {"Disabled", "Both", "PIC0 Only", "PIC1 Only"};

static char *g_fake_free_mem[] = {"Auto", "16MB", "24MB", "32MB", "40MB", "48MB", "52MB", "60MB", "64MB"};

#endif