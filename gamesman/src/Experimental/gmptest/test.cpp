#include "interface.H"

#include <iostream>
#include <vector>
#include <gmpxx.h>
#include <typeinfo>

using namespace std;

template <typename T> class TTTBoard : public Board<T>
{

public:
  TTTBoard();
  TTTBoard(T position);
  vector<Move<T> *> moves();
  T hash();
  void output(std::ostream &stream);
  
private:
  char grid[3][3];
  
};

template <typename T> TTTBoard<T>::TTTBoard()
{
  int piece;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      switch (piece) {
      case 0:
	grid[i][j] = ' ';
	break;
      case 1:
	grid[i][j] = 'O';
	break;
      case 2:
	grid[i][j] = 'X';
	break;
      default:
	break;
      }
      piece = (piece+1) % 3;
    }
  }
}

template <typename T> TTTBoard<T>::TTTBoard(T position)
{
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      int cell = position % 3;
      switch (cell) {
      case 0:
	grid[i][j] = ' ';
	break;
      case 1:
	grid[i][j] = 'O';
	break;
      case 2:
	grid[j][j] = 'X';
	break;
      default:
	break;
      }
      position /= 3;
    }
  }
}

TTTBoard<string>::TTTBoard(string position)
{
  int index = 0;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      grid[i][j] = position[index];
      index++;
    }
  }
}

template <typename T> T TTTBoard<T>::hash()
{

  T mult = 1;
  T result = 0;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      switch (grid[i][j]) {
      case 'O':
	result += mult * 1;
	break;
      case 'X':
	result += mult * 2;
	break;
      default:
	break;
      }
      mult *= 3;
    }
  }
}

string TTTBoard<string>::hash()
{
  string result = "";
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      result += grid[i][j];
    }
  }
}

template <typename T> vector<Move<T> *> TTTBoard<T>::moves()
{
  vector <Move<T> *> vec;
  return vec;
}

template <typename T> void TTTBoard<T>::output(std::ostream &stream)
{
  //const type_info& info = typeid(i);
  //cout << "Class name: " << info.name();
  T t;
  stream << "Specialized with type " << typeid(t).name();
  for (int i = 0; i < 3; i++) {
    stream << endl << "-------" << endl;
    for (int j = 0; j < 3; j++) {
      stream << "|" << grid[i][j];
    }
    stream << "|";
  }
  stream << endl << "-------" << endl;
}



int main() {

  TTTBoard<int> tttint;
  TTTBoard<long long int> tttlong;
  TTTBoard<mpz_class> tttbignum;
  TTTBoard<string> tttstring;
  
  /* After default constructor */
  cout << "Before hash/unhash: " << endl;
  cout << tttint;
  cout << tttlong;
  cout << tttbignum;
  cout << tttstring;

  /* Hash and unhash */
  tttint = TTTBoard<int>(tttint);
  tttlong = TTTBoard<long long int>(tttlong);
  tttbignum = TTTBoard<mpz_class>(tttbignum);
  tttstring = TTTBoard<string>(tttstring);

  cout << "After hash/unhash: " << endl;
  
  cout << tttint;
  cout << tttlong;
  cout << tttbignum;
  cout << tttstring;
  
  
}
