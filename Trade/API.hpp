#include "Account.hpp"
#include "UI.hpp"

#pragma comment(lib, "wldap32.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#define CURL_STATICLIB
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <mutex>
#include <fstream>  // This must be included for std::ofstream
#include <ctime>    // For timestamp




UI ui;
Account account;

using json = nlohmann::json;

class API
{
public:
    std::string base_url = "https://demo-api-capital.backend-capital.com";
    std::string positions_url = base_url + "/api/v1/positions/";
    std::string session_url = base_url + "/api/v1/session";
    std::string markets_url = base_url + "/api/v1/markets/";
    std::string accounts_url = base_url + "/api/v1/accounts/";
    std::string price_url = base_url + "/api/v1/markets/";
    std::string ping_url = base_url + "/api/v1/ping";
    std::string securityToken;
    std::string cstToken;
    bool sessioncreated = false;
    std::time_t sessioncreatetime;
    /*
    Levrage
    currencies - 30:1
    indices - 20:1
    commodities 20:1
    crypto 2:1
    shares 5:1
    */
    // Static log function
    static void LOG(const std::string& message) {
        static std::ofstream logFile("log.txt", std::ios::app);  // Open log file in append mode

        // Get current time
        std::time_t currentTime = std::time(nullptr);
        char timeStr[26];
        ctime_s(timeStr, sizeof(timeStr), &currentTime);  // Use ctime_s for thread-safe, secure time

        timeStr[24] = '\0';  // Remove newline character (ctime_s appends a newline at the end)

        if (logFile.is_open()) {
            logFile << "[" << timeStr << "] " << message << std::endl;
        }
        else {
            std::cerr << "Failed to write log: " << message << std::endl;
        }
    }

    class TradePosition {
    public:
        // Position-related data
        double contractSize;
        std::string createdDate;
        std::string createdDateUTC;
        std::string dealId;
        std::string dealReference;
        std::string workingOrderId;
        double size;
        int leverage;
        double upl;
        std::string direction;
        double level;
        std::string currency;
        bool guaranteedStop;

        // Market-related data
        std::string instrumentName;
        std::string expiry;
        std::string marketStatus;
        std::string epic;
        std::string symbol;
        std::string instrumentType;
        int lotSize;
        double percentageChange;
        double bid;
        double offer;
        std::string updateTime;
        std::string updateTimeUTC;
        int delayTime;
        bool streamingPricesAvailable;
        int scalingFactor;
        std::vector<std::string> marketModes;

        // Default constructor
        TradePosition()
            : contractSize(0.0), marketModes({}) {}

        // Constructor
        TradePosition(double contractSize, const std::string& createdDate, const std::string& createdDateUTC,
            const std::string& dealId, const std::string& dealReference, const std::string& workingOrderId,
            double size, int leverage, double upl, const std::string& direction, double level,
            const std::string& currency, bool guaranteedStop, const std::string& instrumentName,
            const std::string& expiry, const std::string& marketStatus, const std::string& epic,
            const std::string& symbol, const std::string& instrumentType, int lotSize,
            double percentageChange, double bid, double offer, const std::string& updateTime,
            const std::string& updateTimeUTC, int delayTime, bool streamingPricesAvailable,
            int scalingFactor, const std::vector<std::string>& marketModes)
            : contractSize(contractSize), createdDate(createdDate), createdDateUTC(createdDateUTC), dealId(dealId),
            dealReference(dealReference), workingOrderId(workingOrderId), size(size), leverage(leverage),
            upl(upl), direction(direction), level(level), currency(currency), guaranteedStop(guaranteedStop),
            instrumentName(instrumentName), expiry(expiry), marketStatus(marketStatus), epic(epic),
            symbol(symbol), instrumentType(instrumentType), lotSize(lotSize), percentageChange(percentageChange),
            bid(bid), offer(offer), updateTime(updateTime), updateTimeUTC(updateTimeUTC), delayTime(delayTime),
            streamingPricesAvailable(streamingPricesAvailable), scalingFactor(scalingFactor), marketModes(marketModes) {}

        // Static method to parse JSON and populate vector of TradePosition objects
        static bool ParseFromJsonArray(const std::string& jsonResponse, std::vector<TradePosition>& tradePositions) {
            try {
                // Parse the JSON response using nlohmann::json
                nlohmann::json jsonResponseParsed = nlohmann::json::parse(jsonResponse);

                // Clear the tradePositions vector
                tradePositions.clear();

                // Ensure the JSON response has a "positions" array
                if (jsonResponseParsed.contains("positions") && jsonResponseParsed["positions"].is_array()) {
                    for (const auto& jsonPosition : jsonResponseParsed["positions"]) {
                        // Extract position and market data from JSON
                        auto positionData = jsonPosition["position"];
                        auto marketData = jsonPosition["market"];

                        // Create TradePosition objects and populate them
                        TradePosition tradePosition(
                            positionData["contractSize"].get<double>(),
                            positionData["createdDate"].get<std::string>(),
                            positionData["createdDateUTC"].get<std::string>(),
                            positionData["dealId"].get<std::string>(),
                            positionData["dealReference"].get<std::string>(),
                            positionData["workingOrderId"].get<std::string>(),
                            positionData["size"].get<double>(),
                            positionData["leverage"].get<int>(),
                            positionData["upl"].get<double>(),
                            positionData["direction"].get<std::string>(),
                            positionData["level"].get<double>(),
                            positionData["currency"].get<std::string>(),
                            positionData["guaranteedStop"].get<bool>(),
                            marketData["instrumentName"].get<std::string>(),
                            marketData["expiry"].get<std::string>(),
                            marketData["marketStatus"].get<std::string>(),
                            marketData["epic"].get<std::string>(),
                            marketData["symbol"].get<std::string>(),
                            marketData["instrumentType"].get<std::string>(),
                            marketData["lotSize"].get<int>(),
                            marketData["percentageChange"].get<double>(),
                            marketData["bid"].get<double>(),
                            marketData["offer"].get<double>(),
                            marketData["updateTime"].get<std::string>(),
                            marketData["updateTimeUTC"].get<std::string>(),
                            marketData["delayTime"].get<int>(),
                            marketData["streamingPricesAvailable"].get<bool>(),
                            marketData["scalingFactor"].get<int>(),
                            marketData["marketModes"].get<std::vector<std::string>>()
                        );

                        if (tradePosition.marketStatus != "TRADEABLE") continue;
                        // Add the tradePosition to the vector
                        tradePositions.push_back(tradePosition);
                    }
                }
                else {
                    // Unexpected JSON format
                    return false;
                }
            }
            catch (const std::exception& e) {
                // Handle JSON parsing errors
                std::cerr << "Error parsing JSON: " << e.what() << std::endl;
                return false;
            }

            return true;
        }

