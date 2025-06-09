/**
  *  Contains the declaration and definition of the AveragingFilter class that provides
  *      an averaging filter.
  *
  *
  *   Note:  These functions are not thread or interrupt-safe and should be called
  *          called with appropriate guards if used within an ISR or shared between tasks.
  -------------------------------------------------------------------------------------**/
//   Contains the declaration of  DigitalInput class, which provides facilities to describe
//     GPIO input pins.  An I/O pin can be configured in an input mode with floating input, 
//     with an internal pull - up or pulldown. DigitalInput employ lazy initialization, so that
//     they are initialized the first time a pin in manipulated.
// </summary>
// -----------------------------------------------------------------------
#include <internal_interface_drivers/DigitalInput.h>
#include "driver/gpio.h"

///----------------------------------------------------------------
/// <summary>
/// Constructor allocates an instance of the class with a fixed bank and pin.
/// </summary>
/// <param name="pin">Pin on that bank 0..16 </param>
/// <param name="activeState"Indicates whether set bit is activeC</param>
	
DigitalInput::DigitalInput(gpio_num_t pinArg, ActiveState activeStateArg) noexcept :
    DigitalGpio(pinArg, activeStateArg)
{
    // No code at this time
}

///------------------------------------------------------------------
///<summary>
/// Virtual function to initialize peripheral.
///</summary>
///<returns>true if able to initialize, false otherwise.</returns>

bool DigitalInput::Initialize() noexcept {
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << pin);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    return gpio_config(&io_conf) == ESP_OK;
}

///------------------------------------------------------------------
/// <summary>
/// This function identifies the logical pin state.
/// </summary>
///<returns>true if the pin is logically set, false otherwise</returns>
      
bool DigitalInput::IsActive() noexcept {
    if (EnsureInitialized()) {
        int level = gpio_get_level(pin);
        if (IsActiveHigh()) {
            return level == 1;
        } else {
            return level == 0;
        }
    }
    return false;
}
      		


///------------------------------------------------------------------
/// <summary>
/// This function return the current state.
/// </summary>
///<returns>Current state</returns>

DigitalGpio::State DigitalInput::GetState() noexcept {
    if (EnsureInitialized()) {
        int level = gpio_get_level(pin);
        if (IsActiveHigh()) {
            return (level == 1) ? State::Active : State::Inactive;
        } else {
            return (level == 0) ? State::Active : State::Inactive;
        }
    }
    return State::Inactive;
}


