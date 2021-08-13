
//~NOTE(tbt): single floats

static float
Smoothstep1F(float t)
{
 return t * t * (3 - 2 * t);
}

static float
InterpolateLinear1F(float a, float b,
                    float t) // NOTE(tbt): (0.0 <= t <= 1.0)
{
 return a + t * (b - 1);
}

static float
InterpolateSmooth1F(float a, float b,
                    float t) // NOTE(tbt): (0.0 <= t <= 1.0)
{
 return InterpolateLinear1F(a, b, Smoothstep1F(t));
}

static float
Min1F(float a, float b)
{
 if(a < b)
 {
  return a;
 }
 else
 {
  return b;
 }
}

static float
Max1F(float a, float b)
{
 if(a > b)
 {
  return a;
 }
 else
 {
  return b;
 }
}

static float
Clamp1F(float a,
        float min,
        float max)
{
 return (Max1F(min, Min1F(max, a)));
}

static float
Abs1F(float a)
{
 union { float f; unsigned int u; } u_from_f;
 u_from_f.f = a;
 u_from_f.u &= ~(1 << 31);
 return u_from_f.f;
}

static float
Sqrt1F(float a)
{
 float result;
#if UseSSE2 // NOTE(tbt): only needs SSE really
 __m128 _a;
 _a = _mm_load_ss(&a);
 _a = _mm_sqrt_ss(_a);
 _mm_store_ss(&result, _a);
#else
 int accuracy = 10; // NOTE(tbt): tune lower for speed, higher for accuracy
 
 float guess = 1.0f;
 for(int i = 0; i < accuracy; i += 1)
 {
  guess -= (guess * guess - a) / (2 * guess);
 }
 result = guess
#endif
 return result;
}

static float
ReciprocalSqrt1F(float a)
{
 union { float f; long i; } i_from_f;
 
 i_from_f.f = a;
 i_from_f.i = 0x5f375a86 - (i_from_f.i >> 1);
 i_from_f.f *= 1.5f - (i_from_f.f * 0.5f * i_from_f.f * i_from_f.f);
 
 return i_from_f.f;
}

// NOTE(tbt): not very fast or accurate
static float
Sin1F(float a)
{
 // NOTE(tbt): range reduction
 int k = a * (1.0f / (2 * Pi));
 a = a - k * (2 * Pi);
 a = Min1F(a, Pi - a);
 a = Max1F(a, -Pi - a);
 a = Min1F(a, Pi - a);
 
 float result = 0.0f;
 
 float a1 = a;
 float a2 = a1 * a1;
 float a4 = a2 * a2;
 float a5 = a1 * a4;
 float a9 = a4 * a5;
 float a13 = a9 * a4;
 
 float term_1 = a1  * (1.0f - a2 /   6.0f);
 float term_2 = a5  * (1.0f - a2 /  42.0f) / 120.0f;
 float term_3 = a9  * (1.0f - a2 / 110.0f) / 362880.0f;
 float term_4 = a13 * (1.0f - a2 / 225.0f) / 6227020800.0f;
 
 result += term_4;
 result += term_3;
 result += term_2;
 result += term_1;
 
 return result;
}

static float
Cos1F(float a)
{
 return Sin1F(a + 0.5 * Pi);
}

static float
Tan1F(float a)
{
 return Sin1F(a) / Cos1F(a);
}

//~NOTE(tbt): float vectors

static V4F
Add4F(V4F a, V4F b)
{
 V4F result;
#if UseSSE2
 __m128 _a = _mm_load_ps(a.elements);
 __m128 _b = _mm_load_ps(b.elements);
 _a = _mm_add_ps(_a, _b);
 _mm_store_ps(result.elements, _a);
#else
 result = a;
 result.x += b.x;
 result.y += b.y;
 result.z += b.z;
 result.w += b.w;
#endif
 return result;
}

static V4F
Sub4F(V4F a, V4F b)
{
 V4F result;
#if UseSSE2
 __m128 _a = _mm_load_ps(a.elements);
 __m128 _b = _mm_load_ps(b.elements);
 _a = _mm_sub_ps(_a, _b);
 _mm_store_ps(result.elements, _a);
#else
 result = a;
 result.x -= b.x;
 result.y -= b.y;
 result.z -= b.z;
 result.w -= b.w;
#endif
 return result;
}

