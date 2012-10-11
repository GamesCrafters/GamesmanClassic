var child = require('child_process')
var url = require('url')
var query = require('querystring')
var util = require('util')
var vm = require('vm')
var fs = require('fs')
var async = require('async')

var root_game_dir = './bin/';
var root_script_dir = './src/js/games/';

value_conversion_table = {
  T : "tie",
  L : "lost",
  W : "win"
}

function start_game_process (root_game_dir, game) {
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
    console.log(err)
    console.trace(err)
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
    console.log('request queued')
  }
  function get_slot (name, position, callback) {
    function inner(data) {
      // console.log('data', data)
      // console.log('inner')
      var output = data.split('\n')
      // var still_ready = output.split(' =>> ')
      var result = output[0].split(' =>> ')
      // console.log('result = ' + result)
      if (result[0] == 'error') {
        callback('error', null)
      } else if (result[0] == 'result') {
        callback(null, result[1])
      }
      game_p.stdout.removeListener('data', inner)
    }
    game_p.stdout.on('data', inner)
    game_p.stdin.write(util.format('%s %d\n', name, position))
  }
  function handle_request (qry, continuation) {
    // console.log('handling request')
    // console.log(qry)
    var position = game.posFromBoard(qry.board)
    if ("getMoveValue" in qry) {
      // console.log('doing getMoveValue')
      async.waterfall([
        function (callback) {
          // console.log('getting remoteness')
          get_slot('remoteness', position, function (err, remoteness) {
            callback(err, parseInt(remoteness))
          })
        },
        function (remoteness, callback) {
          // console.log('getting value')
          get_slot('value', position, function (err, value) {
            callback(err, {
              remoteness : remoteness,
              value : value_conversion_table[value[0]]
            })
          })
        },
      ],function (err, result) {
        // console.log('in game_p_error')
        if (err) {
          continuation('{"status":"error"}')
        } else {
          continuation(result)
        }
      })
    }
  }
  game_p.requests = async.queue(handle_request, 1);
  game_p.stderr.pipe(process.stderr)
  return game_p
}

function start_shell (game) {
  var readline = require('readline')
  var completions = ('quit shutdown start value choices moves ' +
                     'named_moves board remoteness mex result').split(' ')
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

function respond_to_url (continuation, the_url) {
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
    continuation(util.format('{"status":"error","reason":"%s is not loaded by the server."', game))
  }
}

function start_game (name) {
  var game = {
    name : name,
    processes : [],
    addRequest : function addRequest (qry, continuation) {
      // console.log('queueing request')
      this.chooseProcess().addRequest(qry, continuation)
    },
    chooseProcess : function chooseProcess () {
      return this.processes[0]
    },
    posFromBoard : function posFromBoard (board_string) {
      return 0
    },
    boardFromPos : function boardFromPos (position) {
      return "         "
    }
  }
  var script_filename = root_script_dir + name + '.js'
  // TODO: run the script.
  game.processes.push(start_game_process(root_game_dir, game))
  return game
}

var game_table = {
  ttt : start_game("ttt")
}

// respond_to_url(console.log,
//                'http://localhost:8080/gcweb/service/gamesman/puzzles/ttt/getMoveValue;blargh=100;board="  O X   "')
start_shell(game_table.ttt.processes[0])
