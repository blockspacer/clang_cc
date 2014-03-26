#include <boost/thread.hpp>
#include <iostream>
void func()
{
	std::cout << "Hello world\n";
}
int main()
{
	boost::thread thr(func);
	thr.join();
}