        static bool ParseFromJson(const std::string& jsonResponse, TradePosition& tradePositions) {
            try {
                // Parse the JSON response using nlohmann::json
                nlohmann::json jsonResponseParsed = nlohmann::json::parse(jsonResponse);

                // Ensure the JSON response has a "positions" array
                if (jsonResponseParsed.contains("position") && jsonResponseParsed.contains("market")) {
                
                        // Extract position and market data from JSON
                        auto positionData = jsonResponseParsed["position"];
                        auto marketData = jsonResponseParsed["market"];

                        // Create TradePosition objects and populate them
                        TradePosition tradePosition(
                            positionData["contractSize"].get<double>(),
                            positionData["createdDate"].get<std::string>(),
                            positionData["createdDateUTC"].get<std::string>(),
                            positionData["dealId"].get<std::string>(),
                            positionData["dealReference"].get<std::string>(),
                            positionData["workingOrderId"].get<std::string>(),
                            positionData["size"].get<double>(),
                            positionData["leverage"].get<int>(),
                            positionData["upl"].get<double>(),
                            positionData["direction"].get<std::string>(),
                            positionData["level"].get<double>(),
                            positionData["currency"].get<std::string>(),
                            positionData["guaranteedStop"].get<bool>(),
                            marketData["instrumentName"].get<std::string>(),
                            marketData["expiry"].get<std::string>(),
                            marketData["marketStatus"].get<std::string>(),
                            marketData["epic"].get<std::string>(),
                            marketData["symbol"].get<std::string>(),
                            marketData["instrumentType"].get<std::string>(),
                            marketData["lotSize"].get<int>(),
                            marketData["percentageChange"].get<double>(),
                            marketData["bid"].get<double>(),
                            marketData["offer"].get<double>(),
                            marketData["updateTime"].get<std::string>(),
                            marketData["updateTimeUTC"].get<std::string>(),
                            marketData["delayTime"].get<int>(),
                            marketData["streamingPricesAvailable"].get<bool>(),
                            marketData["scalingFactor"].get<int>(),
                            marketData["marketModes"].get<std::vector<std::string>>()
                        );

                        if (tradePosition.marketStatus != "TRADEABLE") return false;
                        // Add the tradePosition to the vector
                        tradePositions = tradePosition;
                   
                }
                else {
                    // Unexpected JSON format
                    return false;
                }
            }
            catch (const std::exception& e) {
                // Handle JSON parsing errors
                std::cerr << "Error parsing JSON: " << e.what() << std::endl;
                return false;
            }

            return true;
        }

        // Method to print details
        void printDetails() {
            std::cout << "Position Details:\n";
            std::cout << "Deal ID: " << dealId << "\n"
                << "Direction: " << direction << "\n"
                << "dealReference: " << dealReference << "\n"
                << "Size: " << size << "\n"
                << "Leverage: " << leverage << "\n"
                << "UPL: " << upl << "\n"
                << "Level: " << level << "\n"
                << "Currency: " << currency << "\n\n";

            std::cout << "Market Details:\n";
            std::cout << "Instrument Name: " << instrumentName << "\n"
                << "Epic: " << epic << "\n"
                << "Bid: " << bid << "\n"
                << "Offer: " << offer << "\n"
                << "Market Status: " << marketStatus << "\n";
        }
    };

    class MarketData {
    public:
        // Default constructor
        MarketData() = default;

        // Constructor from JSON
        explicit MarketData(const nlohmann::json& json) {
            from_json(json, *this);
        }

        // Convert to JSON
        nlohmann::json to_json() const {
            nlohmann::json json;
            json["delayTime"] = delayTime;
            json["epic"] = epic;
            json["symbol"] = symbol;
            json["lotSize"] = lotSize;
            json["expiry"] = expiry;
            json["instrumentType"] = instrumentType;
            json["instrumentName"] = instrumentName;
            json["percentageChange"] = percentageChange;
            json["updateTime"] = updateTime;
            json["updateTimeUTC"] = updateTimeUTC;
            json["bid"] = bid;
            json["offer"] = offer;
            json["streamingPricesAvailable"] = streamingPricesAvailable;
            json["marketStatus"] = marketStatus;
            json["scalingFactor"] = scalingFactor;
            json["marketModes"] = marketModes;
            json["pipPosition"] = pipPosition;
            json["tickSize"] = tickSize;
            return json;
        }

        // JSON deserialization
        friend void from_json(const nlohmann::json& json, MarketData& marketData) {
            // Safely retrieve values or set default values if keys are missing
            marketData.delayTime = json.value("delayTime", 0);
            marketData.epic = json.value("epic", "");
            marketData.symbol = json.value("symbol", "");
            marketData.lotSize = json.value("lotSize", 0);
            marketData.expiry = json.value("expiry", "-");
            marketData.instrumentType = json.value("instrumentType", "");
            marketData.instrumentName = json.value("instrumentName", "");
            marketData.percentageChange = json.value("percentageChange", 0.0);  // Default to 0.0 if missing
            marketData.updateTime = json.value("updateTime", "");
            marketData.updateTimeUTC = json.value("updateTimeUTC", "");
            marketData.bid = json.value("bid", 0.0);
            marketData.offer = json.value("offer", 0.0);
            marketData.streamingPricesAvailable = json.value("streamingPricesAvailable", false);
            marketData.marketStatus = json.value("marketStatus", "UNKNOWN");
            marketData.scalingFactor = json.value("scalingFactor", 1);
            marketData.marketModes = json.value("marketModes", std::vector<std::string>{});
            marketData.pipPosition = json.value("pipPosition", 0);
            marketData.tickSize = json.value("tickSize", 0.0);
        }

