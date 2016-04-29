// file library.h
class ParentCommon;
class Common {
  public:
   Common(int x)
      :  m_x(x)
#ifdef NEED_PARENT
         , m_parent(0)
#endif
      {}
   int getX() const;

  private:
#ifdef NEED_PARENT
   ParentCommon* m_parent;
   friend class ParentCommon;
#endif
   int m_x;
};

class ParentCommon {
  private:
   Common* m_content;

  public:
   ParentCommon(int val)
      : m_content(new Common(val))
#ifdef NEED_PARENT
      { m_content->m_parent = this; }
#else
      {}
#endif
   int getX() const { return m_content->getX(); }
};
