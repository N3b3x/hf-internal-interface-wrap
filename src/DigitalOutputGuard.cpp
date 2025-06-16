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
#include "DigitalOutput.h"

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
DigitalOutputGuard::DigitalOutputGuard(DigitalOutput &output) : p_output_(&output) {
  bool active = p_output_->SetActive();
}

/**
 * @brief Constructor.
 * @param output Reference to the DigitalOutput instance to manage.
 *
 * The constructor sets the associated DigitalOutput instance active.
 */
DigitalOutputGuard::DigitalOutputGuard(DigitalOutput *output) : p_output_(output) {
  bool active = p_output_->SetActive();
}

/**
 * @brief Destructor.
 *
 * The destructor sets the associated DigitalOutput instance inactive.
 */
DigitalOutputGuard::~DigitalOutputGuard() {
  bool inactive = p_output_->SetInactive();
}
