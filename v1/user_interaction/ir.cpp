#include "ir.h"

String getKeyString(IrKey key) {
    switch (key) {
        case K1: return String("1");
        case K2: return String("2");
        case K3: return String("3");
        case K4: return String("4");
        case K5: return String("5");
        case K6: return String("6");
        case K7: return String("7");
        case K8: return String("8");
        case K9: return String("9");
        case KStar: return String("*");
        case K0: return String("0");
        case KHash: return String("#");
        case KUp: return String("Up");
        case KLeft: return String("Left");
        case KOk: return String("OK");
        case KRight: return String("Right");
        case KDown: return String("Down");
        case KOther: return String("Other");
    }
}
