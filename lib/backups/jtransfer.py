"""
    Name: jtransfer.py

    Description: This is a python module to transfer files to/from different
                 computers. 

    Log:

    09/05/2020 Jordan Garcia    Implementation. 
"""
import sys

class FileTransfer(object):
	""" Contains all functions to transfer from a file """

	def __init___(self):
		self.username = ""
		self.password = ""
		self.serverName = ""

		self.fileList = []
		
		self.localMachine = sys.platform()
		self.remoteMachine = sys.platform()
		
		self.windows = False
		self.linux = False
		self.unix = False
		self.macos = False

		self.determine_platform()

		# start gui

	def get_username(self):
		self.username = str(input("Username: "))

	def get_password(self):
		self.password = str(input("Password: "))

	def get_server(self):
		self.serverName = str(input("Server: "))

	def establish_connection(self):
		""" Connects to server with username/password supplied by user """ 

	def determine_platform(self):
		""" Determines what the system platform is """

		print("Local platform: " + self.localMachine)






