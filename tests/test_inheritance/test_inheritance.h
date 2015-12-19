#ifndef _test_inheritance_H_
#define _test_inheritance_H_

class B
{
public:
  //B();
  //~B() {}

  int b();
};

class A
{
public:
  A() {};
  A(int toto);
  virtual ~A() {}

  int a();

  //virtual int c() = 0;

  int nv()
  {
    printf("A::nv()\n");
    return 0;
  }

  virtual int v()
  {
    printf("A::v()\n");
    return 0;
  }

  B* get_b();

  int toto;

private:
  B m_b;
};

class C : public A
{
public:
   C() {};
   C(int titi, int toto);
  //~C();

  int nv();

  //virtual int v() {};
  virtual int v();

  int titi;
};

class D : public C
{
public:
  //  D();
//~D();

  int nv();

  virtual int v();
};

#endif
