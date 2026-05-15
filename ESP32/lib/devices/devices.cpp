#include "devices.h"

devices::devices()
{}

void devices::set_state(bool st)
{
    if (st)
    {
        this->state = 100;
    }
    else
    {
        this->state = 0;
    }
    this->binary = true;
}

void devices::set_state(double st)
{
    this->state = st;
    this->binary = false;
}

String devices::get_state()
{
    if (this->binary)
    {
        bool temp;
        if (this->state == 0)
        {
            temp = false;
        }
        else
        {
            temp = true;
        }
        if (this->device_class == "moisture")
        {
            return temp? "wet" : "dry";
        }
        else if (this->device_class == "battery")
        {
            return temp? "low" : "normal";
        }
        else if (this->device_class == "battery_charging")
        {
            return temp? "charging" : "not charging";
        }
        else if (this->device_class == "carbon_monoxide" || this->device_class == "gas" || this->device_class == "motion" || this->device_class == "moving" || this->device_class == "occupancy" || this->device_class == "problem" || this->device_class == "smoke" || this->device_class == "sound" || this->device_class == "tamper" || this->device_class == "vibration")
        {
            return temp? "detected" : "clear";
        }
        else if (this->device_class == "cold")
        {
            return temp? "cold" : "normal";
        }
        else if (this->device_class == "heat")
        {
            return temp? "hot" : "normal";
        }
        else if (this->device_class == "connectivity")
        {
            return temp? "connected" : "disconnected";
        }
        else if (this->device_class == "door" || this->device_class == "garage_door" || this->device_class == "garage" || this->device_class == "opening" || this->device_class == "window")
        {
            return temp? "open" : "closed";
        }
        else if (this->device_class == "lock")
        {
            return temp? "locked" : "unlocked";
        }
        else if (this->device_class == "presence")
        {
            return temp? "home" : "away";
        }
        else
        {
            return temp? "on" : "off";
        }
    }
    else
    {
        String temp;
        if (this->state == (int)this->state)
        {
            temp = String((int)this->state);
        }
        else
        {
            temp = String(this->state);
        }
        if (this->unit_of_measurement != "")
        {
            if (this->unit_of_measurement != "%")
            {
                temp += " ";
            }
            temp += this->unit_of_measurement;
        }
        return temp;
    }
}

void devices::set_updated(String tie)
{
    const char* format = "%Y-%m-%dT%H:%M:%S%z";
    struct tm tm;
    strptime(tie.c_str(), format, &tm);
    last_updated = mktime(&tm);
}

String devices::get_updated(unsigned long current)
{
    unsigned long difference = current - this->last_updated;
    int diff = 0;
    String returndiff;
    if (difference > 31556952)
    {
        diff = difference/31556952;
        returndiff = "over ";
        returndiff += diff;
        if (diff == 1)
        {
            returndiff += " year ago";
        }
        else
        {
            returndiff += " years ago";
        }
        return returndiff;
    }
    else if (difference > 2629746)
    {
        diff = difference/2629746;
        returndiff = "over ";
        returndiff += diff;
        if (diff == 1)
        {
            returndiff += " month ago";
        }
        else
        {
            returndiff += " months ago";
        }
        return returndiff;
    }
    else if (difference > 86400)
    {
        diff = difference/86400;
        returndiff = "over ";
        returndiff += diff;
        if (diff == 1)
        {
            returndiff += " day ago";
        }
        else
        {
            returndiff += " days ago";
        }
        return returndiff;
    }
    else if (difference > 3600)
    {
        diff = difference/3600;
        int mins = (difference - diff*3600)/60;
        returndiff = diff;
        returndiff += "h ";
        if (mins > 0)
        {
            returndiff += "and ";
            returndiff += mins;
            returndiff += "m ";
        }
        returndiff += "ago";
        return returndiff;
    }
    else if (difference > 60)
    {
        diff = difference/60;
        int secs = difference - diff*60;
        returndiff = diff;
        returndiff += "m ";
        if (secs > 0)
        {
            returndiff += "and ";
            returndiff += secs;
            returndiff += "s ";
        }
        returndiff += "ago";
        return returndiff;
    }
    else
    {
        returndiff = difference;
        returndiff += "s ago";
        return returndiff;
    }
}

bool devices::get_warning(unsigned long current)
{
    if (this->manual_warning == '1')
    {
        return true;
    }
    else if (this->manual_warning == '0')
    {
        return false;
    }
    unsigned long difference = current - this->last_updated;
    if
    (
        (!this->binary  &&  this->device_class == "battery"             && this->state <=   20                      ) ||
        ( this->binary  &&  this->device_class == "battery"             && this->state !=   0                       ) ||
        ( this->binary  &&  this->device_class == "moisture"            && this->state !=   0                       ) ||
        (                   this->device_class == "garage_door"         && this->state !=   0 && difference > 300   ) ||
        (                   this->device_class == "garage"              && this->state !=   0 && difference > 300   ) ||
        (                   this->device_class == "gate"                && this->state !=   0 && difference > 300   ) ||
        (                   this->device_class == "door"                && this->state !=   0 && difference > 300   ) ||
        (                   this->device_class == "window"              && this->state !=   0 && difference > 600   ) ||
        ( this->binary  &&  this->device_class == "carbon_monoxide"     && this->state !=   0                       ) ||
        (!this->binary  &&  this->device_class == "carbon_dioxide"      && this->state >=   1000                    ) ||
        (!this->binary  &&  this->device_class == "carbon_monoxide"     && this->state >=   50                      ) ||
        (!this->binary  &&  this->device_class == "nitrogen_dioxide"    && this->state >=   100                     ) ||
        (!this->binary  &&  this->device_class == "nitrogen_monoxide"   && this->state >=   40                      ) ||
        (!this->binary  &&  this->device_class == "nitrous_oxide"       && this->state >=   45                      ) ||
        (!this->binary  &&  this->device_class == "ozone"               && this->state >=   100                     ) ||
        (!this->binary  &&  this->device_class == "sulphur_dioxide"     && this->state >=   120                     ) ||
        ( this->binary  &&  this->device_class == "gas"                 && this->state !=   0                       ) ||
        (                   this->device_class == "smoke"               && this->state !=   0                       ) ||
        (                   this->device_class == "problem"             && this->state !=   0                       )
    )
    {
        return true;
    }
    else
    {
        return false;
    }
}

void devices::set_warning(char warn)
{
    this->manual_warning = warn;
}

bool devices::operator<(const devices& x) const  
{
    if (this->area_name != "" && x.area_name == "")
    {
        return true;
    }
    if (this->area_name == x.area_name)
    {
        return this->friendly_name < x.friendly_name;
    }
    else
    {
        return this->area_name < x.area_name;
    }
}

devices::~devices()
{}
