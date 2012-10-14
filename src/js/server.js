var child = require('child_process')
var url = require('url')
var query = require('querystring')
var util = require('util')
var fs = require('fs')
var http = require('http')
var async = require('async')

var root_game_dir = './bin/';
var root_script_dir = './src/js/games/';
var process_timeout = 5000;
var error_trace_printed = false

value_conversion_table = {
  T : "tie",
  L : "lost",
  W : "win"
}
process.setMaxListeners(500)

function start_game_process (root_game_dir, game, continuation) {
  var game_p = child.spawn(root_game_dir + 'm' + game.name
                        ,['--interact']
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
  process.on('SIGKILL', kill_game)
  process.on('uncaughtException', handle_uncaught_exception)
  // Unless the game_p has already died.
  game_p.on('exit', function () {
    console.log(game.name + ' exited.')
    process.removeListener('exit', kill_game)
    process.removeListener('SIGINT', kill_game)
    process.removeListener('SIGTERM', kill_game)
    process.removeListener('SIGKILL', kill_game)
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
        callback('error', null)
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
    game_p.stdin.write(encoded_request)
  }
  function handle_request (qry, continuation) {
    if ("getMoveValue" in qry) {
      get_slot(util.format('move_value_response %s', qry.board), function (err, response) {
        if (err) {
          continuation('{"status":"error"}')
        } else {
          continuation(response)
        }
      })
    } else if ("getNextMoveValues" in qry) {
      var request_string = ''
      if (qry.board[0] == '"') {
        request_string = util.format('next_move_values_response %s', qry.board)
      } else {
        request_string = util.format('next_move_values_response %j', qry.board)
      }
      get_slot(request_string, function (err, response) {
        if (err) {
          continuation('{"status":"error"}')
        } else {
          continuation(response)
        }
      })
    } else {
      continuation('{"status":"error"}')
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
}

function start_shell (game) {
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
}

function respond_to_url (the_url, continuation) {
  var parsed = url.parse(the_url)
  var path = parsed.path.split('/')
  var qry = query.parse(path[path.length - 1], ';', '=')
  var game = path[path.length - 2]
  if (game in game_table) {
    if ("getMoveValue" in qry) {
      game_table[game].addRequest(qry, continuation)
    } else if ("getNextMoveValues" in qry) {
      game_table[game].addRequest(qry, continuation)
    } else {
      continuation('{"status":"error","reason":"Did not receive getMoveValue or getNextMoveValues command."')
    }
  } else {
    continuation(util.format('{"status":"error","reason":"%s is not loaded by the server."}', game))
  }
}

function start_game (name, continuation) {
  var game = {
    name : name,
    processes : [],
    broken : false,
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
      if (game.processes.length != 0) {
        continuation(null, game.processes[0])
      } else {
        continuation("error", null)
      }
    }
  }
  start_game_process(root_game_dir, game, function (game_p) {
    game.processes.push(game_p)
    continuation(game)
  })
}

var game_table = {
}

function start_games () {
  fs.readdir(root_game_dir, function add_games_to_table (error, files) {
    if (error) {
      console.log('error, could not read game directory ' + root_game_dir)
      return;
    }
    for (file in files) {
      if (files[file][0] == 'm') {
        var game_name = files[file].slice(1)
        console.log('starting ' + game_name)
        start_game(game_name, function add_game_to_table (game) {
          game_table[game.name] = game
          console.log(game.name + ' added.')
        })
      }
    }
  })
}

start_games()

setTimeout( function () {
  respond_to_url('http://localhost:8080/gcweb/service/gamesman/puzzles/ttt/getMoveValue;blargh=100;board="  O X    "', console.log)
  respond_to_url('http://localhost:8080/gcweb/service/gamesman/puzzles/ttt/getNextMoveValues;blargh=100;board=%20%20O%20X%20%20%20%20', console.log)
  respond_to_url('http://localhost:8080/gcweb/service/gamesman/puzzles/ttt/getNextMoveValues;blargh=100;board="  O X    "', console.log)
  start_shell(game_table.ttt.processes[0])
}, 2000)

setTimeout( function () {
  var server = http.createServer(function (req, res) {
    respond_to_url(req.url, function (body) {
      res.writeHead(200, {
        'Content-Length': body.length,
        'Content-Type': 'text/plain' });
      res.write(body)
      res.end()
    })
  }).listen(8080)
  console.log('Server ready')
}, 500)
