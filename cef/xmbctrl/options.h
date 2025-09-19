

#ifndef __XMBCTRL_OPTIONS_H__
#define __XMBCTRL_OPTIONS_H__

static char *plugins_options[] = {"Off", "On", "Remove"};

static char *cpuspeeds_options[] = {
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

static char *umd_driver_options[] = {
	"Inferno",
	"M33 Driver",
	"Sony NP9660",
};

static char *region_options[] = {
	"Disabled",        "Japan",         "America",   "Europe",    "Korea",
	"United Kingdom", "Latin America", "Australia", "Hong Kong", "Taiwan",
	"Russia",         "China",         "Debug I",   "Debug II"
};

static char *highmem_options[] = {"Disabled", "Stable", "Max"};

static char *use_extended_color_option[] = {"Disabled", "02g", "03g"};

static char *boolean_options[] = {"Off", "On"};

static char *iso_cache_options[] = {"LRU", "RR", "Off"};
static char *iso_cache_num_options[] = {"Auto", "1", "2", "4", "8", "16", "32", "64", "128"};
static char *iso_cache_size_options[] = {"Auto", "1KB", "2KB", "4KB", "8KB", "16KB", "32KB", "64KB"};
static char *iso_umd_seek_read_options[] = {"Auto", "1x", "2x", "3x", "4x"};

static char *hide_pics_options[] = {"Disabled", "Both", "PIC0 Only", "PIC1 Only"};

#endif