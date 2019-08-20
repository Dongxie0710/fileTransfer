#ifndef SHA1_H
#define SHA1_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

char* generate_sha1_of_file(char* filename);

#endif