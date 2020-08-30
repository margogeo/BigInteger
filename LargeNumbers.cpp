//LargeNumbers.cpp
//Author: Margarita Shimanskaia
//Arbitrary precission arithmetic calculations

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#define numOp(op,fun) friend LargeNum* operator op (LargeNum a, LargeNum b) {return a.fun(&b);};
#define cmpOp(op) friend int operator op (LargeNum& a, LargeNum& b) {return (a.cmp(&b) op 0);}; 
#define imax(a,b) ( a < b ? b : a) 

FILE *fout;

class LargeNum {
  int sign,
	  nDigs;
  char *digs;
  void Reserve(int maxLength);
 public:
  void fprintf(FILE *fl);
  int isWrong() {
	  return nDigs < 0;
  }
  void SetLim(int digsNum, int isMax);
  LargeNum(char *str);
  LargeNum(int nMaxSize);
  LargeNum* sqrt();
  LargeNum* add(LargeNum *b, int isSub = 0);
  LargeNum* sub(LargeNum *b);
  LargeNum* mul(LargeNum *b);
  LargeNum* div(LargeNum *b, int isMod = 0);
  LargeNum* mod(LargeNum *b);
  void CopyFrom(LargeNum *a);
  void Half();
  int cmp(LargeNum *n);
  numOp( + , add);
  numOp( - , sub);
  numOp( * , mul);
  numOp( / , div);
  numOp( % , mod);
  cmpOp( < );
  cmpOp( <= );
  cmpOp( > );
  cmpOp( >= );
  cmpOp( == );
  cmpOp( != );
};

//Set min or max value with given digits number
void LargeNum::SetLim(int digsNum, int isMax)
{
	Reserve(digsNum);
	nDigs = digsNum;
	if (isMax)
		memset(digs, 9, nDigs);
	else {
		memset(digs, 0, nDigs);
		digs[nDigs - 1] = 1;
	}
}

//Reserve array for given number of digits
void LargeNum::Reserve(int maxLength)
{
	if (maxLength < nDigs - 1)
		return;
	maxLength += 10;
	digs = (char*)realloc(digs, maxLength);
	if (digs == NULL) {
		printf("No enough memory\n");
		exit(0);
	}
	memset(digs + nDigs, 0, maxLength - nDigs);
}

LargeNum::LargeNum(char *str)
{
	digs = NULL;
	sign = 1;
	if (*str == '+')
		str++;
	if (*str == '-') {
		sign = -1;
		str++;
	}
	nDigs = 0;
	Reserve(strlen(str));
	for (int k=0, i = strlen(str) - 1; i >= 0 ; i--, k++) {
		digs[k] = str[i] - '0';
		if (digs[k])
			nDigs = k + 1;
		if (digs[k] < 0 || digs[k] > 9) {  //Incorrect number
			nDigs = -1;
			return;
		}
	}
}

LargeNum::LargeNum(int nMaxSize)
{
	nDigs = sign = 0;
	digs = NULL;
	Reserve(nMaxSize);
}

void LargeNum::CopyFrom(LargeNum *a)
{
	Reserve(a->nDigs);
	nDigs = a->nDigs;
	sign = a->sign;
	memcpy(digs, a->digs, nDigs);
}

LargeNum* LargeNum::sqrt()
{
	LargeNum a("1"), b("1"), *c, *c2, *ret = new LargeNum(nDigs);
	if (nDigs == 0)
		return ret;
	if (sign < 0) {
		ret->nDigs = -1;
		return ret;
	}
	int rDigs = (nDigs + 1) / 2;
	a.SetLim(rDigs, 0);
	b.SetLim(rDigs, 1);

	for ( ;; ) {
		c = (a + b);
		c->Half();
	
		if (a.cmp(c) == 0 || b.cmp(c) == 0) {
			c2 = b.mul(&b);
			if (c2->cmp(this) <= 0)
				ret->CopyFrom(&b);
			else
				ret->CopyFrom(c);
			delete c;
			delete c2;
			break;
		}
		c2 = c->mul(c); 
		if (cmp(c2) < 0)
			b.CopyFrom(c);
		else
			a.CopyFrom(c);
		delete c;
		delete c2;
	}
	return ret;
}

