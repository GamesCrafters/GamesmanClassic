#!/usr/bin/python
import re
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
    
    def getMoveValue(self, game, configuration):
        results = self.get_move_value_base(game, configuration,
                                           '--GetMoveValue')
        move_value = self.parse_move_value(results)
        if move_value is None:
            raise TException('failed to get move value for %s %s' % \
                                 (game, configuration))
        return GetMoveResponse(status='ok', response=move_value)
    
    def getNextMoveValues(self, game, configuration):
        results = self.get_move_value_base(game, configuration,
                                           '--GetNextMoveValues')
        if results is None:
            raise TException('failed to get move values for %s %s' % \
                                 (game, configuration))
        pattern = r'\([^,]+, [^,]+, [^,]+\)'
        move_values = []
        for matched in re.findall(pattern, results):
            move_values.append(self.parse_move_value(matched.strip()))
        return GetNextMoveResponse(status='ok', response=move_values)
    
    def get_move_value_base(self, game, configuration, flag):
        self.verify_name(game)
        board, player, variant, extra = self.parse_configuration(configuration)
        arguments = [self.path + '/m' + game, flag, board, player, variant,
                     extra]
        print arguments
        process = Popen(arguments, stdout=PIPE, stderr=PIPE, cwd=self.path)
        out, err = process.communicate()
        if err:
            raise TException(game + ' raised an error: ' + err)
        return out
    
    def parse_configuration(self, configuration):
        fields = configuration.strip(';').split(';')
        board, player, variant, extra = '', '1', '1', []
        for field in fields:
            prop, value = field.split('=', 1)
            if prop == 'board':
                board = value
            elif prop == 'player':
                player = str(int(value))
            elif prop == 'variant':
                variant = str(int(value))
            else:
                extra.append('--' + prop + '=' + value)
        extra = ' '.join(extra)
        return board, player, variant, extra
    
    def parse_move_value(self, result):
        match = re.match(r'\(([^,]+), ([^,]+), ([^,]+)\)', result)
        if match is None:
            return None
        board, move, value = match.group(1, 2, 3)
        value = self.decode_move(int(move))
        return GamestateResponse(board=board, move=move, value=value)
    
    def decode_move(self, move):
        values = ('undecided', 'win', 'lose', 'tie')
        if 0 <= move < len(values):
            return values[move]
        return 'undecided'

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
    print 'GamesmanClassic server listening on port %d...' % (port,)
    server.serve()
