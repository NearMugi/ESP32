#include "M5Atom.h"
#include "I2CScanner.h"

I2CScanner scanner;

void setup()
{
    M5.begin(true, false, true);
    delay(10);
    scanner.Init();
}

void loop()
{
    scanner.Scan();
    delay(5000);
}