void LargeNum::Half()
{
  int dop = 0;
  for (int i=nDigs-1 ; i >= 0 ; i--) {
	  int t = digs[i] + dop;
	  if (t & 1)
		  dop = 10;
	  else 
		  dop = 0;
	  digs[i] = t / 2;
  }
  if (nDigs > 0 && digs[nDigs-1] == 0)
	  nDigs--;
}

LargeNum* LargeNum::add(LargeNum *b, int isSub)
{
	int bSize = b->nDigs;
	char *bDigs = b->digs;
	int bSign = isSub ? -b->sign : b->sign;
	int newSize = imax(nDigs, bSize);
	LargeNum *ret = new LargeNum(newSize + 1);
	ret->sign = sign;
	char *retDigs = ret->digs;
	int dop = 0, rSize = newSize, isNegate = 0;
	if (sign == bSign) {
		for (int i = 0; i < newSize ; i++) {
			if (i < nDigs)
				dop += digs[i];
			if (i < bSize)
				dop += bDigs[i];
			retDigs[i] = dop % 10;
			dop /= 10;
		}
		if (dop) {
			retDigs[newSize] = dop;
			rSize = newSize + 1;
		}
		ret->nDigs = rSize;
	}
	else { //sub
		for (int i = 0; i < newSize ; i++) {
			int ai = i < nDigs ? digs[i] : 0;
			if (i < bSize) {
				dop += bDigs[i];
			}
			int r = ai - dop;
			if (r < 0) {
				retDigs[i] = r + 10;
				dop = 1;
			}
			else {
				retDigs[i] = r;
				dop = 0;
			}
		}
		if (dop) {
			ret->sign = -ret->sign;
			dop = 0;
			for (int i = 0 ; i < newSize ; i++) {
				int rt = retDigs[i];
				retDigs[i] = (20 - rt - dop) %10;
				if (rt)
					dop = 1;
			}
		}	
		for (int i = 0 ; i < newSize ; i++) 
			if (retDigs[i])
				ret->nDigs = i + 1;
	}
	return ret;
}

LargeNum* LargeNum::sub(LargeNum *b) {
	return add(b, 1);
}

LargeNum* LargeNum::mul(LargeNum *b)
{
	int bSize = b->nDigs;
	char *bDigs = b->digs;
	int rSize = nDigs + bSize;
	LargeNum *ret = new LargeNum(rSize + 1);
	char *retDigs = ret->digs;
	for (int i = 0; i < bSize ; i++) {
		int j, dop = 0, dg = bDigs[i];
		if (dg)
			for (j=0; j < nDigs ; j++) {
				dop += dg * digs[j] + retDigs[i + j];
				retDigs[i + j] = dop % 10;
				dop /= 10;
			}
			for ( ; dop ; j++) {
				dop += retDigs[i + j];
				retDigs[i + j] = dop % 10;
				dop /= 10;
			}
	}
	ret->sign = sign * b->sign;
	for ( ; rSize > 0 && retDigs[rSize] == 0 ; rSize--);
	ret->nDigs = rSize + 1;
	return ret;
}

LargeNum* LargeNum::div(LargeNum *b, int isMod)
{
	int bSize = b->nDigs;
	char *bDigs = b->digs;
	int rSize = nDigs;
	LargeNum *ret = new LargeNum(rSize + 1);
	if (bSize == 0) {
		ret->nDigs = -1;
		return ret;
	}
	if (bSize > nDigs) {
		if (isMod)
			ret->CopyFrom(this);
		return ret;
	}
	int kr = nDigs - bSize;
	char *retDigs = ret->digs;
	char *subd = (char*)malloc(nDigs+1); //To keep source digits set

	for (int i = 0; i <= kr ; i++) {
		int dk = 0, kri = kr - i;

		for ( ; dk < 10 ; dk++) {
			memcpy(subd, digs, nDigs);
			int dop = 0;
			for(int j = 0; j < bSize ; j++) {
				dop += bDigs[j];
				int r = digs[j + kri] - dop;
				if (r < 0) {
					digs[j + kri] = r + 10;
					dop = 1;
				}
				else {
					digs[j + kri] = r;
					dop = 0;
				}
			}
			if (dop)
				if (digs[kri + bSize]) {
					digs[kri + bSize]--;
				}
				else {
					memcpy(digs, subd, nDigs);
					break;
				}
		}
		retDigs[kri] = dk; 
		digs[bSize + kri] = 0;
	}
	ret->sign = sign * b->sign;
	if (isMod)
		ret->CopyFrom(this);
	for (kr = nDigs ; kr >= 0 && retDigs[kr] == 0 ; kr--);
	ret->nDigs = kr + 1;

	free(subd);
	return ret;
}

