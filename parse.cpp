/*
 * @Author: Firefly
 * @Date: 2020-01-18 20:17:57
 * @Descripttion:
 * @LastEditTime : 2020-01-24 00:29:06
 */
#include "parse.h"

Token *token;
int tokenIndex = 0;
TreeNode *root;  //定义, 声明在全局变量
string parseInfo;

string token2str() { return "1: " + token->type + "; 2: " + token->value; }
string addWhite(string info,int len){
  string temp = "";
  for(int i = 0; i < len; i++) temp += " ";
  return temp + info;
}
void log_parse(string method, string info) {
  if (method == "match")
    parseInfo += "[" + method + "]: " + info + "\n";
  else
    parseInfo += addWhite("\t\t\t[" + method + "]: " + info + "\n",20);
}


TreeNode *newNode(NodeKind nodekind) {
  TreeNode *node = new TreeNode();
  for (int i = 0; i < 4; i++) node->child[i] = NULL;
  node->sibling = NULL;
  node->nodekind = nodekind;
  node->tokenIndex = tokenIndex;
  node->value = "";
  node->isVisit = 0; // 一开始都没有访问过
  return node;
}
void match_value(string value) {
  log_parse("match", tokenCode[tokenIndex]->value);
  // todo  数字到字符
  if (tokenCode[tokenIndex]->value == value) {
    tokenIndex++;
    if (tokenIndex < tokenCode.size()) token = getToken();
    //!  查看越界，艹——艹
  } else {
    cout << "error: " << value << " is expected!" << endl;
    cout << "at\trow: " << tokenCode[tokenIndex]->row
         << "; col: " << tokenCode[tokenIndex]->column << "." << endl;
    exit(0);
  }
}

void match_type(string type) {
  log_parse("match", tokenCode[tokenIndex]->value);
  // todo  数字到字符
  if (tokenCode[tokenIndex]->type == type) {
    tokenIndex++;
    if (tokenIndex < tokenCode.size()) token = getToken();
    //!  查看越界
  } else {
    cout << "error: " << type << " is expected!" << endl;
    cout << "at\trow: " << tokenCode[tokenIndex]->row
         << "; col: " << tokenCode[tokenIndex]->column << "." << endl;
    exit(0);
  }
}

// param ->  INT
TreeNode *param() {
  TreeNode *node1 = newNode(PARAM);

  TreeNode *node2 = newNode(PARAM_DECLARE);
  node2->value = "int";
  match_value("int");  //参数只有 int 形的
  TreeNode *node3 = newNode(ID);

  node3->value = token->value;
  match_type("ID");
  //先匹配是不是ID，不是的话，后面就不用赋值了
  //但是先匹配， inde就++了，所以还是要先赋值 ^_^

  node1->child[0] = node2;
  node1->child[1] = node3;
  return node1;
}

//* param_list  ->  param  |  param_list, param
TreeNode *param_list() {
  TreeNode *node1 = param();
  TreeNode *node = node1;
  TreeNode *node2 = NULL;
  while (token->value == ",") {
    match_value(",");
    node2 = param();
    if (node2 != NULL) {
      node->sibling = node2;
      node = node2;
    }
  }
  return node1;
}

//* params -> NULL |  param_list
TreeNode *params(void) {
  TreeNode *node = newNode(PARAMS);
  if (token->value == ")") {  //匹配到了右括号表示没有参数记录一下
    node->value = "NULL";     //没有参数
  } else {
    node->child[0] = param_list();
  }
  return node;
}

//* local_declaration  ->   var-decaration  | NULL

