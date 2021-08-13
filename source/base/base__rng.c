
//~NOTE(tbt): 'noise' via hashing a position

static unsigned int
Noise1U(unsigned int a)
{
    unsigned int result = a;
    result ^= 2747636419;
    result *= 2654435769;
    result ^= result >> 16;
    result *= 2654435769;
    result ^= result >> 16;
    result *= 2654435769;
    return result;
}

static int
Noise2I(V2I a)
{
    int result;
    result = Noise1U(a.y);
    result = Noise1U(result + a.x);
    return result;
}

static float
Noise2F(V2F a)
{
    float result;
    
    int x_int = a.x;
    int y_int = a.y;
    float x_frac = a.x - x_int;
    float y_frac = a.y - y_int;
    int s = Noise2I(V2I(x_int, y_int));
    int t = Noise2I(V2I(x_int + 1, y_int));
    int u = Noise2I(V2I(x_int, y_int + 1));
    int v = Noise2I(V2I(x_int + 1, y_int + 1));
    float low = InterpolateSmooth1F(s, t, x_frac);
    float high = InterpolateSmooth1F(u, v, x_frac);
    result = InterpolateSmooth1F(low, high, y_frac);
    
    return result;
}

//~NOTE(tbt): pseudo-random sequence

static unsigned int rand_int_seed = 0;

static void
RandIntInit(int seed)
{
    rand_int_seed = seed;
}

static int
RandIntNextRaw(void)
{
    int result = Noise1U(rand_int_seed);
    rand_int_seed += 1;
    return result;
}

static int
RandIntNext(int min, int max)
{
    int result = RandIntNextRaw();
    result = WrapToBounds(result, min, max);
    return result;
}

//~NOTE(tbt): perlin noise

static float
Perlin2D(V2F a,
         float freq,
         int depth)
{
    float result;
    
    float xa = a.x * freq;
    float ya = a.y * freq;
    float amp = 1.0f;
    float fin = 0.0f;
    float div = 0.0f;
    for (int i = 0;
         i < depth;
         i += 1)
    {
        div += 256 * amp;
        fin += Noise2F(V2F(xa, ya)) * amp;
        amp /= 2.0f;
        xa *= 2;
        ya *= 2;
    }
    result = fin / div;
    
    return result;
}
