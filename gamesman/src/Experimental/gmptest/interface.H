#ifndef INTERFACE_H
#define INTERFACE_H

#include <vector>

using namespace std;

template <typename T> class Move;
template <typename T> class Board;

template <typename T> class Move
{
  virtual Board<T> *apply() = 0;
  virtual Board<T> *undo() = 0;
};

template <typename T> class Board {
public:
  operator T () {
    return hash();
  }
  virtual vector<Move<T> *> moves() = 0;
  virtual void output(std::ostream &stream) = 0;
  virtual T hash() = 0;
};

template <typename T> std::ostream &operator<< (std::ostream &stream, Board<T> &board)
{
  board.output(stream);
  return stream;
}

#endif /* INTERFACE_H */
