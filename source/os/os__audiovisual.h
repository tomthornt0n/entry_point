
//~NOTE(tbt): misc

#if !defined(OS_DefaultWindowW)
# define OS_DefaultWindowW 640
#endif
#if !defined(OS_DefaultWindowH)
# define OS_DefaultWindowH 480
#endif

typedef enum
{
    OS_CursorKind_Default, // NOTE(tbt): normal arrow
    OS_CursorKind_HResize, // NOTE(tbt): double ended horizontal arrow
    OS_CursorKind_VResize, // NOTE(tbt): double ended vertical arrow
    OS_CursorKind_Hidden,  // NOTE(tbt): no cursor
    
    OS_CursorKind_MAX,
} OS_CursorKind;

typedef struct OS_AV_FrameState OS_AV_FrameState;

//~NOTE(tbt): events

// TODO(tbt): game pad input

typedef enum
{
    OS_EventKind_Key,
    OS_EventKind_Char,
    OS_EventKind_MouseMove,
    OS_EventKind_MouseScroll,
    OS_EventKind_WindowSize,
    OS_EventKind_MAX,
} OS_EventKind;

typedef struct
{
    OS_EventKind kind;
    KeyModifiers modifiers;
    Key key;
    Bool is_down;
    Bool is_repeat;
    V2F delta;
    V2I size;
    V2I position;
    unsigned int codepoint;
} OS_Event;

static void OS_EventPush(OS_AV_FrameState *ctx, OS_Event *e);

//~NOTE(tbt): utility functions

static V2I OS_AV_MousePositionGet(OS_AV_FrameState *ctx);
static V2I OS_AV_WindowDimensionsGet(OS_AV_FrameState *ctx);

//~NOTE(tbt): main API

typedef struct OS_AV_FrameState
{
    // NOTE(tbt): memory
    M_Arena frame_arena;                  // NOTE(tbt): arena which is automatically cleared at the start of each frame
    
    // NOTE(tbt): input
    OS_Event events[4096];                // NOTE(tbt): event queue
    size_t events_count;                  // NOTE(tbt): number of events in the queue
    Bool is_key_down[Key_MAX];            // NOTE(tbt): automatically process events and keep track of which keys are down for easy polling
    V2I mouse_position;                   // NOTE(tbt): can be written to to set the mouse position
    V2F mouse_wheel_delta;                // NOTE(tbt): the number of clicks the mouse wheel has moved since the last frame
    
    // NOTE(tbt): config
    OS_CursorKind cursor_kind;            // NOTE(tbt): set the style of the cursor
    float target_fps;                     // NOTE(tbt): try to enforce this frame rate
    Bool is_fullscreen;                   // NOTE(tbt): set fullscreen
    Bool should_block_to_wait_for_events; // NOTE(tbt): when true, the main loop sleeps to wait for messages from the OS
    Bool is_running;                      // NOTE(tbt): set to false to close the window
} OS_AV_FrameState;

static Bool OS_AV_ContextMake(OS_AV_FrameState **result, S8 window_title);
static Bool OS_AV_FrameBegin(OS_AV_FrameState *ctx);
static void OS_AV_FrameEnd(OS_AV_FrameState *ctx);
static void OS_AV_ContextDestroy(OS_AV_FrameState *ctx);
