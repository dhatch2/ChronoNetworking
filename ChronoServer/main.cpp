#include <iostream>
#include <thread>

#include <functional>

void printNum(int num);

int main(int argc, char **argv)
{
	std::function<void(int)> func = printNum;
    int i = 0;
    std::thread th(func, i);
    std::thread th1(func, i + 1);
    std::thread th2(func, i + 2);
    th.join();
    th1.join();
    th2.join();
	return 0;
}
// TODO: Get started on world object, and making the queue of messages to write to it. After that, start thinking about the listener thread.
// zero mq or just a standard queue. Or look up an atomic queue implementation.
void printNum(int num) {
    std::cout << num << std::endl;
}