LargeNum* LargeNum::mod(LargeNum *b)
{
	return div(b, 1);
}

//compare this < n : -1 , this = n : 0 this > n : 1 
int LargeNum::cmp(LargeNum *n)
{
	if (sign < n->sign)
		return -1;
	if (sign > n->sign)
		return 1;
	if (nDigs > n->nDigs)
		return sign;
	if (nDigs < n->nDigs)
		return -sign;
	for (int i = nDigs - 1 ; i >= 0; i--) {
	   if (digs[i] < n->digs[i])	
		   return -sign;
	   if (digs[i] > n->digs[i])	
		   return sign;
	}
	return 0;
}

void LargeNum::fprintf(FILE *fl)
{
	if (nDigs == 0) {
		fputc('0', fl);
		return;
	}
	if (nDigs < 0) {   //Incorrect / undefined number
		fputs("NaN", fl);
		return;
	}
	if (sign < 0)
		fputc('-', fl);
	for (int i = nDigs - 1; i >= 0 ; i--)
		fputc(digs[i] + '0', fl);
}

//Print number into the file and delete object
void printNum(LargeNum *c)
{
	c->fprintf(fout);
	delete c;
}

//Print logical result of comparation
void printCmp(int logExpr)
{
	fputc('0' + logExpr, fout);
}

//Get next word from buf, write zero byte at the end
char *nextWord(char *bf, int &i, int sz)
{
	char *ret = NULL;
	for ( ; bf[i] <= ' ' && i < sz ; i++);
	if (i == sz)
		return NULL;
	ret = bf + i;
	for ( ; bf[i] > ' '&& i < sz ; i++);
	bf[i] = 0;
	return ret;
}

int main(int argc, char **argv) 
{
	int i = 0, j, iop;
	if (argc != 3) {
		printf("Usage:\r\nlab4 <input file> <output file>\n");
		return 1;
	}
	FILE *fin = fopen(argv[1],"rb");
	if (fin == NULL) {
		printf("Can't open input file %s \n", argv[1]);
		return 2;
	}

	fseek(fin, 0L, SEEK_END);
	int sz = ftell(fin);
	fseek(fin, 0L, SEEK_SET);
	char *bf = (char*)malloc(sz+1);
	fread(bf, 1, sz, fin);
	char *sa = nextWord(bf, i, sz),
		 *cop = nextWord(bf, i, sz),
		 *sb = nextWord(bf, i, sz);

	fclose(fin);

	if (sa == NULL || cop == NULL || sb == NULL && *cop != '#') {
		printf("Wrong input file format\n");
		return 3;		
	}

	char ops[] = "#+-*/%<>!=";
	for (iop = 0 ; ops[iop] && *cop != ops[iop]; iop++);

	if (ops[iop] == 0 || iop < 6 && cop[1] > ' ' || cop[1] > ' ' && cop[1] != '=') {
		printf("Incorrect operation %s\n", cop);
		return 3;
	}

	LargeNum a(sa);
	if (a.isWrong()) {
		printf("First number is incorect %s \n", sa);
		return 3;
	}

	fout = fopen(argv[2], "wb");
	if (fout == NULL) {
		printf("Can't create output file %s \n", argv[2]);
		return 3;
	}

	if (*cop == '#') {
		printNum( a.sqrt() );
	}
	else {
		LargeNum b(sb);
		if (b.isWrong()) {
			printf("Second number is incorect %s \n", sb);
			fclose(fout);
			return 3;
		}

		if (*cop == '+') 
			printNum(a + b);
		if (*cop == '-') 
			printNum(a - b);
		if (*cop == '*') 
			printNum(a * b);
		if (*cop == '/') 
			printNum(a / b);
		if (*cop == '%') 
			printNum(a % b);

		if (cop[1] == '=') {
			if (*cop == '>')
				printCmp(a >= b);
			if (*cop == '<')
				printCmp(a <= b);
			if (*cop == '!')
				printCmp(a != b);
			if (*cop == '=')
				printCmp(a == b);
		}
		else {
			if (*cop == '>')
				printCmp(a > b);
			if (*cop == '<')
				printCmp(a < b);
		}
	}
	fclose(fout);
	return 0;
}
