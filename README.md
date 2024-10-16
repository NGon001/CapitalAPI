
# CapitalAPI

CapitalAPI is a C++ project that integrates with the Capital.com API to handle session creation, manage trading activities, and retrieve market data.

## Usage/Examples

General Information:

Base URL: https://api-capital.backend-capital.com/

Demo Base URL: https://demo-api-capital.backend-capital.com/

Before using any of the endpoints, a session must be created using the **POST /session** endpoint. Sessions are active for 10 minutes, and if inactive for longer, an error will be triggered on the next request.

You can create seesion like this:

```c++
api.CreateSession()
```

But for long time program you need to make sure you updating your session:

```c++
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

if (api.sessioncreated)
{

//code here

}
```




## After starting the session

On starting the session you will receive the __CST__ and __X-SECURITY-TOKEN__ parameters in the response headers. CST is an authorization token, **X-SECURITY-TOKEN shows which financial account is used for the trades.** These headers should be passed on subsequent requests to the API. Both tokens are valid for __10 minutes__ after the last use.

Getting a **X-SECURITY-TOKEN** from CreateSession
```c++
size_t secTokenStart = headerData.find("X-SECURITY-TOKEN:") + 17;
size_t secTokenEnd = headerData.find("\r", secTokenStart);
if (secTokenStart != std::string::npos && secTokenEnd != std::string::npos) {
    securityToken = headerData.substr(secTokenStart, secTokenEnd - secTokenStart);
}
```

Getting a **CST** from CreateSession
```c++
size_t cstTokenStart = headerData.find("CST:") + 4;
size_t cstTokenEnd = headerData.find("\r", cstTokenStart);
if (cstTokenStart != std::string::npos && cstTokenEnd != std::string::npos) {
    cstToken = headerData.substr(cstTokenStart, cstTokenEnd - cstTokenStart);
}
```

You will use **X-SECURITY-TOKEN** and **CST** for almost every API functions as API requear.
Here is example of how you can use it in request

```c++
        struct curl_slist* headers = NULL;


        headers = curl_slist_append(headers, ("X-SECURITY-TOKEN: " + securityToken).c_str();
        headers = curl_slist_append(headers, ("CST: " + cstToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

```

## Adding API Functions
You can easily integrate other Capital.com API functions like this example:

```c++
    bool NewAPIFunction(//any imputs)
    {
        std::string response; // get responce from api after req
        std::string headerData;
        struct curl_slist* headers = NULL;


        headers = curl_slist_append(headers, ("X-SECURITY-TOKEN: " + securityToken).c_str();
        headers = curl_slist_append(headers, ("CST: " + cstToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        std::string url = "your api url";

        bool success = CurlReq(url, "", headers, response, headerData, "GET"); // API req, change "GET" to other format if req, or add more headers if API req

        //any actions what you need for this function from reponse

        return success;
    }
```

## Example

After session created successed you can use any api function from **API.h** or you can add new one,
here is example of API usage:

```c++
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
```

Expected result like 

```
Epic: CTSH CurrentSell: 77.13 CurrentBuy: 77.26 minDealSizeOut: 1
Epic: BRBY CurrentSell: 7 CurrentBuy: 7.15 minDealSizeOut: 1
Epic: HBR CurrentSell: 2.596 CurrentBuy: 2.656 minDealSizeOut: 1
Epic: RBSL CurrentSell: 3.436 CurrentBuy: 3.445 minDealSizeOut: 1
```
## Documentation

For more API info use:

[Capital API Documentation](https://open-api.capital.com/)


## Authors

- [@NGon001](https://github.com/NGon001)

