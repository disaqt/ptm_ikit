#pragma once
#include <iostream>
#include <string>
#include <sstream>

class IMeter {
public:
    struct Date {
        int year;
        int month;
        int day;
        friend std::ostream& operator<<(std::ostream& os, const Date& date) {
            os << date.year << "-" << date.month << "-" << date.day;
            return os;
        }
    };

protected:
    Date _verification_date;
    double _value;
    std::string _resource_type;

public:
    IMeter(const std::string& resource_type, const Date& verification_date, double value)
        : _resource_type(resource_type), _verification_date(verification_date), _value(value) {
    }

    virtual std::string to_string() const {
        std::stringstream ss;
        ss << "Type: " << _resource_type << ", Date: " << _verification_date << ", Value: " << _value;
        return ss.str();
    }

    Date get_verification_date() const { return _verification_date; }
    double get_value() const { return _value; }
    std::string get_resource_type() const { return _resource_type; }

    virtual bool parse(const std::string& data) { return false; }

    virtual ~IMeter() {}
};

class CWaterMeter : public IMeter {
public:
    enum class WaterType { Cold, Warm, Unknown };

private:
    WaterType _water_type;

public:
    CWaterMeter(const Date& verification_date, double value, WaterType water_type)
        : IMeter("Water", verification_date, value), _water_type(water_type) {
    }

    std::string to_string() const override {
        std::stringstream ss;
        ss << IMeter::to_string() << ", Water Type: " << (_water_type == WaterType::Cold ? "Cold" : "Warm");
        return ss.str();
    }
};

class CElectricMeter : public IMeter {
private:
    double _voltage;
    double _current;

public:
    CElectricMeter(const Date& verification_date, double value, double voltage, double current)
        : IMeter("Electricity", verification_date, value), _voltage(voltage), _current(current) {
    }

    std::string to_string() const override {
        std::stringstream ss;
        ss << IMeter::to_string() << ", Voltage: " << _voltage << "V, Current: " << _current << "A";
        return ss.str();
    }

    double get_voltage() const { return _voltage; }
    double get_current() const { return _current; }
};

class CGasMeter : public IMeter {
private:
    std::string _gas_type;
    double _pressure;

public:
    CGasMeter(const Date& verification_date, double value, const std::string& gas_type, double pressure)
        : IMeter("Gas", verification_date, value), _gas_type(gas_type), _pressure(pressure) {
    }

    std::string to_string() const override {
        std::stringstream ss;
        ss << IMeter::to_string() << ", Gas Type: " << _gas_type << ", Pressure: " << _pressure << " kPa";
        return ss.str();
    }

    std::string get_gas_type() const { return _gas_type; }
    double get_pressure() const { return _pressure; }
};