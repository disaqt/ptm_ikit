#pragma once
#include <windows.h>
#include <d3dcompiler.h>
#include <d3d11.h>
#include <dxgi.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <string>
#include <vector>

#include <Meter.hpp>
#include <json.hpp> 

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

    bool isVisible() const { return visible; }
    void setVisible(bool value) { visible = value; }

    void show(std::vector<IMeter*>& meters);

private:
    void resetForm();
};