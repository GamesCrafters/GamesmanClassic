var child = require('child_process')
var url = require('url')
var query = require('querystring')
var util = require('util')
var fs = require('fs')
var http = require('http')
var async = require('async')
var repl = require('repl')

var server_port = 8081
var root_game_dir = './bin/'
var use_remote_game_list = false
var game_list_url = ''
var process_timeout = 5000 // Milliseconds
var error_trace_printed = false
var local_games = []
var java_games  = ['pegsolitaire', 'pyraminx', 'tcross', 'atarigo',
                   'lightsout', 'connect4', 'othello', 'rubik', 'oskarscubel', 'dino', 'y']
var java_host = 'localhost' // 'nyc.cs.berkeley.edu'
var java_port = '8080'
var use_java = false
var allow_game_restart = true

var game_table = {}


function in_array (array, item) {
  for (var i = 0; i < array.length; i++) {
    if (array[i] === item) {
      return true
    }
  }
  return false
}

process.setMaxListeners(500)

function start_game_process (root_game_dir, game, continuation) {
  var game_p = child.spawn(root_game_dir + 'm' + game.name
                           ,['--interact', '--notiers']
                           ,{stdio: 'pipe'})
  function kill_game () {
    console.log('Killing ' + game.name + '.')
    game_p.kill('SIGTERM')
    game_p.kill('SIGKILL')
  }
  function handle_uncaught_exception (err) {
    kill_game()
    if (!error_trace_printed) {
      error_trace_printed = true
      console.log(err)
      console.log(err.stack)
      console.trace(err)
    }
  }
  // On the server shutting down, kill the game_p.
  process.on('exit', kill_game)
  process.on('SIGINT', kill_game)
  process.on('SIGTERM', kill_game)
  process.on('uncaughtException', handle_uncaught_exception)
  // Unless the game_p has already died.
  game_p.exited = false
  game_p.on('exit', function () {
    game_p.exited = true
    console.log(game.name + ' exited.')
    process.removeListener('exit', kill_game)
    process.removeListener('SIGINT', kill_game)
    process.removeListener('SIGTERM', kill_game)
    process.removeListener('uncaughtException', handle_uncaught_exception)
  })
  game_p.stdout.setEncoding('utf8')
  game_p.stderr.setEncoding('utf8')
  game_p.stderr.on('data', function (data) {
    if (/^execvp\(\)/.test(data)) {
      console.log('Failed to start ' + game.name + '.');
    }
  })
  game_p.addRequest = function addRequest (qry, callback) {
    game_p.requests.push(qry, callback)
  }
  function get_slot (request, callback) {
    function inner(data) {
      var output = data.split('\n')
      var result;
      for (var i = 0; i < output.length; i++) {
        result = output[i].split(' =>> ')
        if (result[0] == 'result' ||
            result[0] == ' error') {
          break;
        }
      }
      if (result[0] == ' error') {
        game_p.stdout.removeListener('data', inner)
        callback('error', data)
      } else if (result[0] == 'result') {
        game_p.stdout.removeListener('data', inner)
        callback(null, result[1])
      }
      // Strangely, sometimes data == '\n'
      // Then neither action should be taken, and inner shouldn't be unhooked.
      // That's why game_p.stdout.removeListener is called above and not here.
    }
    game_p.stdout.on('data', inner)
    var encoded_request = util.format('%s\n', request)
    if (!game_p.exited) {
      // This can still crash if another request kills the process between these lines
      game_p.stdin.write(encoded_request)
    }
  }
  function handle_request (qry, continuation) {
    try {
      if (game_p.exited) {
        continuation('{"status":"error", "reason":"Process killed."}')
      }
      var response_done = false
      setTimeout(function () {
        if (!response_done) {
          continuation('{"status":"error", "reason":"Process timed out."}')
        }
      }, process_timeout)
      if ("getMoveValue" in qry) {
        var request_string = ''
        if (qry.board[0] == '"') {
          request_string = util.format('move_value_response %s', qry.board)
        } else {
          request_string = util.format('move_value_response "%s"', qry.board)
        }
        get_slot(request_string, function (err, response) {
          response_done = true
          if (err) {
            console.log(response)
            continuation(util.format('{"status":"error", "subprocess_output": %j}', response))
          } else {
            continuation(response)
          }
        })
      } else if ("getNextMoveValues" in qry) {
        var request_string = ''
        if (qry.board[0] == '"') {
          request_string = util.format('next_move_values_response %s', qry.board)
        } else {
          request_string = util.format('next_move_values_response "%s"', qry.board)
        }
        get_slot(request_string, function (err, response) {
          response_done = true
          if (err) {
            console.log(response)
            continuation(util.format('{"status":"error", "subprocess_output": %j}', response))
          } else {
            continuation(response)
          }
        })
      } else if ("getStart" in qry) {
        request_string = util.format('start_response', qry.board)
        get_slot(request_string, function (err, response) {
          response_done = true
          if (err) {
            console.log(response)
            continuation(util.format('{"status":"error", "subprocess_output": %j}', response))
          } else {
            continuation(response)
          }
        })
      } else {
        response_done = true
        continuation('{"status":"error", "reason":"Could not parse request."}')
      }
    } catch (e) {
      continuation('{"satus":"error", "reason":"Unknown."}')
    }
  }
  var timeout_game = setTimeout(function () {
    console.log('%s timed out', game.name)
    game.broken = true
    kill_game()
  }, 5000)
  function set_broken () {
    game.broken = true
  }
  game_p.on('exit', set_broken)
  game_p.stdout.on('data', function hook_into_process (data) {
    var output = data.split('\n')
    if (output[output.length - 1] != ' ready =>> ') {
      return;
    }
    game_p.stdout.removeListener('data', hook_into_process)
    game_p.requests = async.queue(handle_request, 1)
    clearTimeout(timeout_game)
    game_p.removeListener('exit', set_broken)
    continuation(game_p)
  })
  game_p.on('exit', function () {
    game_p.requests
  })
  game_p.close = function close_game_process() {
    game_p.removeListener('exit', set_broken)
    game_p.kill('SIGTERM')
  }
}

