float verifyValue(float value)
{
    if (value > 1.5f)
    {
        return -1.5f;
    }
    else if (value < -1.5f)
    {
        return 1.5f;
    }
    return value;
}
