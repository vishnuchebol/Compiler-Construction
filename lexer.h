#include "lexerDef.h"
FILE *getStream(FILE *fp);

void retract(int amt,twinBuffer* B);

int checkSingleChar(char ch, char chToEqual);

void accept(twinBuffer* B);

void initBuffer(int size);

char getNextChar();

void initLexer(int f);



char* copyString(char* start, char* end);

tokenInfo createToken(TokenName tokenName,char* lexeme,int lineNumber,int isNumber,Value* value);

int checkInRange(char ch,char start, char end);
tokenInfo getNextToken(twinBuffer* B);
removeComments(char *testcaseFile, char *cleanFile);