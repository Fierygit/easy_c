#include "genMid.h"
#include "global.h"
#include "util.h"

//定义存储的数据结构
vector<MidCode *> midCode;
map<pair<string, string>, MidArgs *> stackInfo;

string curFunc = "global";  //当前遍历的函数
int offset = 0;             //偏移量
int argOffset = 0;          //参数的偏移量
int labelIndex = 0;
MidCode *curMidCode = NULL;
int curStackSize = 0;
vector<string> rodata;

//* 根据函数初始化， 代码段
void newMidCodeFunc(string funcName) {
  curMidCode = new MidCode();
  curMidCode->funcName = funcName;
  curStackSize = 0;
  return;
}

MidArgs *newMidArgs(string type, string value, int offset, bool isGlobal) {
  MidArgs *temp = new MidArgs();
  temp->type = type;
  temp->value = value;
  temp->offset = offset;  //新建立一个就加一
  temp->isGlobal = isGlobal;
  return temp;
}

MidCodeItem *newMidCodeItem(string dest, string op, MidArgs *arg1,
                            MidArgs *arg2) {
  MidCodeItem *temp = new MidCodeItem();
  temp->dest = dest;
  temp->op = op;
  temp->arg1 = arg1;
  temp->arg2 = arg2;
  return temp;
}

void addItem2CurFunc(MidCodeItem *item) { curMidCode->item.push_back(item); }

void add2StackInfo(string name, MidArgs *midArgs) {
  stackInfo[make_pair(curFunc, name)] = midArgs;
}

// c 语言也可以
// 遍历语法树, 函数作为参数传递
void traverse(TreeNode *node, void (*preProc)(TreeNode *),
              void (*postProc)(TreeNode *)) {
  if (node == NULL) return;

  if (mapNode(node->nodekind) == "FUNCTION") {
    curFunc = node->child[1]->value;
    node->child[1]->isVisit = 1;
    node->isVisit = 1;
    // cout << "FUNC: " << curFunc << endl;
    offset = argOffset = 0;
    newMidCodeFunc(curFunc);
  }

  if (preProc != NULL) preProc(node);

  for (int i = 0; i < 4; i++) {
    traverse(node->child[i], preProc, postProc);
  }
  if (postProc != NULL) postProc(node);
  //退出后当前的代码不属于 当前 FUNCTION 了
  if (mapNode(node->nodekind) == "FUNCTION") {
    curMidCode->stackSize = offset;
    midCode.push_back(curMidCode);
    curFunc == "global";
  }
  traverse(node->sibling, preProc, postProc);
}

void log_GenMid(string method, string info) {
  midCodeInfo += "\t" + format("[" + method + "]: ", 25) + info + "\n";
}

MidArgs *Gen_E(TreeNode *node) {
  MidArgs *return_arg = NULL;
  if (node == NULL) return NULL;
  node->isVisit = 1;
  if (mapNode(node->nodekind) == "ID") {
    string name_str = node->value;
    return_arg = stackInfo[make_pair(curFunc, name_str)];
    if (return_arg == 0) {
      return_arg = stackInfo[make_pair("global", name_str)];
    }
    log_GenMid("Gen_E ID", return_arg->value);
  } else if (mapNode(node->nodekind) == "NUM") {
    log_GenMid("Gen_E NUM", node->value);
    return newMidArgs("NUM", node->value, 0, 0);
  }

  //  CMP OPTION_ADD,  OPTION_MUL,
  else if (mapNode(node->nodekind) == "CMP" ||
           mapNode(node->nodekind) == "OPTION_ADD" ||
           mapNode(node->nodekind) == "OPTION_MUL") {
    MidArgs *arg1 = Gen_E(node->child[0]);
    MidArgs *arg2 = Gen_E(node->child[1]);
    string dest_name = "__t" + int2str(offset);
    return_arg = newMidArgs("TMP", dest_name, offset++, 0);
    add2StackInfo(dest_name, return_arg);
    addItem2CurFunc(newMidCodeItem(dest_name, node->value, arg1, arg2));
    log_GenMid(mapNode(node->nodekind), "dest_name: " + dest_name);
  }

  else if (mapNode(node->nodekind) == "CALL") {
    TreeNode *args = node->child[1];
    if (args == NULL) {
    } else {
      TreeNode *flag, *arg = args->child[0];
      flag = arg;
      vector<int> temp_int;
      while (arg != NULL) {
        args->isVisit = 1;
        string temp_arg = "__t" + int2str(offset);
        temp_int.push_back(offset);
        add2StackInfo(temp_arg, newMidArgs("TMP", temp_arg, offset++, 0));
        addItem2CurFunc(newMidCodeItem(temp_arg, "=", Gen_E(arg), NULL));
        log_GenMid(mapNode(node->nodekind) + "ARG", "tmp_name: " + temp_arg);
        arg = arg->sibling;
      }

      int temp = 0;
      //参数要反过来放
      vector<MidCodeItem *> temp_arg1;
      while (flag != NULL) {
        string temp_arg = "__t" + int2str(temp_int[temp++]);
        MidArgs *arg1 = stackInfo[make_pair(curFunc, temp_arg)];
        temp_arg1.push_back(newMidCodeItem("", "ARG", arg1, NULL));
        flag = flag->sibling;
      }
      for (int i = temp_arg1.size() - 1; i >= 0; i--) {
        addItem2CurFunc(temp_arg1[i]);
        log_GenMid("ARG", "value: " + temp_arg1[i]->arg1->value);
      }
      string dest_name = "__t" + int2str(offset);
      return_arg = newMidArgs("TMP", dest_name, offset++, 0);
      MidArgs *callName = newMidArgs("CALL", node->child[0]->value, 0, 0);
      MidArgs *argNum =
          newMidArgs("ARG_NUM", int2str(temp_arg1.size() * 4), 0, 0);
      addItem2CurFunc(newMidCodeItem(dest_name, "CALL", callName, argNum));
      add2StackInfo(dest_name, return_arg);
    }

  } else {
    log_GenMid("error", "Gen_E");
  }
  return return_arg;
}

