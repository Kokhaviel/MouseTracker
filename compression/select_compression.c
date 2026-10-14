/* 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Adrien Golinelli <golinelli.adrien@gmail.com>
 */
#include "compression.h"
#include <stdio.h>
#include <string.h>

extern const struct compressor none_compressor;

#ifdef HAVE_BZIP2
extern const struct compressor bzip2_compressor;
#endif

#ifdef HAVE_ZSTD
extern const struct compressor zstd_compressor;
#endif

#ifdef HAVE_XZ
extern const struct compressor xz_compressor;
#endif

#ifdef HAVE_GZIP
extern const struct compressor gzip_compressor;
#endif


static const struct compressor *available_compressors[] = {
#ifdef HAVE_BZIP2
    &bzip2_compressor,
#endif
#ifdef HAVE_ZSTD
    &zstd_compressor,
#endif
#ifdef HAVE_XZ
    &xz_compressor,
#endif
#ifdef HAVE_GZIP
    &gzip_compressor,
#endif
    &none_compressor,
    NULL
};

void list_compressors(void)
{
    printf("Available compressors:\n");

    for (size_t i = 0; available_compressors[i]; ++i) {
        const struct compressor *c = available_compressors[i];
        printf("  %-8s  .%s\n", c->name, c->extension);
    }
}

const struct compressor *compression_select_auto(void) {
  /* first non-none compressor in priority order */
  for (size_t i = 0; available_compressors[i]; ++i) {
    if (available_compressors[i] != &none_compressor)
      return available_compressors[i];
  }

  return &none_compressor;
}

const struct compressor *compression_select_by_name(const char *name) {
  if (!name)
    return NULL;
  for (size_t i = 0; available_compressors[i]; i++) {
    if (strcmp(available_compressors[i]->name, name) == 0)
      return available_compressors[i];
  }

  return NULL;
}

const struct compressor *compression_select_by_ext(const char *ext) {
  if (!ext)
    return NULL;
  for (size_t i = 0; available_compressors[i]; i++) {
    if (strcmp(available_compressors[i]->extension, ext) == 0) {
      return available_compressors[i];
    }
  }

  return NULL;
}
