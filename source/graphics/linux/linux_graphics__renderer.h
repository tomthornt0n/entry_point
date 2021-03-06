
typedef struct LINUX_Sprite LINUX_Sprite;
struct LINUX_Sprite
{
    R_Sprite parent;
    LINUX_Sprite *free_list_next;
    uint32_t id;
};

Global LINUX_Sprite linux_sprite_nil =
{
    .parent.dimensions = { 1, 1 },
    .free_list_next = &linux_sprite_nil,
};

typedef struct
{
    Semaphore lock;
    M_Arena arena;
    LINUX_Sprite *free_list;
} LINUX_SpriteAllocator;
Global LINUX_SpriteAllocator linux_sprite_allocator = {0};

Function LINUX_Sprite *LINUX_SpriteAllocate (void);
Function void          LINUX_SpriteFree     (LINUX_Sprite *sprite);

typedef struct
{
    Bool is_initialised;
    uint32_t default_shader;
    int projection_matrix_location;
    uint32_t vao;
    uint32_t vbo;
    uint32_t ibo;
} LINUX_RendererData;
Global LINUX_RendererData linux_r_data = {0};

typedef struct
{
    R_Vertex vertices[R_Batch_MaxVertices];
    size_t vertices_count;
    
    GLuint indices[R_Batch_MaxIndices];
    size_t indices_count;
    
    LINUX_Sprite *texture;
    uint32_t shader;
    I2F mask;
    
    LINUX_Window *window;
    
    Bool should_flush;
} LINUX_RenderBatch;

Function void LINUX_RenderBatchQuadPush (LINUX_RenderBatch *batch, I2F rect, I2F uv, V4F colour);
Function void LINUX_RenderBatchArcPush  (LINUX_RenderBatch *batch, V2F pos, float r, I1F segment, V4F colour);
Function void LINUX_RenderBatchFlush    (LINUX_RenderBatch *batch);
