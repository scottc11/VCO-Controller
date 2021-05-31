#include "TouchChannel.h"

int TouchChannel::setBenderMode(int targetMode/*0*/) {
    if (benderMode < 3) {
        benderMode += 1;
    } else {
        benderMode = 0;
    }
    return benderMode;
}