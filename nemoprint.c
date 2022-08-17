
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include "rvcc.h"

FILE * getFp()
{
	FILE *fp = fopen("./nemodump.log", "a");
	if(NULL == fp)
	{
		printf("open nemodump.log fail errno = %d reason = %s \n", errno, strerror(errno));
		return NULL;
	}
	return fp;
}
char *transSlashes(char *str)
{
    /* allocate enough memory to escape all characters in str */
    char *new_str = calloc(1, strlen(str) * 2 + 1);
	//char *P = NULL;
	//int C = 0;
	char *ptr = new_str;
	char output[7] = {0};
	int i, j = 0;
	//strncpy(ptr, "0x", 2);
	//ptr += 2;
	for (i = 0; i < strlen(str); i++)
    {
		if(str[i] >= 0x20 && str[i] <= 0x7E)
		{
			*ptr = str[i];
			ptr++;
		}
		else
		{
			sprintf(output, "(0x%02x)", (unsigned char)str[i]);
			strncpy(ptr, output, strlen(output));
			ptr += strlen(output);
		}
	}
	strncpy(ptr, "\\0", 2);
	return new_str;
}

//////////////////////////////////////////打印/////////////////////////////////////////////
void nemoPrintStr(char * str)
{
	FILE *fp = getFp();
	if(NULL == fp)
	{
		printf("nemoPrintToken fp is null!\n");
		return;
	}
	fprintf(fp, "##################################################################\n");
	fprintf(fp, "Prefix:\n");
	fprintf(fp, "  t:type kind; n:Node kind; \n");
	fprintf(fp, "Input Code:\n  %s\n", str);
	fclose(fp);
}
//////////////////////////////////////////打印Token/////////////////////////////////////////////
// 判断标记符的首字母规则
// [a-zA-Z_]
static bool isIdent1(char C) {
  // a-z与A-Z在ASCII中不相连，所以需要分别判断
  return ('a' <= C && C <= 'z') || ('A' <= C && C <= 'Z') || C == '_';
}

// 判断标记符的非首字母的规则
// [a-zA-Z0-9_]
static bool isIdent2(char C) { return isIdent1(C) || ('0' <= C && C <= '9'); }

// 判断Str是否以SubStr开头
static bool startsWith(char *Str, char *SubStr) {
  // 比较LHS和RHS的N个字符是否相等
  return strncmp(Str, SubStr, strlen(SubStr)) == 0;
}

void nemoPrintToken(Token *Tok)
{
	char *P = NULL;
	char *display = NULL;
	FILE *fp = getFp();
	if(NULL == fp)
	{
		printf("nemoPrintToken fp is null!\n");
		return;
	}
	if(Tok == NULL)
	{
		fprintf(fp, "Tok is null\n");
		return;
	}
	//fprintf(fp, "====================================================\n");
	fprintf(fp, "Token list:\n  ");
	//fprintf(fp, "-------------------------------------------------\n");
	while(Tok->Kind != TK_EOF)
	{
		switch (Tok->Kind)
		{
		case TK_KEYWORD:
		case TK_IDENT:
			P = Tok->Loc;
			do {
				fprintf(fp, "%c", *P);
				++P;
			} while (isIdent2(*P));
			fprintf(fp, " ");
			break;
		case TK_PUNCT: // 打印运算符号
			if (startsWith(Tok->Loc, "=="))
			{
				fprintf(fp, "== ");
			}
			else if(startsWith(Tok->Loc, "!="))
			{
				fprintf(fp, "!= ");
			}
			else if(startsWith(Tok->Loc, "<="))
			{
				fprintf(fp, "<= ");
			}
			else if(startsWith(Tok->Loc, ">="))
			{
				fprintf(fp, ">= ");
			}
			else
			{
				fprintf(fp, "%c ", *(char *)(Tok->Loc));
			}
			break;
		case TK_STR: // 打印字符串字面量
			display = transSlashes(Tok->Str);
			fprintf(fp, "%s ", display);
			free(display);
			break;
		case TK_NUM: // 打印运算整数
			fprintf(fp, "%d ", Tok->Val);
			break;
		default:
			break;
		}
		Tok = Tok->Next;
	}
	fprintf(fp, "\n");
	//fprintf(fp, "\n====================================================\n");
	fclose(fp);
}

