#pragma once
#include "DHTesp.h"
#include "Passwords.h"

const char* HOST = "script.google.com";
const int HTTPS_PORT = 443;
const int TEMPERATURE_PIN_1 = D6;
const String GOOGLE_SCRIPT_URL(String("/macros/s/") + GOOGLE_SCRIPT_ID + "/exec");
