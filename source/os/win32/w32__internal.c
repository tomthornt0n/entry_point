static uint64_t
W32_U64FromHiAndLoWords(DWORD hi,
                        DWORD lo)
{
 uint64_t result = 0;
 result |= ((uint64_t)hi) << 32;
 result |= ((uint64_t)lo) <<  0;
 return result;
}
