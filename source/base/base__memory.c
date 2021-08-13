
//~NOTE(tbt): utilities

static void *
M_Set(void *memory, unsigned char value, size_t bytes)
{
 uint64_t *u64 = memory;
 uint64_t value_as_u64 = (((uint64_t)value << 56) |
                          ((uint64_t)value << 48) |
                          ((uint64_t)value << 40) |
                          ((uint64_t)value << 32) |
                          ((uint64_t)value << 24) |
                          ((uint64_t)value << 16) |
                          ((uint64_t)value <<  8) |
                          ((uint64_t)value <<  0));
 size_t u64_count = bytes / 8;
 for(size_t u64_index = 0;
     u64_index < u64_count;
     u64_index += 1)
 {
  u64[u64_index] = value_as_u64;
 }
 
 uint8_t *u8 = memory;
 for(size_t u8_index = u64_count * 8;
     u8_index < bytes;
     u8_index += 1)
 {
  u8[u8_index] = value;
 }
 
 return memory;
}

static void *
M_Copy(void *dest, const void *src, size_t bytes)
{
 uint64_t *dest_u64 = dest;
 const uint64_t *src_u64 = src;
 size_t u64_count = bytes / 8;
 for(size_t u64_index = 0;
     u64_index < u64_count;
     u64_index += 1)
 {
  dest_u64[u64_index] = src_u64[u64_index];
 }
 
 uint8_t *dest_u8 = dest;
 const uint8_t *src_u8 = src;
 for(size_t u8_index = u64_count * 8;
     u8_index < bytes;
     u8_index += 1)
 {
  dest_u8[u8_index] = src_u8[u8_index];
 }
 
 return dest;
}

static Bool
M_Compare(const void *a, const void *b, size_t bytes)
{
 const uint64_t *a_u64 = a;
 const uint64_t *b_u64 = b;
 size_t u64_count = bytes / 8;
 for(size_t u64_index = 0;
     u64_index < u64_count;
     u64_index += 1)
 {
  if(a_u64[u64_index] != b_u64[u64_index])
  {
   return False;
  }
 }
 
 const uint8_t *a_u8 = a;
 const uint8_t *b_u8 = b;
 for(size_t u8_index = u64_count * 8;
     u8_index < bytes;
     u8_index += 1)
 {
  if(a_u8[u8_index] != b_u8[u8_index])
  {
   return False;
  }
 }
 
 return True;
}

static uintptr_t
M_AlignForward(uintptr_t ptr, size_t align)
{
 Assert((align & (align - 1)) == 0 && "alignment must be power of two");
 
 uintptr_t result = (uintptr_t)ptr;
 uintptr_t alignment = (uintptr_t)align;
 uintptr_t modulo = result & (alignment - 1);
 if (modulo != 0)
 {
  result += alignment - modulo;
 }
 
 return result;
}

//~NOTE(tbt): arenas

static M_Arena
M_ArenaMake(M_Callbacks callbacks)
{
 M_Arena arena = {0};
 arena.callbacks = callbacks;
 arena.max = M_Arena_DefaultCap;
 arena.base = arena.callbacks.reserve_func(arena.max);
 arena.alloc_position = 0;
 arena.commit_position = 0;
 return arena;
}

static M_Arena
M_LocalArenaMake(void *backing, size_t size)
{
 M_Arena arena = {0};
 arena.max = size;
 arena.alloc_position = 0;
 arena.base = backing;
 arena.commit_position = size;
 return arena;
}
#define M_LocalArenaMake(b) M_LocalArenaMake(b, sizeof(b))

static M_Arena
M_FixedArenaMake(M_Callbacks callbacks, size_t size)
{
 M_Arena arena = {0};
 arena.callbacks = callbacks;
 arena.max = size;
 arena.alloc_position = 0;
 arena.base = arena.callbacks.reserve_func(arena.max);
 arena.callbacks.commit_func(arena.base, arena.max);
 arena.commit_position = arena.max;
 return arena;
}

