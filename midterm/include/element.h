#pragma once

#include <stdlib.h>

typedef struct {
  size_t* node_ids[8];  // IDs of nodes in canonical form
  size_t id;            // element ID
} Element;
