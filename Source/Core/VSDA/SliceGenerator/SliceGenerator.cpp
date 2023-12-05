#include <VSDA/SliceGenerator/SliceGenerator.h>





namespace BG {
namespace NES {
namespace Simulator {




bool VSDA_EM_Initialize(BG::Common::Logger::LoggingSystem* _Logger, Simulation* _Sim) {

    // Check Preconditions
    assert(_Logger != nullptr);
    assert(_Sim != nullptr);
    if (_Sim->VSDAData_.State_ != VSDA_NOT_INITIALIZED) { // Check if the VSDA has already been initialized, if so - we don't touch it.
        _Logger->Log("VSDA EM Initialize Error, Cannot Reinit A Region Which Has Already Been Initialized", 6);
        return false; 
    }

    // Initialize it
    _Sim->VSDAData_.State_ = VSDA_INIT_BEGIN;


    // That's all we have to do apparently...
    return true;

}

bool VSDA_EM_SetupMicroscope(BG::Common::Logger::LoggingSystem* _Logger, Simulation* _Sim, MicroscopeParameters _Params) {

    // Check Preconditions
    assert(_Logger != nullptr);
    assert(_Sim != nullptr);
    if (_Sim->VSDAData_.State_ != VSDA_INIT_BEGIN) { // Check that the VSDA is during its init phase, and not yet done initializing.
        _Logger->Log("VSDA EM SetupMicroscope Error, Cannot Setup Microscope On Syatem With Unknown State", 6);
        return false; 
    }

    // Copy over the parameters
    _Sim->VSDAData_.Params_ = _Params;

    return true;

}

bool VSDA_EM_DefineScanRegion(BG::Common::Logger::LoggingSystem* _Logger, Simulation* _Sim, ScanRegion _ScanRegion, int* _RegionID) {

    // Check Preconditions
    assert(_Logger != nullptr);
    assert(_Sim != nullptr);
    if (_Sim->VSDAData_.State_ != VSDA_INIT_BEGIN) { // Check that the VSDA is during its init phase, and not yet done initializing.
        _Logger->Log("VSDA EM DefineScanRegion Error, Cannot Define Microscope Scan Region On Syatem With Unknown State", 6);
        return false; 
    }

    // Copy over the parameters, update region ID
    _Logger->Log(std::string("Adding Scan Region With Dimensions ") + _ScanRegion.Dimensions(), 2);
    _Sim->VSDAData_.Regions_.push_back(_ScanRegion);
    (*_RegionID) = _Sim->VSDAData_.Regions_.size()-1;

    return true;

}

bool VSDA_EM_QueueRenderOperation(BG::Common::Logger::LoggingSystem* _Logger, Simulation* _Sim, int _RegionID) {

    // Check Preconditions
    assert(_Logger != nullptr);
    assert(_Sim != nullptr);
    if (_Sim->VSDAData_.State_ != VSDA_INIT_BEGIN) { // Check that the VSDA is during its init phase, and not yet done initializing.
        _Logger->Log("VSDA EM QueueRenderOperation Error, Cannot Queue Render Operation On Syatem With Unknown State", 6);
        return false; 
    }
    if (_RegionID < 0 || _RegionID >= _Sim->VSDAData_.Regions_.size()) { // the region ID is out of range
        _Logger->Log("VSDA EM QueueRenderOperation Error, Region ID Is Out Of Range!", 6);
        return false;    
    }

    if (_Sim->WorkRequested || _Sim->CurrentTask != SIMULATION_NONE) { // Cannot start an operation, simulation is busy
        _Logger->Log("VSDA EM QueueRenderOperation Error, Simulation Is Currently Running!", 6);
        return false;    
    }

    // Setup Enums, Indicate that work is requested
    _Sim->VSDAData_.ActiveRegionID_ = _RegionID;
    _Sim->VSDAData_.State_ = VSDA_RENDER_REQUESTED;
    _Sim->CurrentTask = SIMULATION_VSDA;
    _Sim->WorkRequested = true;

    return true;

}



bool CreateVoxelArrayBorderFrame(VoxelArray* _Array) {

    // X Alligned Border
    for (int X = 0; X < _Array->GetX(); X++) {
        _Array->SetVoxel(X, 0, 0, BORDER);
        _Array->SetVoxel(X, _Array->GetY()-1, 0, BORDER);
        _Array->SetVoxel(X, 0, _Array->GetZ()-1, BORDER);
        _Array->SetVoxel(X, _Array->GetY()-1, _Array->GetZ()-1, BORDER);
    }

    // Y Alligned Border
    for (int Y = 0; Y < _Array->GetY(); Y++) {
        _Array->SetVoxel(0, Y, 0, BORDER);
        _Array->SetVoxel(_Array->GetX()-1, Y, 0, BORDER);
        _Array->SetVoxel(0, Y, _Array->GetZ()-1, BORDER);
        _Array->SetVoxel(_Array->GetX()-1, Y, _Array->GetZ()-1, BORDER);
    }

    // Z Alligned Border
    for (int Z = 0; Z < _Array->GetZ(); Z++) {
        _Array->SetVoxel(0, 0, Z, BORDER);
        _Array->SetVoxel(_Array->GetX()-1, 0, Z, BORDER);
        _Array->SetVoxel(0, _Array->GetY()-1, Z, BORDER);
        _Array->SetVoxel(_Array->GetX()-1, _Array->GetY()-1, Z, BORDER);
    }

    return true;

}

bool FillBoundingBox(VoxelArray* _Array, BoundingBox* _BB, float _VoxelScale) {

    for (float X = _BB->bb_point1[0]; X < _BB->bb_point2[0]; X+= _VoxelScale) {
        for (float Y = _BB->bb_point1[1]; Y < _BB->bb_point2[1]; Y+= _VoxelScale) {
            for (float Z = _BB->bb_point1[2]; Z < _BB->bb_point2[2]; Z+= _VoxelScale) {
                _Array->SetVoxelAtPosition(X, Y, Z, FILLED);
            }
        }
    }

    return true;

}

bool FillShape(VoxelArray* _Array, Geometries::Geometry* _Shape, float _VoxelScale) {

    BoundingBox BB = _Shape->GetBoundingBox();
    std::cout<<BB.ToString()<<std::endl;
        std::cout<<"2.1\n";

    for (float X = BB.bb_point1[0]; X < BB.bb_point2[0]; X+= _VoxelScale) {
        std::cout<<"2.2\n";
        for (float Y = BB.bb_point1[1]; Y < BB.bb_point2[1]; Y+= _VoxelScale) {
        std::cout<<"2.3\n";
            for (float Z = BB.bb_point1[2]; Z < BB.bb_point2[2]; Z+= _VoxelScale) {
        std::cout<<"2.4\n";
                if (_Shape->IsPointInShape(Geometries::Vec3D(X, Y, Z))) {
                    _Array->SetVoxelAtPosition(X, Y, Z, FILLED);
                }
            }
        }
    }
        std::cout<<"2.5\n";

    return true;

}

bool CreateVoxelArrayFromSimulation(BG::Common::Logger::LoggingSystem* _Logger, Simulation* _Sim, MicroscopeParameters* _Params, VoxelArray* _Array) {
    assert(_Array != nullptr);
    assert(_Params != nullptr);
    assert(_Sim != nullptr);
    assert(_Logger != nullptr);

    _Logger->Log(std::string("Building Voxel Array For Simulation '") + _Sim->Name + "'", 2);

    // Build Bounding Boxes For All Compartments
    for (unsigned int i = 0; i < _Sim->BSCompartments.size(); i++) {

        Compartments::BS* ThisCompartment = &_Sim->BSCompartments[i];
        std::variant<Geometries::Sphere, Geometries::Cylinder, Geometries::Box> ThisShape = _Sim->Collection.Geometries[ThisCompartment->ShapeID];

        std::cout<<"1\n";

        if (std::holds_alternative<Geometries::Sphere>(ThisShape)) {
        std::cout<<"2\n";
            Geometries::Sphere ThisSphere = std::get<Geometries::Sphere>(ThisShape);
            _Logger->Log("Adding Sphere To Voxel Array", 0);
            FillShape(_Array, &ThisSphere, _Params->VoxelResolution_um);
        std::cout<<"3\n";
        }
        else if (std::holds_alternative<Geometries::Box>(ThisShape)) {
            Geometries::Box ThisSphere = std::get<Geometries::Box>(ThisShape);
            _Logger->Log("Adding Box To Voxel Array", 0);
            FillShape(_Array, &ThisSphere, _Params->VoxelResolution_um);
        }
        else if (std::holds_alternative<Geometries::Cylinder>(ThisShape)) {
            Geometries::Cylinder ThisSphere = std::get<Geometries::Cylinder>(ThisShape);
            _Logger->Log("Adding Cylinder To Voxel Array", 0);
            FillShape(_Array, &ThisSphere, _Params->VoxelResolution_um);
        }


    }

        std::cout<<"4\n";

    CreateVoxelArrayBorderFrame(_Array);
        std::cout<<"5\n";

    return true;

}

bool RenderSliceFromArray(BG::Common::Logger::LoggingSystem* _Logger, Renderer::Interface* _Renderer, MicroscopeParameters* _Params, VoxelArray* _Array, int SliceNumber) {
    assert(_Array != nullptr);
    assert(_Params != nullptr);
    assert(_Logger != nullptr);


    _Logger->Log(std::string("Rendering Slice '") + std::to_string(SliceNumber) + "'", 2);

    _Renderer->ResetScene();

    // Ensure Scene Is Now Threadsafe
    _Renderer->LockScene();
    _Renderer->WaitUntilGPUDone();

    // Setup Voxel Info
    float VoxelSize = _Array->GetResolution();
    BoundingBox VoxelBB = _Array->GetBoundingBox();
    

    // Enumerate Slice, Build Cubes Where Needed
    for (unsigned int X = 0; X < _Array->GetX(); X++) {
        for (unsigned int Y = 0; Y < _Array->GetY(); Y++) {

            // Get Voxel At The Specified Position
            VoxelType ThisVoxel = _Array->GetVoxel(X, Y, SliceNumber);
            if (ThisVoxel != EMPTY) {


                // Build Shader with different color so we can tell each voxel apart
                Renderer::Shaders::Phong BoxShader;

                if (ThisVoxel == FILLED) {
                    BoxShader.DiffuseColor_  = vsg::vec4(X/(float)_Array->GetX(), Y/(float)_Array->GetY(), 1.0, 1.0f);
                    BoxShader.SpecularColor_ = vsg::vec4(0.f, 0.f, 0.f, 0.0f);
                } else if (ThisVoxel == BORDER) {
                    BoxShader.DiffuseColor_  = vsg::vec4(1.0, 0.5, 0.3, 1.0f);
                    BoxShader.SpecularColor_ = vsg::vec4(0.f, 0.f, 0.f, 0.0f);
                }
                BoxShader.Type_ = Renderer::Shaders::SHADER_PHONG;


                // If It's Not Empty, Create A Cube There
                Renderer::Primitive::Cube CubeCreateInfo;
                CubeCreateInfo.Position_ = vsg::vec3(X*VoxelSize + VoxelBB.bb_point1[0], Y*VoxelSize + VoxelBB.bb_point1[1], 0);//SliceNumber*VoxelSize + VoxelBB.bb_point1[2]);
                // CubeCreateInfo.Position_ /= 10.f;
                CubeCreateInfo.Rotation_ = vsg::vec3(0.0f, 0.0f, 0.0f);
                CubeCreateInfo.Scale_ = vsg::vec3(VoxelSize, VoxelSize, VoxelSize);
                // CubeCreateInfo.Scale_ /= 10.f;
                CubeCreateInfo.Shader_ = &BoxShader;

                _Renderer->AddBox(&CubeCreateInfo);
                // std::cout<<"Adding Cube for voxel "<< X <<"x, "<< Y<<"y, " << SliceNumber<<"z  at position (um) " << CubeCreateInfo.Position_<<std::endl;

            }

        }
    }

    // Now, Update and Unlock Scene When Done
    _Renderer->UpdateScene();
    _Renderer->UnlockScene();

    return true;
}


}; // Close Namespace Logger
}; // Close Namespace Common
}; // Close Namespace BG
