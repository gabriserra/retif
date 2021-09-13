#include <retif.h>

/**
 * A list of functions to change ANSI terminal colors or clear it
 */

void red();

void yellow();

void green();

void reset();

void clear();

void printerr(char *msg);

void spinner();

/**
 * Utility to print out enum strings instead of int values
 */

enum PLUGIN_NAME
{
    EDF,
    RM,
    RR,
    FP
};

const char *stringFromState(enum CLIENT_STATE s);

const char *stringFromName(enum PLUGIN_NAME s);
