
//~NOTE(tbt): 'noise' via hashing a position

static unsigned int Noise1U(unsigned int a);
static int Noise2I(V2I a);
static float Noise2F(V2F a);

//~NOTE(tbt): pseudo-random sequence

static void RandIntInit(int seed);
static int RandIntNextRaw(void);
static int RandIntNext(int min, int max);

//~NOTE(tbt): perlin noise

static float Perlin2D(V2F a, float freq, int depth);
