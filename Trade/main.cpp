#include "API.hpp"
#include "Tools.hpp"
API api;
Tool tools;

void Example()
{
        //START
        //check is session created and recreated every 5 min, because of api req
         // get current time
        auto now = std::chrono::system_clock::now();
        std::time_t currenttime_t = std::chrono::system_clock::to_time_t(now);


        if (!api.sessioncreated) {
            if (!api.CreateSession()) {
                std::cerr << "Session creation failed. Exiting." << std::endl;
                return;
            }
            api.sessioncreatetime = currenttime_t;
            api.sessioncreated = true;
        }

        auto differencetime = tools.CalculateTimeDifference(currenttime_t, api.sessioncreatetime);
        if (differencetime >= 300) { // timer for session recreation
            if (!api.logoutSession()) {
                std::cerr << "Session logout failed. Exiting." << std::endl;
                return;
            }
            api.sessioncreated = false;
        }
        //END
        if (api.sessioncreated)
        {
            std::vector<API::MarketData> allmarkets;
            api.GetEpics(allmarkets); // get all avalible markets
            for (auto market : allmarkets) // access all elements in this market array
            {
                //code here
                double CurrentSell = 0;
                double CurrentBuy = 0;
                double minDealSizeOut = 0;
                api.fetchPrice(CurrentSell, CurrentBuy, minDealSizeOut, market.epic); // get current buy, sell pricess and minimum size for buy
                std::cout <<"Epic: " << market.epic << "CurrentSell: " << CurrentSell << " CurrentBuy: " << CurrentBuy << " minDealSizeOut: " << minDealSizeOut << std::endl;

            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(3));
}

int main()
{
    ui.printLogo();
    Example();

    return 0;
}