        static bool ParseFromJson(const std::string& jsonResponse, std::vector<MarketData>& marketDataList) {
            try {
                // Parse the JSON response
                nlohmann::json jsonResponseParsed = nlohmann::json::parse(jsonResponse);

                if (!jsonResponseParsed.contains("markets")) return false;
                auto markets = jsonResponseParsed["markets"];

                // Ensure the JSON response is an array
                if (markets.is_array()) {
                    // Clear the marketDataList vector
                    marketDataList.clear();

                    int failedCount = 0;  // To keep track of failures

                    // Iterate through the JSON array and populate MarketData objects
                    for (const auto& jsonItem : markets) {
                        try {
                            MarketData data = jsonItem.get<MarketData>();
                            marketDataList.push_back(data);

                            // Check for missing fields
                            if (!jsonItem.contains("percentageChange")) {
                                std::cerr << "Missing 'percentageChange' for item: " << jsonItem.dump() << std::endl;
                            }
                        }
                        catch (const std::exception& e) {
                            std::cerr << "Error parsing item: " << e.what() << std::endl;
                            continue;
                        }
                    }


                    // Debugging information
                    std::cout << "Number of successfully parsed MarketData items: " << marketDataList.size() << std::endl;
                    std::cout << "Number of failed items: " << failedCount << std::endl;
                }
                else {
                    std::cerr << "Unexpected JSON format: 'markets' is not an array." << std::endl;
                    return false;
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Error parsing JSON response: " << e.what() << std::endl;
                return false;
            }

            return true;
        }

        // Print method to display all class information
        void Print() const {
            std::cout << "--------------" << std::endl;
            std::cout << "Delay Time: " << delayTime << std::endl;
            std::cout << "EPIC: " << epic << std::endl;
            std::cout << "Symbol: " << symbol << std::endl;
            std::cout << "Lot Size: " << lotSize << std::endl;
            std::cout << "Expiry: " << expiry << std::endl;
            std::cout << "Instrument Type: " << instrumentType << std::endl;
            std::cout << "Instrument Name: " << instrumentName << std::endl;
            std::cout << "Percentage Change: " << percentageChange << std::endl;
            std::cout << "Update Time: " << updateTime << std::endl;
            std::cout << "Update Time UTC: " << updateTimeUTC << std::endl;
            std::cout << "Bid: " << bid << std::endl;
            std::cout << "Offer: " << offer << std::endl;
            std::cout << "Streaming Prices Available: " << std::boolalpha << streamingPricesAvailable << std::endl;
            std::cout << "Market Status: " << marketStatus << std::endl;
            std::cout << "Scaling Factor: " << scalingFactor << std::endl;
            std::cout << "Market Modes: ";
            for (const auto& mode : marketModes) {
                std::cout << mode;
                if (&mode != &marketModes.back()) {
                    std::cout << ", ";  // Add comma between modes, but not at the end
                }
            }
            std::cout << std::endl;
            std::cout << "Pip Position: " << pipPosition << std::endl;
            std::cout << "Tick Size: " << tickSize << std::endl;
            std::cout << "--------------" << std::endl;
        }
    


        // Member variables
        int delayTime;
        std::string epic;
        std::string symbol;
        int lotSize;
        std::string expiry;
        std::string instrumentType;
        std::string instrumentName;
        double percentageChange;
        std::string updateTime;
        std::string updateTimeUTC;
        double bid;
        double offer;
        bool streamingPricesAvailable;
        std::string marketStatus;
        int scalingFactor;
        std::vector<std::string> marketModes;
        int pipPosition;
        double tickSize;
    };

    class SingleMarketDetail {
    public:
        // Default constructor
        SingleMarketDetail() = default;

        // Constructor from JSON
        explicit SingleMarketDetail(const nlohmann::json& json) {
            from_json(json, *this);
        }

        // Convert to JSON
        nlohmann::json to_json() const {
            nlohmann::json json;
            json["instrument"] = instrument.to_json();
            json["dealingRules"] = dealingRules.to_json();
            json["snapshot"] = snapshot.to_json();
            return json;
        }

        // JSON deserialization
        friend void from_json(const nlohmann::json& json, SingleMarketDetail& marketData) {
            marketData.instrument = json.value("instrument", Instrument());
            marketData.dealingRules = json.value("dealingRules", DealingRules());
            marketData.snapshot = json.value("snapshot", Snapshot());
        }

        // Print method to display class information
        void Print() const {
            instrument.Print();
            dealingRules.Print();
            snapshot.Print();
        }

        // Nested Instrument class
        class Instrument {
        public:
            std::string epic;
            std::string symbol;
            std::string expiry;
            std::string name;
            int lotSize;
            std::string type;
            bool guaranteedStopAllowed;
            bool streamingPricesAvailable;
            std::string currency;
            double marginFactor;
            std::string marginFactorUnit;
            int leverage;

            struct OpeningHours {
                std::vector<std::string> mon;
                std::vector<std::string> tue;
                std::vector<std::string> wed;
                std::vector<std::string> thu;
                std::vector<std::string> fri;
                std::vector<std::string> sat;
                std::vector<std::string> sun;
                std::string zone;

                nlohmann::json to_json() const {
                    return {
                        {"mon", mon},
                        {"tue", tue},
                        {"wed", wed},
                        {"thu", thu},
                        {"fri", fri},
                        {"sat", sat},
                        {"sun", sun},
                        {"zone", zone}
                    };
                }

                friend void from_json(const nlohmann::json& json, OpeningHours& hours) {
                    hours.mon = json.value("mon", std::vector<std::string>{});
                    hours.tue = json.value("tue", std::vector<std::string>{});
                    hours.wed = json.value("wed", std::vector<std::string>{});
                    hours.thu = json.value("thu", std::vector<std::string>{});
                    hours.fri = json.value("fri", std::vector<std::string>{});
                    hours.sat = json.value("sat", std::vector<std::string>{});
                    hours.sun = json.value("sun", std::vector<std::string>{});
                    hours.zone = json.value("zone", "UTC");
                }

               
                std::vector<std::string> getOpeningHoursForDay(int tm_wday) {
                    switch (tm_wday) {
                    case 0: return sun;
                    case 1: return mon;
                    case 2: return tue;
                    case 3: return wed;
                    case 4: return thu;
                    case 5: return fri;
                    case 6: return sat;
                    default: return {}; // Return an empty vector for invalid days
                    }
                }

                
            } openingHours;

            struct OvernightFee {
                double longRate;
                double shortRate;
                long long swapChargeTimestamp;
                int swapChargeInterval;

                nlohmann::json to_json() const {
                    return {
                        {"longRate", longRate},
                        {"shortRate", shortRate},
                        {"swapChargeTimestamp", swapChargeTimestamp},
                        {"swapChargeInterval", swapChargeInterval}
                    };
                }

                friend void from_json(const nlohmann::json& json, OvernightFee& fee) {
                    fee.longRate = json.value("longRate", 0.0);
                    fee.shortRate = json.value("shortRate", 0.0);
                    fee.swapChargeTimestamp = json.value("swapChargeTimestamp", 0LL);
                    fee.swapChargeInterval = json.value("swapChargeInterval", 0);
                }
            } overnightFee;

            nlohmann::json to_json() const {
                return {
                    {"epic", epic},
                    {"symbol", symbol},
                    {"expiry", expiry},
                    {"name", name},
                    {"lotSize", lotSize},
                    {"type", type},
                    {"guaranteedStopAllowed", guaranteedStopAllowed},
                    {"streamingPricesAvailable", streamingPricesAvailable},
                    {"currency", currency},
                    {"marginFactor", marginFactor},
                    {"marginFactorUnit", marginFactorUnit},
                    {"openingHours", openingHours.to_json()},
                    {"overnightFee", overnightFee.to_json()}
                };
            }

