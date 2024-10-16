#include "API.hpp"
#include "Tools.hpp"
#include "ta_libc.h"
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
        if (differencetime >= 300) {
            if (!api.PingSession()) {
                std::cerr << "Session logout failed. Exiting." << std::endl;
                API::LOG("Session logout failed. Exiting.");
                return;
            }
            api.sessioncreatetime = currenttime_t;
            API::LOG("Session pinged successfully.");
        }

        if (!api.sessioncreated) {
            if (!api.CreateSession()) {
                std::cerr << "Session creation failed. Exiting." << std::endl;
                API::LOG("Session creation failed. Exiting.");
                return;
            }
            api.sessioncreatetime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            api.sessioncreated = true;
            API::LOG("Session created successfully.");
        }
        //END
        if (api.sessioncreated)
        {
            std::vector<API::MarketData> allmarkets;
            api.GetEpics(allmarkets); // get all avalible markets
            for (auto market : allmarkets) // access all elements in this market array
            {
                API::SingleMarketDetail singlemarket;
                api.GetSingleMarketInfo(singlemarket, "BTCUSD");
                std::cout <<"Epic: " << market.epic << "CurrentSell: " << singlemarket.snapshot.bid << " CurrentBuy: " << singlemarket.snapshot.offer << " minDealSizeOut: " << std::to_string(singlemarket.dealingRules.minDealSize.value) << std::endl;
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
