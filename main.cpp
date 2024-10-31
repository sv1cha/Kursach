#include <iostream>
#include "Connector_to_base.h"
#include "Interface.h"
#include "Client_Communicate.h"
#include "Calculator.h"
#include "Errors.h"
#include "Logger.h"

int main(int argc, const char** argv)
{
    Interface UI;
    UI.comm_proc(argc, argv);
    return 0;
}