function start_shell (game, continuation) {
  var readline = require('readline')
  var completions = ('quit shutdown start value choices moves ' +
                     'named_moves board remoteness mex result ' +
                     'move_value_response next_move_values_response').split(' ')
  function complete (line) {
    var hits = completions.filter(function (c) { return c.indexOf(line) == 0 })
    return [hits.length ? hits : completions, line]
  }
  var rl = readline.createInterface({
    input: process.stdin
    ,output: process.stdout
    ,completer: complete
  })
  rl.setPrompt(' ready =>> ', 11)
  rl.on('line', function (line) {
    game.stdin.write(line + '\n')
  })
  game.stdout.on('data', function (text) {
    process.stdout.write(text)
  })
  game.on('exit', function () {
    rl.close()
  })
  continuation(rl)
}

function respond_to_java_request(game, parsed, qry, continuation) {
  console.log('Sending request for java game:', game)
  request = {
    host: java_host,
    port: java_port,
    path: parsed.path,
  }
  console.log('request:', request)
  console.log('qry:', qry)
  req = http.request(request, function (res) {
    res.setEncoding('utf8')
    res.on('data', function (data) {
      try {
        console.log(JSON.parse(data))
        continuation(data)
      } catch (syntax_err) {
        continuation('{"status": "error", "reason":"Java server did not return JSON."}')
      }
    })
  })
  req.on('error', function (err) {
    console.log('error when querying java server: ', err.message)
    continuation(util.format(
      '{"status": "error", "reason":"Error when connecting to Java server: %s"}',
      escape(util.format(err.message))))
    // The call to escape ensures that the result is valid JSON,
    // since it can't contain double quotes.
  })
  req.end()
}

function make_check_is_json (continuation) {
  return function check_is_json (data) {
    try {
      console.log(JSON.parse(data))
      continuation(data)
    } catch (syntax_err) {
      console.log('attempt to send:', data)
      continuation('{"status":"error", "reason":"Process did not return JSON."}')
    }
  }
}

