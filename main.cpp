/*
 * @Author: Firefly
 * @Date: 2020-01-18 09:22:22
 * @Descripttion:
 * @LastEditTime : 2020-01-28 20:57:50
 */
#include <iostream>
#include <vector>
#include "analyse.h"
#include "asmber.h"
#include "genMid.h"
#include "global.h"
#include "parse.h"
#include "scan.h"
#include "util.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

bool printSourceCode = 1;
bool printTokenCode = 1;
bool printParseTree = 1;  // 打印生成的语法树
bool printParseInfo = 1;  //打印生成过程
bool printSymbalTable = 1;
bool printMidCode = 1;
bool saveTofile = 1;

string sourceCode = "";
vector<Token*> tokenCode;

map<string, FunctionInfo> fun_table;
map<string, vector<VariableInfo> > var_table;
string midCodeInfo;

string help;

string file = "demo.ec";
string asmFile = "demo.s";
string paramstr = "111111";
int main(int argc, char** argv) {
  help += "ecc:     (easy c complier)\nauthor:     Firefly\n";
  help += "usage:   ecc [filename] [outputname] [params]\n";
  help += "example: ecc demo.ec demo 111111\n";
  help += "params:\n";
  help += "\tprintSourceCode 1\n";
  help += "\tprintTokenCode 1\n";
  help += "\tprintParseTree 1\n";  // 打印生成的语法树
  help += "\tprintParseInfo 1\n";  //打印生成过程
  help += "\tprintSymbalTable 1\n";
  help += "\tprintMidCode 1\n";

  if (argc <= 3) {
    cout << help;
    return 0;
  }
  file = argv[1];
  asmFile = argv[2];
  paramstr = argv[3];
  if (paramstr[0] == '0') printSourceCode = 0;
  if (paramstr[1] == '0') printTokenCode = 0;
  if (paramstr[2] == '0') printParseTree = 0;
  if (paramstr[3] == '0') printParseInfo = 0;
  if (paramstr[4] == '0') printSymbalTable = 0;
  if (paramstr[5] == '0') printMidCode = 0;
  
  if (saveTofile) {
    close(STDOUT_FILENO);  // 关闭标准输入与输出
    open("./logger.txt", O_CREAT | O_WRONLY | O_TRUNC);
  }

  if (!getSourceCodeFromFile(file)) {
    cout << "open failed!!!\nnote: file not exit!!!";
    return 0;
  } else {
    if (printSourceCode) {
      cout << "<<< start output sourceCode >>>\n";
      cout << sourceCode << endl;
      cout << endl << endl;
    }
  }

  //词法分析器
  tokenAll();
  if (printTokenCode) outputTokenCode(tokenCode);

  // 生成语法树 和 进行语法检查
  parse();
  if (printParseInfo) outputParseInfo(parseInfo);
  if (printParseTree) outputTraceTree(root);

  // 语义分析
  analyse();
  if (printSymbalTable) outputSymbalTable();

  // 生成 中间代码
  genMid();
  if (printMidCode) outputMidCode();

  // 词法   ->  语法  -> 语义 -> 汇编   ->  连接  ->
  genAsm();
  outputAsm2File(asmFile + ".s");
 // system("gcc " + asmFile + ".s -o " + asmFile);
  return 0;
}

/*

1962 高考失利， 1963 高考参军， 到部队
1968 年支援越南战争， 1970 年退役， 白发


*/