////////////////////////////////////////打印语法树///////////////////////////////////////////////
// 获取type的类型对应的字符串
char *getTypeString(TypeKind kind)
{
	switch (kind)
	{
	case TY_CHAR:
		return "tChr";
		break;
	case TY_INT:
		return "tInt";
		break;
	case TY_PTR:
		return "tPtr";
		break;
	case TY_FUNC:
		return "tFnc";
		break;
	case TY_ARRAY:
		return "tArr";
		break;
	default:
		break;
	}
	return "?";
}
// 打印二叉树:   type : 0表示根节点，１表示左节点，２表示右节点. level表示层次，用于控制显示的距离
void nemoPrintTree(FILE *fp, Node *node, int type,  int level)
{
	int i;

	if (NULL == node)
		return;
	nemoPrintTree(fp, node->RHS, 2, level+1);
	// 打印时的缩进
	switch (type)
	{
	case 0:
		for (i = 0; i < level; i++)	
			fprintf(fp, "\t");
		break;
	case 1:
		for (i = 0; i < level; i++)	
			fprintf(fp, "\t");
		fprintf(fp, "\\ ");
		break;
	case 2:
		for (i = 0; i < level; i++)	
			fprintf(fp, "\t");
		fprintf(fp, "/ ");
		break;	
	}
	// 打印节点数据类型
	if(node->Ty != NULL)
	{
		fprintf(fp, "%s ", getTypeString(node->Ty->Kind));
	}
	// 打印节点类型
	switch (node->Kind)
	{
	case ND_ADD:
		fprintf(fp, "+\n");
		break;
	case ND_SUB:
		fprintf(fp, "-\n");
		break;
	case ND_MUL:
		fprintf(fp, "*\n");
		break;
	case ND_DIV:
		fprintf(fp, "/\n");
		break;
	case ND_NEG:
		fprintf(fp, "NEG\n");
		break;
	case ND_EQ:
		fprintf(fp, "==\n");
		break;
	case ND_NE:
		fprintf(fp, "!=\n");
		break;
	case ND_LT:
		fprintf(fp, "<\n");
		break;
	case ND_LE:
		fprintf(fp, "<=\n");
		break;
	case ND_NUM:
		fprintf(fp, "%-10d\n", node->Val);
		break;
	case ND_ASSIGN:
		fprintf(fp, "=\n");
		break;
	case ND_ADDR:
		fprintf(fp, "&\n");
		break;
	case ND_DEREF:
		fprintf(fp, "*\n");
		break;
	case ND_RETURN:
		fprintf(fp, "return\n");
		break;
	case ND_IF:
		fprintf(fp, "if\n");
		for (i = 0; i < level; i++)	
			fprintf(fp, "\t");
		fprintf(fp, "    cond: \n");
		nemoPrintTree(fp, node->Cond, 0,  level+1);
		for (i = 0; i < level; i++)	
			fprintf(fp, "\t");
		fprintf(fp, "    then: \n");
		nemoPrintTree(fp, node->Then, 0,  level+1);
		if(node->Els != NULL)
		{
			for (i = 0; i < level; i++)	
				fprintf(fp, "\t");
			fprintf(fp, "    else: \n");
			nemoPrintTree(fp, node->Els, 0,  level+1);
		}
		break;
	case ND_FOR:
		fprintf(fp, "for\n");
		for (i = 0; i < level; i++)	
			fprintf(fp, "\t");
		fprintf(fp, "    init: \n");
		nemoPrintTree(fp, node->Init, 0,  level+1);
		for (i = 0; i < level; i++)	
			fprintf(fp, "\t");
		fprintf(fp, "    cond: \n");
		nemoPrintTree(fp, node->Cond, 0,  level+1);
		for (i = 0; i < level; i++)	
			fprintf(fp, "\t");
		fprintf(fp, "    inc: \n");
		nemoPrintTree(fp, node->Inc, 0,  level+1);
		for (i = 0; i < level; i++)	
			fprintf(fp, "\t");
		fprintf(fp, "    then: \n");
		nemoPrintTree(fp, node->Then, 0,  level+1);
		break;
	case ND_EXPR_STMT:
		fprintf(fp, "EXPR\n");
		break;
	case ND_STMT_EXPR:
		fprintf(fp, "STMT ");
		if(node->Body != NULL)
		{
			fprintf(fp, "Body:\n");
			for (Node *N = node->Body; N; N = N->Next)
				nemoPrintTree(fp, N, 1, level+1);
		}
		else
		{
			fprintf(fp, "\n");
		}
		break;
	case ND_VAR:
		fprintf(fp, "%s\n", node->Var->Name);
		break;
  	case ND_BLOCK:
		fprintf(fp, "Block ");
		if(node->Body != NULL)
		{
			fprintf(fp, "Body:\n");
			for (Node *N = node->Body; N; N = N->Next)
				nemoPrintTree(fp, N, 1, level+1);
		}
		else
		{
			fprintf(fp, "\n");
		}
		break;
	case ND_FUNCALL:
		fprintf(fp, "nFnc %s ", node->FuncName);
		if(node->Args != NULL)
		{
			fprintf(fp, "Args:\n");
			for (Node *N = node->Args; N; N = N->Next)
				nemoPrintTree(fp, N, 1, level+1);
		}
		else
		{
			fprintf(fp, "\n");
		}
		break;
	default:
		fprintf(fp, "?\n");
		break;
	}
	nemoPrintTree(fp, node->LHS, 1,  level+1);
}

