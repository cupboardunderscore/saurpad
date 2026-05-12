//generated from device.json using https://arduinojson.org/v7/assistant/

#include <ArduinoJson.h>

#include "defs.hpp"

String jsn()
{
    JsonDocument doc;

    JsonObject dev = doc["dev"].to<JsonObject>();
    dev["ids"] = "bwav1";
    dev["name"] = namestyle;
    dev["mf"] = "cupboard_";
    dev["mdl"] = defaultname;
    dev["sw"] = ver;
    dev["sn"] = "bwav1";
    dev["hw"] = hver;

    JsonObject o = doc["o"].to<JsonObject>();
    o["name"] = String(defaultname) + "mqtt";
    o["sw"] = mver;
    o["url"] = "https://github.com/cupboardunderscore/saurpad";

    JsonObject cmps = doc["cmps"].to<JsonObject>();

    JsonObject cmps_tem = cmps[String(defaultname) + "_tem"].to<JsonObject>();
    cmps_tem["p"] = "sensor";
    cmps_tem["device_class"] = "temperature";
    cmps_tem["state_class"] = "measurement";
    cmps_tem["unit_of_measurement"] = "°C";
    cmps_tem["value_template"] = "{{ value_json.temperature}}";
    cmps_tem["unique_id"] = String(defaultname) + "_t";

    JsonObject cmps_hum = cmps[String(defaultname) + "_hum"].to<JsonObject>();
    cmps_hum["p"] = "sensor";
    cmps_hum["device_class"] = "humidity";
    cmps_hum["state_class"] = "measurement";
    cmps_hum["unit_of_measurement"] = "%";
    cmps_hum["value_template"] = "{{ value_json.humidity}}";
    cmps_hum["unique_id"] = String(defaultname) + "_h";

    JsonObject cmps_lig = cmps[String(defaultname) + "_lig"].to<JsonObject>();
    cmps_lig["p"] = "sensor";
    cmps_lig["device_class"] = "illuminance";
    cmps_lig["state_class"] = "measurement";
    cmps_lig["unit_of_measurement"] = "lx";
    cmps_lig["value_template"] = "{{ value_json.illuminance}}";
    cmps_lig["unique_id"] = String(defaultname) + "_l";

    JsonObject cmps_bat = cmps[String(defaultname) + "_bat"].to<JsonObject>();
    cmps_bat["p"] = "sensor";
    cmps_bat["device_class"] = "battery";
    cmps_bat["state_class"] = "measurement";
    cmps_bat["unit_of_measurement"] = "%";
    cmps_bat["value_template"] = "{{ value_json.battery}}";
    cmps_bat["unique_id"] = String(defaultname) + "_b";

    JsonObject cmps_cha = cmps[String(defaultname) + "_cha"].to<JsonObject>();
    cmps_cha["p"] = "binary_sensor";
    cmps_cha["device_class"] = "battery_charging";
    cmps_cha["state_class"] = "measurement";
    cmps_cha["value_template"] = "{{ value_json.battery_charging}}";
    cmps_cha["unique_id"] = String(defaultname) + "_c";
    doc["state_topic"] = String(defaultname) + "/state";
    doc["qos"] = 0;

    String output;

    doc.shrinkToFit();  // optional

    serializeJson(doc, output);
    
    return output;
}
