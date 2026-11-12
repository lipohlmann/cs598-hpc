#include "rcb.h"

typedef struct {
  double c[3];  // element centroid
  size_t id;    // element ID
} Item;

// Comparators, one per axis. Ties are broken by element ID so the sort (and so
// the partition) is deterministic.
#define DEFINE_CMP(name, axis)                                \
  static int name(const void* a, const void* b) {            \
    const Item* ia = a;                                       \
    const Item* ib = b;                                       \
    if (ia->c[axis] < ib->c[axis]) return -1;                 \
    if (ia->c[axis] > ib->c[axis]) return 1;                  \
    return (ia->id > ib->id) - (ia->id < ib->id);             \
  }
DEFINE_CMP(cmp_x, 0)
DEFINE_CMP(cmp_y, 1)
DEFINE_CMP(cmp_z, 2)

static void rcb_recurse(Item* items, const size_t count, const int first_part,
                        const int n_parts, int* owner) {
  if (count == 0) return;

  if (n_parts == 1) {
    for (size_t i = 0; i < count; i++) owner[items[i].id] = first_part;
    return;
  }

  // bounding box of the centroids, pick the longest axis (ties: x, then y)
  double lo[3], hi[3];
  for (int a = 0; a < 3; a++) lo[a] = hi[a] = items[0].c[a];
  for (size_t i = 1; i < count; i++)
    for (int a = 0; a < 3; a++) {
      if (items[i].c[a] < lo[a]) lo[a] = items[i].c[a];
      if (items[i].c[a] > hi[a]) hi[a] = items[i].c[a];
    }
  int axis = 0;
  for (int a = 1; a < 3; a++)
    if (hi[a] - lo[a] > hi[axis] - lo[axis]) axis = a;

  int (*cmp[3])(const void*, const void*) = {cmp_x, cmp_y, cmp_z};
  qsort(items, count, sizeof(Item), cmp[axis]);

  // split ranks in half, and elements in proportion to the ranks
  const int p_left = n_parts / 2;
  const int p_right = n_parts - p_left;
  const size_t n_left = count * (size_t)p_left / (size_t)n_parts;

  rcb_recurse(items, n_left, first_part, p_left, owner);
  rcb_recurse(items + n_left, count - n_left, first_part + p_left, p_right,
              owner);
}

int* rcb_partition(const Mesh* mesh, const int n_parts) {
  const size_t n_elements = (mesh->nx - 1) * (mesh->ny - 1) * (mesh->nz - 1);

  int* owner = calloc(n_elements, sizeof(int));
  Item* items = malloc(n_elements * sizeof(Item));

  // centroid = midpoint of the element's opposite corners (nodes 1 and 7 in
  // the canonical numbering, i.e. node_ids[0] and node_ids[6])
  for (size_t e = 0; e < n_elements; e++) {
    const Triple lo = flat_node_id_to_triple(mesh->elements[e].node_ids[0], mesh);
    const Triple hi = flat_node_id_to_triple(mesh->elements[e].node_ids[6], mesh);
    items[e].c[0] = 0.5 * (mesh->xs[lo.i] + mesh->xs[hi.i]);
    items[e].c[1] = 0.5 * (mesh->ys[lo.j] + mesh->ys[hi.j]);
    items[e].c[2] = 0.5 * (mesh->zs[lo.k] + mesh->zs[hi.k]);
    items[e].id = mesh->elements[e].id;
  }

  rcb_recurse(items, n_elements, 0, n_parts, owner);
  free(items);
  return owner;
}

size_t* rcb_local_elements(const int* owner, const size_t n_elements,
                           const int part, size_t* n_local) {
  size_t count = 0;
  for (size_t e = 0; e < n_elements; e++) count += (owner[e] == part);

  size_t* ids = malloc((count + 1) * sizeof(size_t));
  size_t next = 0;
  for (size_t e = 0; e < n_elements; e++)
    if (owner[e] == part) ids[next++] = e;

  *n_local = count;
  return ids;
}
