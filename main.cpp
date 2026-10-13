#include <iostream>
#include <thread>
int main()
{
  for(int i = 0;;i++)
  {
    std::cout << "gkanton\n", i;
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}