static M_Arena
M_TransientArenaMake(M_Callbacks callbacks, size_t size)
{
 M_Arena arena = {0};
 arena.callbacks = callbacks;
 arena.flags |= M_ArenaFlags_Transient;
 arena.max = size;
 arena.base = arena.callbacks.reserve_func(arena.max);
 arena.alloc_position = 0;
 arena.commit_position = 0;
 return arena;
}

static void *
M_ArenaPushAligned(M_Arena *arena, size_t size, size_t align)
{
 void *memory = NULL;
 
 uintptr_t new_alloc_position = M_AlignForward(arena->alloc_position + size, align);
 if(new_alloc_position < arena->max)
 {
  if(arena->alloc_position + size > arena->commit_position)
  {
   size_t commit_size = size;
   commit_size += M_Arena_CommitChunkSize - 1;
   commit_size -= commit_size % M_Arena_CommitChunkSize;
   arena->callbacks.commit_func((unsigned char *)arena->base + arena->commit_position, commit_size);
   arena->commit_position += commit_size;
  }
  
  memory = (unsigned char *)arena->base + arena->alloc_position;
  arena->alloc_position += size;
  M_Set(memory, 0, size);
 }
 else if(arena->flags & M_ArenaFlags_Transient)
 {
  M_ArenaClear(arena);
  memory = M_ArenaPush(arena, size);
 }
 
 Assert(memory);
 return memory;
}

static void *
M_ArenaPush(M_Arena *arena, size_t size)
{
 return M_ArenaPushAligned(arena, size, M_DefaultAlignment);
}

static void
M_ArenaPop(M_Arena *arena, size_t size)
{
 if(size > arena->alloc_position)
 {
  size = arena->alloc_position;
 }
 arena->alloc_position -= size;
 
 if(arena->flags & M_ArenaFlags_DynamicDecommit)
 {
  size_t decommit_threshold = arena->commit_position - M_Arena_CommitChunkSize;
  if(arena->alloc_position < decommit_threshold)
  {
   arena->callbacks.decommit_func((unsigned char *)arena->base + decommit_threshold, M_Arena_CommitChunkSize);
   arena->commit_position = decommit_threshold;
  }
 }
}

static void
M_ArenaClear(M_Arena *arena)
{
 M_ArenaPop(arena, arena->alloc_position);
}

static void
M_ArenaRelease(M_Arena *arena)
{
 arena->callbacks.release_func(arena->base);
}

//~NOTE(tbt): temporary memory

static M_Temp
M_TempBegin(M_Arena *arena)
{
 M_Temp result =
 {
  .arena = arena,
  .checkpoint_alloc_position = arena->alloc_position,
 };
 return result;
}

static void
M_TempEnd(M_Temp *temp)
{
 M_ArenaPop(temp->arena, temp->arena->alloc_position - temp->checkpoint_alloc_position);
}


//~NOTE(tbt): scratch pool

static void
M_ScratchPoolMake(M_ScratchPool *pool, M_Callbacks mem_callbacks)
{
 for(int arena_index = 0;
     arena_index < ArrayCount(pool->arenas);
     arena_index += 1)
 {
  M_Arena *arena = &pool->arenas[arena_index];
  *arena = M_ArenaMake(mem_callbacks);
 }
}

static M_Temp
M_ScratchGet(M_ScratchPool *pool,
             void *non_conflict_array[],
             int non_conflict_count)
{
 M_Temp result = {0};
 
 M_Arena *scratch;
 for(size_t arena_index = 0;
     arena_index < ArrayCount(pool->arenas);
     arena_index += 1)
 {
  scratch = &pool->arenas[arena_index];
  Bool is_conflicting = False;
  for(size_t conflict_index = 0;
      conflict_index < non_conflict_count;
      conflict_index += 1)
  {
   void *non_conflict = non_conflict_array[conflict_index];
   if(non_conflict >= scratch->base &&
      non_conflict <= (uint8_t *)scratch->base + scratch->max - 1)
   {
    is_conflicting = True;
    break;
   }
  }
  if(!is_conflicting)
  {
   result = M_TempBegin(scratch);
   break;
  }
 }
 
 Assert(result.arena);
 return result;
}
