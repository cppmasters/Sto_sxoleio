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

return 0;



#include <iostrea>
int main ()
{
    double a,b;
    char c;
    std::cout <<" dose ton proto arithmo";
    std::cin >>  a;
    std::cout <<"dose ton deytero arithmo:";
    std::cin  >> b;
    std::cout <<"dose thn prakdi pou thes na ginei me toys dyo arithmous sou :";
    std::cin  >> p;
    if(p=='+')
    {
        std::cout << a+b;
    }
    if(p=='-')
    {
        std::cout << a-b;
    }
    if(p=='/')
    {
        std::cout << a/b;
    }
    if(p=='*')
    {
        std::cout << a*b;
    }


}
return 0;