function respond_to_url (the_url, continuation) {
  var parsed = url.parse(the_url)
  var qry = query.parse(parsed.query)
  var path = parsed.pathname.split('/')
  var request_type = path[path.length - 1]
  qry[request_type] = true
  var game = path[path.length - 2]
  
  if (game in game_table) {
    if ("getMoveValue" in qry) {
      game_table[game].addRequest(qry, make_check_is_json(continuation))
    } else if ("getNextMoveValues" in qry) {
      game_table[game].addRequest(qry, make_check_is_json(continuation))
    } else if ("getStart" in qry) {
      game_table[game].addRequest(qry, make_check_is_json(continuation))
    } else {
      continuation('{"status":"error","reason":"Did not receive getMoveValue or getNextMoveValues command."}')
    }
  } else if (use_java && in_array(java_games, game)) {
    respond_to_java_request(game, parsed, qry, make_check_is_json(continuation))
  } else if (in_array(local_games, game)) {
    if (!game.broken || allow_game_restart) {
      start_game(game, function (game) {
        add_game_to_table(game)
        game.addRequest(qry, make_check_is_json(continuation))
      })
    } else {
      continuation('{"status":"error","reason":"Game appears to be broken."}')
    }
    // It should be easy to make games close automatically here.
    // When doing so, use the game processes close method.
    // Otherwise, the game will be marked as broken, and won't be
    // started by the server automatically.
  } else {
    continuation(util.format('{"status":"error","reason":"%s is not loaded by the server."}', game))
  }
}

function start_game (name, continuation) {
  var game = {
    name : name,
    processes : [],
    broken : false,
    makeProcess : function (continuation) {
      start_game_process(root_game_dir, game, function (game_p) {
        game.processes.push(game_p)
        continuation(game_p)
      })
    },
    addRequest : function addRequest (qry, continuation) {
      game.getProcess(function add_request (err, proc) {
        if (err) {
          continuation(util.format('{"status":"error","reason":"%s is not ready."}', game.name))
        } else {
          proc.addRequest(qry, continuation)
        }
      })
    },
    getProcess : function getProcess (continuation) {
      // Remove exited processes
      for (var i = 0; i < game.processes.length; i++) {
        if (game.processes[i].exited) {
          game.processes.splice(i, 1)
        }
      }
      if (game.processes.length != 0) {
        continuation(null, game.processes[0])
      } else {
        if (allow_game_restart) {
          game.makeProcess(function (game_p) {
            continuation(null, game_p)
          })
        } else {
          continuation("error", null)
        }
      }
    }
  }
  game.makeProcess(function (game_p) {
    continuation(game)
  })
}

function get_game_list (continuation) {
  http.get(game_list_url, function (response) {
    if (response.statusCode != 200) {
      continuation({message: "http.get status is %s".format(response.statusCode)}, null)
    } else {
      response.on('data', function (text) {
        continuation(null, text)
      }).setEncoding('utf8')
    }
  }).on('error', function (err) {
    continuation(err, null)
  })
}

function add_game_to_table (game) {
  game_table[game.name] = game
  console.log(game.name + ' added.')
}

function start_games (start_all) {
  fs.readdir(root_game_dir, function add_games_to_table (error, files) {
    if (error) {
      console.log('error, could not read game directory ' + root_game_dir)
    } else {
      for (file in files) {
        if (files[file][0] == 'm') {
          var game_name = files[file].slice(1)
          local_games.push(game_name)
          if (start_all) {
            console.log('starting ' + game_name)
            start_game(game_name, add_game_to_table)
          }
        }
      }
    }
  })
  if (use_remote_game_list) {
    get_game_list(function (err, text) {
      if (err) {
        console.log(util.format('could not get game list from %s, but got error %s', game_list_url, err.message))
      } else {
        games = text.split(' ')
        for (game in games) {
          start_game(game_name, add_game_to_table)
        }
      }
    })
  }
}

// Change the false here to true to automatically start all the games.
start_games(false)

setTimeout(function () {
  try {
    var server = http.createServer(function (req, res) {
      try {
        respond_to_url(req.url, function (body) {
          res.writeHead(200, {
            'Content-Length': body.length,
            'Content-Type': 'text/plain',
            'Access-Control-Allow-Origin': '*'});
          res.write(body)
          res.end()
        })
      } catch (error) {
        console.log('received error:', error, 'in response to request:', req)
      }
    }).listen(server_port).on('error', function (e) {
      console.log('Problem starting server. Check that port', server_port, 'is available.')
    })
  } catch (e) {
  }
}, 0)
