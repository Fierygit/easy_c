/*
 * @Author: Firefly
 * @Date: 2020-01-26 23:05:11
 * @Descripttion:
 * @LastEditTime : 2020-01-28 19:40:22
 */
#include "asmber.h"
#include "util.h"

int stringIndex = 0;
// 全局的最终代码
string asmCode;

void genString() {
  asmCode += ".section\t";
  asmCode += ".rodata\n";
  int rodataIndex = 0;
  asmCode += ".LC" + int2str(rodataIndex++) + ":\n";
  asmCode += "\t.string\t\"%d\"\n";
  asmCode += ".LC" + int2str(rodataIndex++) + ":\n";
  asmCode += "\t.string\t\" \"\n";
  asmCode += ".LC" + int2str(rodataIndex++) + ":\n";
  asmCode += "\t.string\t\"\\n\"\n";
  asmCode += ".LC" + int2str(rodataIndex++) + ":\n";
  asmCode += "\t.string\t\"%d\\n\"\n";

  for (int i = 0; i < rodata.size(); i++) {
    asmCode += ".LC" + int2str(rodataIndex++) + ":\n";
    asmCode += "\t.string\t\"" + rodata[i] + "\"\n";
  }
}

string getStackLoc(MidArgs* temp, int stackSize) {
  if (temp->type == "NUM") {
    return "$" + temp->value;
  } else {
    if (temp->isGlobal) {
      return temp->value;
    } else if (temp->type == "ARG")
      return int2str((temp->offset + 2) * 4) + "(%ebp)";
    else
      return int2str(stackSize - (temp->offset + 1) * 4) + "(%esp)";
  }
}

