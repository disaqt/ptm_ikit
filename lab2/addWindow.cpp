#include <addWindow.hpp>

struct Date {
    int year;
    int month;
    int day;
};

Date parseDate2(const std::string& dateString) {
    Date date;
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

bool parseDateFromString1(const std::string& str, Date& outDate) {
    std::stringstream ss(str);
    std::string yearStr, monthStr, dayStr;

    if (!std::getline(ss, yearStr, '-') ||
        !std::getline(ss, monthStr, '-') ||
        !std::getline(ss, dayStr, '-')) {
        return false;
    }

    try {
        outDate.year = std::stoi(yearStr);
        outDate.month = std::stoi(monthStr);
        outDate.day = std::stoi(dayStr);
    }
    catch (...) {
        return false;
    }

    return true;
}

class AddMeterWindow {
private:
    bool visible;
    std::string input_date_str;
    double new_value;
    std::string new_gas_type;
    double new_pressure;
    double new_voltage;
    double new_current;
    int selected_type; // 0: Water, 1: Electricity, 2: Gas
    int water_type_index;
    std::string input_error;

public:
    AddMeterWindow()
        : visible(false),
        input_date_str("2023-10-01"),
        new_value(0.0),
        new_gas_type("Co2"),
        new_pressure(0.0),
        new_voltage(5.0),
        new_current(2.0),
        selected_type(0),
        water_type_index(0) {}

    void show(std::vector<IMeter*>& meters) {
        if (!visible) return;

        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
        ImGui::Begin("Добавить объект", &visible);

        const char* types[] = { "Water", "Electricity", "Gas" };
        ImGui::Combo("Тип", &selected_type, types, IM_ARRAYSIZE(types));

        ImGui::InputText("Дата (ГГГГ-ММ-ДД)", input_date_str.data(), 20);
        ImGui::InputDouble("Значение", &new_value);

        if (selected_type == 0) {
            const char* water_types[] = { "Cold", "Warm" };
            ImGui::Combo("Тип воды", &water_type_index, water_types, IM_ARRAYSIZE(water_types));
        }
        else if (selected_type == 1) {
            ImGui::InputDouble("Напряжение", &new_voltage);
            ImGui::InputDouble("Ток", &new_current);
        }
        else if (selected_type == 2) {
            ImGui::InputText("Тип газа", new_gas_type.data(), 20);
            ImGui::InputDouble("Давление", &new_pressure);
        }

        if (ImGui::Button("Добавить")) {
            Date parsedDate;
            if (!parseDateFromString1(input_date_str, parsedDate)) {
                input_error = "Ошибка: неверный формат даты";
            }
            else {
                input_error.clear();

                IMeter* newMeter = nullptr;
                if (selected_type == 0) {
                    CWaterMeter::WaterType wt = (water_type_index == 0)
                        ? CWaterMeter::WaterType::Cold
                        : CWaterMeter::WaterType::Warm;
                    newMeter = new CWaterMeter({ parsedDate.year, parsedDate.month, parsedDate.day }, new_value, wt);
                }
                else if (selected_type == 1) {
                    newMeter = new CElectricMeter({ parsedDate.year, parsedDate.month, parsedDate.day }, new_value, new_voltage, new_current);
                }
                else if (selected_type == 2) {
                    newMeter = new CGasMeter({ parsedDate.year, parsedDate.month, parsedDate.day }, new_value, new_gas_type, new_pressure);
                }

                if (newMeter) {
                    meters.push_back(newMeter);
                    resetForm();
                }
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Отмена")) {
            visible = false;
            input_error.clear();
        }

        if (!input_error.empty()) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", input_error.c_str());
        }

        ImGui::End();
    }

private:
    void resetForm() {
        input_date_str = "2023-10-01";
        new_value = 0.0;
        new_gas_type = "Co2";
        new_pressure = 0.0;
        new_voltage = 5.0;
        new_current = 2.0;
        water_type_index = 0;
    }
};