var child = require('child_process')

var root_game_dir = './bin/';

function load_game (root_game_dir, game_name) {
  var game = child.spawn(root_game_dir + 'm' + game_name
                        ,['--interact']
                        ,{stdio: 'pipe'})
  function kill_game () {
    console.log('Killing ' + game_name + '.')
    game.kill('SIGKILL')
  }
  // On the server shutting down, kill the game.
  process.on('exit', kill_game)
  process.on('SIGINT', kill_game)
  // Unless the game has already died.
  game.on('exit', function () {
    console.log(game_name + ' exited.')
    process.removeListener('exit', kill_game)
    process.removeListener('SIGINT', kill_game)
  })
  game.stdout.setEncoding('utf8')
  game.stderr.setEncoding('utf8')
  game.stderr.on('data', function (data) {
    if (/^execvp\(\)/.test(data)) {
      console.log('Failed to start ' + game_name + '.');
    }
  })
  game.stderr.pipe(process.stderr)
  return game
}

function split_output (output) {
  var s = output.split(' =>> ')
  return s[0], s[1]
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
    split_output(text)
    process.stdout.write(text)
  })
  game.on('exit', function () {
    rl.close()
  })
}

var ttt = load_game(root_game_dir, 'ttt')
start_shell(ttt)