void nemoPrintAST(Obj *globalVars)
{
	int count = 0;
	FILE *fp = getFp();
	if(NULL == fp)
	{
		printf("nemoPrintAST fp is null!\n");
		return;
	}
	fprintf(fp, "===============================================================\n");
	//fprintf(fp, "+++++++++++++++++++++++++++++++++++++++++++++++++\n");
	// 打印全局变量Globals
	fprintf(fp, "----------------------- Globals list --------------------------\n");
	//fprintf(fp, "Globals list:\n");
	for (Obj *gvar = globalVars; gvar != NULL; gvar = gvar->Next)
	{
		if(gvar->IsFunction)
		{
			fprintf(fp, "func:%s\n", gvar->Name);
		}
		else
		{
			fprintf(fp, "glbVar:%s, type:%s\n", gvar->Name, getTypeString(gvar->Ty->Kind));
			if (gvar->InitData)
			{
				fprintf(fp, "  InitData:%s\n", transSlashes(gvar->InitData));
			}
			if(gvar->Ty->Kind == TY_ARRAY) // 打印数组及其基础类型
			{
				fprintf(fp, "  array arrylen: %d, size: %d\n", gvar->Ty->ArrayLen, gvar->Ty->Size);
				for (Type * base = gvar->Ty->Base; base; base = base->Base, ++count)
				{
					fprintf(fp, "  base%d arrylen: %d, size: %d, type: %s\n", count,base->ArrayLen, base->Size, getTypeString(base->Kind));
				}
			}
		}
	}
	// 打印函数中的局部变量
	fprintf(fp, "------------------------ Locals list --------------------------\n");
	for (Obj *func = globalVars; func != NULL; func = func->Next)
	{
		if(func->IsFunction)
		{
			fprintf(fp, "Func %s:\n", func->Name);
			// 打印栈大小
			fprintf(fp, "  StackSize: %d\n", func->StackSize);
			fprintf(fp, "  Variables:\n");
			// for (Obj *param = func->Params; param; param = param->Next) {
			// 	fprintf(fp, "  param Name: %s, type: %s, Offset: %d\n", param->Name, getTypeString(param->Ty->Kind), param->Offset);
			// }
			for (Obj *Var = func->Locals; Var; Var = Var->Next) {
				fprintf(fp, "    Name: %s, type: %s, Offset: %d\n", Var->Name, getTypeString(Var->Ty->Kind), Var->Offset);
				if(Var->Ty->Kind == TY_ARRAY)
				{
					fprintf(fp, "      array arrylen: %d, size: %d\n", Var->Ty->ArrayLen, Var->Ty->Size);
					for (Type * base = Var->Ty->Base; base; base = base->Base, ++count)
					{
						fprintf(fp, "      base%d arrylen: %d, size: %d, type: %s\n", count,base->ArrayLen, base->Size, getTypeString(base->Kind));
					}
				}
			}
		}
	}
	// fprintf(fp, "+++++++++++++++++++++++++++++++++++++++++++++++++\n");
	// fprintf(fp, "AST tree: \n");
	// 循环遍历所有的函数
	fprintf(fp, "------------------------- Func Tree ---------------------------\n");
	for (Obj *func = globalVars; func != NULL; func = func->Next)
	{
		if(func->IsFunction)
		{
			fprintf(fp, "%s Body:\n", func->Name);
			nemoPrintTree(fp, func->Body, 0,  1);
		}
	}

	fprintf(fp, "===============================================================\n\n");
	fclose(fp);
}