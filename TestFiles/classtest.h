namespace morenamespace {
template<class T>
class IntrusiveRefCntPtr
{
  public:
    IntrusiveRefCntPtr(){}

    template <typename Z>
    IntrusiveRefCntPtr(Z ff){}
    ~IntrusiveRefCntPtr();
    template <typename Tur, int a>
    void foo(Tur sd){}

    T* getPtr() {return t;}
    T* t;
};


template <class T> struct simplify_type{};
template<class T> struct simplify_type<const IntrusiveRefCntPtr<T>>
{
    typedef T* SimpleType;
    static SimpleType getSimplifiedValue(const IntrusiveRefCntPtr<T>& Val)
    {
        return Val.getPtr();
    }
};
}
typedef int sayi,sayi2;
struct {int x;} a,b;
namespace bohu {

/// This is a slash comment
class Deneme :public morenamespace::IntrusiveRefCntPtr<Deneme>
{
  public:
    //! This is an exclamation comment
    class Deneme2{
      public:
        int zztop;
    };
    /**
    *   This is a javadoc comment
    */
    enum Colour {RED, /**< The colour red. */
                 YELLOW //!< The colour yellow
                 };
	/*! \brief This is a QT comment.
	*
	* with detailed description for a func.
	*/
    void func(int a,int b);
	/// a virtual function
	virtual int virtul(Deneme n); 	const int public_member = 501;
	virtual int virtul(float n,int z);
	void ConstFunction() const;
	void VolatileFunction() volatile;
	void ConstVolatileFunction() const volatile;
	virtual void sil4();
	void sil5();
   	void functionWithAVeryLongNameToTestListWindowWidth(const float& withLongVariableName);
	Deneme() {};
	//~Deneme() {};
	Deneme(const Deneme& rhs) = default;
	Deneme & operator= (const Deneme& rhs) = delete;
	operator bool() {return true;}

  private:
    typedef Deneme2 silme;
    morenamespace::IntrusiveRefCntPtr<int> m_ptr;
    mutable int member1;
	int member3 :6;
protected:
    float member2;
};

extern "C" {
    struct df
    {
        int a;
        float b;
    };
}
extern "C" void cfunc();
template<typename T,typename U>
class Burrito
{
public:
    U convert(T t) {return static_cast<U>(t); }
};
template<typename U>
class Burrito<U,float>
{
   float convert(U t){return t;}
};

}

