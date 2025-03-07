#pragma once

#include <Arduino.h>

enum IrKey {
    K1, K2, K3, 
    K4, K5, K6, 
    K7, K8, K9, 
    K0, KStar, KHash, 
    KUp, KDown, KLeft, KRight,
    KOk, KOther,
};

String getKeyString(IrKey key);