void Gen_list(TreeNode *node) {
  if (node != NULL) {
    Gen_S(node);
    if (node->sibling != NULL) {
      Gen_list(node->sibling);
    }
  }
}

void Gen_S(TreeNode *node) {
  node->isVisit = 1;

  if (mapNode(node->nodekind) == "ASSIGN") {
    // name_t1 属于id， 应该是定义过的
    string name_t1 = node->child[0]->value;
    node->child[0]->isVisit = 1;

    MidArgs *arg1 = NULL;
    MidArgs *dest = stackInfo[make_pair(curFunc, name_t1)];
    if (dest == NULL) {
      cout << name_t1 << " is global" << endl;
      MidArgs *dest = stackInfo[make_pair("global", name_t1)];
    }
    arg1 = Gen_E(node->child[1]);
    addItem2CurFunc(newMidCodeItem(name_t1, "=", arg1, NULL));
    // outCode(t1 + " = " + t2);
    log_GenMid("Gen_S ASSIGN", "left: " + name_t1 + "  right: " + arg1->value);
  } else if (mapNode(node->nodekind) == "PRINT") {
    if (node->value == "VAR") {
      MidArgs *arg1 = Gen_E(node->child[0]);
      addItem2CurFunc(newMidCodeItem("", "PRINT", arg1, NULL));
    } else {
      MidArgs *arg1;
      if (node->child[0]->value == "endl") {
        arg1 = newMidArgs("STR", "endl", 0, 1);
      } else if (node->child[0]->value == "space") {
        arg1 = newMidArgs("STR", "space", 0, 1);
      } else {
        rodata.push_back(node->child[0]->value);
        arg1 = newMidArgs("STR", int2str(rodata.size() + 3), 0, 1);
      }
      addItem2CurFunc(newMidCodeItem("", "PRINT", arg1, NULL));
    }
  } else if (mapNode(node->nodekind) == "RETURN") {
    MidArgs *arg1 = Gen_E(node->child[0]);
    node->child[0]->isVisit = 1;
    addItem2CurFunc(newMidCodeItem("", "return", arg1, NULL));
  }

  else if (mapNode(node->nodekind) == "CALL") {
    Gen_E(node);
  }
  //*             while
  else if (mapNode(node->nodekind) == "WHILE") {
    int indexTemp0 = labelIndex++;
    // goto l0
    addItem2CurFunc(newMidCodeItem("Label" + int2str(indexTemp0) + ":", "LABEL",
                                   NULL, NULL));
    MidArgs *e = Gen_E(node->child[0]);

    int indexTemp1 = labelIndex++;

    MidArgs *label1 =
        newMidArgs("LABEL", "Label" + int2str(indexTemp1) + ":", offset++, 0);

    addItem2CurFunc(newMidCodeItem("", "if", e, NULL));
    addItem2CurFunc(newMidCodeItem("", "goto", label1, NULL));

    int indexTemp2 = labelIndex++;
    MidArgs *label2 =
        newMidArgs("LABEL", "Label" + int2str(indexTemp2) + ":", offset++, 0);
    addItem2CurFunc(newMidCodeItem("", "goto", label2, NULL));

    // success code
    addItem2CurFunc(newMidCodeItem("Label" + int2str(indexTemp1) + ":", "LABEL",
                                   NULL, NULL));
    Gen_list(node->child[1]);
    // return first
    MidArgs *label0 =
        newMidArgs("TMP", "Label" + int2str(indexTemp0) + ":", offset++, 0);
    addItem2CurFunc(newMidCodeItem("", "goto", label0, NULL));

    // out label
    addItem2CurFunc(newMidCodeItem("Label" + int2str(indexTemp2) + ":", "LABEL",
                                   NULL, NULL));

    log_GenMid("Gen_S WHILE", "end");
  }

  else if (mapNode(node->nodekind) == "SELECTION") {
    //只有一个IF
    if (node->value == "") {
      MidArgs *e = Gen_E(node->child[0]);
      int indexTemp1 = labelIndex++;
      MidArgs *label1 =
          newMidArgs("LABEL", "Label" + int2str(indexTemp1) + ":", offset++, 0);
      addItem2CurFunc(newMidCodeItem("", "if", e, NULL));
      addItem2CurFunc(newMidCodeItem("", "goto", label1, NULL));
      int indexTemp2 = labelIndex++;
      MidArgs *label2 =
          newMidArgs("LABEL", "Label" + int2str(indexTemp2) + ":", offset++, 0);
      addItem2CurFunc(newMidCodeItem("", "goto", label2, NULL));

      // success code
      addItem2CurFunc(newMidCodeItem("Label" + int2str(indexTemp1) + ":",
                                     "LABEL", NULL, NULL));
      Gen_list(node->child[1]);
      // out label
      addItem2CurFunc(newMidCodeItem("Label" + int2str(indexTemp2) + ":",
                                     "LABEL", NULL, NULL));

    } else if (node->value == "else") {
      MidArgs *t1 = Gen_E(node->child[0]);
      int indexTemp1 = labelIndex++;
      MidArgs *e;
      if (t1->isGlobal)
        e = newMidArgs("TMP", "__" + t1->value, offset++, 0);
      else
        e = newMidArgs("TMP", t1->value, offset++, 0);
      MidArgs *label1 =
          newMidArgs("LABEL", "Label" + int2str(indexTemp1) + ":", offset++, 0);
      addItem2CurFunc(newMidCodeItem("", "if", e, NULL));
      addItem2CurFunc(newMidCodeItem("", "goto", label1, NULL));
      int indexTemp2 = labelIndex++;
      MidArgs *label2 =
          newMidArgs("LABEL", "Label" + int2str(indexTemp2) + ":", offset++, 0);
      addItem2CurFunc(newMidCodeItem("", "goto", label2, NULL));

      // success code
      addItem2CurFunc(newMidCodeItem("Label" + int2str(indexTemp1) + ":",
                                     "LABEL", NULL, NULL));
      Gen_list(node->child[1]);

      // out label
      int indexTemp3 = labelIndex++;
      MidArgs *label3 =
          newMidArgs("LABEL", "Label" + int2str(indexTemp3) + ":", offset++, 0);
      addItem2CurFunc(newMidCodeItem("", "goto", label3, NULL));

      // LABEL 2
      addItem2CurFunc(newMidCodeItem("Label" + int2str(indexTemp2) + ":",
                                     "LABEL", NULL, NULL));
      Gen_list(node->child[2]);

      // out
      addItem2CurFunc(newMidCodeItem("Label" + int2str(indexTemp3) + ":",
                                     "LABEL", NULL, NULL));
      log_GenMid("Gen_S IF", "end");
    }
  } else {
    cout << "no deal: " << mapNode(node->nodekind) << " " << node->value
         << endl;
  }

  // log_Gen("Gen_S IF", "");
  return;
}

