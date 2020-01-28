/*
 * @Author: Firefly
 * @Date: 2020-01-18 10:29:27
 * @Descripttion: 
 * @LastEditTime : 2020-01-28 19:41:57
 */
#include "global.h"
#include <vector>
#include <sstream>

#ifndef UTIL_H
#define UTIL_H
void outputParseInfo(string info);
void outputTokenCode(vector<Token*> token);
void outputTraceTree(TreeNode * root);
void outputSymbalTable();
string format(string info, int len) ;
string mapNode(NodeKind n);
string int2str(int num) ;
int str2int(string num);

void outputMidCode() ;


void outputAsm2File(string path);


#endif