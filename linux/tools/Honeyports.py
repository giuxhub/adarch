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
        
        params = "-A INPUT -s {} -j REJECT".format(ip)
        subprocess.run(['iptables', params])
        
    def start(self, b):
        super().shutdown()