void GenMidByNode(TreeNode *node) {
  //访问过了游戏结束
  if (node->isVisit) return;

  if (mapNode(node->nodekind) == "VAR_DECLARE") {
    node->isVisit = 1;
    string type = mapNode(node->child[0]->nodekind);  // 只有 INT_DECLARE
    string name = node->child[1]->value;

    MidArgs *temp =
        newMidArgs("ID", name, offset++, curFunc == "global" ? 1 : 0);

    add2StackInfo(name, temp);
    if (curFunc == "global") {
      cout << "global declare!!" << endl;
      newMidCodeFunc("VAR_DECLARE");
      addItem2CurFunc(newMidCodeItem("", "INT", temp, NULL));
      midCode.push_back(curMidCode);
    }
    node->child[0]->isVisit = 1, node->child[1]->isVisit = 1;
    log_GenMid("add " + type, name);
  } else if (mapNode(node->nodekind) == "PARAM") {
    node->isVisit = 1;
    string type = mapNode(node->child[0]->nodekind);  // 只有 INT_DECLARE
    string name = node->child[1]->value;
    MidArgs *temp = newMidArgs("ARG", name, argOffset++, 0);
    add2StackInfo(name, temp);
    node->child[0]->isVisit = 1, node->child[1]->isVisit = 1;
    log_GenMid("add arg" + type, name);
  }

  else
    Gen_S(node);
}

void genMid() {
  cout << "<<< start gen mid >>>\n" << endl;
  traverse(root, GenMidByNode, NULL);
}
