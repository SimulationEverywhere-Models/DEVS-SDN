#include <math.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <string>

#include "message.hpp"

ostream &operator<<(ostream &os, const Message_t &msg) {
    /****** COMMENT this line for Graphical representation output *******************/
    os << msg.packetSec << " " << msg.dstIP;
    /************/

    /****** UN-COMMENT this line for Graphical representation output *******************/
//    os << "<" << msg.packetSec << ", " << msg.dstIP << ">";
    return os;
}

istream &operator>>(istream &is, Message_t &msg) {
    is >> msg.packetSec;
    is >> msg.dstIP;
    return is;
}
