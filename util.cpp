/*
 * @Author: Firefly
 * @Date: 2020-01-18 10:28:42
 * @Descripttion:
 * @LastEditTime : 2020-01-28 20:53:52
 */

#include "util.h"
#include <fstream>

void outputParseInfo(string info) {
  cout << "<<< start ouput parse info >>>" << endl;
  cout << info;
  cout << endl << endl;
}

void outputTokenCode(vector<Token*> tokens) {
  cout << "<<< start output tokenCode >>>" << endl;
  for (int i = 0; i < tokens.size(); i++) {
    cout << "<" << tokens[i]->type << ", " << tokens[i]->value << ", "
         << tokens[i]->row << ", " << tokens[i]->column << ">\t";
    if (i + 1 < tokens.size() && tokens[i + 1]->row != tokens[i]->row) {
      cout << endl;
    }
  }
  cout << endl << endl;
}

// string int2str(int a) {
//   stringstream ss;
//   ss << a;
//   string b;
//   ss >> b;
//   return b;
// }
string int2str(int num) {
  string str;
  while (num >= 10) {
    int temp = num % 10;
    num /= 10;
    str = char('0' + temp) + str;
  }
  str = char('0' + num) + str;
  return str;
}

void outt(int num) {
  for (int i = 0; i < num; i++) cout << "\t";
}

//从根节点打印树
void output_tree(TreeNode* node, int num) {
  outt(num);
  cout << mapNode(node->nodekind) << "(" << node->tokenIndex << ")";
  if (node->value != "") cout << ":\t" << node->value;
  cout << endl;

  for (int i = 0; i < 4; i++) {
    if (node->child[i] != NULL) {
      output_tree(node->child[i], num + 1);
    }
  }

  if (node->sibling != NULL) {
    output_tree(node->sibling, num);
  }
}

void outputTraceTree(TreeNode* root) {
  cout << "<<< start ouput TraceTree >>>\n";
  output_tree(root, 0);
  cout << endl << endl;
  NodeKind n = OPTION_MUL;
}

string format(string info, int len) {
  int infoLen = info.length();
  // cout << info.length() << "*" << len ;
  if (infoLen >= len) return info.substr(0, infoLen);
  int left = info.length();
  for (int i = 0; i < len - left; i++) info += " ";
  return info;
}

void outputSymbalTable() {
  cout << "<<< strat output symbal Table >>>" << endl;
  map<string, vector<VariableInfo> > v = var_table;

  for (map<string, vector<VariableInfo> >::iterator i = v.begin(); i != v.end();
       i++) {
    cout << "symbal table:\t" << i->first << endl;
    cout << "____________________________________________________________\n";
    vector<VariableInfo> va = i->second;
    for (int j = 0; j < va.size(); j++) {
      cout << format(va[j].name, 20) << format(va[j].type, 20) << va[j].lineNo
           << endl;
    }
    cout << "------------------------------------------------------------\n\n";
  }
}

string outputArg(MidArgs* arg) {
  if (arg == NULL) return "NONE";
  return "[" + arg->value + ", " + arg->type + ", " + int2str(arg->offset) +
         ", " + (arg->isGlobal == 1 ? "1" : "0") + "]";
}

// extern vector<MidCode *> midCode;
// 声明 ID 加入， value随机初始     TMP 生成一条语句加入 ， 或更改id，
// extern map<string , MidArgs* > stackInfo;  // ID, TMP, 加入

void outputMidCode() {
  cout << "<<<start output midCode >>>" << endl;
  cout << midCodeInfo << endl;
  cout << "-------------------------------------------------" << endl;
  for (int i = 0; i < midCode.size(); i++) {
    cout << "FUNCTION: " << midCode[i]->funcName;
    cout << "\t(stackSize: " << midCode[i]->stackSize << ")" << endl;
    vector<MidCodeItem*> temp = midCode[i]->item;
    for (int j = 0; j < temp.size(); j++) {
      cout << "\t" << format(temp[j]->dest, 20) << format(temp[j]->op, 20)
           << "\t" << format(outputArg(temp[j]->arg1), 30)
           << format(outputArg(temp[j]->arg2), 30) << endl;
    }
  }
}
int str2int(string num){
    stringstream ss;
    ss << num;
    int a;
    ss >> a;
    return a;
}

void outputAsm2File(string path) {
  ofstream ofs;
  ofs.open(path);
  ofs << asmCode;
  ofs.close();
}

// 真的烦人，   enum 既然不是 递增的？？??? ？？
// enum[string1] = 0, enum[string2] != 1????

string mapNode(NodeKind n) {
  switch (n) {
    case CALL:
      return "CALL";
      break;
    case ARGS:
      return "ARGS";
      break;
    case OPTION_ADD:
      return "OPTION_ADD";
      break;
    case ITERAT:
      return "WHILE";
      break;
    case SELECTION:
      return "SELECTION";
      break;
    case INT_DECLARE:
      return "INT_DECLARE";
      break;
    case VOID_DECLARE:
      return "VOID_DECLARE";
      break;
    case ID:
      return "ID";
      break;
    case PARAMS:
      return "PARAMS";
      break;
    case PARAM:
      return "PARAM";
      break;
    case PARAM_DECLARE:
      return "PARAM_DECLARE";
      break;
    case VAR_DECLARE:
      return "VAR_DECLARE";
      break;
    case FUNCTION:
      return "FUNCTION";
      break;
    case COMPONENT:
      return "COMPONENT";
      break;
    case PRINT:
      return "PRINT";
      break;
    case RETURN:
      return "RETURN";
      break;
    case OPTION_MUL:
      return "OPTION_MUL";
      break;
    case ASSIGN:
      return "ASSIGN";
      break;
    case CMP:
      return "CMP";
      break;
    case STR:
      return "STR";
      break;
    case NUM:
      return "NUM";
      break;
    default:
      break;
  }
  return "NULL";
}