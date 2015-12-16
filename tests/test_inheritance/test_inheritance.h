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

  virtual int c() = 0;

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

  virtual int c() {};
  int titi;
};


class D : public C
{
public:
  //  D();
//~D();

  virtual int c();
};

#endif
