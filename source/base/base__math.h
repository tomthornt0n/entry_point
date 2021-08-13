
//~NOTE(tbt): single floats

static float Smoothstep1F(float t);
static float InterpolateLinear1F(float a, float b, float t);
static float InterpolateSmooth1F(float a, float b, float t);
static float Min1F(float a, float b);
static float Max1F(float a, float b);
static float Clamp1F(float a, float min, float max);
static float Abs1F(float a);
static float Sqrt1F(float a);
static float ReciprocalSqrt1F(float a);
static float Sin1F(float a);
static float Cos1F(float a);
static float Tan1F(float a);

//~NOTE(tbt): floating point vectors

typedef union
{
 struct
 {
  float x;
  float y;
 };
 float elements[2];
} Vector2F;
typedef Vector2F V2F;
#define V2F(...) ((V2F){ __VA_ARGS__ })

typedef union
{
 struct
 {
  float x;
  float y;
  float z;
 };
 float elements[3];
} Vector3F;
typedef Vector3F V3F;
#define V3F(...) ((V3F){ __VA_ARGS__ })

typedef union
{
 struct
 {
  float x;
  float y;
  float z;
  float w;
 };
 float elements[4];
} Vector4F;
typedef Vector4F V4F;
#define V4F(...) ((V4F){ __VA_ARGS__ })

static V4F Add4F(V4F a, V4F b);
static V4F Sub4F(V4F a, V4F b);
static V4F Mul4F(V4F a, V4F b);
static V4F Div4F(V4F a, V4F b);
static float Dot4F(V4F a, V4F b);
static V4F Scale4F(V4F a, float b);
static float LengthSquared4F(V4F a);
static float Length4F(V4F a);
static V4F Normalise4F(V4F a);

static V3F Add3F(V3F a, V3F b);
static V3F Sub3F(V3F a, V3F b);
static V3F Mul3F(V3F a, V3F b);
static V3F Div3F(V3F a, V3F b);
static float Dot3F(V3F a, V3F b);
static V3F Cross3F(V3F a, V3F b);
static V3F Scale3F(V3F a, float b);
static float LengthSquared3F(V3F a);
static float Length3F(V3F a);
static V3F Normalise3F(V3F a);

static V2F Add2F(V2F a, V2F b);
static V2F Sub2F(V2F a, V2F b);
static V2F Mul2F(V2F a, V2F b);
static V2F Div2F(V2F a, V2F b);
static float Dot2F(V2F a, V2F b);
static V2F Scale2F(V2F a, float b);
static float LengthSquared2F(V2F a);
static float Length2F(V2F a);
static V2F Normalise2F(V2F a);

//~NOTE(tbt): matrices

typedef struct
{
 float elements[4][4];
} Matrix4x4F;
typedef Matrix4x4F M4x4F;

static M4x4F InitialiseDiagonal4x4F(float diag);
static M4x4F Mul4x4F(M4x4F a, M4x4F b);
static M4x4F PerspectiveMake4x4f(float fov, float aspect_ratio, float near, float far);
static M4x4F OrthoMake4x4F(float left, float right, float top, float bottom, float near, float far);
static M4x4F LookAtMake4x4F(V3F eye, V3F centre, V3F up);
static M4x4F TranslationMake4x4F(V3F translation);
static M4x4F ScaleMake4x4F(V3F scale);
static V4F Transform4F(V4F a, M4x4F b);

//~NOTE(tbt): single integers

static int InterpolateLinear1I(int a, int b, unsigned char t);
static int Min1I(int a, int b);
static int Max1I(int a, int b);
static int Clamp1I(int a, int min, int max);
static int Abs1I(int a);

//~NOTE(tbt): integer vectors

typedef union
{
 struct
 {
  int x;
  int y;
 };
 int elements[2];
} Vector2I;
typedef Vector2I V2I;
#define V2I(...) ((V2I){ __VA_ARGS__ })

typedef union
{
 struct
 {
  int x;
  int y;
  int z;
 };
 int elements[3];
} Vector3I;
typedef Vector3I V3I;
#define V3I(...) ((V3I){ __VA_ARGS__ })

typedef union
{
 struct
 {
  int x;
  int y;
  int z;
  int w;
 };
 int elements[4];
} Vector4I;
typedef Vector4I V4I;
#define V4I(...) ((V4I){ __VA_ARGS__ })

static V4I Add4I(V4I a, V4I b);
static V4I Sub4I(V4I a, V4I b);
static V4I Mul4I(V4I a, V4I b);
static V4I Div4I(V4I a, V4I b);
static int Dot4I(V4I a, V4I b);

static V3I Add3I(V3I a, V3I b);
static V3I Sub3I(V3I a, V3I b);
static V3I Mul3I(V3I a, V3I b);
static V3I Div3I(V3I a, V3I b);
static int Dot3I(V3I a, V3I b);
static V3I Cross3I(V3I a, V3I b);

static V2I Add2I(V2I a, V2I b);
static V2I Sub2I(V2I a, V2I b);
static V2I Mul2I(V2I a, V2I b);
static V2I Div2I(V2I a, V2I b);
static int Dot2I(V2I a, V2I b);

//~NOTE(tbt): intervals

typedef union
{
 struct
 {
  float min;
  float max;
 };
 float elements[2];
} Interval1F;
typedef Interval1F I1F;
#define I1F(...) ((I1F){ __VA_ARGS__ })

typedef union
{
 struct
 {
  unsigned long long min;
  unsigned long long max;
 };
 unsigned long long elements[2];
} Interval1U;
typedef Interval1U I1U;
#define I1U(...) ((I1U){ __VA_ARGS__ })

typedef union
{
 struct
 {
  V2F min;
  V2F max;
 };
 struct
 {
  V2F p0;
  V2F p1;
 };
 struct
 {
  float x0;
  float y0;
  float x1;
  float y1;
 };
 V2F points[2];
 float elements[4];
} Interval2F;
typedef Interval2F I2F;
#define I2F(...) ((I2F){ __VA_ARGS__ })

static Bool Sat2F(I2F a, I2F b);
static I2F Intersection2F(I2F a, I2F b);
