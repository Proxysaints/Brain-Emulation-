#include <BG/Renderer/Interface.h>



/**
 * MEMORY LEAK NOTICE:
 * 
 * There are false positives in this code! 
 * It's not actually leaking memory, valgrind just repots memory leaks due to dl_open calls.
 * To *actually* check for memory leaks, please use a null icd driver (see this for more info):
 *     https://github.com/KhronosGroup/Vulkan-Loader/issues/256
 * 
 * THIS IS A KNOWN BUG IN VALGRIND!
*/


// TODO: Update Doxygen Docs!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


namespace BG {
namespace NES {
namespace Renderer {


Interface::Interface(BG::Common::Logger::LoggingSystem* _Logger) {
    assert(_Logger != nullptr);
    Logger_ = _Logger;

    // Instantiate Subclasses
    RendererManager_ = std::make_unique<Manager>(_Logger);
    GeometryBuilder_ = std::make_unique<GeometryBuilder>(Logger_);


}

Interface::~Interface() {
    assert(Logger_ != nullptr);
    Logger_->Log("Shutting Down NES Rendering Subsystem", 3);


}


bool Interface::Initialize() {
    assert(Logger_ != nullptr);

    if (!RendererManager_->SetupScene()) {
        return false;
    }

    if (!RendererManager_->Initialize()) {
        return false;
    }

    RendererManager_->SetupViewer();

    return RendererManager_->CompileScene();

}

bool Interface::DrawFrame() {
    return RendererManager_->DrawFrame();
}


bool Interface::AddBox(Primitive::Cube* _CreateInfo) {
    assert(_CreateInfo != nullptr);
    return GeometryBuilder_->CreateCube(RendererManager_->GetScene(), _CreateInfo);
}

bool Interface::AddSphere(Primitive::Sphere* _CreateInfo) {
    assert(_CreateInfo != nullptr);
    return GeometryBuilder_->CreateSphere(RendererManager_->GetScene(), _CreateInfo);
}

bool Interface::AddCylinder(Primitive::Cylinder* _CreateInfo) {
    assert(_CreateInfo != nullptr);
    return GeometryBuilder_->CreateCylinder(RendererManager_->GetScene(), _CreateInfo);
}

bool Interface::UpdateScene() {
    return RendererManager_->CompileScene();
}

void Interface::WaitUntilGPUDone() {
    RendererManager_->WaitUntilGPUDone();
    return;
}
void Interface::LockScene() {
    RendererManager_->LockScene();
}

void Interface::UnlockScene() {
    RendererManager_->UnlockScene();
}


void Interface::ResetViewer() {
    LockScene();
    WaitUntilGPUDone();
    RendererManager_->SetupViewer();
    RendererManager_->CompileScene();
    UnlockScene();
}

void Interface::ResetScene() {
    RendererManager_->ClearScene();

    // LockScene();
    // WaitUntilGPUDone();

    // // hacky
    // Shaders::Phong CubeShader;
    // CubeShader.DiffuseColor_  = vsg::vec4(0.5f, 0.5f, 0.5f, 0.0f);
    // CubeShader.SpecularColor_ = vsg::vec4(0.f, 0.0f, 0.0f, 0.0f);
    // CubeShader.Type_ = Shaders::SHADER_PHONG;
    // Primitive::Cube CreateInfo;
    // CreateInfo.Position_ = vsg::vec3(0.0f, -1.1f, 0.0f);
    // CreateInfo.Rotation_ = vsg::vec3(0.0f, 0.0f, 0.0f);
    // CreateInfo.Scale_    = vsg::vec3(4.1f, 4.1f, 4.1f);
    // CreateInfo.Shader_ = &CubeShader;
    // AddBox(&CreateInfo);

    // UnlockScene();
}

}; // Close Namespace Logger
}; // Close Namespace Common
}; // Close Namespace BG