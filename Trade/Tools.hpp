#include <string>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <time.h>
#include <cmath>
#include <thread>

class Tool
{
public:
	std::time_t stringToTimeT(const std::string& time_str) const {
		std::tm tm = {};
		std::istringstream ss(time_str);
		ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
		return std::mktime(&tm);
	}

	std::string nowtime() const {
		auto now = std::chrono::system_clock::now();
		std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

		std::tm tm = {};
		localtime_s(&tm, &now_time_t);

		std::ostringstream oss;
		oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
		return oss.str();
	}

	double CalculateTimeDifference(std::time_t current_time, std::time_t session_create_time) {
		return std::abs(std::difftime(current_time, session_create_time));
	}

	double ProcentDifferenceCalculate(double number1, double number2)
	{
		return ((number2 - number1) / number1) * 100;
	}
	float ProcentageAddOrSub(float number, float procent, const std::string& operation)
	{
		static const std::map<std::string, std::function<double(double, double)>> operations = {
	   {"+", std::plus<double>()},
	   {"-", std::minus<double>()},
		};
		auto it = operations.find(operation);
		if (it != operations.end()) {
			// Apply the operation
			float percentageDecimal = procent / 100.0;
			float increase = number * percentageDecimal;
			return it->second(number, increase);
		}
		else {
			// Handle invalid operation
			std::cerr << "Invalid operation: " << operation << std::endl;
			return 0;
		}
	}
};