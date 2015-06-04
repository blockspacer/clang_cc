
struct Domo
{

};
template <class T, class U>
struct templ {
T val;
U foo (T sd)
{
  return sd;
}
};
template <class T>
struct templ<T*,Domo&>
{
  T VAL;
};
template<class U>
struct templ<U*,int>
{
    U m_Val;
    U foo(int sd)
    {
        return m_Val;
    }

};
template <>
struct templ<int,char>
{
    char char_val;
    char foo(int sd)
    {
        char_val = sd;
        return char_val;
    }
    char baz()
    {
        return 'c';
    }
    char baz2(int a, int b)
    {
        return 'd';
    }
};

int zoo()
{
    templ<float,double> zoof;
    zoof.foo(5);

    templ<Domo*,int> doof;
    doof.foo('c');
    int ss;
    templ<int, char> coof;
    coof.foo(65);
    coof.char_val =45;
    doof.foo(34);


    return 0;



}