            friend void from_json(const nlohmann::json& json, Instrument& instrument) {
                instrument.epic = json.value("epic", "");
                instrument.symbol = json.value("symbol", "");
                instrument.expiry = json.value("expiry", "-");
                instrument.name = json.value("name", "");
                instrument.lotSize = json.value("lotSize", 1);
                instrument.type = json.value("type", "");
                instrument.guaranteedStopAllowed = json.value("guaranteedStopAllowed", false);
                instrument.streamingPricesAvailable = json.value("streamingPricesAvailable", false);
                instrument.currency = json.value("currency", "USD");
                instrument.marginFactor = json.value("marginFactor", 0.0);
                instrument.marginFactorUnit = json.value("marginFactorUnit", "PERCENTAGE");
                instrument.openingHours = json.value("openingHours", OpeningHours());
                instrument.overnightFee = json.value("overnightFee", OvernightFee());
            }

            void Print() const {
                std::cout << "Instrument: " << name << " (" << symbol << ")" << std::endl;
            }

            
        } instrument;

        // Nested DealingRules class
        class DealingRules {
        public:
            struct RuleUnitValue {
                std::string unit;
                double value;

                nlohmann::json to_json() const {
                    return {
                        {"unit", unit},
                        {"value", value}
                    };
                }

                friend void from_json(const nlohmann::json& json, RuleUnitValue& rule) {
                    rule.unit = json.value("unit", "POINTS");
                    rule.value = json.value("value", 0.0);
                }
            };

            RuleUnitValue minStepDistance;
            RuleUnitValue minDealSize;
            RuleUnitValue maxDealSize;
            RuleUnitValue minSizeIncrement;
            RuleUnitValue minGuaranteedStopDistance;
            RuleUnitValue minStopOrProfitDistance;
            RuleUnitValue maxStopOrProfitDistance;
            std::string marketOrderPreference;
            std::string trailingStopsPreference;

            nlohmann::json to_json() const {
                return {
                    {"minStepDistance", minStepDistance.to_json()},
                    {"minDealSize", minDealSize.to_json()},
                    {"maxDealSize", maxDealSize.to_json()},
                    {"minSizeIncrement", minSizeIncrement.to_json()},
                    {"minGuaranteedStopDistance", minGuaranteedStopDistance.to_json()},
                    {"minStopOrProfitDistance", minStopOrProfitDistance.to_json()},
                    {"maxStopOrProfitDistance", maxStopOrProfitDistance.to_json()},
                    {"marketOrderPreference", marketOrderPreference},
                    {"trailingStopsPreference", trailingStopsPreference}
                };
            }

            friend void from_json(const nlohmann::json& json, DealingRules& rules) {
                rules.minStepDistance = json.value("minStepDistance", RuleUnitValue());
                rules.minDealSize = json.value("minDealSize", RuleUnitValue());
                rules.maxDealSize = json.value("maxDealSize", RuleUnitValue());
                rules.minSizeIncrement = json.value("minSizeIncrement", RuleUnitValue());
                rules.minGuaranteedStopDistance = json.value("minGuaranteedStopDistance", RuleUnitValue());
                rules.minStopOrProfitDistance = json.value("minStopOrProfitDistance", RuleUnitValue());
                rules.maxStopOrProfitDistance = json.value("maxStopOrProfitDistance", RuleUnitValue());
                rules.marketOrderPreference = json.value("marketOrderPreference", "");
                rules.trailingStopsPreference = json.value("trailingStopsPreference", "");
            }

            void Print() const {
                std::cout << "Dealing Rules: Market Order Preference = " << marketOrderPreference << std::endl;
            }
        } dealingRules;

        // Nested Snapshot class
        class Snapshot {
        public:
            std::string marketStatus;
            double netChange;
            double percentageChange;
            std::string updateTime;
            int delayTime;
            double bid;
            double offer;
            double high;
            double low;
            int decimalPlacesFactor;
            int scalingFactor;
            std::vector<std::string> marketModes;

            nlohmann::json to_json() const {
                return {
                    {"marketStatus", marketStatus},
                    {"netChange", netChange},
                    {"percentageChange", percentageChange},
                    {"updateTime", updateTime},
                    {"delayTime", delayTime},
                    {"bid", bid},
                    {"offer", offer},
                    {"high", high},
                    {"low", low},
                    {"decimalPlacesFactor", decimalPlacesFactor},
                    {"scalingFactor", scalingFactor},
                    {"marketModes", marketModes}
                };
            }

            friend void from_json(const nlohmann::json& json, Snapshot& snapshot) {
                snapshot.marketStatus = json.value("marketStatus", "TRADEABLE");
                snapshot.netChange = json.value("netChange", 0.0);
                snapshot.percentageChange = json.value("percentageChange", 0.0);
                snapshot.updateTime = json.value("updateTime", "");
                snapshot.delayTime = json.value("delayTime", 0);
                snapshot.bid = json.value("bid", 0.0);
                snapshot.offer = json.value("offer", 0.0);
                snapshot.high = json.value("high", 0.0);
                snapshot.low = json.value("low", 0.0);
                snapshot.decimalPlacesFactor = json.value("decimalPlacesFactor", 0);
                snapshot.scalingFactor = json.value("scalingFactor", 1);
                snapshot.marketModes = json.value("marketModes", std::vector<std::string>{});
            }

            void Print() const {
                std::cout << "Snapshot: " << std::endl;
                std::cout << "Market Status: " << marketStatus << std::endl;
                std::cout << "Net Change: " << netChange << std::endl;
                std::cout << "Percentage Change: " << percentageChange << std::endl;
                std::cout << "Update Time: " << updateTime << std::endl;
                std::cout << "Delay Time: " << delayTime << std::endl;
                std::cout << "Bid: " << bid << std::endl;
                std::cout << "Offer: " << offer << std::endl;
                std::cout << "High: " << high << std::endl;
                std::cout << "Low: " << low << std::endl;
                std::cout << "Decimal Places Factor: " << decimalPlacesFactor << std::endl;
                std::cout << "Scaling Factor: " << scalingFactor << std::endl;
                std::cout << "Market Modes: ";
                for (const auto& mode : marketModes) {
                    std::cout << mode;
                    if (&mode != &marketModes.back()) {
                        std::cout << ", ";
                    }
                }
                std::cout << std::endl;
            }
        } snapshot;

