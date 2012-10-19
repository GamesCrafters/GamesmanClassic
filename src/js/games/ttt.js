function init (game) {
  var powers_of_3 = [1, 3, 9, 27, 81, 243, 729, 2187, 6561]
  var boardsize = 9
  var game_map = {
    ' ': 0,
    'o': 1,
    'x': 2
  }
  game.posFromBoard = function posFromBoard (board_string) {
    var position = 0;
    for(var i = 0; i < boardsize; i++) {
      position += powers_of_3[i] * game_map[board_string[i]];
    }
    return position
  }
  game.boardFromPos = function boardFromPos (pos) {
    var board = '         '
    for(var i = 8; i >= 0; i--) {
      if(pos >= game_map.x * powers_of_3[i]) {
        board[i] = 'x'
        pos -= game_map.x * powers_of_3[i]
        
      } else if(pos >= game_map.o * powers_of_3[i]) {
        board[i] = 'o'
        pos -= game_map.o * powers_of_3[i]
      } else if(pos >= game_map[' '] * powers_of_3[i]) {
        board[i] = ' '
        pos -= game_map[' '] * powers_of_3[i]
      }
    }
    return board
  }
}
