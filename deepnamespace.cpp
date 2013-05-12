
template <typename T>
class Dur
{
public:
    template <typename U>
    T convert(U u)
    {
        return u;
    }
    int more() {return 0;}
};
int func()
{
    Dur<int> fugieras;
    fugieras.convert<

}