        static bool ParseFromJson(const std::string& jsonResponse, SingleMarketDetail& marketDetails) {
            try {
                // Parse the JSON response using nlohmann::json
                nlohmann::json jsonResponseParsed = nlohmann::json::parse(jsonResponse);

                // Ensure the JSON response contains an array of markets
                        // Parse the instrument, dealingRules, and snapshot sections of each market
                        auto instrumentData = jsonResponseParsed["instrument"];
                        auto dealingRulesData = jsonResponseParsed["dealingRules"];
                        auto snapshotData = jsonResponseParsed["snapshot"];

                        if (instrumentData.empty() || dealingRulesData.empty() || snapshotData.empty()) return false;
                        // Create a SingleMarketDetail object and populate it
                        SingleMarketDetail marketDetail;

                       
                       // std::cout << "snapshot START" << std::endl;
                        marketDetail.snapshot.updateTime = snapshotData["updateTime"].get<std::string>();
                       // std::cout << "snapshot updateTime " << snapshotData.contains("updateTime") << std::endl;
                        marketDetail.snapshot.bid = snapshotData["bid"].get<double>();
                        //std::cout << "snapshot bid " << snapshotData.contains("bid") << std::endl;
                        marketDetail.snapshot.offer = snapshotData["offer"].get<double>();
                        marketDetail.snapshot.marketStatus = snapshotData["marketStatus"].get<std::string>();
                       // std::cout << "snapshot offer " << snapshotData.contains("offer") << std::endl;
    

                        // Instrument data
                        marketDetail.instrument.epic = instrumentData["epic"].get<std::string>();
                        marketDetail.instrument.symbol = instrumentData["symbol"].get<std::string>();
                        marketDetail.instrument.expiry = instrumentData["expiry"].get<std::string>();
                        marketDetail.instrument.name = instrumentData["name"].get<std::string>();
                        marketDetail.instrument.lotSize = instrumentData["lotSize"].get<int>();
                        marketDetail.instrument.type = instrumentData["type"].get<std::string>();
                        marketDetail.instrument.guaranteedStopAllowed = instrumentData["guaranteedStopAllowed"].get<bool>();
                        marketDetail.instrument.streamingPricesAvailable = instrumentData["streamingPricesAvailable"].get<bool>();
                        marketDetail.instrument.currency = instrumentData["currency"].get<std::string>();
                        marketDetail.instrument.marginFactor = instrumentData["marginFactor"].get<double>();
                        marketDetail.instrument.marginFactorUnit = instrumentData["marginFactorUnit"].get<std::string>();


                        if (marketDetail.instrument.type == "CRYPTOCURRENCIES")
                            marketDetail.instrument.leverage = 2;
                        if (marketDetail.instrument.type == "INDICES")
                            marketDetail.instrument.leverage = 20;
                        if (marketDetail.instrument.type == "SHARES")
                            marketDetail.instrument.leverage = 5;
                        if (marketDetail.instrument.type == "COMMODITIES")
                            marketDetail.instrument.leverage = 20;
                        if (marketDetail.instrument.type == "CURRENCIES")
                            marketDetail.instrument.leverage = 30;



                        // Instrument opening hours
                        if (instrumentData.contains("openingHours")) {
                            auto openingHours = instrumentData["openingHours"];
                            marketDetail.instrument.openingHours.mon = openingHours["mon"].get<std::vector<std::string>>();
                            marketDetail.instrument.openingHours.tue = openingHours["tue"].get<std::vector<std::string>>();
                            marketDetail.instrument.openingHours.wed = openingHours["wed"].get<std::vector<std::string>>();
                            marketDetail.instrument.openingHours.thu = openingHours["thu"].get<std::vector<std::string>>();
                            marketDetail.instrument.openingHours.fri = openingHours["fri"].get<std::vector<std::string>>();
                            marketDetail.instrument.openingHours.sat = openingHours["sat"].get<std::vector<std::string>>();
                            marketDetail.instrument.openingHours.sun = openingHours["sun"].get<std::vector<std::string>>();
                            marketDetail.instrument.openingHours.zone = openingHours["zone"].get<std::string>();
                        }

                        // Overnight fee details
                        auto overnightFeeData = instrumentData["overnightFee"];
                        marketDetail.instrument.overnightFee.longRate = overnightFeeData["longRate"].get<double>();
                        marketDetail.instrument.overnightFee.shortRate = overnightFeeData["shortRate"].get<double>();
                        marketDetail.instrument.overnightFee.swapChargeTimestamp = overnightFeeData["swapChargeTimestamp"].get<long long>();
                        marketDetail.instrument.overnightFee.swapChargeInterval = overnightFeeData["swapChargeInterval"].get<int>();

                     //   std::cout << "Instrument" << std::endl;

                        // Dealing rules
                        marketDetail.dealingRules.minStepDistance.unit = dealingRulesData["minStepDistance"]["unit"].get<std::string>();
                        marketDetail.dealingRules.minStepDistance.value = dealingRulesData["minStepDistance"]["value"].get<double>();

                        marketDetail.dealingRules.minDealSize.unit = dealingRulesData["minDealSize"]["unit"].get<std::string>();
                        marketDetail.dealingRules.minDealSize.value = dealingRulesData["minDealSize"]["value"].get<double>();

                        marketDetail.dealingRules.maxDealSize.unit = dealingRulesData["maxDealSize"]["unit"].get<std::string>();
                        marketDetail.dealingRules.maxDealSize.value = dealingRulesData["maxDealSize"]["value"].get<double>();

                        marketDetail.dealingRules.minSizeIncrement.unit = dealingRulesData["minSizeIncrement"]["unit"].get<std::string>();
                        marketDetail.dealingRules.minSizeIncrement.value = dealingRulesData["minSizeIncrement"]["value"].get<double>();

                        marketDetail.dealingRules.minGuaranteedStopDistance.unit = dealingRulesData["minGuaranteedStopDistance"]["unit"].get<std::string>();
                        marketDetail.dealingRules.minGuaranteedStopDistance.value = dealingRulesData["minGuaranteedStopDistance"]["value"].get<double>();

                        marketDetail.dealingRules.minStopOrProfitDistance.unit = dealingRulesData["minStopOrProfitDistance"]["unit"].get<std::string>();
                        marketDetail.dealingRules.minStopOrProfitDistance.value = dealingRulesData["minStopOrProfitDistance"]["value"].get<double>();

                        marketDetail.dealingRules.maxStopOrProfitDistance.unit = dealingRulesData["maxStopOrProfitDistance"]["unit"].get<std::string>();
                        marketDetail.dealingRules.maxStopOrProfitDistance.value = dealingRulesData["maxStopOrProfitDistance"]["value"].get<double>();

                        marketDetail.dealingRules.marketOrderPreference = dealingRulesData["marketOrderPreference"].get<std::string>();
                        marketDetail.dealingRules.trailingStopsPreference = dealingRulesData["trailingStopsPreference"].get<std::string>();

                      //  std::cout << "dealingRules" << std::endl;

                        // Add the marketDetail to the vector
                        marketDetails = marketDetail;
                 
            }
            catch (const std::exception& e) {
                // Handle JSON parsing errors
                std::cerr << "Error parsing JSON: " << e.what() << std::endl;
                return false;
            }

            return true;
        }
};

    class Account {
public:
    class Balance {  // Nested Balance class
    public:
        double balance;
        double deposit;
        double profitLoss;
        double available;

        // Default constructor
        Balance()
            : balance(0), available(0) {}

        Balance(double bal, double dep, double pLoss, double avail)
            : balance(bal), deposit(dep), profitLoss(pLoss), available(avail) {}
    };

public:
    std::string accountId;
    std::string accountName;
    std::string status;
    std::string accountType;
    bool preferred;
    Balance balance;  // Member of Account
    std::string currency;
    std::string symbol;

