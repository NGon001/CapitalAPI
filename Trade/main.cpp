#include "API.hpp"
#include "Tools.hpp"
API api;
Tool tools;

void BotStart()
{
    double CurrentSell = 0;
    double CurrentBuy = 0;

    while (true)
    {
        //START
        //check is session created and recreated every 5 min, because of api req
        std::time_t currenttime_t = tools.stringToTimeT(tools.nowtime());
        tools.nowtime();
        if (!api.sessioncreated) //create session
        {
            if (!api.CreateSession()) {
                std::cerr << "Session creation failed. Exiting." << std::endl;
                return;
            }
            api.sessioncreatetime = currenttime_t;
            api.sessioncreated = true;
        }
        auto differencetime = tools.CalculateTimeDifference(currenttime_t, api.sessioncreatetime);
        if (differencetime >= 300) //logout session
        {
            if (!api.logoutSession()) {
                std::cerr << "Session logout failed. Exiting." << std::endl;
                return;
            }
            api.sessioncreated = false;
        }
        //END
        if (api.sessioncreated)
        {
            //code here
            api.fetchPrice(CurrentSell,CurrentBuy);
            std::cout << "CurrentSell: " << CurrentSell << " CurrentBuy: " << CurrentBuy  << std::endl;
            std::cout << "Procent dif: " << tools.ProcentDifferenceCalculate(CurrentSell, CurrentBuy) << std::endl;
            std::cout << "Procent Add Test: " << tools.ProcentageAddOrSub(CurrentBuy, 1, "+") << std::endl;
            std::cout << "Procent Sub Test: " << tools.ProcentageAddOrSub(CurrentBuy,1,"-") << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
}

int main()
{
    ui.printLogo();
    BotStart();

    return 0;
}
