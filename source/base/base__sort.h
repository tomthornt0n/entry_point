
typedef int SortComparator(const void *a, const void *b);

// TODO(tbt): this is broken
Function void QSort (void *base, size_t n_items, size_t bytes_per_item, SortComparator *comparator);