    // Default constructor
    Account()
        : accountId(""), symbol("") {}

    // Constructor to initialize all members including the nested Balance class
    Account(const std::string& id, const std::string& name, const std::string& stat, const std::string& type, bool pref,
        const Balance& bal, const std::string& curr, const std::string& sym)
        : accountId(id), accountName(name), status(stat), accountType(type), preferred(pref),
        balance(bal), currency(curr), symbol(sym) {}

    // Static function to parse JSON response into a vector of Account objects
    static bool ParseFromJson(const std::string& jsonResponse, std::vector<Account>& accountList) {
        try {
            // Parse the JSON response
            nlohmann::json jsonResponseParsed = nlohmann::json::parse(jsonResponse);

            if (!jsonResponseParsed.contains("accounts")) return false;  // Check for the presence of "accounts"
            auto accounts = jsonResponseParsed["accounts"];

            // Ensure the JSON response is an array
            if (accounts.is_array()) {
                // Clear the accountList vector
                accountList.clear();
                int failedCount = 0;  // To keep track of failures

                // Iterate through the JSON array and populate Account objects
                for (const auto& jsonItem : accounts) {
                    try {
                        // Extracting balance details from the JSON
                        Balance bal(
                            jsonItem["balance"]["balance"].get<double>(),
                            jsonItem["balance"]["deposit"].get<double>(),
                            jsonItem["balance"]["profitLoss"].get<double>(),
                            jsonItem["balance"]["available"].get<double>()
                        );

                        // Creating an Account object
                        Account account(
                            jsonItem["accountId"].get<std::string>(),
                            jsonItem["accountName"].get<std::string>(),
                            jsonItem["status"].get<std::string>(),
                            jsonItem["accountType"].get<std::string>(),
                            jsonItem["preferred"].get<bool>(),
                            bal,
                            jsonItem["currency"].get<std::string>(),
                            jsonItem["symbol"].get<std::string>()
                        );

                        // Add the account to the list
                        accountList.push_back(account);
                    }
                    catch (const std::exception& e) {
                        std::cerr << "Error parsing account item: " << e.what() << std::endl;
                        failedCount++;
                        continue;
                    }
                }

                // Debugging information
                std::cout << "Number of successfully parsed Account items: " << accountList.size() << std::endl;
                std::cout << "Number of failed items: " << failedCount << std::endl;
            }
            else {
                std::cerr << "Unexpected JSON format: 'accounts' is not an array." << std::endl;
                return false;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error parsing JSON response: " << e.what() << std::endl;
            return false;
        }

        return true;
    }
};
    
    class SessionDetail {
    public:
        std::string clientId;
        std::string accountId;
        int timezoneOffset;   // Assuming this is an integer
        std::string locale;
        std::string currency;
        std::string streamEndpoint;

        // Default constructor
        SessionDetail()
            : clientId(""), streamEndpoint("") {}

        // Constructor to initialize all members
        SessionDetail(const std::string& clientId, const std::string& accountId,
            int timezoneOffset, const std::string& locale,
            const std::string& currency, const std::string& streamEndpoint)
            : clientId(clientId), accountId(accountId),
            timezoneOffset(timezoneOffset), locale(locale),
            currency(currency), streamEndpoint(streamEndpoint) {}

        // Static function to parse JSON response into a SessionDetail object
        static bool ParseFromJson(const std::string& jsonResponse, SessionDetail& sessionDetail) {
            try {
                // Parse the JSON response
                nlohmann::json jsonResponseParsed = nlohmann::json::parse(jsonResponse);

                // Extract fields
                sessionDetail.clientId = jsonResponseParsed.at("clientId").get<std::string>();
                sessionDetail.accountId = jsonResponseParsed.at("accountId").get<std::string>();
                sessionDetail.timezoneOffset = jsonResponseParsed.at("timezoneOffset").get<int>();
                sessionDetail.locale = jsonResponseParsed.at("locale").get<std::string>();
                sessionDetail.currency = jsonResponseParsed.at("currency").get<std::string>();
                sessionDetail.streamEndpoint = jsonResponseParsed.at("streamEndpoint").get<std::string>();

                return true;  // Return true on successful parsing
            }
            catch (const std::exception& e) {
                std::cerr << "Error parsing JSON response: " << e.what() << std::endl;
                return false;  // Return false if parsing fails
            }
        }
    };

    class PriceHistoryData {
    public:
        std::string snapshotTime;
        std::string snapshotTimeUTC;

        struct PriceData {
            double bid;
            double ask;
        };

        PriceData openPrice;
        PriceData closePrice;
        PriceData highPrice;
        PriceData lowPrice;

        int lastTradedVolume;

        PriceHistoryData() : lastTradedVolume(0) {};

        static bool fromJson(const nlohmann::json& jsonItem, PriceHistoryData& price) {
            try {
                price.snapshotTime = jsonItem.at("snapshotTime").get<std::string>();
                price.snapshotTimeUTC = jsonItem.at("snapshotTimeUTC").get<std::string>();

                auto openPrice = jsonItem.at("openPrice");
                price.openPrice.bid = openPrice.at("bid").get<double>();
                price.openPrice.ask = openPrice.at("ask").get<double>();

                auto closePrice = jsonItem.at("closePrice");
                price.closePrice.bid = closePrice.at("bid").get<double>();
                price.closePrice.ask = closePrice.at("ask").get<double>();

                auto highPrice = jsonItem.at("highPrice");
                price.highPrice.bid = highPrice.at("bid").get<double>();
                price.highPrice.ask = highPrice.at("ask").get<double>();

                auto lowPrice = jsonItem.at("lowPrice");
                price.lowPrice.bid = lowPrice.at("bid").get<double>();
                price.lowPrice.ask = lowPrice.at("ask").get<double>();

                price.lastTradedVolume = jsonItem.at("lastTradedVolume").get<int>();

                return true;
            }
            catch (const std::exception& e) {
                std::cerr << "Error parsing price data: " << e.what() << std::endl;
                return false;
            }
        }

        static bool ParseFromJson(const std::string& jsonResponse, std::vector<PriceHistoryData>& priceList) {
            try {
                // Parse the JSON response
                nlohmann::json jsonResponseParsed = nlohmann::json::parse(jsonResponse);

                if (!jsonResponseParsed.contains("prices")) return false;
                auto prices = jsonResponseParsed["prices"];

                // Ensure the JSON response is an array
                if (prices.is_array()) {
                    // Clear the priceList vector
                    priceList.clear();

                    int failedCount = 0;  // To keep track of failures

                    // Iterate through the JSON array and populate Price objects
                    for (const auto& jsonItem : prices) {
                        PriceHistoryData price;
                        if (PriceHistoryData::fromJson(jsonItem, price)) {
                            priceList.push_back(price);
                        }
                        else {
                            failedCount++;
                        }
                    }

                }
                else {
                    std::cerr << "Unexpected JSON format: 'prices' is not an array." << std::endl;
                    return false;
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Error parsing JSON response: " << e.what() << std::endl;
                return false;
            }

            return true;
        }
    };

