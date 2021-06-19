###########################################################
## This file is part of the BrainGenix Simulation System ##
###########################################################

import pymysql

'''
Name: DB Logger Thread
Description: This system is responsible for sending logs to the database.
Date-Created: 2021-06-18
'''


class DatabaseLogTransmissionSystem(): # Transmits Logs From The Logger To The Database #

    def __init__(self, Logger:object, LogBufferQueue:object, ControlQueue:object, DatabaseConfig:dict): # Init #

        # Init Logger #
        self.Logger = Logger
        self.LogBufferQueue = LogBufferQueue
        self.ControlQueue = ControlQueue
        self.Logger.Log('Initializing Database Log Transmission System', 4)

        # Connect To DB #
        DBUsername = str(DatabaseConfig.get('DatabaseUsername'))
        DBPassword = str(DatabaseConfig.get('DatabasePassword'))
        DBHost = str(DatabaseConfig.get('DatabaseHost'))
        DBDatabaseName = str(DatabaseConfig.get('DatabaseName'))


        # Connect To Database #
        self.DatabaseConnection = pymysql.connect(
            host = DBHost,
            user = DBUsername,
            password = DBPassword,
            db = DBDatabaseName
        )

        # Create Database Cursor #
        self.LoggerCursor = self.DatabaseConnection.cursor()


    def ShouldStop(self): # Check If Thread Should Exit #
        return not self.ControlQueue.empty()


    def __call__(self): # Thread To Transmit Logs To DB #

        # Enter Loop #
        while not self.ShouldStop():

            # Read Log Item #
            LogDataDict = self.LogBufferQueue.get()

            # Decode Dict #
            Level = LogDataDict['LogLevel']
            LogTime = LogDataDict['LogDateTime']
            CallingModuleName = LogDataDict['CallingModule']
            CallingFunctionName = LogDataDict['FunctionName']
            Message = LogDataDict['LogOutput']
            Node = LogDataDict['Node']


            # Transmit Log Line #
            insertStatement= ("INSERT INTO log(LogLevel,LogDatetime,CallingModule,FunctionName,LogOutput,Node) VALUES (%s, %s, \"%s\", \"%s\", \"%s\", \"%s\")")
            val = (Level, str(LogTime), CallingModuleName.split("/")[-1].split(".")[0], CallingFunctionName, Message, Node)
            self.LoggerCursor.execute(insertStatement,val)


        # Log System Shutdown #
        self.Logger.Log('Shutting Down Database Log Transmission Daemon', 5)
        
        self.Logger.Log('Commiting Oustanding Cursor Commands', 3)
        self.LoggerCursor.commit()
        self.Logger.Log('Committed Outstanding Cursor Statements', 2)

        self.Logger.Log('Closing pymysql Connection', 3)
        self.DatabaseConnection.close()
        self.Logger.Log('Closed pymysql Connection', 2)

        self.Logger.Log('Database Log Transmission System Shutdown Complete', 4)
