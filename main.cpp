#include <iostream>
int main()
{
    // metablhtes
    double input, athrisma = 0;
    int plithos = 0;

    // grafoume to arxiko mnm (mia fora)
    std::cout << " dose tous tous batmous sou\n";

    // loupa
    while (input != -1)//ti paei na pei to thabmastiko
    {
        // perimenei dedomena apo thn xrhsth
        std::cout << ": ";
        std::cin >> input;

        // an o xrhsths grapsei kati ontos dekto
        if (input >= 0 && input <= 20)
        {
            // to athrisma ton bathmon
            athrisma = athrisma + input;
            // metrame poses fores egrapse o xrhsths
            plithos++;
        }
    }
    // otan en telei bgei apo thn loupa (o xrisths grafei -1):

    // theloume na mhn ginei diairesh me to 0
    if (plithos != 0)
    {
        std::cout << "o mesos oros sou einai: " << athrisma / plithos;
    }
    // an to plitos einai 0 tote kai o mesos oros einai 0
    else
    {
        std::cout << "o mesos oros sou einai: " << 0;
    }
}
