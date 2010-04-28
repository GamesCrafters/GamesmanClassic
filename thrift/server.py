#!/usr/bin/python
from subprocess import Popen, PIPE
from thrift.Thrift import TException
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol
from thrift.server import TServer
from gamesman import *
from gamesman.ttypes import *

class RequestHandler(GamestateRequestHandler.Iface):
    
    def __init__(self, path):
        self.path = path
    
    def getMoveValue(self, gameName, configuration):
        results = self.get_move_value_base(gameName, configuration,
                                           '--GetMoveValue')
        raise TException('not implemented')
    
    def getNextMoveValues(self, gameName, configuration):
        results = self.get_move_value_base(gameName, configuration,
                                           '--GetNextMoveValues')
        raise TException('not implemented')
    
    def get_move_value_base(self, game, configuration, flag):
        self.verify_name(game)
        board, player, variant, extra = self.parse_configuration(configuration)
        arguments = [self.path + '/' + game, board, player, variant, extra]
        process = Popen(arguments, stdout=PIPE, stderr=PIPE, cwd=self.path)
        out, err = process.communicate()
        if err:
            raise TException(game + ' raised an error: ' + err)
        return out
    
    def parse_configuration(self, configuration):
        print configuration
        return None
    
    def verify_name(self, name):
        if re.match(r'^[a-zA-Z\d]+$', name) is None:
            raise TException(name + ' is not a valid game name')
    
    def verify_arguments(self, *args):
        pass

if __name__ == '__main__':
    port = 24444
    path = '../bin'
    handler = RequestHandler(path)
    processor = GamestateRequestHandler.Processor(handler)
    transport = TSocket.TServerSocket(port)
    transport_factory = TTransport.TBufferedTransportFactory()
    protocol_factory = TBinaryProtocol.TBinaryProtocolFactory()
    server = TServer.TThreadedServer(processor, transport, transport_factory,
                                     protocol_factory)
    server.serve()
