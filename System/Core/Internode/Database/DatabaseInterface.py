###########################################################
## This file is part of the BrainGenix Simulation System ##
###########################################################

import pymysql
import atexit
import re
import threading

'''
Name: Database Interface
Description: This file creates an interface from BrainGenix to mySQL.
Date-Created: 2021-01-17
'''

def PymysqlInstanceCreate(Logger:object, DatabaseConfig:dict):
    # start loop #
    MainThread = threading.main_thread.__name__
    ThisThread = threading.PyMysqlInstanceUpdate.__name__
    Threads = list[MainThread, ThisThread] # setup thread list #
    Run = True
    Logger = Logger
    Logger.Log("Pymysql Instance Create Initalized", 3)

    # Connect To DB #
    DBUsername = str(DatabaseConfig.get('DatabaseUsername'))
    DBPassword = str(DatabaseConfig.get('DatabasePassword'))
    DBHost = str(DatabaseConfig.get('DatabaseHost'))
    DBDatabaseName = str(DatabaseConfig.get('DatabaseName'))

    while Run == True:
        # loop throgh threads #
        for ExistingThread in threading.enumerate():
            RunOnce = False
            # loop throgh thread list #
            for Thread in Threads:
                # check if thread has pymysql #
                if Thread == ExistingThread.__name__:
                    RunOnce = True

            if RunOnce != True:
                # if thread has no pymysql add pymysql #
                Threads.extend(ExistingThread.__name__)
                ExistingThread.PymysqlInstance = pymysql.connect(host = DBHost, user = DBUsername, password = DBPassword, db = DBDatabaseName)
                Logger.Log("Pymysql Instance Created", 3)

class DBInterface(): # Interface to MySQL database #

    '''
    This class is used to provide a layer of abstraction between the BrainGenix system and mysql.
    It uses pymysql to interact with the database over sockets.
    This class is designed with some specific methods to check user information, and other such information.
    As the database changes structure, this file will also change, to add new database features.
    '''

    def __init__(self, Logger:object, DatabaseConfig:dict):

        '''
        Initializes the DB Interface by connecting to mysql.
        The connection information is passed in as arguments.
        These arguments are loaded from the local config YAML file.
        '''

        # Extract Values From Dictionary #
        Username = str(DatabaseConfig.get('DatabaseUsername'))
        Password = str(DatabaseConfig.get('DatabasePassword'))
        Host = str(DatabaseConfig.get('DatabaseHost'))
        DatabaseName = str(DatabaseConfig.get('DatabaseName'))


        # Create Local Pointer #
        self.Logger = Logger

        # Create Prohibited Characters List #
        self.ProhibitedChars = ['*','{','}','[',']','(',')',';']

        # Establish DB Connection #
        self.DBConnection = pymysql.connect(host=Host, user=Username, password=Password, db=DatabaseName)

        # Start Pymysql Instance creator #
        #self.Thread = threading.Thread(target=PymysqlInstanceCreate, args=(Logger, DatabaseConfig))
        #self.Thread.__name__ = "PymysqlInstanceUpdate"
        #self.Thread.start()

        self.Logger.Log(f'Connected To DB At {Host}')

        # Register Shutdown Function #
        atexit.register(self.ShutdownConnection)


    def CreateCursor(self): # Creates A Database Cursor #

        '''
        This method creates a cursor object as an attribute in self.
        '''

        # Create Cursor #
        self.DatabaseCursor = pymysql.cursors.Cursor()


    def CheckIfUserExists(self, UserName:str, Table:str): # Checks If A Given User Exists #

        '''
        Returns a boolean indicating if the target username exists or does not exist.
        This method is part of the DBInterface class.
        '''

        # Execute Some MYSQL Cursor Code #
        # something here...

        # Strip SQL Injection Vectors #
        self.CheckIfStringClean(Table)
        self.CheckIfStringClean(UserName)

        Table = re.sub(r'\W+', '', Table)
        UserName = re.sub(r'\W+', '', UserName)

        #Count of number of Users having the same UserName in the table
        cnt= self.DatabaseCursor.execute("SELECT * from user where UserName= %s", UserName)

        UserExists= True
        # If User Exists
        if cnt==1:
            UserExists=True

        else:
            UserExists=False

        return UserExists


    def CheckIfStringClean(self, String): # Checks If A String Is Clean (SQL INJECTION DETECTION) #

        # Perform The Check Against The Prohibited Strings List #
        for Character in self.Prohibited:
            if Character in String:
                self.Logger.Log(f'Potential MYSQL Injection Attack Detected: "{String}"', 4)

                return False

        return True


    def GetUserInformation(self, UserName:str, Table:str): # Returns A List Of User Information #

        '''
        Returns some user information from the specific row of the user.
        This is going to include everything outlined in the user DB block diagram.
        The specific format of the information is a dictionary with key values being the names of the columns.
        '''

        # Detect and Log Potential Injections #
        self.CheckIfStringClean(UserName)
        self.CheckIfStringClean(Table)

        sql = "SELECT UserID, UserName, FirstName, LastName, AccountEnabled, AccountExpirationDate FROM user WHERE UserName =%s"
        self.DatabaseCursor.execute(sql, UserName)

        #List of user details in the table
        UserDetails=[]
        for row in self.DatabaseCursor:
            l=[]
            l.append(row['UserID'])
            l.append(row['UserName'])
            l.append(row['FirstName'])
            l.append(row['LastName'])
            l.append(row['AccountEnabled'])
            l.append(row['AccountExpirationDate'])
            UserDetails.append(l)

        return UserDetails


    def ReadNeuronInformation(self, UserName:str): # ... #

        pass


    def ShutdownConnection(self): # Closes The Database Connection #

        # Close DB Connection #
        self.DBConnection.close()
        self.Logger.Log('Shutdown Database Connection')


    def GetAllNeurons(self): # gets all neurons with equations#

        allNeurons= self.DatabaseCursor.execute("SELECT neuronId, xCoord, yCoord, zCoord, Assigned_Node, equationId, "
        + "equationText from bgdb.neuron n inner join bgdb.equation e on e.equationId = n.equationId" )

        return allNeurons


    def GetNeuronsOfNode(self, NodeName:str): # gets all neurons from a node#

        allNeuronsOfNode= self.DatabaseCursor.execute("SELECT neuronId, xCoord, yCoord, zCoord, Assigned_Node, "
        + "equationId, equationText from bgdb.neuron n inner join bgdb.equation e on e.equationId = n.equationId "
        + "Where Assigned_Node = %s", (NodeName,))

        return allNeuronsOfNode


    def GetSynapsesOfNodes(self, NodeIds:list): # gets all synapses with equations for a list of neuronId#

        Placeholders = ','.join(NodeIds)

        SynapsesOfNeurons= self.DatabaseCursor.execute("SELECT synapseId, xCoord, yCoord, zCoord, neuronId, equationId, equationText from bgdb.synapse s "
                 + "inner join bgdb.equation e on e.equationId = s.equationId Where neuronId in ("+ Placeholders +")")

        return SynapsesOfNeurons
