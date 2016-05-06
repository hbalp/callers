
#if 1
#define assert Frama_C_assert
#else
void assert(int val)
{
  if(val == 0)
    abort();
}
#endif

int main()
{
  //int res = 0;
  int res = 2;
  if(res == 0)
  {
    assert(0);
  }
  res = 4;
  return res;
}