static V4F
Mul4F(V4F a, V4F b)
{
 V4F result;
#if UseSSE2
 __m128 _a = _mm_load_ps(a.elements);
 __m128 _b = _mm_load_ps(b.elements);
 _a = _mm_mul_ps(_a, _b);
 _mm_store_ps(result.elements, _a);
#else
 result = a;
 result.x += b.x;
 result.y += b.y;
 result.z += b.z;
 result.w += b.w;
#endif
 return result;
}

static V4F
Div4F(V4F a, V4F b)
{
 V4F result;
#if UseSSE2
 __m128 _a = _mm_load_ps(a.elements);
 __m128 _b = _mm_load_ps(b.elements);
 _a = _mm_add_ps(_a, _b);
 _mm_store_ps(result.elements, _a);
#else
 result = a;
 result.x += b.x;
 result.y += b.y;
 result.z += b.z;
 result.w += b.w;
#endif
 return result;
}

static float
Dot4F(V4F a, V4F b)
{
 float result;
#if UseSSE3
 __m128 _a = _mm_load_ps(a.elements);
 __m128 _b = _mm_load_ps(b.elements);
 __m128 mul_res = _mm_mul_ps(_a, _b);
 __m128 shuf_reg = _mm_movehdup_ps(mul_res);
 __m128 sums_reg = _mm_add_ps(mul_res, shuf_reg);
 shuf_reg = _mm_movehl_ps(shuf_reg, sums_reg);
 sums_reg = _mm_add_ss(sums_reg, shuf_reg);
 result = _mm_cvtss_f32(sums_reg);
#else
 result = (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
#endif
 return result;
}

static V4F
Scale4F(V4F a, float b)
{
 V4F _b = { b, b, b, b };
 return Mul4F(a, _b);
}

static float
LengthSquared4F(V4F a)
{
 return a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w;
}

static float
Length4F(V4F a)
{
 return Sqrt1F(LengthSquared4F(a));
}

static V4F
Normalise4F(V4F a)
{
 float one_over_length = ReciprocalSqrt1F(LengthSquared4F(a));
 return Scale4F(a, one_over_length);
}

//-

static V3F
Add3F(V3F a, V3F b)
{
 V3F result;
 
 V4F _a = { a.x, a.y, a.z };
 V4F _b = { b.x, b.y, b.z };
 _a = Add4F(_a, _b);
 result.x = _a.x;
 result.y = _a.y;
 result.z = _a.z;
 
 return result;
}

static V3F
Sub3F(V3F a, V3F b)
{
 V3F result;
 
 V4F _a = { a.x, a.y, a.z };
 V4F _b = { b.x, b.y, b.z };
 _a = Sub4F(_a, _b);
 result.x = _a.x;
 result.y = _a.y;
 result.z = _a.z;
 
 return result;
}

static V3F
Mul3F(V3F a, V3F b)
{
 V3F result;
 
 V4F _a = { a.x, a.y, a.z };
 V4F _b = { b.x, b.y, b.z };
 _a = Mul4F(_a, _b);
 result.x = _a.x;
 result.y = _a.y;
 result.z = _a.z;
 
 return result;
}

static V3F
Div3F(V3F a, V3F b)
{
 V3F result;
 
 V4F _a = { a.x, a.y, a.z };
 V4F _b = { b.x, b.y, b.z };
 _a = Div4F(_a, _b);
 result.x = _a.x;
 result.y = _a.y;
 result.z = _a.z;
 
 return result;
}

static float
Dot3F(V3F a, V3F b)
{
 V4F _a = { a.x, a.y, a.z };
 V4F _b = { b.x, b.y, b.z };
 return Dot4F(_a, _b);
}

static V3F
Cross3F(V3F a, V3F b)
{
 V3F result =
 {
  .x = a.y * b.z - a.z * b.y,
  .y = a.z * b.x - a.x * b.z,
  .z = a.x * b.y - a.y * b.x,
 };
 return result;
}

static V3F
Scale3F(V3F a, float b)
{
 V3F _b = { b, b, b };
 return Mul3F(a, _b);
}

static float
LengthSquared3F(V3F a)
{
 return a.x * a.x + a.y * a.y + a.z * a.z;
}

static float
Length3F(V3F a)
{
 return Sqrt1F(LengthSquared3F(a));
}

static V3F
Normalise3F(V3F a)
{
 float one_over_length = ReciprocalSqrt1F(LengthSquared3F(a));
 return Scale3F(a, one_over_length);
}

//-

static V2F
Add2F(V2F a, V2F b)
{
 V2F result;
 
 V4F _a = { a.x, a.y };
 V4F _b = { b.x, b.y };
 _a = Add4F(_a, _b);
 result.x = _a.x;
 result.y = _a.y;
 
 return result;
}

static V2F
Sub2F(V2F a, V2F b)
{
 V2F result;
 
 V4F _a = { a.x, a.y };
 V4F _b = { b.x, b.y };
 _a = Sub4F(_a, _b);
 result.x = _a.x;
 result.y = _a.y;
 
 return result;
}

static V2F
Mul2F(V2F a, V2F b)
{
 V2F result;
 
 V4F _a = { a.x, a.y };
 V4F _b = { b.x, b.y };
 _a = Mul4F(_a, _b);
 result.x = _a.x;
 result.y = _a.y;
 
 return result;
}

static V2F
Div2F(V2F a, V2F b)
{
 V2F result;
 
 V4F _a = { a.x, a.y };
 V4F _b = { b.x, b.y };
 _a = Div4F(_a, _b);
 result.x = _a.x;
 result.y = _a.y;
 
 return result;
}

static float
Dot2F(V2F a, V2F b)
{
 V4F _a = { a.x, a.y };
 V4F _b = { b.x, b.y };
 return Dot4F(_a, _b);
}

static V2F
Scale2F(V2F a, float b)
{
 V2F _b = { b, b };
 return Mul2F(a, _b);
}

static float
LengthSquared2F(V2F a)
{
 return a.x * a.x + a.y * a.y;
}

static float
Length2F(V2F a)
{
 return Sqrt1F(LengthSquared2F(a));
}

static V2F
Normalise2F(V2F a)
{
 float one_over_length = ReciprocalSqrt1F(LengthSquared2F(a));
 return Scale2F(a, one_over_length);
}

//~NOTE(tbt): matrices

static M4x4F
InitialiseDiagonal4x4F(float diag)
{
 M4x4F result = 
 {
  {
   { diag, 0.0f, 0.0f, 0.0f },
   { 0.0f, diag, 0.0f, 0.0f },
   { 0.0f, 0.0f, diag, 0.0f },
   { 0.0f, 0.0f, 0.0f, diag },
  },
 };
 return result;
}

#if UseSSE2
typedef struct
{
 __m128 rows[4];
} SSE_M4x4F;

static SSE_M4x4F
SSEM4x4FFromM4x4F_(M4x4F a)
{
 SSE_M4x4F result;
 result.rows[0] = _mm_load_ps(&a.elements[0][0]);
 result.rows[1] = _mm_load_ps(&a.elements[1][0]);
 result.rows[2] = _mm_load_ps(&a.elements[2][0]);
 result.rows[3] = _mm_load_ps(&a.elements[3][0]);
 return result;
}

static M4x4F
M4x4FFromSSEM4x4F_(SSE_M4x4F a)
{
 M4x4F result;
 _mm_store_ps(&result.elements[0][0], a.rows[0]);
 _mm_store_ps(&result.elements[1][0], a.rows[1]);
 _mm_store_ps(&result.elements[2][0], a.rows[2]);
 _mm_store_ps(&result.elements[3][0], a.rows[3]);
 return result;
}

static __m128
LinearCombine4x4F_(const __m128 *a,
                   const SSE_M4x4F *b)
{
 __m128 result;
 result = _mm_mul_ps(_mm_shuffle_ps(*a, *a, 0x00), b->rows[0]);
 result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(*a, *a, 0x55), b->rows[1]));
 result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(*a, *a, 0x55), b->rows[2]));
 result = _mm_add_ps(result, _mm_mul_ps(_mm_shuffle_ps(*a, *a, 0x55), b->rows[3]));
 return result;
}
#endif