TreeNode *local_declaration() {
  log_parse("start local_declaration", token2str());
  TreeNode *node1, *node2, *node3, *node4, *node;
  node = node1 = node2 = node3 = node4 = NULL;
  while (token->value == "int") {
    node1 = newNode(VAR_DECLARE);
    node2 = newNode(INT_DECLARE);

    match_value("int");

    node3 = newNode(ID);
    node3->value = token->value;
    match_type("ID");

    while (token->value == "," ||
           token->value == ";") {  // 如果是逗号， 可以一直读取
      node1->child[0] = node2;
      node1->child[1] = node3;
      if (node4 == NULL) {  // 找下一个局部变量声明！！！
        node = node4 = node1;
      } else {
        node4->sibling = node1;
        node4 = node1;
      }
      if (token->value == ";") {  //如果结束了， 那就直接 停止逗号循环
        match_value(";");
        break;
      }
      match_value(",");
      node1 = newNode(VAR_DECLARE);
      node2 = newNode(INT_DECLARE);
      node3 = newNode(ID);
      node3->value = token->value;
      match_type("ID");
    }
  }
  log_parse("end local_declaration", token2str());
  return node;
}

//* a rg-l i s t → a rg-list , e x p re s s i o n | e x p re s s i o n

TreeNode *arg_list() {
  TreeNode *node, *node1, *node2, *node3;
  node1 = expression();
  node = node2 = newNode(ARGS);
  node2->child[0] = node1;
  while (token->value == ",") {
    match_value(",");
    node3 = expression();
    node1->sibling = node3;
    node1 = node1->sibling;
  }
  return node;
}

//* a rg s → a rg - l i s t | e m p t y

TreeNode *args() {
  if (token->value == ")") return NULL;
  return arg_list();
}

//* c a l l →  I D ( a rg s )
TreeNode *call(TreeNode *t) {
  TreeNode *node;
  node = newNode(CALL);
  match_value("(");
  node->child[0] = t;
  node->child[1] = args();
  match_value(")");
  return node;
}

//* f a c t o r   ->   ( e x p re s s i o n ) | v a r | c a l l | N U M
TreeNode *factor(TreeNode *t) {
  log_parse("factor", token2str());

  TreeNode *node1, *node2, *node3, *node4, *node;
  node = node1 = node2 = node3 = node4 = NULL;
  if (t != NULL) {
    // t 不是空的  那么如果下一个是 （ 的话  就是函数调用， 否则 返回
    // 提前接受的这个值 8978（）？
    if (token->value == "(") {
      // todo 记得回来写
      node = call(t);
    } else {
      node = t;
    }

  } else {
    if (token->value == "(") {
      match_value("(");
      node1 = expression();
      match_value(")");
    } else if (token->type == "ID") {
      node1 = newNode(ID);

      // todo 记得回来写
      node1->value = token->value;
      match_value(token->value);

    } else if (token->type == "NUM") {
      node1 = newNode(NUM);
      node1->value = token->value;
      match_value(token->value);
    }  // todo 还有其它
    node = node1;
  }
  return node;
}

//*  term   -> factor  op  term  | factor   右循环
TreeNode *term(TreeNode *t) {
  log_parse("term", token2str());
  TreeNode *node1, *node2, *node3, *node4, *node;
  node = node1 = node2 = node3 = node4 = NULL;
  node = node1 = factor(t);
  while (token->value == "*" || token->value == "/") {
    node2 = newNode(OPTION_MUL);
    node2->value = token->value;
    node2->child[0] = node1;
    match_value(token->value);
    // todo
    node2->child[1] = term(NULL);
    node = node2;
  }
  return node;
}

//* additive_expression   ->    term  |   term op  additive_expression ;  op = +
//* | -
TreeNode *additive_expression(TreeNode *t) {
  log_parse("start additive_expression", token2str());
  TreeNode *node1, *node2, *node3, *node4, *node;
  node = node1 = node2 = node3 = node4 = NULL;
  node = node1 = term(t);
  while (token->value == "+" || token->value == "-") {
    node2 = newNode(OPTION_ADD);
    node2->value = token->value;
    node2->child[0] = node1;

    match_value(token->value);

    node2->child[1] = additive_expression(NULL);
    node = node2;
  }
  log_parse("end additive_expression", token2str());
  return node;
}

