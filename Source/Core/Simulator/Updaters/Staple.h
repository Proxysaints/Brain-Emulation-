//=================================================================//
// This file is part of the BrainGenix-NES Neuron Emulation System //
//=================================================================//

/*
    Description: This file provides the Staple Updater.
    Additional Notes: None
    Date Created: 2023-06-28
*/

#pragma once

// Standard Libraries (BG convention: use <> instead of "")
#include <string>

// Third-Party Libraries (BG convention: use <> instead of "")

// Internal Libraries (BG convention: use <> instead of "")
#include <Simulator/Structs/Staple.h>


namespace BG {
namespace NES {
namespace Simulator {
namespace Updater {


/**
 * @brief Resets the target Staple.
 * 
 * @param _Target 
 */
void StapleReset(Connections::Staple* _Target);


/**
 * @brief Updates the target Staple.
 * 
 * @param _Target 
 */
void StapleUpdate(Connections::Staple* _Target);



}; // Close Namespace Updater
}; // Close Namespace Simulator
}; // Close Namespace NES
}; // Close Namespace BG