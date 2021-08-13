static int
OS_EntropyGet(void)
{
    int result;
    HCRYPTPROV ctx = 0;
    CryptAcquireContextW(&ctx, NULL, NULL, PROV_DSS, CRYPT_VERIFYCONTEXT);
    CryptGenRandom(ctx, sizeof(result), (BYTE *)&result);
    CryptReleaseContext(ctx, 0);
    return result;
}