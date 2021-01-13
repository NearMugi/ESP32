#ifndef M5ATOMBASE_H
#define M5ATOMBASE_H

#include "M5Atom.h"
class m5AtomBase
{
    class m5AtomLED
    {
    public:
        explicit m5AtomLED() : buf({0x05, 0x05, 0x00, 0x00, 0x00}){};
        virtual ~m5AtomLED(){};
        void setLED(uint8_t _r, uint8_t _g, uint8_t _b)
        {
            buf[2] = _r;
            buf[3] = _g;
            buf[4] = _b;
            M5.dis.displaybuff(buf);
        };
        void setRed()
        {
            setLED(0xff, 0x00, 0x00);
        };

        void setGreen()
        {
            setLED(0x00, 0xff, 0x00);
        };

        void setBlue()
        {
            setLED(0x00, 0x00, 0xff);
        };

        void setWhite()
        {
            setLED(0xff, 0xff, 0xff);
        };

    private:
        uint8_t buf[5];
    };

public:
    m5AtomBase(){};
    virtual ~m5AtomBase(){};
    m5AtomLED LED;
};

#endif