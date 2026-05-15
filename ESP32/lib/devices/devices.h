#ifndef DEVICES_H
#define DEVICES_H

#include <Arduino.h>
#include <time.h>

class devices
{
    double state;
    unsigned long last_updated;
    char manual_warning = '-';

public:
    String name;
    String friendly_name = "";
    String unit_of_measurement = "";
    String device_class = "";
    String area_name;
    bool binary = false;
    devices();
    void set_state(bool st);
    void set_state(double st);
    String get_state();
    void set_updated(String tie);
    String get_updated(unsigned long current);
    bool get_warning(unsigned long current);
    void set_warning(char warn);
    bool operator<(const devices& x) const;
    ~devices();
};
#endif // DEVICES_H