    // Implementations are included within the class definition
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
        //LOG("WriteCallback");
        size_t total_size = size * nmemb;
        output->append(reinterpret_cast<char*>(contents), total_size);
        return total_size;
    }

    static size_t HeaderCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        //LOG("HeaderCallback");
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

    bool CurlIn(CURL*& curl)
    {
       // LOG("CurlIn");
        CURLcode res;
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        if (!curl)
            return 0;
        return 1;
    }

    bool CurlClean(CURL*& curl, struct curl_slist*& headers)
    {
      //  LOG("CurlClean");
        // Cleanup
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return 1;
    }

    bool CurlReq(const std::string& url ,const std::string& postFields, curl_slist* headers, std::string& response, std::string& headerData, const std::string& requestType) {
        LOG("CurlReq called with parameters: "
            "URL: " + url + ", "
            "Post Fields: " + postFields + ", "
            "Headers: " + (headers ? "Provided" : "None") + ", "
            "Request Type: " + requestType);
        CURL* curl;
        if (!CurlIn(curl)) return 0;

        if (curl == nullptr) {
            std::cerr << "Invalid CURL handle" << std::endl;
            return false;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        if (headers) {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerData);

        if (requestType == "POST") {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
        }
        else if (requestType == "GET") {
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        }
        else if (requestType == "DELETE") {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        }
        else if (requestType == "PUT") {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
        }
        else {
            std::cerr << "Unsupported request type: " << requestType << std::endl;
            CurlClean(curl, headers);
            return false;
        }

        CURLcode res = curl_easy_perform(curl);
        LOG("Curle finish cdoe is: " + std::to_string(res));
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            CurlClean(curl, headers);
            return false;
        }

        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        //  std::cout << "HTTP Response Code: " << response_code << std::endl;

        CurlClean(curl, headers);
        LOG("CURL END");
        return true;
    }


    bool CreateSession() {
        LOG("CreateSession start");
        std::string response;
        std::string headerData;

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("X-CAP-API-KEY: " + account.api_key).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        json body_json = { {"identifier", account.email}, {"password", account.password} };
        std::string body = body_json.dump();

        bool success = CurlReq(session_url, body, headers, response, headerData, "POST");

        LOG(response);

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
        LOG("CST Token: " + cstToken  + " Security Token: " + securityToken);
        return true;
    }

    bool logoutSession() {
        std::string response, headerData;

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("X-SECURITY-TOKEN: " + securityToken).c_str());
        headers = curl_slist_append(headers, ("CST: " + cstToken).c_str());

        bool success = CurlReq(session_url, "", headers, response,headerData, "DELETE");

        return success;
    }

