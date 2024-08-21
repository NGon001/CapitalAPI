#pragma comment(lib, "wldap32.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#define CURL_STATICLIB
#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include "Account.hpp"
#include "UI.hpp"

UI ui;
Account account;

using json = nlohmann::json;

class API
{
private:
    const std::string from = "2024-08-20T16:26:00"; // UTC Time
    const std::string to = "2024-08-20T20:39:00";   // UTC Time
    std::vector<double> closeBidPrices;

    std::string epic = "GOLD";  // Example epic
    std::string base_url = "https://demo-api-capital.backend-capital.com";
    std::string order_url = base_url + "/api/v1/positions";
    std::string session_url = base_url + "/api/v1/session";
    std::string price_url = base_url + "/api/v1/markets/" + epic;
    std::string pricehistory_url = base_url + "/api/v1/prices/" + epic + "?resolution=MINUTE&max=1000&from=" + from + "&to=" + to;
    std::string securityToken;
    std::string cstToken;
public:
    bool sessioncreated = false;
    std::time_t sessioncreatetime;
    // Implementations are included within the class definition
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
        size_t total_size = size * nmemb;
        output->append(reinterpret_cast<char*>(contents), total_size);
        return total_size;
    }

    static size_t HeaderCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        std::string header((char*)contents, size * nmemb);
        std::string* headerData = static_cast<std::string*>(userp);

        // Check for X-SECURITY-TOKEN
        if (header.find("X-SECURITY-TOKEN:") == 0) {
            size_t pos = header.find(":");
            if (pos != std::string::npos) {
                std::string token = header.substr(pos + 1);
                token.erase(std::remove(token.begin(), token.end(), ' '), token.end()); // Trim whitespace
                *headerData += "X-SECURITY-TOKEN:" + token + "\n";
                return size * nmemb;
            }
        }

        // Check for CST
        if (header.find("CST:") == 0) {
            size_t pos = header.find(":");
            if (pos != std::string::npos) {
                std::string token = header.substr(pos + 1);
                token.erase(std::remove(token.begin(), token.end(), ' '), token.end()); // Trim whitespace
                *headerData += "CST:" + token + "\n";
                return size * nmemb;
            }
        }

        return size * nmemb;
    }

    bool CurlReq(CURL* curl, const std::string& url, const std::string& postFields, const struct curl_slist* headers, std::string& response, std::string& headerData, const std::string& requestType) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerData);

        if (requestType == "POST") {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
        }
        else if (requestType == "GET") {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
        }
        else if (requestType == "DELETE") {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        }
        else {
            std::cerr << "Unsupported request type: " << requestType << std::endl;
            return false;
        }

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return false;
        }

        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        // std::cout << "Response Code: " << response_code << std::endl;
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return true;

    }

    bool CreateSession() {
        CURL* curl;
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        if (!curl)
            return false;
        std::string response;
        std::string headerData;

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("X-CAP-API-KEY: " + account.api_key).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        json body_json = { {"identifier", account.email}, {"password", account.password} };
        std::string body = body_json.dump();

        bool success = CurlReq(curl, session_url, body, headers, response, headerData, "POST");
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        curl_global_cleanup();

        if (!success) {
            std::cerr << "Failed to perform curl request for session creation." << std::endl;
            return false;
        }

        size_t secTokenStart = headerData.find("X-SECURITY-TOKEN:") + 17;
        size_t secTokenEnd = headerData.find("\r", secTokenStart);
        if (secTokenStart != std::string::npos && secTokenEnd != std::string::npos) {
            securityToken = headerData.substr(secTokenStart, secTokenEnd - secTokenStart);
        }

        size_t cstTokenStart = headerData.find("CST:") + 4;
        size_t cstTokenEnd = headerData.find("\r", cstTokenStart);
        if (cstTokenStart != std::string::npos && cstTokenEnd != std::string::npos) {
            cstToken = headerData.substr(cstTokenStart, cstTokenEnd - cstTokenStart);
        }

        securityToken.erase(std::remove_if(securityToken.begin(), securityToken.end(), [](unsigned char c) { return std::isspace(c); }), securityToken.end());
        cstToken.erase(std::remove_if(cstToken.begin(), cstToken.end(), [](unsigned char c) { return std::isspace(c); }), cstToken.end());

        if (securityToken.empty()) {
            std::cerr << "Security Token is missing or null." << std::endl;
            return false;
        }
        if (cstToken.empty()) {
            std::cerr << "CST Token is missing or null." << std::endl;
            return false;
        }
        std::cout << ui.color_cyan << "Security Token: " << securityToken << std::endl;
        std::cout << "CST Token: " << cstToken << ui.color_reset << std::endl;
        return true;
    }

    bool logoutSession() {
        CURL* curl;
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        if (!curl) return false;
        std::string url = "https://demo-api-capital.backend-capital.com/api/v1/session";
        std::string response, headerData;

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("X-SECURITY-TOKEN: " + securityToken).c_str());
        headers = curl_slist_append(headers, ("CST: " + cstToken).c_str());

        bool success = CurlReq(curl, url, "", headers, response, headerData, "DELETE");
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        curl_global_cleanup();

        return success;
    }

    // Fetch the current price of an asset
    bool fetchPrice(double &CurrentSellPrice, double &CurrentBuyPrice) {
        CURL* curl;
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        if (!curl) return false;

        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;


        headers = curl_slist_append(headers, ("X-SECURITY-TOKEN: " + securityToken).c_str());
        headers = curl_slist_append(headers, ("CST: " + cstToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        bool success = CurlReq(curl, price_url, "", headers, response, headerData, "GET");
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        curl_global_cleanup();

        if (!success) {
            std::cerr << "Failed to perform curl request for price fetching." << std::endl;
            return false;
        }

        // Debug: Print raw response and header data
        //std::cout << "Price Fetch Response: " << response << std::endl;

        try {
            nlohmann::json response_json = nlohmann::json::parse(response);

            if (response_json.contains("snapshot")) {
                const auto& snapshot = response_json["snapshot"];
                if (snapshot.contains("bid") && snapshot.contains("offer")) {
                    CurrentSellPrice = snapshot["bid"].get<double>();
                    CurrentBuyPrice = snapshot["offer"].get<double>();
                }
                else {
                    std::cerr << "Bid or offer price not found in snapshot." << std::endl;
                }
            }
            else {
                std::cerr << "Snapshot not found in response." << std::endl;
            }
        }
        catch (const nlohmann::json::exception& e) {
            std::cerr << "JSON parsing error: " << e.what() << std::endl;
        }
        return true;
    }

    bool fetchPriceHistory() {
        CURL* curl;
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        if (!curl) return false;

        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;


        headers = curl_slist_append(headers, ("X-SECURITY-TOKEN: " + securityToken).c_str());
        headers = curl_slist_append(headers, ("CST: " + cstToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        bool success = CurlReq(curl, pricehistory_url, "", headers, response, headerData, "GET");
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        curl_global_cleanup();

        if (!success) {
            std::cerr << "Failed to perform curl request for price fetching." << std::endl;
            return false;
        }

        // Debug: Print raw response and header data
       // std::cout << "Price Fetch Response: " << response << std::endl;

        try {
            nlohmann::json response_json = nlohmann::json::parse(response);

            if (response_json.contains("prices")) {
                const auto& prices = response_json["prices"];
                for (const auto& price : prices) {
                    if (price.contains("closePrice") && price["closePrice"].contains("bid")) {
                        closeBidPrices.push_back(price["closePrice"]["bid"].get<double>());
                    }
                }
            }
            else {
                std::cerr << "Prices not found in response." << std::endl;
                return false;
            }
        }
        catch (const nlohmann::json::exception& e) {
            std::cerr << "JSON parsing error: " << e.what() << std::endl;
            return false;
        }
        return true;
    }

    bool Order(const std::string& direction, const std::string& dealReference = "") {
        CURL* curl;
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        if (!curl) return false;

        std::string response, headerData;
        struct curl_slist* headers = nullptr;

        headers = curl_slist_append(headers, ("X-SECURITY-TOKEN: " + securityToken).c_str());
        headers = curl_slist_append(headers, ("CST: " + cstToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        json body_json = {
            {"epic", epic},
            {"direction", direction},
            {"size", account.btchave},
            {"guaranteedStop", false},
            {"trailingStop", false}
        };

        if (!dealReference.empty()) {
            body_json["dealReference"] = dealReference;
        }

        std::string body = body_json.dump();

        if (!CurlReq(curl, order_url, body, headers, response, headerData, "POST")) {
            return false;
        }
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        curl_global_cleanup();

        try {
            json response_json = json::parse(response);
            if (response_json.contains("errorCode")) {
                std::cerr << "Error in order creation: " << response_json["errorCode"] << std::endl;
                return false;
            }
            else {
                if (direction == "BUY") {
                    std::cout << "Buy order created successfully." << std::endl;
                    account.dealReference = response_json["dealReference"];
                    std::cout << "Deal Reference: " << account.dealReference << std::endl;
                    return true;
                }
                else {
                    std::cout << "Sell order completed successfully." << std::endl;
                    return true;
                }
            }
        }
        catch (const json::exception& e) {
            std::cerr << "JSON parsing error: " << e.what() << std::endl;
            return false;
        }

        return true;
    }
};