static M4x4F
Mul4x4F(M4x4F a, M4x4F b)
{
 M4x4F result = {0};
#if UseSSE2
 SSE_M4x4F _result;
 SSE_M4x4F _a = SSEM4x4FFromM4x4F_(a);
 SSE_M4x4F _b = SSEM4x4FFromM4x4F_(b);
 _result.rows[0] = LinearCombine4x4F_(&_a.rows[0], &_b);
 _result.rows[1] = LinearCombine4x4F_(&_a.rows[1], &_b);
 _result.rows[2] = LinearCombine4x4F_(&_a.rows[2], &_b);
 _result.rows[3] = LinearCombine4x4F_(&_a.rows[3], &_b);
 result = M4x4FFromSSEM4x4F_(_result);
#else
 for(int j = 0; j < 4; ++j)
 {
  for(int i = 0; i < 4; ++i)
  {
   result.elements[i][j] = (a.elements[0][j] * b.elements[i][0] +
                            a.elements[1][j] * b.elements[i][1] +
                            a.elements[2][j] * b.elements[i][2] +
                            a.elements[3][j] * b.elements[i][3]);
  }
 }
#endif
 return result;
}

static M4x4F
PerspectiveMake4x4f(float fov,
                    float aspect_ratio,
                    float near, float far)
{
 M4x4F result = {0};
 float tan_half_theta = Tan1F(fov * (Pi / 360.0f));
 result.elements[0][0] = 1.0f / tan_half_theta;
 result.elements[1][1] = aspect_ratio / tan_half_theta;
 result.elements[2][3] = -1.0f;
 result.elements[2][2] = (near + far) / (near - far);
 result.elements[3][2] = (2.0f * near * far) / (near - far);
 result.elements[3][3] = 0.0f;
 return result;
}

