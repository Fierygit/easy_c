/*
 * @Author: Firefly
 * @Date: 2020-01-18 09:22:10
 * @LastEditTime : 2020-01-19 20:16:48
 * @LastEditTime : 2020-01-18 09:32:09
 */

#include "scan.h"
#include <fstream>
#include "global.h"
using namespace std;

enum state {
  START,
  INNUM,
  INID,
  INCOMMENT,
  PREDIV,
  OCOMMENT,
  DONE,
  INOPER,
  INCMP,
  INASSIGN
};

//* 改变长度要改代码
string keywords[] = {"if", "else", "int", "return", "void", "while", "print"};

/*

assign        =
symbol:       / * - + { } [ ] 
keywords
cmp         <= >= == && || !
comment       \\/\ \*\/
oper        += -= *= /=

*/

bool isKeyword(string prime) {
  for (int i = 0; i < 7; i++) {
    if (keywords[i] == prime) return true;
  }
  return false;
}

void tokenOne(string type, int start, int end, int row, int column) {
  string temp_value = sourceCode.substr(start, end - start + 1);
  Token* temp;
  if (type == "ID" && isKeyword(temp_value)) {
    temp = new Token("KEYWORD", temp_value, row, column);
  } else {
    temp = new Token(type, temp_value, row, column);
  }
  tokenCode.push_back(temp);
}

void tokenAll() {
  //初始状态
  state curState = START;
  int row, col, index, startIndex;

  row = col = index = startIndex = 0;

  while (true) {
    // todo 回来检查边界
    if (index >= sourceCode.length()) break;

    //每次读取一个源文件字符
    char c = sourceCode[index++];
    col++;

    switch (curState) {
      case START:
        startIndex = index - 1;
        if (isdigit(c)) {
          curState = INNUM;
        } else if (isalpha(c)) {
          curState = INID;
        } else if (c == ' ' || c == '\t') {
          // do nothing !
        } else if (c == '\n') {
          row++;
          col = 0;
          // do nothing
        } else if (c == '{' || c == '}' || c == ',' || c == '(' || c == ')' ||
                   c == ';' || c == '[' || c == ']') {
          tokenOne("SYMBOL", startIndex, index - 1, row, col);
          curState = DONE;
        } else if (c == '=') {
          curState = INASSIGN;
        } else if (c == '>' || c == '<' || c == '!') {
          curState = INCMP;
        } else if (c == '+' || c == '-' || c == '*') {
          curState = INOPER;
        } else if (c == '/') {
          curState = PREDIV;
        } else {
          cout << "<<< !!!!! error !!!!! >>> \n" << sourceCode[index-1] << " is not expected!!!\n" ;
          cout << "at row: " << row << "; col:" << col << endl;;
          exit(0);
        }
        break;

      case INASSIGN:
        if (c == '=') {
          tokenOne("CMP", startIndex, index - 1, row, col);
          curState = DONE;
        } else {
          index--,col--;
          tokenOne("ASSIGN", startIndex, index - 1, row, col);
          curState = DONE;
        }
        break;

      case INCMP:
        if (c == '=') {
          tokenOne("CMP", startIndex, index - 1, row, col);
          curState = DONE;
        } else {
          index--,col--;
          tokenOne("CMP", startIndex, index - 1, row, col);
          curState = DONE;
        }
        break;

      case INOPER:
        if (c == '=') {
          tokenOne("ASSIGN", startIndex, index - 1, row, col);
          curState = DONE;
        } else {
          index--,col--;
          tokenOne("SYMBOL", startIndex, index - 1, row, col);
          curState = DONE;
        }
        break;

      case INNUM:
        if (!isdigit(c)) {
          index--,col--;
          tokenOne("NUM", startIndex, index - 1, row, col);
          curState = DONE;
        } else {
          curState = INNUM;
        }
        break;

      case INID:
        if (!isalpha(c)) {
          index--,col--;
          tokenOne("ID", startIndex, index - 1, row, col);
          curState = DONE;
        } else {
          curState = INID;
        }
        break;

      case INCOMMENT:
        if (c != '*')
          curState = INCOMMENT;
        else
          curState = OCOMMENT;
        break;

      case PREDIV:
        if (c == '*')
          curState = INCOMMENT;
        else if (c == '=') {
          tokenOne("ASSIGN", startIndex, index - 1, row, col);
          curState = DONE;
        } else {
          tokenOne("SYMBOL", startIndex, index - 1, row, col);
          curState = DONE;
        }
        break;

      case OCOMMENT:
        if (c == '/') {
          tokenOne("COMMENT", startIndex + 2, index - 2, row, col);
          curState = DONE;
        } else
          curState = INCOMMENT;
        break;

      case DONE:
        index--,col--;
        curState = START;
        break;

      default:
        break;
    }
  }
}


bool getSourceCodeFromFile(string file) {
  ifstream ifs;
  ifs.open(file);
  if (!ifs.is_open()) return false;

  string line;
  while (getline(ifs, line)) {
    sourceCode += line;
    sourceCode += "\n";
  }
  return 1;
}