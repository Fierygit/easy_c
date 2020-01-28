/*
 * @Author: Firefly
 * @Date: 2020-01-20 21:03:09
 * @Descripttion:
 * @LastEditTime : 2020-01-23 23:41:28
 */

#include "analyse.h"
#include <sstream>
#include "util.h"
void table_init();
string cur_fun = "global";

// 这里不用指针， 要自己区分不同的节点

FunctionInfo newFunInfo(int param_num, vstr vs, string return_type,
                        TreeNode* node) {
  FunctionInfo temp;
  temp.param_num = param_num;
  temp.param_type = vs;
  temp.return_type = return_type;
  temp.node = node;
  return temp;
}

// 但是返回 一个 temp ， temp应该是栈中的位置， 返回后清空， 为什么还生效
VariableInfo newVarInfo(string name, int lineNo, int location, string type,
                        TreeNode* node) {
  VariableInfo temp;
  temp.name = name;
  temp.lineNo = lineNo;
  temp.location = location;
  temp.type = type;
  temp.node = node;
  return temp;
}



void error_log(string errorType, TreeNode* node, string info) {
  cout << errorType << " error !!!  at line "
       << tokenCode[node->tokenIndex]->row << endl;
  cout << info << endl;
}

// 节点为FUNCTION 的传进来
void insertFunction(TreeNode* node) {
  NodeKind NodeKind = node->nodekind;
  if (mapNode(NodeKind) != "FUNCTION") {
    cout << "insert fun error!!!";
  }

  string return_type = mapNode(node->child[0]->nodekind);  // 返回类型
  string name = node->child[1]->value;                     // 函数名字
  TreeNode* params = node->child[2];                       // 参数

  vstr p;
  if (params != NULL) {
    TreeNode* temp = params->child[0];
    while (temp != NULL) {
      // cout << temp->child[0]->value << " * " << endl;
      p.push_back(temp->child[0]->value);
      temp = temp->sibling;
    }
  }

  // cout << p.size() << endl;
  fun_table[name] = newFunInfo(p.size(), p, return_type, node);
  var_table[cur_fun].push_back(
      newVarInfo(name, tokenCode[node->tokenIndex]->row, 0, "FUNC", node));
}

void table_init() {
  // 初始的时候， 只有有一个输出函数， 暂时不加入输入函数，
  // 如果汇编容易输入那再加入
  vstr null;
  fun_table["output"] = newFunInfo(1, null, "INT", NULL);

  //全局变量里面也加上去
  var_table["global"].push_back(newVarInfo("output", 0, 0, "FUNC", NULL));
  // 初始化结束！！！！！！！
}

bool findInCurTable(TreeNode* node) {
  string name;
  switch (node->nodekind) {
    case VAR_DECLARE:
      name = node->child[1]->value;
      break;
    case PARAM:
      name = node->child[1]->value;
      break;
    default:
      break;
  }

  vector<VariableInfo> vi = var_table[cur_fun];
  bool flag = 1;
  if (vi.size() != 0) {
    for (int i = 0; i < vi.size(); i++) {
      if (vi[i].name == name) {
        flag = 0;
        error_log("redefine", node,
                  name + " has been define in line " + int2str(vi[i].lineNo));
        exit(0);
      }
    }
  }
  return flag;
}

void insertVariable(TreeNode* node) {
  string name;
  switch (node->nodekind) {
    case VAR_DECLARE:
      name = node->child[1]->value;
      break;
    case PARAM:
      name = node->child[1]->value;
      break;
    default:
      break;
  }
  int lineNo = tokenCode[node->tokenIndex]->row;
  var_table[cur_fun].push_back(newVarInfo(name, lineNo, 0, "ID_INT", node));
}

// 检查函数定义， 前面已经检查过变量声明了， 已经加入了符号表
void checkId(TreeNode* node) {
  vector<VariableInfo> v = var_table[cur_fun];
  bool flag = 0;
  for (int i = 0; i < v.size(); i++) {
    if (v[i].name == node->value) {
      flag = 1;
    }
  }
  v = var_table["global"];
  for (int i = 0; i < v.size(); i++) {
    if (v[i].name == node->value) {
      flag = 1;
    }
  }
  if (!flag) {
    error_log("No Declare", node,
              node->value + " has not been declared!!!!!!!!");
    exit(0);
  }
}

// 检查函数调用 的 参数个数是否正确，  //todo 类型是否一致先 //!不做
/*
CALL(34)
                       ID(33):	max
                       ARGS(36)
                               ID(35):	e
                               ID(37):	c
                               ID(39):	d
*/
void checkCall(TreeNode* node) {
  string name = node->child[0]->value;
  FunctionInfo fi = fun_table[name];

  int num = fi.param_num;
  // cout <<  mapNode(node->nodekind) <<endl;

  TreeNode* args = node->child[1];
  // cout << mapNode(args->nodekind) <<endl;

  if (args == NULL && num != 0) {
    error_log("call", node, "the number of params is " + num);
    exit(0);
  }

  if (args != NULL) {
    args = args->child[0];
    int trueNum = 0;
    while (args != NULL) {
      trueNum++;
      args = args->sibling;
    }

    if (num != trueNum) {
      error_log("call", node,
                "the number of params  is " + int2str(num) + ", but find " +
                    int2str(trueNum));
      exit(0);
    }
  }
}

// 检查返回类型是否对应， void 不能 对应 int
/*
 TreeNode *node = newNode(RETURN);
  //接受 return
  match_value("return");
  node->child[0] = expression();
  match_value(";");

                RETURN(42)
                        NULL(43):	0
*/
void checkReturn(TreeNode* node) {
  FunctionInfo fi = fun_table[cur_fun];
  string fir = fi.return_type;

  if (fir == "VOID_DECLARE" && node->child[0] != NULL ||
      fir == "INT_DECLARE" && node->child[0] == NULL) {
    error_log("return", node, "the return type is " + fir);
    exit(0);
  }
}

void build_table(TreeNode* node) {
  // 中序遍历

  // 如果是函数， 直接插入全局变量表和函数表
  if (node->nodekind == FUNCTION) {
    insertFunction(node);
    cur_fun = node->child[1]->value;
  }

  //检查变量声明， 不能重复定义
  if (node->nodekind == VAR_DECLARE || node->nodekind == PARAM) {
    bool flag = findInCurTable(node);
    if (flag) {
      insertVariable(node);
    }
  }

  // 检查函数定义， 前面已经检查过变量声明了， 已经加入了符号表
  if (node->nodekind == ID) {
    checkId(node);
  }
  // 检查返回类型是否对应， void 不能 对应 int
  if (node->nodekind == RETURN) {
    checkReturn(node);
  }
  // 检查函数调用 的 参数个数是否正确，  类型是否一致
  if (node->nodekind == CALL) {
    checkCall(node);
  }

  for (int i = 0; i < 4; i++) {
    if (node->child[i] != NULL) {
      build_table(node->child[i]);
    }
  }

  if (node->nodekind == FUNCTION) cur_fun = "global";
  if (node->sibling != NULL) {
    build_table(node->sibling);
  }
}


void analyse() {
  //初始化 output  函数 和  全局变量！！
  table_init();

  build_table(root);
}