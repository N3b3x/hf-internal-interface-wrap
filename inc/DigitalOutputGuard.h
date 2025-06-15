/**
 * Nebula Tech Corporation
 *
 * Copyright © 2023 Nebula Tech Corporation.   All Rights Reserved.
 * This file is part of HardFOC and is licensed under the GNU General Public
License v3.0 or later.

/**
 * @file DigitalOutputGuard.h
 * @brief Header file for the DigitalOutputGuard class.
 *
 * The DigitalOutputGuard class ensures that a DigitalOutput instance is set
active
 * in its constructor and inactive in its destructor (RAII pattern).
 */

#ifndef HAL_INTERNAL_INTERFACE_DRIVERS_DIGITALOUTPUTGUARD_H_
#define HAL_INTERNAL_INTERFACE_DRIVERS_DIGITALOUTPUTGUARD_H_

#include "DigitalOutput.h"
#include "UTILITIES/common/ThingsToString.h"

/**
 * @class DigitalOutputGuard
 * @brief Guard class for managing the state of a DigitalOutput instance.
 *
 * The DigitalOutputGuard sets the associated DigitalOutput instance active in
 * its constructor and inactive in its destructor. This ensures proper resource
 * management and consistent behavior.
 */
class DigitalOutputGuard {
public:
  /**
   * @brief Constructor.
   * @param output Reference to the DigitalOutput instance to manage.
   *
   * The constructor sets the associated DigitalOutput instance active.
   */
  DigitalOutputGuard(DigitalOutput &output);

  /**
   * @brief Constructor.
   * @param output Reference to the DigitalOutput instance to manage.
   *
   * The constructor sets the associated DigitalOutput instance active.
   */
  DigitalOutputGuard(DigitalOutput *output);

  /**
   * @brief Destructor.
   *
   * The destructor sets the associated DigitalOutput instance inactive.
   */
  ~DigitalOutputGuard();

private:
  DigitalOutput *p_output_; ///< Reference to the managed DigitalOutput instance.
};

#endif /* HAL_INTERNAL_INTERFACE_DRIVERS_DIGITALOUTPUTGUARD_H_ */
