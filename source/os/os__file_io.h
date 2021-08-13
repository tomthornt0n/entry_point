typedef uint32_t OS_F_PropertiesFlags;
typedef enum
{
    OS_F_PropertiesFlags_Exists      = Bit(0),
    OS_F_PropertiesFlags_IsDirectory = Bit(1),
    OS_F_PropertiesFlags_Hidden      = Bit(2),
} OS_F_PropertiesFlags_ENUM;

typedef struct
{
    OS_F_PropertiesFlags flags;
    size_t size;
    DataAccessFlags access_flags;
    T_DenseTime creation_time;
    T_DenseTime access_time;
    T_DenseTime write_time;
} OS_F_Properties;

static S8 OS_F_ReadEntire(M_Arena *arena, S8 filename);
static Bool OS_WriteEntire(S8 filename, S8 data);

static OS_F_Properties OS_F_PropertiesGet(S8 filename);

static Bool OS_F_Delete(S8 filename);                // NOTE(tbt): deletes a single file
static Bool OS_F_Move(S8 filename, S8 new_filename); // NOTE(tbt): renames or moves a file or directory
static Bool OS_F_DirectoryMake(S8 filename);         // NOTE(tbt): creates an empty directory
static Bool OS_F_DirectoryDelete(S8 filename);       // NOTE(tbt): deletes an empty directory

typedef struct
{
    S8 current_name;
    OS_F_Properties current_properties;
} OS_F_Iterator;

static OS_F_Iterator *OS_F_IteratorMake(M_Arena *arena, S8 filename);
static Bool OS_F_IteratorNext(M_Arena *arena, OS_F_Iterator *iter);
static void OS_F_IteratorEnd(OS_F_Iterator *iter);