//*  simple_expression  ->  additive_expression  |   additive_expression op
//*                                                  additive_expression (op ==
//< | > >= <= != )
TreeNode *simple_expression(TreeNode *t) {
  log_parse("start simple_expression", token2str());
  TreeNode *node1, *node2, *node3, *node4, *node;
  node = node1 = node2 = node3 = node4 = NULL;

  node = node1 = additive_expression(t);

  if (token->type == "CMP") {
    node2 = newNode(CMP);
    node2->value = token->value;
    node2->child[0] = node1;
    match_value(token->value);
    node2->child[1] = additive_expression(NULL);
    node = node2;
  }
  log_parse("end simple_expression", token2str());
  return node;
}

//*  iteration_stmt    ->   while(expression) { statement_list }
TreeNode *iteration_stmt() {
  TreeNode *node = newNode(ITERAT);

  match_value(token->value);
  match_value("(");  //匹配左括号

  //匹配表达式
  node->child[0] = expression();
  match_value(")");
  match_value("{");
  //匹配语句
  node->child[1] = statement_list();
  match_value("}");
  return node;
}

TreeNode *selection_stmt() {
  TreeNode *node1, *node2, *node3, *node4, *node;
  node = node1 = node2 = node3 = node4 = NULL;
  node1 = newNode(SELECTION);
  match_value("if");
  match_value("(");
  node1->child[0] = expression();
  match_value(")");
  match_value("{");
  node1->child[1] = statement_list();
  match_value("}");
  if (token->value == "else") {
    match_value("else");
    match_value("{");
    node1->value = "else";
    node1->child[2] = statement_list();
    match_value("}");
  }
  return node1;
}

//* expression    ->   simple_expression  |  var = expression

TreeNode *expression() {
  log_parse("expression", token2str());
  TreeNode *node1, *node2, *node3, *node4, *node;
  node = node1 = node2 = node3 = node4 = NULL;

  if (token->type == "ID") {
    node1 = newNode(ID);
    node1->value = token->value;
    match_type("ID");
    if (token->value == "=") {
      node2 = newNode(ASSIGN);
      match_value("=");
      node2->child[0] = node1;
      node2->child[1] = expression();
      node = node2;
    } else {  //不是 = ， 就是一个简单语句了
      node = simple_expression(node1);  // bug  两个 id？
    }

    //! 这里要前看啊啊啊啊啊啊啊啊啊啊啊啊
  } else {  //不是ID开头的一定是简单的语句  可能是NUM 也可能是左括号
    node = simple_expression(node1);
  }
  return node;
}

TreeNode *print_stmt() {
  TreeNode *node = newNode(PRINT);
  //接受 print
  match_value("print");

  if (token->value == "(") {
    match_value("(");
    node->child[0] = expression();
    node->value = "VAR";
    match_value(")");
    match_value(";");
  } else if (token->value == "<") {
    match_value("<");
    TreeNode *temp = newNode(STR);
    temp->value = token->value;
    match_type("ID");
    node->child[0] = temp;
    node->value = "STR";
    match_value(">");
    match_value(";");
  }
  return node;
}

TreeNode *return_stmt() {
  TreeNode *node = newNode(RETURN);
  //接受 return
  match_value("return");
  node->child[0] = expression();
  match_value(";");
  return node;
}

//*  expression_stmt   ->   expression ; || ;
TreeNode *expression_stmt() {
  TreeNode *node;
  node = NULL;
  if (token->value == ";") {  //空语句
    match_value(token->value);
    return node;
  } else {
    node = expression();
    match_value(";");
  }
  return node;
}

//*    statement ->  iteration_stmt | selection_stmt | expression_stmt |
//*                  return_stmt | print_stmt

TreeNode *statement() {
  TreeNode *node = NULL;
  string t = token->value;
  if (t == "while")
    node = iteration_stmt();
  else if (t == "if")
    node = selection_stmt();
  else if (token->type == "ID")
    node = expression_stmt();
  else if (t == "return")
    node = return_stmt();
  else if (t == "print")
    node = print_stmt();
  return node;
}

//* statement_list   ->   statement_list statement || NULL

