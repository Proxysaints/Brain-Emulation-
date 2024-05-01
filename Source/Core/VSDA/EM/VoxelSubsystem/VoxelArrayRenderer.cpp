//=================================//
// This file is part of BrainGenix //
//=================================//


// Standard Libraries (BG convention: use <> instead of "")
#include <filesystem>

// Third-Party Libraries (BG convention: use <> instead of "")

// Internal Libraries (BG convention: use <> instead of "")
#include <VSDA/EM/VoxelSubsystem/VoxelArrayRenderer.h>

#include <VSDA/EM/VoxelSubsystem/ImageProcessorPool/Image.h>




namespace BG {
namespace NES {
namespace Simulator {

// TODO: Make renderslicefromarray use the imageprocessorpool to do the enumeration of the voxel array (the pixel population part) using threads (so that nested for loop goes away, and this therad just positions the cameras)
// Then, if needed, we can move renderslicefromarray inside the threadedrendervoxelarray if that still isn't fast enough.


// std::vector<std::vector<std::string>> ThreadedRenderVoxelArray(BG::Common::Logger::LoggingSystem* _Logger, VSDAData* _VSDAData, std::string _FilePrefix, ImageProcessorPool* _ImageProcessorPool) {

// }


bool RenderSliceFromArray(BG::Common::Logger::LoggingSystem* _Logger, int MaxImagesX, int MaxImagesY, VSDAData* _VSDAData, VoxelArray* _Array, std::string _FilePrefix, int _SliceNumber, int _SliceThickness, ImageProcessorPool* _ImageProcessorPool, double _OffsetX, double _OffsetY, double _RegionOffsetX, double _RegionOffsetY, int _SliceOffset) {
    assert(_VSDAData != nullptr);
    assert(_Logger != nullptr);


    // Get Params and Array From VSDAData
    MicroscopeParameters* Params = &_VSDAData->Params_;
    VoxelArray* Array = _Array;
    float VoxelSize = Array->GetResolution();
    BoundingBox VoxelBB = Array->GetBoundingBox();

    // _Logger->Log(std::string("Rendering Slice '") + std::to_string(SliceNumber) + "'", 1);

    ScanRegion* ThisScanRegion = &_VSDAData->Regions_[_VSDAData->ActiveRegionID_];

    
    // Calculate Desired Image Size
    // In order for us to deal with multiple different pixel/voxel setting, we create an image of start size where one pixel = 1 voxel
    // then later on, we resample it to be the right size (for the target image)
    // The number of voxels we move per step depends on image size and overlap percentage:
    int ImageWidth_vox = ceil(_VSDAData->Params_.ImageWidth_px / _VSDAData->Params_.NumPixelsPerVoxel_px);
    int ImageHeight_vox = ceil(_VSDAData->Params_.ImageHeight_px / _VSDAData->Params_.NumPixelsPerVoxel_px);
    int VoxelsPerStepX = ceil(_VSDAData->Params_.ImageWidth_px * (1. - (_VSDAData->Params_.ScanRegionOverlap_percent / 100.)) / _VSDAData->Params_.NumPixelsPerVoxel_px);
    int VoxelsPerStepY = ceil(_VSDAData->Params_.ImageHeight_px * (1. - (_VSDAData->Params_.ScanRegionOverlap_percent / 100.)) / _VSDAData->Params_.NumPixelsPerVoxel_px);
    int VoxelsOverlapX = ceil(_VSDAData->Params_.ImageWidth_px * (_VSDAData->Params_.ScanRegionOverlap_percent / 100.) / _VSDAData->Params_.NumPixelsPerVoxel_px);
    int VoxelsOverlapY = ceil(_VSDAData->Params_.ImageHeight_px * (_VSDAData->Params_.ScanRegionOverlap_percent / 100.) / _VSDAData->Params_.NumPixelsPerVoxel_px);
    int NumChannels = 3;
    float CameraStepSizeX_um = VoxelsPerStepX * _VSDAData->Params_.VoxelResolution_um;
    float CameraStepSizeY_um = VoxelsPerStepY * _VSDAData->Params_.VoxelResolution_um;

    double TotalSliceWidth = abs((double)Array->GetBoundingBox().bb_point1[0] - (double)Array->GetBoundingBox().bb_point2[0]);
    double TotalSliceHeight = abs((double)Array->GetBoundingBox().bb_point1[1] - (double)Array->GetBoundingBox().bb_point2[1]);

    // Number of X*Y images to take to cover the whole slice:
    int TotalXSteps = ceil(TotalSliceWidth / CameraStepSizeX_um);
    int TotalYSteps = ceil(TotalSliceHeight / CameraStepSizeY_um);

    // Limit the total steps to those defined in the task
    TotalXSteps = std::min(TotalXSteps, MaxImagesX);
    TotalYSteps = std::min(TotalYSteps, MaxImagesY);


    // Now, we enumerate through all the steps needed, one at a time until we reach the end
    for (int XStep = 0; XStep < TotalXSteps; XStep++) {
        for (int YStep = 0; YStep < TotalYSteps; YStep++) {

            // we have to calculate an 'adjusted' slice number since we might skip over z slices to simulate more thickness for each slice
            // so to keep our slices sequential, we just divide the current 'true' slice number by the number of slices skipped so they're once again sequential
            int AdjustedSliceNumber = (_SliceNumber + _SliceOffset) / (_VSDAData->Params_.SliceThickness_um / _VSDAData->Params_.VoxelResolution_um);

            // Calculate the filename of the image to be generated, add to list of generated images
            std::string DirectoryPath = "Renders/" + _FilePrefix + "/Slice" + std::to_string(AdjustedSliceNumber) + "/";
            double RoundedXCoord = std::ceil(((CameraStepSizeX_um * XStep) + _OffsetX) * 100.0) / 100.0;
            double RoundedYCoord = std::ceil(((CameraStepSizeY_um * YStep) + _OffsetY) * 100.0) / 100.0;
            std::string FilePath = "X" + std::to_string(RoundedXCoord) + "_Y" + std::to_string(RoundedYCoord) + ".png";




            // Setup and submit task to queue for rendering
            std::unique_ptr<ProcessingTask> ThisTask = std::make_unique<ProcessingTask>();
            ThisTask->Array_ = _Array;
            ThisTask->Width_px = _VSDAData->Params_.ImageWidth_px;
            ThisTask->Height_px = _VSDAData->Params_.ImageHeight_px;
            ThisTask->VoxelStartingX = VoxelsPerStepX * XStep;
            ThisTask->VoxelStartingY = VoxelsPerStepY * YStep;
            ThisTask->VoxelEndingX = ThisTask->VoxelStartingX + ImageWidth_vox;
            ThisTask->VoxelEndingY = ThisTask->VoxelStartingY + ImageHeight_vox;
            ThisTask->VoxelZ = _SliceNumber;
            ThisTask->SliceThickness_vox = _SliceThickness;
            ThisTask->TargetFileName_ = FilePath;
            ThisTask->TargetDirectory_ = DirectoryPath;
            ThisTask->EnableImageNoise = Params->GenerateImageNoise;
            ThisTask->ImageNoiseAmount = Params->ImageNoiseIntensity;
            ThisTask->PreBlurNoisePasses = Params->PreBlurNoisePasses;
            ThisTask->PostBlurNoisePasses = Params->PostBlurNoisePasses;
            ThisTask->EnableGaussianBlur = Params->EnableGaussianBlur;
            ThisTask->GaussianBlurSigma = Params->GaussianBlurSigma;
            ThisTask->VoxelScale_um = Params->VoxelResolution_um;
            ThisTask->EnableInterferencePattern = Params->EnableInterferencePattern;
            ThisTask->InterferencePatternXScale_um = Params->InterferencePatternXScale_um;
            ThisTask->InterferencePatternAmplitude = Params->InterferencePatternAmplitude;
            ThisTask->InterferencePatternBias = Params->InterferencePatternBias;
            ThisTask->InterferencePatternWobbleFrequency = Params->InterferencePatternWobbleFrequency;



            // Setup Stats Info About Each Region
            int VoxelOffsetX = ((_OffsetX + _RegionOffsetX) / Params->VoxelResolution_um);
            int VoxelOffsetY = ((_OffsetY + _RegionOffsetY) / Params->VoxelResolution_um);


            VoxelIndexInfo Info;
            Info.StartX = ThisTask->VoxelStartingX + VoxelOffsetX;
            Info.EndX = ThisTask->VoxelEndingX + VoxelOffsetX;
            Info.StartY = ThisTask->VoxelStartingY + VoxelOffsetY;
            Info.EndY = ThisTask->VoxelEndingY + VoxelOffsetY;
            Info.StartZ = AdjustedSliceNumber;
            Info.StartX *= Params->NumPixelsPerVoxel_px;
            Info.EndX *= Params->NumPixelsPerVoxel_px;
            Info.StartY *= Params->NumPixelsPerVoxel_px;
            Info.EndY *= Params->NumPixelsPerVoxel_px;
            Info.EndZ = AdjustedSliceNumber + 1;

            ThisScanRegion->ImageFilenames_.push_back(DirectoryPath + FilePath);
            ThisScanRegion->ImageVoxelIndexes_.push_back(Info);


            // Enqueue Work Operation
            _ImageProcessorPool->QueueEncodeOperation(ThisTask.get());
            _VSDAData->Tasks_.push_back(std::move(ThisTask));




        }
    }

    return true;
}





}; // Close Namespace Logger
}; // Close Namespace Common
}; // Close Namespace BG
