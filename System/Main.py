###########################################################
## This file is part of the BrainGenix Simulation System ##
###########################################################

'''
Name: Main Loop
Description: This is the main file for a BrainGenix instance.
Date-Created: 2020-12-18
'''

import atexit
import time
import os


from Core.Initialization.LoadConfig import LoadLoggerConfig
from Core.Initialization.LoadConfig import LoadDatabaseConfig
from Core.Initialization.LoadConfig import LoadZookeeperConfig
from Core.Initialization.LoadConfig import LoadInternodeQueueConfig
from Core.Initialization.LoadConfig import LoadManagementAPIServerConfig

from Core.Initialization.Instantiator import InstantiateZK
from Core.Initialization.Instantiator import InstantiateDB
from Core.Initialization.Instantiator import InstantiateLogger

from Core.Initialization.CheckLibraries import CheckImports

from Core.Internode.Zookeeper.ZKManager import SystemTelemetryManager

from Core.Management.Telemetry.SystemTelemetry import Follower
from Core.Management.Telemetry.SystemTelemetry import Leader

from Core.Management.Logger.CLAS import CentralizedLoggerAggregationSystem

from Core.Management.API.ManagementAPI import ManagementAPISocketServer


##############################################################################
## NOTE: A Lowercase "m" Preceeding A Class Means It's a Main System        ##
## NOTE: A Lowercase "s" Preceeding A Class Means It's a Subsystem          ##
##############################################################################



# Set Version Information
Version = '0.0.7'
Branch = 'dev' # 'dev' or 'rel'


# Load Config #
LoggerConfigDict = LoadLoggerConfig(ConfigFilePath = 'Config/LoggerConfig.yaml')
DBConfigDict = LoadDatabaseConfig(ConfigFilePath = 'Config/DatabaseConfig.yaml')
ZKConfigDict = LoadZookeeperConfig(ConfigFilePath = 'Config/ZookeeperConfig.yaml')
InternodeConfigDict = LoadInternodeQueueConfig(ConfigFilePath = 'Config/InternodeQueue.yaml')
ManagementAPIServerConfig = LoadManagementAPIServerConfig(ConfigFilePath = 'Config/ManagementAPIConfig.yaml')


# Initialize Logger #
mLogger = InstantiateLogger(DBConfigDict, LoggerConfigDict)


# Initialize CLAS #
sCLAS = CentralizedLoggerAggregationSystem(mLogger)


# Connect To DB #
sDatabaseInterface = InstantiateDB(mLogger, DBConfigDict)


# Start API Server #
sSocketAPI = ManagementAPISocketServer(mLogger, ManagementAPIServerConfig, ZKConfigDict)


# Check Dependencies #
ModulesNeeded = [
                'os',
                'yaml',
                'atexit',
                'inspect',
                'time',
                'datetime',
                'cpuinfo',
                'platform',
                'psutil',
                'GPUtil',
                'threading',
                'kazoo',
                'uuid',
                ]

CheckImports(ModulesNeeded, mLogger)


# Connect To Zookeeper Service #
sZookeeper = InstantiateZK(mLogger, ZKConfigDict)


##############################################################################################################
## THIS WILL CONNECT TO THE C++ INTERFACE WITH BRIAN HERE. THE HEAVY PROCESING WORK IS DONE ON THE C++ SIDE ##
##############################################################################################################



# Start System Telemetry #
TelemetryFollower = Follower(Logger=mLogger, Zookeeper=sZookeeper)
TelemetryLeader = Leader(Logger=mLogger, Zookeeper=sZookeeper) #<-- Note: This does NOT start it yet, you need to call start first. The manager handles this.
TelManager = SystemTelemetryManager(sZookeeper, TelemetryLeader)


# Initialize The API ZK Watcher #
#ZookeeperAPIWatcher = PollWatcher(mLogger, sZookeeper, TelemetryLeader)


# Get NodeCount #
NodeCount = sZookeeper.ConcurrentConnectedNodes()

# Get API Server Count #
APIServerCount = len(sZookeeper.ZookeeperConnection.get_children('/BrainGenix/API/Connections'))


# MOTD #
mLogger.Log('Starting BrainGenix Instance')
mLogger.Log('')
mLogger.Log('---------------------------------------------------------------------------')
mLogger.Log('██████╗ ██████╗  █████╗ ██╗███╗   ██╗ ██████╗ ███████╗███╗   ██╗██╗██╗  ██╗')
mLogger.Log('██╔══██╗██╔══██╗██╔══██╗██║████╗  ██║██╔════╝ ██╔════╝████╗  ██║██║╚██╗██╔╝')
mLogger.Log('██████╔╝██████╔╝███████║██║██╔██╗ ██║██║  ███╗█████╗  ██╔██╗ ██║██║ ╚███╔╝ ')
mLogger.Log('██╔══██╗██╔══██╗██╔══██║██║██║╚██╗██║██║   ██║██╔══╝  ██║╚██╗██║██║ ██╔██╗ ')
mLogger.Log('██████╔╝██║  ██║██║  ██║██║██║ ╚████║╚██████╔╝███████╗██║ ╚████║██║██╔╝ ██╗')
mLogger.Log('╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝ ╚═════╝ ╚══════╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝')
mLogger.Log('---------------------------------------------------------------------------')
mLogger.Log('')
mLogger.Log('    +-----------------------------------------------------------------+')
mLogger.Log('    |                 BrainGenix WBE Simulation System                |')
mLogger.Log(f'    |                 Version: {Version}                                  |')
mLogger.Log(f'    |                 Branch: {Branch}                                     |')
mLogger.Log(f'    |                 Clustersize: {NodeCount}                                  |')
mLogger.Log(f'    |                 MAPIServers: {APIServerCount}                                  |')
mLogger.Log('    +-----------------------------------------------------------------+')
mLogger.Log('')


# Run Shutdown Commands On System Exit #
@atexit.register
def ShutdownSystem():
    mLogger.CleanExit()
    sZookeeper.Exit()
    sSocketAPI.Quit()
    os._exit(1)



# Main Loop #
while True:

    # Execute System Tasks If In Leader Mode #
    TelManager.UpdateSysTel()


    time.sleep(0.5) # <-- Sleep for a polling interval to avoid excessive CPU usage