static M4x4F
OrthoMake4x4F(float left, float right,
              float top, float bottom,
              float near, float far)
{
 M4x4F result = {0};
 result.elements[0][0]  = +2.0f / (right - left);
 result.elements[1][1]  = +2.0f / (top - bottom);
 result.elements[2][2]  = -2.0f / (far - near);
 result.elements[3][3]  = 1.0f;
 result.elements[0][3] = -((right + left) / (right - left));
 result.elements[1][3] = -((top + bottom) / (top - bottom));
 result.elements[2][3] = -((far + near) / (far - near));
 return result;
}

static M4x4F
LookAtMake4x4F(V3F eye, V3F centre, V3F up)
{
 M4x4F result;
 
 V3F f = Normalise3F(Sub3F(centre, eye));
 V3F s = Normalise3F(Cross3F(f, up));
 V3F u = Cross3F(s, f);
 
 result.elements[0][0] = s.x;
 result.elements[0][1] = u.x;
 result.elements[0][2] = -f.x;
 result.elements[0][3] = 0.0f;
 
 result.elements[1][0] = s.y;
 result.elements[1][1] = u.y;
 result.elements[1][2] = -f.y;
 result.elements[1][3] = 0.0f;
 
 result.elements[2][0] = s.z;
 result.elements[2][1] = u.z;
 result.elements[2][2] = -f.z;
 result.elements[2][3] = 0.0f;
 
 result.elements[3][0] = -Dot3F(s, eye);
 result.elements[3][1] = -Dot3F(u, eye);
 result.elements[3][2] = Dot3F(f, eye);
 result.elements[3][3] = 1.0f;
 
 return result;
}

static M4x4F
TranslationMake4x4F(V3F translation)
{
 M4x4F result = InitialiseDiagonal4x4F(1.0f);
 result.elements[3][0] = translation.x;
 result.elements[3][1] = translation.y;
 result.elements[3][2] = translation.z;
 return result;
}

static M4x4F
ScaleMake4x4F(V3F scale)
{
 M4x4F result = InitialiseDiagonal4x4F(1.0f);
 result.elements[0][0] = scale.x;
 result.elements[1][1] = scale.y;
 result.elements[2][2] = scale.z;
 return result;
}

static V4F
Transform4F(V4F a, M4x4F b)
{
 // TODO(tbt): SSE
 V4F result =
 {
  .x = b.elements[0][0] * a.x + b.elements[0][1] * a.y + b.elements[0][2] * a.z + b.elements[0][3] * a.w,
  .y = b.elements[1][0] * a.x + b.elements[1][1] * a.y + b.elements[1][2] * a.z + b.elements[1][3] * a.w,
  .z = b.elements[2][0] * a.x + b.elements[2][1] * a.y + b.elements[2][2] * a.z + b.elements[2][3] * a.w,
  .w = b.elements[3][0] * a.x + b.elements[3][1] * a.y + b.elements[3][2] * a.z + b.elements[3][3] * a.w,
 };
 return result;
}

