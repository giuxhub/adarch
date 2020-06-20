import log
import connection

class Core: 

    DEBUG = log.DEBUG
    INFO = log.INFO
    WARNING = log.WARNING
    ERROR = log.ERROR
    CRITICAL = log.CRITICAL
    
    def __init__(self, sock, port, active_sock, ip): 
        self.__sock = -1 
        self.__port = -1 
        self.__active_sock = -1
        self.__ip = ''
        self.sock = sock 
        self.port = port 
        self.active_sock = active_sock
        self.ip = ip
      
    @property        
    def sock(self):
        return self.__sock
        
    @property        
    def port(self):
        return self.__port

    @property        
    def active_sock(self):
        return self.__active_sock
                
    @property        
    def ip(self):
        return self.__ip
        
    @sock.setter
    def sock(self, val):
        if self.__sock < 0:
            self.__sock = val
        
    @port.setter
    def port(self, val):
        if self.__port < 0:
            self.__port = val
      
    @active_sock.setter
    def active_sock(self, val):
        if self.__active_sock < 0:
            self.__active_sock = val
                  
    @ip.setter
    def ip(self, val):
        if self.__ip == '':
            self.__ip = val
        
    def log(self, lv, s):
        log.write(lv, "[{}::{}]: {}".format(self.ip, self.port, s))
        
    def shutdown(self):
        connection.shutdown(self.sock, self.active_sock)
        
    def send(self, s):
        x = s.encode('utf-8')
        try:
            connection.send(self.sock, self.active_sock, x, len(x))
        except TypeError:
            print('TypeError:\n\t{}\t{}'.format(x, type(x)))
