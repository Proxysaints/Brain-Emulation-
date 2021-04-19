###########################################################
## This file is part of the BrainGenix Simulation System ##
###########################################################

import datetime
import inspect
import os
import pymysql
import socket

'''
Name: SysLog
Description: This class handles the main log for this braingenix instance.
Date-Created: 2020-12-19
'''



def CompressFile(FileName): # Compresses The File #

    '''
    This function is used to compress finished logfiles into a gzip file.
    The function is called whenever a log file is split after a certain number of lines.
    *DO NOT CALL THIS*
    '''

    # First, Read The File Into Ram, Then Remove The Original #
    with open(FileName, 'rb') as FileObject:

        FileText = FileObject.read()

    os.remove(FileName)


    # Write The Compressed File To The Disk #
    with gzip.open(FileName + '.gz', 'wb') as FileObject:

        FileObject.write(FileText)


def TryMakeDir(path): # Makes a Dir, catches exception if already exists #

    '''
    This function attempts to make a directory, and if it already exists, skips it.
    *DO NOT CALL THIS*
    '''

    if not os.path.exists(path):
        os.mkdir(path)



class SysLog(): # Logger Class #

    '''
    This class contains the logger used by the entire BrainGenix system.
    The logger is an essential part of the BrainGenix system, as it allows an entire cluster's log events to be aggregated into a usable format.
    The logger contains some functions which are described below.
    The most frequently used one will be the Log function, which accepts a message and a level, and puts it into the log.
    This is the only function that should be called by an external function, as calling other functions will cause undefined behavior in the logger.
    '''

    def __init__(self, DatabaseConfig:tuple, LineRetentionCount:int, LogPath:str, ConsoleOutputEnabled:bool=True): # Connect To Database #

        '''
        This function is used when the system is starting up, and should not be called anytime after that.
        If this function is called later, it'll cause the logger to be reinitialized, which may loose the buffer.
        *DO NOT CALL THIS*
        '''


        # Create Local Log Path Directory #
        print('Checking If Log Path Exists')

        DoesLogDirExist = os.path.isdir(LogPath)
        if not DoesLogDirExist:

            print('Log Path Does Not Exist, Creating Directory')
            os.mkdir(LogPath)
            print('Created Directory')


        # Initialize Local Variable Information #
        self.LogBuffer = '[Level] [               Time] [     Module Name] [           Function] [Message]\n'
        self.PrintEnabled = ConsoleOutputEnabled
        self.CurrentLogLength = 1
        self.LogPath = LogPath
        self.LogFileNumber = 0
        self.LoggerRetentionLineCount = LineRetentionCount
        self.DatabaseWorking = False
        self.NodeID = socket.gethostname()

        self.StartTime = str(datetime.datetime.now()).replace(' ', '_')

        print(self.LogBuffer[:-1])


        # Open Log File #
        self.LogFileName = 'BG.log'
        self.LogFileObject = open(LogPath + self.LogFileName, 'w')


        # Perform Database Connection Validation #
        if DatabaseConfig == None:
            print('Database Configuration Null, Please Check Config File')

        # Parse Database Configuration #
        DBUname, DBPasswd, DBHost, DBName = DatabaseConfig
        

        # Connect To Database #
        self.DatabaseConnection = pymysql.connect(
            host = DBHost,
            user = DBUname,
            password = DBPasswd,
            db = DBName
        )
      
        # Create Database Cursor #
        self.LoggerCursor = self.DatabaseConnection.cursor()

        self.DatabaseWorking = True


    def Log(self, Message:str, Level:int=0): # Handles The Log Of An Item #

        '''
        This function is the main logging function for the entire system.
        It accepts both a message and a log level as arguments.
        The log level has a default value built in, so if you are logging something at the system level, the Level argument does not need to be specified.
        If you are logging anything at a different level, you should specifiy it at the different level.
        The levels are as follows:

        +==================================+
        | Level             | Number Value |
        | Info              | 0            |
        | Warning           | 1            |
        | Error (Non fatal) | 2            |
        | Error (Fatal)     | 3            |
        +==================================+

        The error codes will be used to sort the log later, if the log is checked.
        If your plugin needs custom error codes, you can specify any number as the error code, however it won't be classified as anything.
        Please note that if you use custom codes, another plugin may conflict with these, so you may want to make it user configurable.
        '''

        # Reformat Log For Human Readabillity #
        Level = str(Level)

        CallStack = inspect.stack()
        CallingModuleName = CallStack[1][1]
        CallingFunctionName = CallStack[1][3]

        LogTime = datetime.datetime.now().strftime('%Y-%m-%d_%H-%M-%S')
        LogString = f'[{Level.rjust(5, " ")}] [{LogTime}] [{CallingModuleName.split("/")[-1].split(".")[0].rjust(16, " ")}] [{CallingFunctionName.rjust(19, " ")}] {Message}\n'
        self.LogBuffer += LogString

        if self.PrintEnabled:
            print(LogString[:-1])


        if self.DatabaseWorking == False:

            self.LogFileObject.write(self.LogBuffer)
            self.LogBuffer = ''

        else:

            self.LogFileObject.write(self.LogBuffer)
            #
            # Write data *from the logbuffer* into the database here
            #
            
            insertStatement= ("INSERT INTO log(LogLevel,LogDatetime,CallingModule,FunctionName,LogOutput,Node) VALUES (%d, '%s', '%s', '%s', '%s', '%s')" %  (int(Level), LogTime, CallingModuleName.split("/")[-1].split(".")[0], CallingFunctionName, Message, self.NodeID))
            self.LoggerCursor.execute(insertStatement)
            
            self.LogBuffer = ''
            

    def PullLog(self, n:int): # Pull n most recent entries from the log table #
        
        PullStatement= ("SELECT * FROM log ORDER BY LogId DESC LIMIT %d" % int(n))
        self.LoggerCursor.execute(PullStatement)
        
        rows = self.LoggerCursor.fetchall()

        return rows
            

    def CheckDelete(self): # Deletes entries from the Log Table prior to a specific time # 
        
        pass

    def CleanExit(self): # Create Logger Shutdown Command #

        # Finalize Any Outstanding Database Commits #

        #
        # Write any data from the logbuffer *into* the database here
        #
        
        if self.LogBuffer != '':
            
            self.LogFileObject.write(self.LogBuffer)
            #
            # Write data *from the logbuffer* into the database here
            #
            
            # insertStatement= "INSERT INTO log(LogDatetime,LogOutput) VALUES (%s,%s)"%(LogTime,LogString)
            # self.LoggerCUrsor.execute(insertStatement)
            
            # self.LogBuffer = ''

        # Destroy Connection To Database #
        print('Destroying Database Connector')
        self.DatabaseConnection.close()
        print('Destroyed Database Connector')

        # Return Done #
        return
