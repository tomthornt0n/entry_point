
typedef void *W_Handle;

typedef enum
{
    W_CursorKind_Default, // NOTE(tbt): normal arrow
    W_CursorKind_HResize, // NOTE(tbt): double ended horizontal arrow
    W_CursorKind_VResize, // NOTE(tbt): double ended vertical arrow
    W_CursorKind_Hidden,  // NOTE(tbt): no cursor
    
    W_CursorKind_MAX,
} W_CursorKind;

typedef void W_DrawHook(W_Handle window);

Function Bool      W_Update  (W_Handle window);

Function V2I       W_DimensionsGet    (W_Handle window);
Function I2F       W_ClientRectGet    (W_Handle window);
Function V2F       W_MousePositionGet (W_Handle window);
Function Bool      W_KeyStateGet      (W_Handle window, I_Key key);
Function M_Arena  *W_FrameArenaGet    (W_Handle window);
Function EV_Queue *W_EventQueueGet    (W_Handle window);
Function void     *W_UserDataGet      (W_Handle window);

Function void      W_FullscreenSet           (W_Handle window, Bool is_fullscreen);
Function void      W_VSyncSet                (W_Handle window, Bool is_vsync);
Function void      W_CursorKindSet           (W_Handle window, W_CursorKind kind);
Function void      W_BlockToWaitForEventsSet (W_Handle window, Bool should_block_to_wait_for_events);
Function void      W_UserDataSet             (W_Handle window, void *data);
