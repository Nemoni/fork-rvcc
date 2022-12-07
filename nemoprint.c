
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

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
//////////////////////////////////////////打印Token/////////////////////////////////////////////
// 为每个终结符都设置种类来表示
typedef enum {
  TK_IDENT, // 标记符，可以为变量名、函数名等
  TK_PUNCT, // 操作符如： + -
  TK_KEYWORD, // 关键字
  TK_NUM,   // 数字
  TK_EOF,   // 文件终止符，即文件的最后
} TokenKind;

// 终结符结构体
typedef struct Token Token;
struct Token {
  TokenKind Kind; // 种类
  Token *Next;    // 指向下一终结符
  int Val;        // 值
  char *Loc;      // 在解析的字符串内的位置
  int Len;        // 长度
};

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
	fprintf(fp, "#############################################################\n");
	fprintf(fp, "====================================================\n");
	fprintf(fp, "Token list:\n");
	fprintf(fp, "-------------------------------------------------\n");
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
		case TK_NUM: // 打印运算整数
			fprintf(fp, "%d ", Tok->Val);
			break;
		default:
			break;
		}
		Tok = Tok->Next;
	}
	fprintf(fp, "\n====================================================\n");
	fclose(fp);
}

////////////////////////////////////////打印语法树///////////////////////////////////////////////
typedef struct Node Node;
typedef struct Type Type;

// 本地变量
typedef struct Obj Obj;
struct Obj {
  Obj *Next;  // 指向下一对象
  char *Name; // 变量名
  Type *Ty;   // 变量类型
  int Offset; // fp的偏移量
};

// 函数
typedef struct Function Function;
struct Function {
  Function *Next; // 下一函数
  char *Name;     // 函数名
  Node *Body;    // 函数体
  Obj *Locals;   // 本地变量
  int StackSize; // 栈大小
};

// 类型种类
typedef enum {
  TY_INT,  // int整型
  TY_PTR,  // 指针
  TY_FUNC, // 函数
} TypeKind;

struct Type {
  TypeKind Kind; // 种类
  Type *Base;    // 指向的类型
  // 变量名
  Token *Name;
  // 函数类型
  Type *ReturnTy; // 函数返回的类型
};

// AST的节点种类
typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_NEG, // 负号-
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // <
  ND_LE,  // <=
  ND_ASSIGN,    // 赋值
  ND_ADDR,      // 取地址 &
  ND_DEREF,     // 解引用 *
  ND_RETURN,    // 返回
  ND_IF,        // "if"，条件判断
  ND_FOR,       // "for" 或 "while"，循环
  ND_BLOCK,     // { ... }，代码块
  ND_FUNCALL,   // 函数调用
  ND_EXPR_STMT, // 表达式语句
  ND_VAR,       // 变量
  ND_NUM, // 整形
} NodeKind;

// AST中二叉树节点
typedef struct Node Node;
struct Node {
  NodeKind Kind; // 节点种类
  Node *Next;    // 下一节点，指代下一语句
  Token *Tok;    // 节点对应的终结符
  Type *Ty;      // 节点中数据的类型
  Node *LHS;     // 左部，left-hand side
  Node *RHS;     // 右部，right-hand side
  // "if"语句或"for"语句
  Node *Cond; // 条件内的表达式
  Node *Then; // 符合条件后的语句
  Node *Els;  // 不符合条件后的语句
  Node *Init; // 初始化语句
  Node *Inc;  // 递增语句
  // 代码块
  Node *Body;
  // 函数调用
  char *FuncName; // 函数名
  Node *Args;     // 函数参数
  Obj *Var;      // 存储ND_VAR种类的变量
  int Val;       // 存储ND_NUM种类的值
};

char *getTypeString(TypeKind kind)
{
	switch (kind)
	{
	case TY_INT:
		return "i";
		break;
	case TY_PTR:
		return "p";
		break;
	case TY_FUNC:
		return "f";
		break;
	default:
		return "?";
		break;
	}
	return "";
}
// 打印二叉树:   type : 0表示根节点，１表示左节点，２表示右节点. level表示层次，用于控制显示的距离
void nemoPrintTree(FILE *fp, Node *node, int type,  int level)
{
	int i;

	if (NULL == node)
		return;
	nemoPrintTree(fp, node->RHS, 2, level+1);
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
	if(node->Ty != NULL)
	{
		fprintf(fp, "%s ", getTypeString(node->Ty->Kind));
	}
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
		fprintf(fp, "F %s ", node->FuncName);
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

void nemoPrintAST(Function *funcs)
{
	FILE *fp = getFp();
	if(NULL == fp)
	{
		printf("nemoPrintAST fp is null!\n");
		return;
	}
	fprintf(fp, "====================================================\n");
	fprintf(fp, "Variables:\n");
	fprintf(fp, "-------------------------------------------------\n");
	// 打印所有变量
	for (Function *func = funcs; func != NULL; func = func->Next)
	{
		fprintf(fp, "Func %s:\n", func->Name);
		for (Obj *Var = func->Locals; Var; Var = Var->Next) {
			fprintf(fp, "  Var Name: %s, type: %s, Offset: %d\n", Var->Name, getTypeString(Var->Ty->Kind), Var->Offset);
		}
	}
	fprintf(fp, "+++++++++++++++++++++++++++++++++++++++++++++++++\n");
	// 打印栈大小
	fprintf(fp, "StackSize: %d\n", funcs->StackSize);
	fprintf(fp, "+++++++++++++++++++++++++++++++++++++++++++++++++\n");
	fprintf(fp, "AST tree: \n");
	fprintf(fp, "\tNode type: F(function) \n");
	fprintf(fp, "\tData type: p(pointor), i(int) \n");
	// 循环遍历所有的语句
	fprintf(fp, "-------------------------------------------------\n");
	for (Function *func = funcs; func != NULL; func = func->Next)
	{
		fprintf(fp, "Func %s:\n", func->Name);
		nemoPrintTree(fp, func->Body, 0,  1);
	}

	fprintf(fp, "====================================================\n\n");
	fclose(fp);
}