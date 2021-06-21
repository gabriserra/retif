#include <argp.h>

static char doc[] =
  "ReTif enables simplified access from userspace to real-time features of the OS";

static struct argp_option options[] = 
{
  {"loglevel",  'l', "LEVEL",   0,  "Set the logging verbosity level" },
  {"output",    'o', "FILE",    0,  "Output to FILE instead of standard output" },
  { 0 }
};

struct arguments
{
    int level;
    char* output_file;
};