void print_telemetry(Cotroller_Ent_telemetry &MSG)
{
    Serial.println("\n±±±±±±± Telemetry Start ±±±±±±±");
    Serial.print("Ent type, id:\t");
    Serial.print(controller.EntTypes[MSG.type]);
    Serial.print(", ");
    Serial.println(MSG.id);
    Serial.print("state:\t\t");
    Serial.println(MSG.type == WIN_ENT ? controller.winMQTTcmds[MSG.state] : controller.SW_MQTT_cmds[MSG.state]);
    Serial.print("trig:\t\t");
    Serial.println(MSG.type == WIN_ENT ? controller.WinTrigs[MSG.trig] : controller.SW_Types[MSG.trig]);
    Serial.print("timeout:\t");
    Serial.println(MSG.timeout);
    Serial.println("±±±±±±±±±±±± END  ±±±±±±±±±±±±\n");
}

void init_Serial_DBG_MODE()
{
    Serial.begin(115200);
    delay(100);
}
