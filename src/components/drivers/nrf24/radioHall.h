#ifndef RADIOHALL_H
#define RADIOHALL_H
#include <cstdint>


class RADIOHALL {

private:
public:
    static void init();
    static void denit();
    static void configureRadio(uint8_t channel, uint8_t payload);
    static void isConnected();

};

extern RADIOHALL radioHall;




#endif //RADIOHALL_H