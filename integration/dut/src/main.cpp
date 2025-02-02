#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/logging/log.h>

#include <communication/CommunicationService.h>
#include <utils/SensibleUICR.h>

LOG_MODULE_REGISTER(dut); // Register logging module

namespace
{
    EventDispatcher dispatcher;
    CommunicationService communicationService(dispatcher);
}

int main()
{
    LOG_INF("Starting DUT");
    Uicr::init();
    communicationService.init();

    return 0;
}
