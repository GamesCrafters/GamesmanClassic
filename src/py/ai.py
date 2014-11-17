import os
import json
AI_DIRECTORY = './src/py/ai/'

class AiRequest(object):
    def __init__(self, handler, query, command = None):
        self.handler = handler
        self.query = query
        if command == None:
            self.command = 'next_move_values_response {}'.format(query['board'])
        else:
            self.command = command
    
    def play(self, response):
        return response

    def over(self):
        self.handler.respond(json.dumps({"over": True}))

    def parse_response(self, response):
        if(response):
            return json.loads(response)['response']
        return []

    def respond(self, response):
        moves = self.parse_response(response)
        if len(moves) == 0:
            self.over()
            return
        ai_response = self.play(moves)
        output = {
            "over": False,
            "response":{
                "move": ai_response[0],
                "url_args": ai_response[1],
            },
        }
        self.handler.respond(json.dumps(output))

def AiRequest_factory(handler, query):
    ai_type = query['ai_type'] + '.py'
    found = False
    for file in os.listdir(AI_DIRECTORY):
        if ai_type == file:
            execfile(AI_DIRECTORY + file)
            found = True
            break
    if not found:
        raise IOError("AI File not found") 
    return eval(query['ai_type'] + '(handler, query)')
