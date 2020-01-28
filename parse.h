/*
 * @Author: Firefly
 * @Date: 2020-01-18 20:16:14
 * @Descripttion: 
 * @LastEditTime : 2020-01-18 20:36:26
 */
#include "global.h"



//方法的声明
TreeNode *declaration_list();
TreeNode *declaration();

TreeNode *param_list();
TreeNode *params();
TreeNode *param();

TreeNode *compound_stmt();

TreeNode *local_declaration();
TreeNode *statement_list();
TreeNode *statement();

TreeNode *selection_stmt();
TreeNode *iteration_stmt();
TreeNode *expression_stmt();
TreeNode *return_stmt();

TreeNode *expression();
TreeNode *simple_expression(TreeNode *t);
TreeNode *term(TreeNode *t);

TreeNode *factor(TreeNode *t);


void parse();
Token*  getToken();