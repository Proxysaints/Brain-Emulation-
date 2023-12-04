
//=================================//
// This file is part of BrainGenix //
//=================================//

/*
    Description: This file defines the struct used to store the voxel array.
    Additional Notes: None
    Date Created: 2023-11-29
    Author(s): Thomas Liao, Randal Koene


    Copyright (C) 2023  Thomas Liao, Randal Koene

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#pragma once



// Standard Libraries (BG convention: use <> instead of "")
#include <inttypes.h>
#include <math.h>
#include <memory>


// Third-Party Libraries (BG convention: use <> instead of "")


// Internal Libraries (BG convention: use <> instead of "")
#include <Simulator/Geometries/Geometry.h>

#include <VSDA/SliceGenerator/Structs/ScanRegion.h>



namespace BG {
namespace NES {
namespace Simulator {


typedef uint8_t VoxelType;

enum VoxelState {
    EMPTY=0,
    FILLED=1,
    BORDER=2
};


/**
 * @brief Defines the voxel array.
 * 
 */
class VoxelArray {

private:

    std::unique_ptr<VoxelType[]> Data_; /**Big blob of memory that holds all the voxels*/
    uint64_t DataMaxLength_ = 0;

    int SizeX_; /**Number of voxels in x dimension*/
    int SizeY_; /**Number of voxels in y dimension*/
    int SizeZ_; /**Number of voxels in z dimension*/

    float VoxelScale_um; /**Set the size of each voxel in micrometers*/

    BoundingBox BoundingBox_; /**Set the bounding box of this voxel array (relative to the simulation orign)*/



    /**
     * @brief Returns the flat index for the voxel at the given coords.
     * 
     * @param _X 
     * @param _Y 
     * @param _Z 
     * @return int 
     */
    int GetIndex(int _X, int _Y, int _Z);



public:

    /**
     * @brief Construct a new Voxel Array object
     * 
     * @param _BB Bounding box of the array, in world space
     * @param _VoxelScale_um Scale of each voxel in micrometers
     */
    VoxelArray(BoundingBox _BB, float _VoxelScale_um);
    VoxelArray(ScanRegion _Region, float _VoxelScale_um);

    /**
     * @brief Destroy the Voxel Array object
     * 
     */
    ~VoxelArray();



    /**
     * @brief Returns the voxel at the given coordinates
     * 
     * @param _X 
     * @param _Y 
     * @param _Z 
     * @return VoxelType 
     */
    VoxelType GetVoxel(int _X, int _Y, int _Z);


    /**
     * @brief Sets the voxel at the given coords to _Value.
     * 
     * @param _X 
     * @param _Y 
     * @param _Z 
     * @param _Value 
     */
    void SetVoxel(int _X, int _Y, int _Z, VoxelType _Value);

    /**
     * @brief Set the Voxel At the given Position (using the given scale) to the given value.
     * Converts the given float x,y,z um position to index, then calls setvoxel normally
     * 
     * @param _X 
     * @param _Y 
     * @param _Z 
     * @param _Value
     */
    void SetVoxelAtPosition(float _X, float _Y, float _Z, VoxelType _Value);

    /**
     * @brief Get the size of the array, populate the int ptrs
     * 
     * @param _X 
     * @param _Y 
     * @param _Z 
     */
    void GetSize(int* _X, int* _Y, int* _Z);

    /**
     * @brief Get the x dimensions
     * 
     * @return int 
     */
    int GetX();

    /**
     * @brief Get the y dimensions
     * 
     * @return int 
     */
    int GetY();

    /**
     * @brief Get the Z dimensions
     * 
     * @return int 
     */
    int GetZ();

    /**
     * @brief Returns the resolution of the given object in micrometers.
     * 
     * @return float 
     */
    float GetResolution();

    /**
     * @brief Returns the bounding box of this voxel array (in simulation world space).
     * 
     * @return BoundingBox 
     */
    BoundingBox GetBoundingBox();


    /**
     * @brief Clears the given array to all 0s
     * 
     */
    void ClearArray();

};


}; // Close Namespace Logger
}; // Close Namespace Common
}; // Close Namespace BG
