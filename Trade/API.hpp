#include "Account.hpp"
#include "UI.hpp"
#pragma comment(lib, "wldap32.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#define CURL_STATICLIB
#include <curl/curl.h>
#include <nlohmann/json.hpp>



UI ui;
Account account;

using json = nlohmann::json;

class API
{
public:
    std::string base_url = "https://demo-api-capital.backend-capital.com";
    std::string order_url = base_url + "/api/v1/positions";
    std::string session_url = base_url + "/api/v1/session";
    std::string price_url = base_url + "/api/v1/markets/";
    std::string securityToken;
    std::string cstToken;
    bool sessioncreated = false;
    std::time_t sessioncreatetime;

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

                        marketDetail.snapshot.updateTime = snapshotData["updateTime"].get<std::string>();

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

    bool CurlIn(CURL*& curl)
    {
        CURLcode res;
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        if (!curl)
            return 0;
        return 1;
    }

    bool CurlClean(CURL*& curl, struct curl_slist*& headers)
    {
        // Cleanup
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return 1;
    }

    bool CurlReq(const std::string& url ,const std::string& postFields, curl_slist* headers, std::string& response, std::string& headerData, const std::string& requestType) {

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
        else {
            std::cerr << "Unsupported request type: " << requestType << std::endl;
            CurlClean(curl, headers);
            return false;
        }

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            CurlClean(curl, headers);
            return false;
        }

        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        //  std::cout << "HTTP Response Code: " << response_code << std::endl;

        CurlClean(curl, headers);
        return true;
    }


    bool CreateSession() {
        std::string response;
        std::string headerData;

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("X-CAP-API-KEY: " + account.api_key).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        json body_json = { {"identifier", account.email}, {"password", account.password} };
        std::string body = body_json.dump();

        bool success = CurlReq(session_url, body, headers, response, headerData, "POST");

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
        std::string url = "https://demo-api-capital.backend-capital.com/api/v1/session";
        std::string response, headerData;

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("X-SECURITY-TOKEN: " + securityToken).c_str());
        headers = curl_slist_append(headers, ("CST: " + cstToken).c_str());

        bool success = CurlReq(url, "", headers, response,headerData, "DELETE");

        return success;
    }

    // Fetch the current price of an asset
    bool fetchPrice(double &CurrentSellPrice, double &CurrentBuyPrice,double& minDealSizeOut,std::string epic) {
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;


        headers = curl_slist_append(headers, ("X-SECURITY-TOKEN: " + securityToken).c_str());
        headers = curl_slist_append(headers, ("CST: " + cstToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        bool success = CurlReq(price_url + epic, "", headers, response,headerData, "GET");

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

    bool fetchPriceHistory(std::string from, std::string to,std::string epic, std::vector<double>& pricehistory) {

        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;


        headers = curl_slist_append(headers, ("X-SECURITY-TOKEN: " + securityToken).c_str());
        headers = curl_slist_append(headers, ("CST: " + cstToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        std::string pricehistory_url = base_url + "/api/v1/prices/" + epic + "?resolution=MINUTE&max=1000&from=" + from + "&to=" + to;

        bool success = CurlReq(pricehistory_url, "", headers, response,headerData, "GET");

        if (!success) {
            //std::cerr << "Failed to perform curl request for price fetching." << std::endl;
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
                        pricehistory.push_back(price["closePrice"]["bid"].get<double>());
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

    bool Order(const std::string& direction, double size, std::string epic, std::string& dealReference) {

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

        if (!dealReference.empty()) {
            body_json["dealReference"] = dealReference;
        }

        std::string body = body_json.dump();

        if (!CurlReq(order_url, body, headers, response,headerData, "POST")) {
            return false;
        }

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
                        dealReference = response_json["dealReference"].get<std::string>();
                    }
                    std::cout << "Deal Reference: " << dealReference << std::endl;
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
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;


        headers = curl_slist_append(headers, ("X-SECURITY-TOKEN: " + securityToken).c_str());
        headers = curl_slist_append(headers, ("CST: " + cstToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        std::string markets_url = "https://demo-api-capital.backend-capital.com/api/v1/markets";

        bool success = CurlReq(markets_url, "", headers, response, headerData, "GET");

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
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;


        headers = curl_slist_append(headers, ("X-SECURITY-TOKEN: " + securityToken).c_str());
        headers = curl_slist_append(headers, ("CST: " + cstToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");   

        bool success = CurlReq(order_url, "", headers, response, headerData, "GET");

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
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;


        headers = curl_slist_append(headers, ("X-SECURITY-TOKEN: " + securityToken).c_str());
        headers = curl_slist_append(headers, ("CST: " + cstToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        std::string singlemarketurl = "https://demo-api-capital.backend-capital.com/api/v1/markets/" + epic;

        bool success = CurlReq(singlemarketurl, "", headers, response, headerData, "GET");

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
        std::string response;
        std::string headerData;
        struct curl_slist* headers = NULL;


        headers = curl_slist_append(headers, ("X-SECURITY-TOKEN: " + securityToken).c_str());
        headers = curl_slist_append(headers, ("CST: " + cstToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        std::string url = "https://demo-api-capital.backend-capital.com/api/v1/positions/" + dealid;

        bool success = CurlReq(url, "", headers, response, headerData, "GET");

        if (!success) {
            //std::cerr << "Failed to perform curl request for price fetching." << std::endl;
            return false;
        }
        if (TradePosition::ParseFromJson(response, singleposition)) {
            return true;
        }
    }
};
