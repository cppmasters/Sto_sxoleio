#include <iostream>
#include <thread>
int main()
{
  for(int i = 0;;i++)
  {
    std::cout << "gkanton " << i << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));//ti einai auto re gkiou
  }
}
