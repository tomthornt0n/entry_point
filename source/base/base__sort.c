
Function void
QSort(void *base,
      size_t n_items, size_t bytes_per_item,
      SortComparator *comparator)
{
    Bool sorted = False;
    while(!sorted)
    {
        sorted = True;
        for(size_t i = 0;
            i < n_items - 1;
            i += 1)
        {
            unsigned char *curr = PtrFromInt(IntFromPtr(base) + (i + 0)*bytes_per_item);
            unsigned char *next = PtrFromInt(IntFromPtr(base) + (i + 1)*bytes_per_item);
            if(comparator(next, curr) < 0)
            {
                Swap_(curr, next, bytes_per_item);
                sorted = False;
            }
        }
    }
}
