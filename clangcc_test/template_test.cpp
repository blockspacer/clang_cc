#include <string>


struct Holdem {
template <typename T1, typename T2>
struct apply {
    typedef std::string type;
    T1 value1;
    T2 value2;
};
};

namespace fg {
template<typename MetaFun, typename T1, typename T2>
struct apply2 {
    typedef typename MetaFun::template apply<T1, T2>::type type;
};
}


void applyThem()
{
    fg::apply2<Holdem,int,int> foo;


}

template <typename T> struct A {
    template <typename T2> struct F {};
};

template <typename T> struct B : A<T>::template F<T> {};

B<int> booo;

namespace bozo = std;
namespace tuto = bozo;

tuto::string text = "No";