    // Fetch the current price of an asset
    bool GetSingleMarket(std::string epic, SingleMarketDetail& epicinfo) {
        LOG("GetSingleMarket: " + epic);
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;


        headers = curl_slist_append(headers, ("X-SECURITY-TOKEN: " + securityToken).c_str());
        headers = curl_slist_append(headers, ("CST: " + cstToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        bool success = CurlReq(markets_url + epic, "", headers, response, headerData, "GET");
        LOG(response);
        if (!success) {
            std::cerr << "Failed to perform curl request for price fetching." << std::endl;
            return false;
        }

        // Debug: Print raw response and header data
        //std::cout << "Price Fetch Response: " << response << std::endl;

        if (SingleMarketDetail::ParseFromJson(response, epicinfo)) {
            return true;
        }
        return true;
    }

    // Fetch the current price of an asset
    bool GetSingleMarket(double& CurrentSellPrice, double& CurrentBuyPrice, double& minDealSizeOut, std::string epic) {
        LOG("GetSingleMarket: " + epic);
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;


        headers = curl_slist_append(headers, ("X-SECURITY-TOKEN: " + securityToken).c_str());
        headers = curl_slist_append(headers, ("CST: " + cstToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        bool success = CurlReq(price_url + epic, "", headers, response, headerData, "GET");
        LOG(response);
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
            if (response_json.contains("dealingRules")) {
                const auto& dealingRules = response_json["dealingRules"];
                if (dealingRules.contains("minDealSize")) {
                    const auto& minDealSize = dealingRules["minDealSize"];
                    if (minDealSize.contains("value")) {
                        minDealSizeOut = minDealSize["value"].get<double>();
                    }
                }
            }
        }
        catch (const nlohmann::json::exception& e) {
            std::cerr << "JSON parsing error: " << e.what() << std::endl;
        }
        return true;
    }

    bool fetchPriceHistory(std::string from, std::string to, std::string epic, std::string resolution, int max, std::vector<PriceHistoryData>& pricehistory) {
        LOG("fetchPriceHistory called with parameters: "
            "From: " + from + ", "
            "To: " + to + ", "
            "Epic: " + epic + ", "
            "Resolution: " + resolution + ", "
            "Max: " + std::to_string(max));
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;


        headers = curl_slist_append(headers, ("X-SECURITY-TOKEN: " + securityToken).c_str());
        headers = curl_slist_append(headers, ("CST: " + cstToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        std::string url;
        if (from == "-1" || to == "-1")
            url = base_url + "/api/v1/prices/" + epic + "?resolution=" + resolution + "&max=" + std::to_string(max);
        else
        {
            url = base_url + "/api/v1/prices/" + epic + "?resolution=" + resolution + "&max=1000&from=" + from + "&to=" + to;
        }
        LOG(url);

        bool success = CurlReq(url, "", headers, response, headerData, "GET");

        LOG(response);
        if (!success) {
            //std::cerr << "Failed to perform curl request for price fetching." << std::endl;
            return false;
        }

        // Debug: Print raw response and header data
       // std::cout << "Price Fetch Response: " << response << std::endl;


        if (PriceHistoryData::ParseFromJson(response, pricehistory)) {
            return true;
        }


        return true;
    }

    bool CreatePosition(const std::string& direction, double size, std::string epic) {
        LOG("CreatePosition called with parameters: "
            "Direction: " + direction + ", "
            "Size: " + std::to_string(size) + ", "
            "Epic: " + epic);
        std::string response, headerData;
        struct curl_slist* headers = nullptr;

        headers = curl_slist_append(headers, ("X-SECURITY-TOKEN: " + securityToken).c_str());
        headers = curl_slist_append(headers, ("CST: " + cstToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        json body_json = {
            {"epic", epic},
            {"direction", direction},
            {"size", size}
        };

        std::string body = body_json.dump();

        if (!CurlReq(positions_url, body, headers, response,headerData, "POST")) {
            return false;
        }
        LOG(response);

        try {
            json response_json = json::parse(response);
            if (response_json.contains("errorCode")) {
                std::cerr << "Error in order creation: " << response_json["errorCode"] << std::endl;
                return false;
            }
            else {
                if (direction == "BUY") {
                    std::cout << ui.color_green << "Buy order created successfully." << ui.color_reset << std::endl;
                    if (!response_json["dealReference"].is_null()) {
                    }
                    std::cout << "Deal Reference: " << response_json["dealReference"].get<std::string>() << std::endl;
                    return true;
                }
                else {
                    std::cout << ui.color_red << "Sell order completed successfully." << ui.color_reset << std::endl;
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

    bool GetEpics(std::vector<MarketData>& markets)
    {
        LOG("GetEpics");
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;


        headers = curl_slist_append(headers, ("X-SECURITY-TOKEN: " + securityToken).c_str());
        headers = curl_slist_append(headers, ("CST: " + cstToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        bool success = CurlReq(markets_url, "", headers, response, headerData, "GET");
        LOG(response);
        if (!success) {
            //std::cerr << "Failed to perform curl request for price fetching." << std::endl;
            return false;
        }

        if (MarketData::ParseFromJson(response, markets)) {
            return true;
        }
        else {
            //std::cerr << "Failed to parse JSON response." << std::endl;
            return false;
        }
    }

    bool GetAllActivePositions(std::vector<TradePosition>& openpositions)
    {
        LOG("GetAllActivePositions");
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;


        headers = curl_slist_append(headers, ("X-SECURITY-TOKEN: " + securityToken).c_str());
        headers = curl_slist_append(headers, ("CST: " + cstToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");   

        bool success = CurlReq(positions_url, "", headers, response, headerData, "GET");
        LOG(response);
        if (!success) {
            //std::cerr << "Failed to perform curl request for price fetching." << std::endl;
            return false;
        }

        if (TradePosition::ParseFromJsonArray(response, openpositions)) {
            return true;
        }

        return true;
    }

    bool GetSingleMarketInfo(SingleMarketDetail& singlemarkets,std::string epic)
    {
        LOG("GetSingleMarketInfo: " + epic);
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;


        headers = curl_slist_append(headers, ("X-SECURITY-TOKEN: " + securityToken).c_str());
        headers = curl_slist_append(headers, ("CST: " + cstToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        bool success = CurlReq(markets_url + epic, "", headers, response, headerData, "GET");
        LOG(response);
        if (!success) {
            //std::cerr << "Failed to perform curl request for price fetching." << std::endl;
            return false;
        }
        if (SingleMarketDetail::ParseFromJson(response, singlemarkets)) {
            return true;
        }
    }

    bool GetSinglePosition(TradePosition& singleposition,std::string dealid)
    {
        LOG("GetSinglePosition: " + dealid);
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;


        headers = curl_slist_append(headers, ("X-SECURITY-TOKEN: " + securityToken).c_str());
        headers = curl_slist_append(headers, ("CST: " + cstToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        bool success = CurlReq(positions_url + dealid, "", headers, response, headerData, "GET");
        LOG(response);
        if (!success) {
            //std::cerr << "Failed to perform curl request for price fetching." << std::endl;
            return false;
        }
        if (TradePosition::ParseFromJson(response, singleposition)) {
            return true;
        }
    }

    bool GetSessionInfo(SessionDetail& sessiondetail)
    {
        LOG("GetSessionInfo");
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;


        headers = curl_slist_append(headers, ("X-SECURITY-TOKEN: " + securityToken).c_str());
        headers = curl_slist_append(headers, ("CST: " + cstToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");


        bool success = CurlReq(session_url, "", headers, response, headerData, "GET");
        LOG(response);
        if (!success) {
            //std::cerr << "Failed to perform curl request for price fetching." << std::endl;
            return false;
        }

        if (SessionDetail::ParseFromJson(response, sessiondetail)) {
            return true;
        }
       

        return success;
    }

    bool GetAllAccounts(std::vector<Account>& accouts)
    {
        LOG("GetAllAccounts");
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;


        headers = curl_slist_append(headers, ("X-SECURITY-TOKEN: " + securityToken).c_str());
        headers = curl_slist_append(headers, ("CST: " + cstToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");


        bool success = CurlReq(accounts_url, "", headers, response, headerData, "GET");
        LOG(response);
        if (!success) {
            //std::cerr << "Failed to perform curl request for price fetching." << std::endl;
            return false;
        }

        if (Account::ParseFromJson(response, accouts)) {
            return true;
        }

        return success;
    }

    bool SwitchAccount(std::string accountId)
    {
        LOG("SwitchAccount: " + accountId);
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;


        headers = curl_slist_append(headers, ("X-SECURITY-TOKEN: " + securityToken).c_str());
        headers = curl_slist_append(headers, ("CST: " + cstToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        std::string requestBody = "{ \"accountId\": \"" + accountId + "\" }";

        bool success = CurlReq(session_url, requestBody, headers, response, headerData, "PUT");
        LOG(response);
        if (!success) {
            //std::cerr << "Failed to perform curl request for price fetching." << std::endl;
            return false;
        }
        securityToken.erase();
        cstToken.erase();
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
    }
    bool PingSession()
    {
        LOG("PingSession");
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;


        headers = curl_slist_append(headers, ("X-SECURITY-TOKEN: " + securityToken).c_str());
        headers = curl_slist_append(headers, ("CST: " + cstToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        bool success = CurlReq(ping_url, "", headers, response, headerData, "GET");
        LOG(response);
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

    bool ClosePosition(std::string dealId)
    {
        LOG("ClosePosition: " + dealId);
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;


        headers = curl_slist_append(headers, ("X-SECURITY-TOKEN: " + securityToken).c_str());
        headers = curl_slist_append(headers, ("CST: " + cstToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        bool status = CurlReq(positions_url + dealId, "", headers, response, headerData, "DELETE");

        LOG(response);
        return status;
    }


    bool updatePosition(std::string dealId, bool guaranteedStop, bool trailingStop, int stopDistance, int profitDistance)
    {
        LOG("updatePosition");

        /*
        
        NOT WORKING / IT IS BUT CAPITAL GAY SO DONT USE IT
        stoplose cant go profit and somethimes error when trying to create it in capital, api sucsess full
            
        */

        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;


        headers = curl_slist_append(headers, ("X-SECURITY-TOKEN: " + securityToken).c_str());
        headers = curl_slist_append(headers, ("CST: " + cstToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        
        std::string requestBody = "{ "
            "\"guaranteedStop\": " + std::string(guaranteedStop ? "true" : "false") + ", "
            "\"trailingStop\": " + std::string(trailingStop ? "true" : "false") + ", "
            "\"stopDistance\": " + std::to_string(stopDistance) + ", "
            "\"profitDistance\": " + std::to_string(profitDistance) +
            "}";


        bool result = CurlReq(positions_url + dealId, requestBody, headers, response, headerData, "PUT");
        LOG(response);
        return result;
    }


    
};
