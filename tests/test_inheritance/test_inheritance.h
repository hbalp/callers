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
  // A();
  virtual ~A() {}

  int a();

  virtual int c() = 0;

  B* get_b();

private:
  B m_b;
};


class C : public A
{
public:
//C();
//~C();

  virtual int c() {};
};


class D : public C
{
public:
  //  D();
//~D();

  virtual int c();
};

#endif
