#!/usr/bin/python
import re
import urllib
from subprocess import Popen, PIPE
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol
from thrift.server import TServer
from gamesman import *
from gamesman.ttypes import *
from daemon import DaemonContext

class RequestHandler(GamestateRequestHandler.Iface):
    
    TUPLE_PATTERN = r'\([^,]*\s*,[^,]*\s*,[^,]*\)'
    GAMES = ['1210', '1ton', '369mm', '3spot', 'Lgame', 'abalone', 'achi',
             'ago', 'asalto', 'ataxx', 'baghchal', 'blocking', 'cambio',
             'change', 'cmass', 'con', 'ctoi', 'dao', 'dinododgem', 'dnb',
             'dodgem', 'fandan', 'foxes', 'gobblet', 'graph', 'hex', 'horse',
             'hshogi', 'iceblocks', 'igo', 'joust', 'kono', 'lctl', 'lewth',
             'lite3', 'loa', 'mancala', 'nim', 'nuttt', 'ooe', 'othello',
             'pylos', 'qland', 'quarto', 'quickchess', 'qx', 'rInfin2',
             'rcheckers', 'rubix', 'seega', 'sim', 'slideN', 'snake',
             'squaredance', 'stt', 'swans', 'tactix', 'tilechess', 'tootnotto',
             'tore', 'ttc', 'ttt', 'tttier', 'win4', 'winkers', 'wuzhi',
             'xigua']
    
    def __init__(self, path):
        self.path = path
    
    def getMoveValue(self, game, configuration):
        try:
            results = self.get_move_value_base(game, configuration,
                                               '--GetMoveValue')
            match = re.search(self.TUPLE_PATTERN, results)
            if match is None:
                raise GameException('failed to get move value for %s %s' % \
                                     (game, configuration))
            move_value = self.parse_move_value(match.group())
            return GetMoveResponse(status='ok', response=move_value)
        except GameException as e:
            return GetMoveResponse(status='error', message=e.args[0])
    
    def getNextMoveValues(self, game, configuration):
        try:
            results = self.get_move_value_base(game, configuration,
                                               '--GetNextMoveValues')
            move_values = []
            for matched in re.findall(self.TUPLE_PATTERN, results):
                move_values.append(self.parse_move_value(matched.strip()))
            return GetNextMoveResponse(status='ok', response=move_values)
        except GameException as e:
            return GetNextMoveResponse(status='error', message=e.args[0])
    
    def get_move_value_base(self, game, configuration, flag):
        '''Securely invokes the specified game binary with the given board
        configuration data.
        This method defends against shell injections by using Popen, which
        doesn't go through the shell. However, an attacker may still specify
        a vulnerable binary; to prevent this, we verify the game name against
        a whitelist of valid games. Finally, we only permit a limited set of
        configuration parameters and strip out any non-alphanumeric or "normal"
        characters. This is a defensive measure that mitigates exploits in
        the actual game binaries.
        '''
        
        self.verify_game(game)
        board, player, variant = self.parse_configuration(configuration)
        arguments = [self.path + '/m' + game, flag, board, player, variant]
        process = Popen(arguments, stdout=PIPE, stderr=PIPE, cwd=self.path)
        out, err = process.communicate()
        if err:
            raise GameException(game + ' raised an error: ' + err)
        return out
    
    def parse_configuration(self, configuration):
        fields = configuration.strip(';').split(';')
        board, player, variant = '', '1','1'
        for field in fields:
            prop, value = field.split('=', 1)
            if prop == 'board':
                board = urllib.unquote(value)
            elif prop == 'player':
                player = str(int(value))
            elif prop == 'option':
                variant = str(int(value))
            # Ignore arguments outside of our whitelist.
        return [self.sanitize_argument(a) for a in board, player, variant]
    
    def parse_move_value(self, result):
        match = re.match(r'\(([^,]*),\s+([^,]*),\s+([^,]*)\)', result)
        if match is None:
            raise GameException('malformed move value: %s' % (result,))
        board, move, value = match.group(1, 2, 3)
        value = self.decode_value(int(value))
        if not move:
            move = None
        return GamestateResponse(board=board, move=move, value=value)
    
    def decode_value(self, value):
        values = ('undecided', 'win', 'lose', 'tie')
        if 0 <= value < len(values):
            return values[value]
        return 'undecided'
    
    def verify_game(self, game):
        if game not in self.GAMES:
            raise GameException(name + ' is not a valid game name')
    
    def sanitize_argument(self, argument):
        '''Removes all characters that aren't common typeable ASCII characters
        from the given program argument.
        '''
        # The only visible ASCII characters are Space through Tilde.
        return re.sub('[^ -~]', '', argument)

class GameException(Exception):
    pass

if __name__ == '__main__':
    with DaemonContext():
        port = 24444
        path = '../bin'
        handler = RequestHandler(path)
        processor = GamestateRequestHandler.Processor(handler)
        transport = TSocket.TServerSocket(port)
        transport_factory = TTransport.TBufferedTransportFactory()
        protocol_factory = TBinaryProtocol.TBinaryProtocolFactory()
        server = TServer.TThreadedServer(processor, transport,
                                         transport_factory,
                                         protocol_factory)
        print 'GamesmanClassic server listening on port %d...' % (port,)
        server.serve()
