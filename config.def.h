/*
    For location IDs, refer to the provided
    locations.txt file.

    77 is the ID for Sarajevo, which is set by default.
*/
static const char *cfg_loc = "77";

/*
    Whether vactija cache should be used.

    If this is set to 1, all vaktija data shall be
    redownloaded on every run, but no cache files
    will be made.
*/
static const int cfg_nocache = 0;

/*
    Whether vactija cache should always be
    updated every time the program is run.

    This can be overridden by CLI flags. (--no-update).

    Off by default.
*/
static const int cfg_alwaysupdate = 0;

/*
    Default directory for the cache file.
*/
static const char *cfg_cachedir = "/home/";