TreeNode *statement_list() {
  log_parse("start statement_list", token2str());
  TreeNode *node1, *node2, *node;
  node = node1 = node2 = NULL;
  node = node1 = statement();
  //!
  // cout << " ************************" << token.second << " " <<endl;
  string t = token->value;


  // 目前只支持 这几种语句, 局部变量这里不给声明 语句，
  // 只能在一开时的地方定义变量 todo 这里先简化
  while (t == "if" || t == "while" || t == "return" || token->type == "ID" ||
         t == "print") {
    // cout << " tryyyyyyyy111" << t << endl;
    node2 = statement();
    node1->sibling = node2;
    node1 = node2;
    t = token->value;  //!! 这里debug半天
                       //!要更新啊啊啊啊啊啊！！！！！！！！啊啊啊啊啊
  }
  log_parse("end statement_list", token2str());
  return node;
}

//* compound_stmt ->  local_declaration & statement_list
TreeNode *compound_stmt() {
  log_parse("start compound_stmt", token2str());

  TreeNode *node = newNode(COMPONENT);
  match_value("{");  //吃掉左大括号
  //!    注意 局部变量为空的时候， 遍历要考虑 child[1]
  // 声明只有int 因此， 只要 读取到了 int 就一直读取！
  // 注意声明， 不包括定义，  int a = 0; 这种做法是不可以的！！
  // 如果没有的话直接返回 空的！！！！！
  node->child[0] = local_declaration();  // 声明首先是  局部变量
  node->child[1] = statement_list();     //然后是 语句
  match_value("}");
  log_parse("end compound_stmt", token2str());
  return node;
}

//* declaration  -> var-declare | fun-declare
TreeNode *declaration() {
  //* loger
  log_parse("start declaration", token2str());
  TreeNode *node1, *node2, *node3, *node;
  node = node1 = node2 = node3 = NULL;
  if (token->value == "int") {
    node1 = newNode(INT_DECLARE);
    match_value("int");
  } else {
    node1 = newNode(VOID_DECLARE);
    match_value("void");
    //只有两种情况，如果是其它的情况是匹配不到的
  }

  if (node1 != NULL && token->type == "ID") {
    node2 = newNode(ID);
    node2->value = tokenCode[tokenIndex]->value;
    match_type("ID");

    if (token->value == "(") {  //有参数,是函数
      node = newNode(FUNCTION);
      node->child[0] = node1;  //函数 返回类型
      node->child[1] = node2;  //函数  名字
      match_value("(");
      node->child[2] = params();  //函数  参数
      match_value(")");
      node->child[3] = compound_stmt();
    } else if (token->value == ";") {  //变量的声明
      node = newNode(VAR_DECLARE);
      node->child[0] = node1;
      node->child[1] = node2;
      match_value(";");
      // todo ****************************  void 也可以声明变量？？？？
    }
  }

  log_parse("end declaration", token2str());

  return node;
}

//* declaration_list   ->  declaration |  declaration_list
TreeNode *declaration_list() {
  //先吃一个
  token = getToken();
  //入口一定是声明
  TreeNode *node1 = declaration();

  log_parse("declaration_list", token2str());

  TreeNode *curNode = node1;
  //先吃一个字符
  if (tokenIndex < tokenCode.size()) token = getToken();
  // cout << token.first <<endl;
  //一直吃一个字符，如果是int 和 void 直到结束
  while (tokenIndex < tokenCode.size() && token->type == "KEYWORD" &&
         (token->value == "int" || token->value == "void")) {
    TreeNode *node2;
    node2 = declaration();
    if (node2 != NULL) {
      curNode->sibling = node2;
      curNode = node2;  // node1 作为curNode,接受下一个节点
    }
  }
  return node1;
}

void parse(void) { root = declaration_list(); }

// todo 会越界
Token *getToken() {
  // cout << "deal: " << dealCode[codeIndex].first << " " <<
  // dealCode[codeIndex].second <<endl; 评论对于语法树来说是没有用的
  while (tokenIndex < tokenCode.size() &&
         tokenCode[tokenIndex]->type == "COMMENT")
    tokenIndex++;
  //!  为什么越界前不会提示？？？？？？？？？？？？？？？？？？？？？？？？？？？？？？
  // cout << codeIndex << " ** " << dealCode.size() <<endl;
  return tokenCode[tokenIndex];
}