void genAsm() {
  cout << "<<< start gen asm >>>" << endl;
  // 全局变量 段
  genString();
  // 开始代码段
  asmCode += "\t.text\n";
  for (int i = 0; i < midCode.size(); i++) {
    if (midCode[i]->funcName == "VAR_DECLARE") {
      asmCode += "\t.comm\t" + midCode[i]->item[0]->arg1->value + ",4\n";
    } else {
      string funcName = midCode[i]->funcName;
      int stackSize = midCode[i]->stackSize * 4;
      asmCode += "\t.globl\t" + funcName + "\n\t.type  " + funcName +
                 ", @function\n" + funcName + ":\n" +
                 "\tpushl\t\%ebp\n\tmovl\t\%esp,\%ebp\n";
      // 分配 栈空间
      // 有多少偏移量就便宜多少
      asmCode += "\tsubl\t$" + int2str(stackSize) + ",%esp\n";

      vector<MidCodeItem*> temp = midCode[i]->item;
      for (int j = 0; j < temp.size(); j++) {
        // 赋值语句
        /*  movl	stackID,%esi
        leal	stackID,%edi
        movl	%esi,(%edi)
    */
        if (temp[j]->op == "=") {
          asmCode += "\t# asign\n";
          MidArgs* arg1 = temp[j]->arg1;
          asmCode += "\tmovl\t" + getStackLoc(arg1, stackSize) + ",%esi\n";
          MidArgs* dest = stackInfo[make_pair(funcName, temp[j]->dest)];

          if (dest == 0) dest = stackInfo[make_pair("global", temp[j]->dest)];
          asmCode += "\tleal\t" + getStackLoc(dest, stackSize) + ",%edi\n";
          //赋值语句
          asmCode += "\tmovl\t%esi,(%edi)\n";
        }

        else if (temp[j]->op == "+") {
          asmCode += "\t# add\n";
          asmCode +=
              "\tmovl\t" + getStackLoc(temp[j]->arg1, stackSize) + ",%eax\n";
          asmCode +=
              "\tmovl\t" + getStackLoc(temp[j]->arg2, stackSize) + ",%edx\n";
          MidArgs* dest = stackInfo[make_pair(funcName, temp[j]->dest)];
          if (dest == 0) dest = stackInfo[make_pair("global", temp[j]->dest)];
          asmCode += "\taddl\t%edx,%eax\n";
          asmCode += "\tmovl\t%eax," + getStackLoc(dest, stackSize) + "\n";
        }

        else if (temp[j]->op == "-") {
          asmCode += "\t# sub\n";
          asmCode +=
              "\tmovl\t" + getStackLoc(temp[j]->arg1, stackSize) + ",%eax\n";
          asmCode +=
              "\tmovl\t" + getStackLoc(temp[j]->arg2, stackSize) + ",%edx\n";
          MidArgs* dest = stackInfo[make_pair(funcName, temp[j]->dest)];
          if (dest == 0) dest = stackInfo[make_pair("global", temp[j]->dest)];
          asmCode += "\tsubl\t%edx,%eax\n";
          asmCode += "\tmovl\t%eax," + getStackLoc(dest, stackSize) + "\n";
        }

        else if (temp[j]->op == "if") {
          asmCode += "\t # if \n";
          asmCode +=
              "\tmovl\t" + getStackLoc(temp[j]->arg1, stackSize) + ",%eax\n";
          asmCode += "\tcmpl\t$0,%eax\n";
          string label0 = temp[j + 1]->arg1->value;
          string label1 = temp[j + 2]->arg1->value;
          label0 = label0.substr(0, label0.length() - 1);
          label1 = label1.substr(0, label1.length() - 1);
          j = j + 2;
          asmCode += "\tjne \t." + label0 + "\n";
          asmCode += "\tjmp \t." + label1 + "\n";

        } else if (temp[j]->op == "LABEL") {
          asmCode += "." + temp[j]->dest + "\n";
        } else if (temp[j]->op == "goto") {
          string label0 = temp[j]->arg1->value;
          label0 = label0.substr(0, label0.length() - 1);
          asmCode += "\tjmp \t." + label0 + "\n";
        }

        /*
        a = a1 + a2;
        a1 -> eax
        a2 -> edx
        addl  edx -> eax
        */
        else if (temp[j]->op == "return") {
          asmCode += "\t# return\n";
          asmCode +=
              "\tmovl\t" + getStackLoc(temp[j]->arg1, stackSize) + ",%eax\n";
          asmCode += "\tjmp \t.leave_" + funcName + "\n";
        } else if (temp[j]->op == "ARG") {
          asmCode += "\t# ARG\n";
          asmCode += "\tsubl\t$" + int2str(4) + ",%esp\n";
          stackSize += 4;
          asmCode +=
              "\tmovl\t" + getStackLoc(temp[j]->arg1, stackSize) + ",%eax\n";
          asmCode += "\tmovl\t%eax, 0(%esp)\n";
        } else if (temp[j]->op == "CALL") {
          // 调用函数
          asmCode += "\t# call \n";
          asmCode += "\tcall\t" + temp[j]->arg1->value + "\n";
          asmCode += "\taddl\t$" + temp[j]->arg2->value + ",%esp\n"; 
          stackSize -= str2int(temp[j]->arg2->value);
          // 把 返回值放到  目标地方
          MidArgs* dest = stackInfo[make_pair(funcName, temp[j]->dest)];
          if (dest == 0) dest = stackInfo[make_pair("global", temp[j]->dest)];
          //!  可以 直接使用吗？？？
          asmCode += "\tmovl\t%eax," + getStackLoc(dest, stackSize) + "\n";
        }

        /*
       movl	508(%esp),%eax
               movl	%eax,4(%esp)
               movl	$.LC1,%ebx
               movl	%ebx,0(%esp)
               call	printf
         */
        else if (temp[j]->op == "PRINT") {
          asmCode += "\t# PRINT\n";
          if (temp[j]->arg1->type == "STR") {
            if (temp[j]->arg1->value == "endl")
              asmCode += "\tmovl\t$.LC2,0(%esp)\n";
            else if (temp[j]->arg1->value == "space")
              asmCode += "\tmovl\t$.LC1,0(%esp)\n";
            else {
              asmCode += "\tsubl\t$" + int2str(8) + ",%esp\n";
              stackSize += 8;
              asmCode += "\tmovl\t$.LC" + temp[j]->arg1->value + ",0(%esp)\n";
            }
            asmCode += "\tcall\tprintf\n";
            asmCode += "\taddl\t$" + int2str(8) + ",%esp\n";
            stackSize -= 8;
          } else if (temp[j]->arg1->type == "NUM") {
            asmCode += "\tsubl\t$" + int2str(8) + ",%esp\n";
            stackSize += 8;
            asmCode += "\tmovl\t$" + temp[j]->arg1->value + ", 4(%esp)\n";
            asmCode += "\tmovl\t$.LC3,0(%esp)\n";
            asmCode += "\tcall\tprintf\n";
            asmCode += "\taddl\t$" + int2str(8) + ",%esp\n";
            stackSize -= 8;
          } else {
            //首先获取值
            asmCode += "\tsubl\t$" + int2str(8) + ",%esp\n";
            stackSize += 8;
            if (temp[j]->arg1->isGlobal) {
              asmCode += "\tmovl\t" + temp[j]->arg1->value + ", %eax\n";
            } else
              asmCode += "\tmovl\t" +
                         int2str(stackSize - (temp[j]->arg1->offset + 1) * 4) +
                         "(%esp), %eax\n";
            asmCode += "\tmovl\t%eax, 4(%esp)\n";
            asmCode += "\tmovl\t$.LC3,0(%esp)\n";
            asmCode += "\tcall\tprintf\n";
            asmCode += "\taddl\t$" + int2str(8) + ",%esp\n";
            stackSize -= 8;
          }
        }
      }
      asmCode += "\tjmp\t.leave_" + funcName + "\n";
      // 函数出来的地方
      asmCode += ".leave_" + funcName + ":\n" + "\tleave\n\tret\n";
    }
  }

  return;
}
