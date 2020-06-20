from Core import Core
import subprocess

class Honeyports(Core):
    
    MSG = ""
    
    def __init__(self, sock, port, active_sock, ip): 
        super().__init__(sock, port, active_sock, ip)
        self.__blacklist(ip)
        
    def __blacklist(self, ip):
        if Honeyports.MSG != "":
            super().send(Honeyports.MSG)            
        super().shutdown()
        
		params = "advfirewall firewall add rule name=block_{ip} dir=in action=block remoteip={ip} localport=all protocol=tcp".format(ip=ip)
        subprocess.run(['netsh', params])
        
    def start(self, b):
        super().shutdown()