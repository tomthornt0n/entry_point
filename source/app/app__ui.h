
// NOTE(tbt): not a typical UI toolkit - just a set of fundamentals to perhaps make implementing your own
//            UI stuff a bit simpler

// NOTE(tbt): a lot of the functions in this file rely on G_CurrentWindowGet() so may not work correctly if you
//            don't use G_MainLoop()

//~NOTE(tbt): rect-cut layouts

typedef enum
{
    UI_CutDir_Left,
    UI_CutDir_Right,
    UI_CutDir_Top,
    UI_CutDir_Bottom,
} UI_CutDir;

Function I2F UI_CutLeft   (I2F *rect, float f);
Function I2F UI_CutRight  (I2F *rect, float f);
Function I2F UI_CutTop    (I2F *rect, float f);
Function I2F UI_CutBottom (I2F *rect, float f);
Function I2F UI_Cut       (I2F *rect, UI_CutDir dir, float f);

Function I2F UI_GetLeft   (I2F rect, float f);
Function I2F UI_GetRight  (I2F rect, float f);
Function I2F UI_GetTop    (I2F rect, float f);
Function I2F UI_GetBottom (I2F rect, float f);
Function I2F UI_Get       (I2F rect, UI_CutDir dir, float f);

//~NOTE(tbt): scrollable rows layouts

typedef struct
{
    I2F *rect;
    float scroll;
    float target_scroll;
    float height_per_row;
    size_t rows_count;
} UI_Scrollable;

Function void UI_ScrollableBegin (UI_Scrollable *state, I2F *rect, float row_height);
Function Bool UI_ScrollableRow   (UI_Scrollable *state, I2F *result);

//~NOTE(tbt): text edits

// TODO(tbt): these are single line only at the moment

Function Bool UI_EditText        (char buffer[], size_t cap, I1U *selection, size_t *len);
Function void UI_DrawS8WithCaret (R_Font *font, S8 string, V2F position, V4F colour, I1U *selection);
