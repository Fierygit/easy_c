/*
 * @Author: Firefly
 * @Date: 2020-01-18 09:23:25
 * @Descripttion:
 * @LastEditTime : 2020-01-28 21:22:47
 */

#include <iostream>
#include <map>
#include <vector>
using namespace std;

#ifndef GLOBAL_H
#define GLOBAL_H

struct Token {
  string type;
  string value;
  int row;
  int column;
  Token(string type, string value, int row, int column) {
    this->type = type;
    this->value = value;
    this->row = row;
    this->column = column;
  }
};

//源代码用string存储
extern string sourceCode;
//词法接受后
extern vector<Token *> tokenCode;

enum NodeKind {
  INT_DECLARE,
  VOID_DECLARE,
  ID,
  PARAMS,
  PARAM,
  PARAM_DECLARE,
  VAR_DECLARE,
  FUNCTION,
  COMPONENT,
  PRINT,
  STR,
  RETURN,
  ASSIGN,
  CMP,
  OPTION_ADD,
  OPTION_MUL,
  NUM,
  ITERAT,
  SELECTION,
  ARGS,
  CALL
};

struct TreeNode {
  struct TreeNode *child[4];  //四个子节点
  struct TreeNode *sibling;   //存储兄弟节点
  int tokenIndex;             //存储代码的位置, 可以获取到信息
  NodeKind nodekind;          //存储类型
  string value;

  bool isVisit;  // 中间代码生成的时候用
};

extern TreeNode *root;
extern string parseInfo;  // 生成语法树的过程

typedef vector<string> vstr;
struct FunctionInfo {
  int param_num;       // 参数的个数
  vstr param_type;     // 每个参数的类型
  string return_type;  // 返回的类型
  TreeNode *node;      //记录声明的节点在哪里
};

struct VariableInfo {
  string name;
  int lineNo;      //所在的代码行数
  int location;    // 在内存的位置
  string type;     //变量的类型， 只有一个类型 INT
  TreeNode *node;  //记录声明的节点在哪里
};

// 记录每一个 函数的信息符号表, 全局变量也在这里
extern map<string, FunctionInfo> fun_table;

// 记录 每一个变量声明的地方  < 函数名字，  < 变量名字， Variableinfo  >
extern map<string, vector<VariableInfo> > var_table;
// 每个函数一个符号表

extern string midCodeInfo;

struct MidArgs {
  // type 分为 STR, ID， TMP,  NUM；
  // bool  用INT  0  1 表示
  string type;
  string value;
  // 代码有全局变量和非全局变量
  // 到符号表里面找， 全变量用伪标签
  // 堆： 自己创建的内存（不用）   栈： 函数临时变量   全局区：
  // 静态和全局变量区（伪标签实现） 字符串 属于全局区
  bool isGlobal;
  int offset;  //栈中, type = tmp, id                  全局变量直接用！！！，
               //不用找
};
// 函数调用     call(10, a);
// op = param arg1 = a  arg2 = NULL   op = param  arg1 = 10  arg2 = NULL
// op = call  arg1 = id       op  = return arg1 = id or null
struct MidCodeItem {
  string dest;
  string op;
  MidArgs *arg1;
  MidArgs *arg2;
};
struct MidCode {
  string funcName;
  int stackSize;  // 只有整形的值， 只考虑4字节， 因此只需要偏移量, 动态变化
  vector<MidCodeItem *> item;
};

extern vector<MidCode *> midCode;
// 声明 ID 加入， value随机初始     TMP 生成一条语句加入 ， 或更改id，
extern map<pair<string, string>, MidArgs *> stackInfo;  // ID, TMP, 加入

extern vector<string> rodata;
//汇编代码的生成
extern string asmCode;

#endif  // ! GLOBAL_H_INCLUDE