//~NOTE(tbt): single integers

static int
InterpolateLinear1I(int a, int b,
                    unsigned char t) // NOTE(tbt): (0 <= t <= 255)
{
 return ((t * (b - a)) >> 8) + a;
}

static int
Min1I(int a, int b)
{
 if(a < b)
 {
  return a;
 }
 else
 {
  return b;
 }
}

static int
Max1I(int a, int b)
{
 if(a > b)
 {
  return a;
 }
 else
 {
  return b;
 }
}

static int
Clamp1I(int a, int min, int max)
{
 return Max1I(min, Min1I(max, a));
}

static int
Abs1I(int a)
{
 return (a < 0) ? -a : a;
}

//~NOTE(tbt): integer vectors

static V4I
Add4I(V4I a, V4I b)
{
 a.x += b.x;
 a.y += b.y;
 a.z += b.z;
 a.w += b.w;
 return a;
}

static V4I
Sub4I(V4I a, V4I b)
{
 a.x -= b.x;
 a.y -= b.y;
 a.z -= b.z;
 a.w -= b.w;
 return a;
}

static V4I
Mul4I(V4I a, V4I b)
{
 a.x *= b.x;
 a.y *= b.y;
 a.z *= b.z;
 a.w *= b.w;
 return a;
}

static V4I
Div4I(V4I a, V4I b)
{
 a.x /= b.x;
 a.y /= b.y;
 a.z /= b.z;
 a.w /= b.w;
 return a;
}

static int
Dot4I(V4I a, V4I b)
{
 int result = 0;
 result += a.x * b.x;
 result += a.y * b.y;
 result += a.z * b.z;
 result += a.w * b.w;
 return result;
}

//-

static V3I
Add3I(V3I a, V3I b)
{
 a.x += b.x;
 a.y += b.y;
 a.z += b.z;
 return a;
}

static V3I
Sub3I(V3I a, V3I b)
{
 a.x -= b.x;
 a.y -= b.y;
 a.z -= b.z;
 return a;
}

static V3I
Mul3I(V3I a, V3I b)
{
 a.x *= b.x;
 a.y *= b.y;
 a.z *= b.z;
 return a;
}

static V3I
Div3I(V3I a, V3I b)
{
 a.x /= b.x;
 a.y /= b.y;
 a.z /= b.z;
 return a;
}

static int
Dot3I(V3I a, V3I b)
{
 int result = 0;
 result += a.x * b.x;
 result += a.y * b.y;
 result += a.z * b.z;
 return result;
}

static V3I
Cross3i(V3I a, V3I b)
{
 V3I result =
 {
  .x = a.y * b.z - a.z * b.y,
  .y = a.z * b.x - a.x * b.z,
  .z = a.x * b.y - a.y * b.x,
 };
 return result;
}

//-

static V2I
Add2I(V2I a, V2I b)
{
 a.x += b.x;
 a.y += b.y;
 return a;
}

static V2I
Sub2I(V2I a, V2I b)
{
 a.x -= b.x;
 a.y -= b.y;
 return a;
}

static V2I
Mul2I(V2I a, V2I b)
{
 a.x *= b.x;
 a.y *= b.y;
 return a;
}

static V2I
Div2I(V2I a, V2I b)
{
 a.x /= b.x;
 a.y /= b.y;
 return a;
}

static int
Dot2I(V2I a, V2I b)
{
 int result = 0;
 result += a.x * b.x;
 result += a.y * b.y;
 return result;
}

//~NOTE(tbt): intervals

static Bool
Sat2F(I2F a, I2F b)
{
 if(a.max.x < b.min.x || a.min.x > b.max.x) { return False; }
 if(a.max.y < b.min.y || a.min.y > b.max.y) { return False; }
 return True;
}

static I2F
Intersection2F(I2F a, I2F b)
{
 I2F result;
 result.min.x = Max1F(a.min.x, b.min.x);
 result.min.y = Max1F(a.min.y, b.min.y);
 result.max.x = Min1F(a.max.x, b.max.x);
 result.max.y = Min1F(a.max.y, b.max.y);
 return result;
}
