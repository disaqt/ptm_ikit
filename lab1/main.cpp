#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include <Meter.hpp>
#include <json.hpp> 

using json = nlohmann::json;


IMeter::Date parseDate(const std::string& dateString) {
    IMeter::Date date;
    std::stringstream ss(dateString);
    std::string yearStr, monthStr, dayStr;

    std::getline(ss, yearStr, '-');
    std::getline(ss, monthStr, '-');
    std::getline(ss, dayStr, '-');

    date.year = std::stoi(yearStr);
    date.month = std::stoi(monthStr);
    date.day = std::stoi(dayStr);

    return date;
}

int main() {
    std::vector<IMeter*> meters;

    std::ifstream file("meters.json");

    if (!file.is_open()) {
        return 1;
    }

    try {
        json j;
        file >> j;

        for (const auto& element : j) {
            std::string resourceType = element["resourceType"];
            std::string dateStr = element["date"];
            double value = element["value"];

            IMeter::Date date = parseDate(dateStr);

            if (resourceType == "Electricity") {
                double volatge = element["voltage"];
                double current = element["current"];
                meters.push_back(new CElectricMeter(date, value, volatge, current));
            } else if (resourceType == "Water") {
                std::string waterTypeStr = element.value("type", "unknown");
                CWaterMeter::WaterType waterType;
                if (waterTypeStr == "cold") {
                    waterType = CWaterMeter::WaterType::Cold;
                } else if (waterTypeStr == "warm") {
                    waterType = CWaterMeter::WaterType::Warm;
                } else {
                    waterType = CWaterMeter::WaterType::Unknown;
                }
                meters.push_back(new CWaterMeter(date, value, waterType));
            }
            else if (resourceType == "Gas") {
                std::string gasType = element["gasType"];
                double pressure = element["pressure"];
                meters.push_back(new CGasMeter(date, value, gasType, pressure));
            }
            else {
                std::cerr << "Unkown resource type: " << resourceType << std::endl;
            }
        }

        for (IMeter* meter : meters) {
            std::cout << meter->to_string() << std::endl;
        }

        for (IMeter* meter : meters) {
            delete meter;
        }
        meters.clear();

    } catch (const json::exception& e) {
        std::cerr << "Error while parsing JSON: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}