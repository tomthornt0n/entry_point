
//~NOTE(tbt): utilities

Function void *
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

Function void *
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

Function Bool
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

Function uintptr_t
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

Function M_Arena
M_ArenaMake(M_Hooks callbacks)
{
    M_Arena arena = {0};
    arena.callbacks = callbacks;
    arena.max = m_arena_default_cap;
    arena.base = arena.callbacks.reserve_func(arena.max);
    arena.alloc_position = 0;
    arena.commit_position = 0;
    return arena;
}

Function M_Arena
M_ArenaMakeSized(M_Hooks callbacks, size_t size)
{
    M_Arena arena = {0};
    arena.callbacks = callbacks;
    arena.max = size;
    arena.base = arena.callbacks.reserve_func(arena.max);
    arena.alloc_position = 0;
    arena.commit_position = 0;
    return arena;
}

Function M_Arena
M_ArenaMakeLocal(void *backing, size_t size)
{
    M_Arena arena = {0};
    arena.max = size;
    arena.alloc_position = 0;
    arena.base = backing;
    arena.commit_position = size;
    return arena;
}

Function M_Arena
M_ArenaMakeFixed(M_Hooks callbacks, size_t size)
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

Function void *
M_ArenaPushAligned(M_Arena *arena, size_t size, size_t align)
{
    void *memory = NULL;
    
    uintptr_t new_alloc_position = M_AlignForward(arena->alloc_position + size, align);
    if(arena->alloc_position + size < arena->max)
    {
        if(new_alloc_position > arena->commit_position)
        {
            size_t commit_size = size;
            commit_size += m_arena_commit_chunk_size - 1;
            commit_size -= commit_size % m_arena_commit_chunk_size;
            arena->callbacks.commit_func((unsigned char *)arena->base + arena->commit_position, commit_size);
            arena->commit_position += commit_size;
        }
        
        memory = (unsigned char *)arena->base + arena->alloc_position;
        arena->alloc_position = new_alloc_position;
    }
    
    if(NULL == memory)
    {
        AssertBreak_();
    }
    
    M_Set(memory, 0, size);
    return memory;
}

Function void *
M_ArenaPush(M_Arena *arena, size_t size)
{
    return M_ArenaPushAligned(arena, size, M_DefaultAlignment);
}

Function void
M_ArenaPop(M_Arena *arena, size_t size)
{
    size_t to_pop = Min1U(size, arena->alloc_position);
    arena->alloc_position -= to_pop;
}

Function void
M_ArenaClear(M_Arena *arena)
{
    M_ArenaPop(arena, arena->alloc_position);
}

Function void
M_ArenaDestroy(M_Arena *arena)
{
    M_Arena a = *arena;
    a.callbacks.decommit_func(a.base, a.commit_position);
    a.callbacks.release_func(a.base);
}

//~NOTE(tbt): temporary memory

Function M_Temp
M_TempBegin(M_Arena *arena)
{
    M_Temp result =
    {
        .arena = arena,
        .checkpoint_alloc_position = arena->alloc_position,
    };
    return result;
}

Function void
M_TempEnd(M_Temp *temp)
{
    M_ArenaPop(temp->arena, temp->arena->alloc_position - temp->checkpoint_alloc_position);
}


//~NOTE(tbt): scratch pool

Function void
M_ScratchPoolMake(M_ScratchPool *pool, M_Hooks mem_callbacks)
{
    for(int arena_index = 0;
        arena_index < ArrayCount(pool->arenas);
        arena_index += 1)
    {
        M_Arena *arena = &pool->arenas[arena_index];
        *arena = M_ArenaMake(mem_callbacks);
    }
}

Function M_Temp
M_ScratchGet(M_ScratchPool *pool,
             M_Arena *non_conflict_array[],
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
            M_Arena *non_conflict = non_conflict_array[conflict_index];
            if(non_conflict == scratch)
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
    
    Assert(NULL != result.arena);
    return result;
}

Function void
M_ScratchPoolDestroy(M_ScratchPool *pool)
{
    for(int arena_index = 0;
        arena_index < ArrayCount(pool->arenas);
        arena_index += 1)
    {
        M_Arena *arena = &pool->arenas[arena_index];
        M_ArenaDestroy(arena);
    }
}

