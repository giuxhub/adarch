from contextlib import contextmanager
from Core import Core
import subprocess
import sqlite3

@contextmanager
def db_helper(path):
    conn = sqlite3.connect(path)
    cursor = conn.cursor()
    yield cursor
    try:
        conn.commit()
    finally:
        conn.close()
            
            
class Invisiport(Core):
    PORT = 443
        
    def __init__(self, sock, port, active_sock, ip): 
        super().__init__(sock, port, active_sock, ip)
        try:
            self.__startup_blacklist()
            self.__blacklist(ip, port)
        except:
            super().shutdown()
               
    def __startup_blacklist(self):
        with db_helper('invisiport_blacklist.db') as c:
            try:
                c.execute("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='blacklist'")
                if c.fetchall()[0][0] > 0:
                    return
                else:
                    c.execute(''' CREATE TABLE blacklist (
                        ip TEXT NOT NULL,
                        time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                        PRIMARY KEY(ip))''')

            except Exception as e:
                super().log(Core.WARNING, 'exception occured {}'.format(e))
            
    def __add_blacklist(self, ip):
        with db_helper('invisiport_blacklist.db') as c:
            try:
                c.execute("INSERT INTO blacklist (ip) VALUES('{}')".format(ip))
            except Exception as e:
                super().log(Core.WARNING, 'exception occured {}'.format(e))
            
    def __check_blacklist(self, ip):
        with db_helper('invisiport_blacklist.db') as c:
            try:
                for x in c.execute("SELECT ip FROM blacklist WHERE ip='{}'".format(ip)):
                    if ip in x:
                        return True
                    else:
                        return False
            except Exception as e:
                super().log(Core.WARNING, 'exception occured {}'.format(e)) 
            
            
    def __blacklist(self, ip, port):
        if self.__check_blacklist(ip):
            return
        
        params = "-A INPUT -s {} -p tcp ! --destination-port {} -j DROP".format(ip, Invisiport.PORT)
        subprocess.run(['iptables', params])
        params = "-t nat -A PREROUTING -s {} -p tcp --dport {} -j REDIRECT --to-port {}".format(ip, port, Invisiport.PORT)
        subprocess.run(['iptables', params])

        self.__add_blacklist(ip)
        super().shutdown() 
    
    def start(self, b):
        super().shutdown()  
    
    
