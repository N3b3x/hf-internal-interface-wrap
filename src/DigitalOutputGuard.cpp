/**
 * Nebula Tech Corporation
 *
 * Copyright © 2023 Nebula Tech Corporation.   All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public
License v3.0 or later.

/**
 * @file DigitalOutputGuard.cpp
 * @brief Source file for the DigitalOutputGuard class.
 *
 * The DigitalOutputGuard class ensures that a DigitalOutput instance is set
active
 * in its constructor and inactive in its destructor (RAII pattern).
 */

#include "DigitalOutputGuard.h"
#include "UTILITIES/common/ThingsToString.h"

#include "DigitalOutput.h"
#include "HAL/component_handlers/ConsolePort.h"

//==============================================================//
// VERBOSE??
//==============================================================//
static constexpr bool verbose = false;

//==============================================================//
// CLASS
//==============================================================//

/**
 * @brief Constructor.
 * @param output Reference to the DigitalOutput instance to manage.
 *
 * The constructor sets the associated DigitalOutput instance active.
 */
DigitalOutputGuard::DigitalOutputGuard(DigitalOutput &output)
    : p_output_(&output) {
  bool active = p_output_->SetActive();
  if (active) {
    WRITE_CONDITIONAL(
        verbose,
        "DigitalOutputGuard() - Digital output - %s - successfully set ACTIVE",
        PinToString(p_output_->GetPin()));
  } else {
    WRITE_CONDITIONAL(verbose,
                      "DigitalOutputGuard() - Digital output - %s - !!! FAILED "
                      "!!! to set ACTIVE",
                      PinToString(p_output_->GetPin()));
  }
}

/**
 * @brief Constructor.
 * @param output Reference to the DigitalOutput instance to manage.
 *
 * The constructor sets the associated DigitalOutput instance active.
 */
DigitalOutputGuard::DigitalOutputGuard(DigitalOutput *output)
    : p_output_(output) {
  bool active = p_output_->SetActive();
  if (active) {
    WRITE_CONDITIONAL(
        verbose,
        "DigitalOutputGuard() - Digital output - %s - successfully set ACTIVE",
        PinToString(p_output_->GetPin()));
  } else {
    WRITE_CONDITIONAL(verbose,
                      "DigitalOutputGuard() - Digital output - %s - !!! FAILED "
                      "!!! to set ACTIVE",
                      PinToString(p_output_->GetPin()));
  }
}

/**
 * @brief Destructor.
 *
 * The destructor sets the associated DigitalOutput instance inactive.
 */
DigitalOutputGuard::~DigitalOutputGuard() {
  bool inactive = p_output_->SetInactive();
  if (inactive) {
    WRITE_CONDITIONAL(verbose,
                      "~DigitalOutputGuard() - Digital output - %s - "
                      "successfully set INACTIVE",
                      PinToString(p_output_->GetPin()));
  } else {
    WRITE_CONDITIONAL(verbose,
                      "~DigitalOutputGuard() - Digital output - %s - !!! "
                      "FAILED !!! to set INACTIVE",
                      PinToString(p_output_->GetPin()));
  }
}
