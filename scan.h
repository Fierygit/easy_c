/*
 * @Author: Firefly
 * @Date: 2020-01-18 09:22:53
 * @Descripttion: 
 * @LastEditTime : 2020-01-18 16:32:03
 */
#include <string>
using namespace std;


bool getSourceCodeFromFile(string file);

bool isKeyword(string prime);
void tokenOne(string type, int start, int end, int row, int column);
void tokenAll();
