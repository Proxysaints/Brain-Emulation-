#include <Simulator/EngineController.h>



namespace BG {
namespace NES {
namespace Simulator {


void SimulationEngineThread(BG::Common::Logger::LoggingSystem* _Logger, Simulation* _Sim, VSDA::RenderPool* _RenderPool, std::atomic<bool>* _StopThreads) {
    assert(_Logger != nullptr);
    assert(_Sim != nullptr);
    assert(_RenderPool != nullptr);
    assert(_StopThreads != nullptr);

    // Log Init message
    _Logger->Log("Starting Simulation Updater Thread", 3);

    // Setup Simulation Engine
    Engine SE;

    // Enter into loop until thread should stop
    while (!(*_StopThreads)) {

        if (_Sim->WorkRequested) {
            _Logger->Log("Simulation Work Requested, Identifiying Task", 2);
            _Sim->IsProcessing = true;

            if (_Sim->CurrentTask == SIMULATION_RESET) {
                _Logger->Log("Worker Performing Simulation Reset For Simulation " + std::to_string(_Sim->ID), 4);
                SE.Reset(_Sim);
                _Sim->CurrentTask = SIMULATION_NONE;
                _Sim->WorkRequested = false;
            } else if (_Sim->CurrentTask == SIMULATION_RUNFOR) {
                _Logger->Log("Worker Performing Simulation RunFor For Simulation " + std::to_string(_Sim->ID), 4);
                SE.RunFor(_Sim);
                _Sim->CurrentTask = SIMULATION_NONE;
                _Sim->WorkRequested = false;
            } else if (_Sim->CurrentTask == SIMULATION_VSDA) {
                _Logger->Log("Worker Performing Simulation VSDA Call For Simulation " + std::to_string(_Sim->ID), 4);
                _Sim->IsRendering = true;
                _RenderPool->QueueRenderOperation(_Sim);

                // Randal - I had no idea how to better do this, please fix this as you see fit
                // The RenderPool main worker func (in RenderPool.cpp) will set isrendering to false when done, unlocking this.
                // Probably a mutex is better but eh idk
                while (_Sim->IsRendering) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // sleep for 10ms
                }
                _Sim->VSDAData_.State_ = VSDA_RENDER_DONE;
                _Sim->CurrentTask = SIMULATION_NONE;
                _Sim->WorkRequested = false;
            }
            _Sim->IsProcessing = false;
            _Logger->Log("Worker Completed Work On Simulation " + std::to_string(_Sim->ID), 4);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // sleep for 10ms
        }

    }

    // Log Shutdown Message
    _Logger->Log("Simulation Engine Thread Exiting", 4);

}


}; // Close Namespace Simulator
}; // Close Namespace NES
}; // Close Namespace BG