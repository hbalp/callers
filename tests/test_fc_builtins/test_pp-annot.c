
#define N 12

int array[ N ];

/*@ ensures 0 <= \result < N; */
int get_index(void);

void main()
{
  int i = get_index();
  array[ i ] = 3;
}

#ifndef FRAMA_C
int get_index(void)
{
  int i = 7;
  return i;
}
#endif
