static void
OS_EventPush(OS_AV_FrameState *ctx, OS_Event * e)
{
    if(ctx->events_count < ArrayCount(ctx->events))
    {
        M_Copy(&ctx->events[ctx->events_count], e, sizeof(*e));
        ctx->events_count += 1;
    }
}
