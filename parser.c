#include "parser.h"
#include "interface.h"
#include "lexer.h"
#include <string.h>
#define GRAMMAR_FILE "grammar.txt"
#define TOTAL_GRAMMAR_NONTERMINALS 49 // TODO get the actual number of nonterminals
#define TOTAL_GRAMMAR_TERMINALS 56
#define TOTAL_GRAMMAR_RULES 87 //TODO actual number of rules
int syntaxErrorFlag;
int lexicalErrorFlag;

Grammar* g;
char* TerminalID[] = {
    "TK_ASSIGNOP",
    "TK_COMMENT",
    "TK_ID",
    "TK_NUM",
    "TK_RNUM",
    "TK_FIELDID",
    "TK_FUNID",
    "TK_RUID",
    "TK_WITH",
    "TK_PARAMETERS",
    "TK_END",
    "TK_WHILE",
    "TK_TYPE",
    "TK_MAIN",
    "TK_GLOBAL",
    "TK_PARAMETER",
    "TK_LIST",
    "TK_SQL",
    "TK_SQR",
    "TK_INPUT",
    "TK_OUTPUT",
    "TK_INT",
    "TK_REAL",
    "TK_COMMA",
    "TK_SEM",
    "TK_COLON",
    "TK_DOT",
    "TK_ENDWHILE",
    "TK_OP",
    "TK_CL",
    "TK_IF",
    "TK_THEN",
    "TK_ENDIF",
    "TK_READ",
    "TK_WRITE",
    "TK_RETURN",
    "TK_PLUS",
    "TK_MINUS",
    "TK_MUL",
    "TK_DIV",
    "TK_CALL",
    "TK_RECORD",
    "TK_ENDRECORD",
    "TK_ELSE",
    "TK_AND",
    "TK_OR",
    "TK_NOT",
    "TK_LT",
    "TK_LE",
    "TK_EQ",
    "TK_GT",
    "TK_GE",
    "TK_NE",
    "TK_EPS",
    "TK_DOLLAR",
    "TK_ERR",
    "TK_UNION"
};
char* copyLexeme(char* str) {
    int len = strlen(str);
    char* lex = (char*)malloc(sizeof(char)*(len+1));
    for(int i=0; i < len; i++)
        lex[i] = str[i];

    lex[len] = '\0';
    return lex;
}
// Utility function to append a character to symbol string
char* appendToSymbol(char* str, char c) {
    int len = strlen(str);
    char* strConcat = (char*)malloc(sizeof(char)*(len+2));
    for(int i=0; i < len; i++)
        strConcat[i] = str[i];

    strConcat[len] = c;
    strConcat[len+1] = '\0';
    return strConcat;
}
// Returns the Enum ID of the string in the TerminalID map if found, otherwise returns -1
int findInTerminalMap(char* str) {
    for(int i=0; i < TOTAL_GRAMMAR_TERMINALS; i++) {
        if(strcmp(str,TerminalID[i]) == 0)
            return i;
    }

    return -1;
}
// Returns the Enum ID of the string in the NonTerminalID map if found, otherwise returns -1
int findInNonTerminalMap(char* str) {
    for(int i=0; i < TOTAL_GRAMMAR_NONTERMINALS; i++) {
        if(strcmp(str,NonTerminalID[i]) == 0)
            return i;
    }

    return -1;
}
// Returns the string corresponding to the enumId (Required when printing is too be done outside parser.c)

char* getTerminal(int enumId) {
    return TerminalID[enumId];
}

char* getNonTerminal(int enumId) {
    return NonTerminalID[enumId];
}
ParsingTable* initialiseParsingTable() {
    ParsingTable* pt = (ParsingTable*)malloc(sizeof(ParsingTable));
    pt->entries = (int**)malloc(TOTAL_GRAMMAR_NONTERMINALS*sizeof(int*));
    for(int i=0; i < TOTAL_GRAMMAR_NONTERMINALS; i++) {
        // Calloc used to initialise with 0 by default, if left empty => error state
        pt->entries[i] = (int*)calloc(TOTAL_GRAMMAR_TERMINALS,sizeof(int));
    }
    return pt;
}
int initialiseGrammar() {

    g = (Grammar*)malloc(sizeof(Grammar));
    g->GRAMMAR_RULES_SIZE = TOTAL_GRAMMAR_RULES+1; // 1 added as 0 index is left as NULL to provide direct mapping by rule number to the rule
    g->GRAMMAR_RULES = (Rule**)malloc(sizeof(Rule*)*g->GRAMMAR_RULES_SIZE);
    g->GRAMMAR_RULES[0] = NULL;
}
Symbol* intialiseSymbol(char* symbol) {

    Symbol* s = (Symbol*)malloc(sizeof(Symbol));
        // Search for enum IDs in both maps
        int idNonTerminal, idTerminal;
        idNonTerminal = findInNonTerminalMap(symbol);
        // If idNonTerminal is found, assign it as the symbol type
        if(idNonTerminal != -1) {
            s->TYPE.NON_TERMINAL = idNonTerminal;
            s->IS_TERMINAL = 0;
        }
        else {
            idTerminal = findInTerminalMap(symbol);
            if(idTerminal != -1) {
                s->TYPE.TERMINAL = idTerminal;
                s->IS_TERMINAL = 1;
            }
        }

    s->next = NULL;

    return s;
}
Rule* initialiseRule(SymbolList* sl, int ruleCount) {
    Rule* r = (Rule*)malloc(sizeof(Rule));
    r->SYMBOLS = sl;
    r->RULE_NO = ruleCount;
    return r;
}
SymbolList* initialiseSymbolList() {
    SymbolList* sl = (SymbolList*)malloc(sizeof(SymbolList));
    sl->HEAD_SYMBOL = NULL;
    sl->TAIL_SYMBOL = NULL;
    sl->RULE_LENGTH = 0;
    return sl;
}
NonTerminalRuleRecords** intialiseNonTerminalRecords() {
    NonTerminalRuleRecords** ntrr = (NonTerminalRuleRecords**)malloc(sizeof(NonTerminalRuleRecords*)*TOTAL_GRAMMAR_NONTERMINALS);
    return ntrr;
}

void initialiseCheckIfDone() {
    for(int i=0; i < TOTAL_GRAMMAR_NONTERMINALS; i++)
        checkIfDone[i] = 0;
}
FirstAndFollow* initialiseFirstAndFollow() {
    FirstAndFollow* fafl = (FirstAndFollow*)malloc(sizeof(FirstAndFollow));

    // Initialize the array of vectors to be equal to the total number of Non terminals
    fafl->FIRST = (int**)malloc(sizeof(int*)*TOTAL_GRAMMAR_NONTERMINALS);
    fafl->FOLLOW = (int**)malloc(sizeof(int*)*TOTAL_GRAMMAR_NONTERMINALS);


    for(int i=0; i < TOTAL_GRAMMAR_NONTERMINALS; i++) {
        // Calloc used to initialize the vectors to 0
        fafl->FIRST[i] = (int*)calloc(vectorSize,sizeof(int));
        fafl->FOLLOW[i] = (int*)calloc(vectorSize,sizeof(int));
    }

    return fafl;

}
void calculateFirst(int** firstVector, int enumId) {

    // printf("Stack overflow being caused by %s\n" , NonTerminalID[enumId]);
    int start = ntrr[enumId]->start;
    int end = ntrr[enumId]->end;
    int producesNull = 0; // Flag which tracks whether the non terminal produces NULL

    for(int i=start; i <= end; i++) {
        Rule* r = g->GRAMMAR_RULES[i];
        Symbol* s = r->SYMBOLS->HEAD_SYMBOL;
        Symbol* trav = s;
        Symbol* nextSymbol = trav->next;
        int ruleYieldsEpsilon = 1;
        while(nextSymbol != NULL) {

            // Case when a terminal is encountered in the RHS
            if(nextSymbol->IS_TERMINAL == 1) {
                if(nextSymbol->TYPE.TERMINAL != TK_EPS) {
                    ruleYieldsEpsilon = 0;
                    firstVector[enumId][nextSymbol->TYPE.TERMINAL] = 1;
                }
                break;
            }

            // Case when it is a Non-terminal

            // Check if it's First has been calculated already, if not calculate it
            // In case of stack overflow, for debugging add a condition that nextSymbol should not habe the same ID as the enumId
            if(checkIfDone[nextSymbol->TYPE.NON_TERMINAL] == 0) {
                calculateFirst(firstVector,nextSymbol->TYPE.NON_TERMINAL);
            }

            for(int j=0; j < vectorSize; j++) {
                if(firstVector[nextSymbol->TYPE.NON_TERMINAL][j] == 1)
                    firstVector[s->TYPE.NON_TERMINAL][j] = 1;
            }

            if(firstVector[nextSymbol->TYPE.NON_TERMINAL][TK_EPS] == 0) {
                ruleYieldsEpsilon = 0;
                break;
            }

            nextSymbol = nextSymbol->next;
        }

        if(ruleYieldsEpsilon)
            producesNull = 1;
    }

    if(producesNull)
        firstVector[enumId][TK_EPS] = 1;
    else
        firstVector[enumId][TK_EPS] = 0;

    checkIfDone[enumId] = 1;

}
void populateFirst(int** firstVector, Grammar* g) {

    // Traversal is done by enum_id (which is iterator i in this case)
    // Grammar Rules are written in GRAMMAR_FILE in the same order as enum name as per convention

    for(int i=0; i < TOTAL_GRAMMAR_NONTERMINALS; i++) {
        if(checkIfDone[i] == 0)
            calculateFirst(firstVector,i);
    }
}
void populateFollow(int** followVector, int** firstVector, Grammar* g) {


    for(int i=1; i <= TOTAL_GRAMMAR_RULES; i++) {
        Rule* r = g->GRAMMAR_RULES[i];
        Symbol* head = r->SYMBOLS->HEAD_SYMBOL;
        Symbol* trav = head->next;
        int epsilonIdentifier = 0;
        while(trav != NULL) {

            if(trav->IS_TERMINAL == 0) {
                Symbol* followTrav = trav->next;
                while(followTrav != NULL) {
                    if(followTrav->IS_TERMINAL == 1 && followTrav->TYPE.TERMINAL != TK_EPS) {
                        followVector[trav->TYPE.NON_TERMINAL][followTrav->TYPE.TERMINAL] = 1;
                        break;
                    }
                    else {

                        for(int j=0; j < vectorSize; j++)
                            if(firstVector[followTrav->TYPE.NON_TERMINAL][j] == 1 && j != TK_EPS)
                                followVector[trav->TYPE.NON_TERMINAL][j] = 1;

                        if(firstVector[followTrav->TYPE.NON_TERMINAL][TK_EPS] == 0)
                            break;

                    }
                    followTrav = followTrav->next;
                }

                // Case when we need to take LHS Non terminal
                // Venkat => followTrav != NULL && followTrav->next == NULL
                if(trav->next == NULL || (followTrav == NULL)) {
                    for(int j=0; j < vectorSize; j++)
                        if(followVector[head->TYPE.NON_TERMINAL][j] == 1 && j != TK_EPS)
                            followVector[trav->TYPE.NON_TERMINAL][j] = 1;
                }

            }


            trav = trav->next;
        }
    }
}
// Function to keep populating the followVector until it stabilises
void populateFollowTillStable(int** followVector, int** firstVector, Grammar* g) {
    int** prevFollowVector = (int**)malloc(TOTAL_GRAMMAR_NONTERMINALS*sizeof(int*));

    for(int i=0; i < TOTAL_GRAMMAR_NONTERMINALS; i++) {
        prevFollowVector[i] = (int*)calloc(vectorSize,sizeof(int));
    }

    followVector[program][TK_DOLLAR] = 1;
    prevFollowVector[program][TK_DOLLAR] = 1;

    while(1) {

        populateFollow(followVector,firstVector,g);
        int stable = 1;

        for(int i=0; i < TOTAL_GRAMMAR_NONTERMINALS; i++) {
            for(int j=0; j < vectorSize; j++) {
                if(prevFollowVector[i][j] != followVector[i][j])
                    stable = 0;
            }
        }
        if(stable)
            break;

        for(int i=0; i < TOTAL_GRAMMAR_NONTERMINALS; i++) {
            for(int j=0; j < vectorSize; j++)
                prevFollowVector[i][j] = followVector[i][j];
        }
    }
}
FirstAndFollow* computeFirstAndFollowSets(Grammar* g) {
    FirstAndFollow* fafl = initialiseFirstAndFollow();
    populateFirst(fafl->FIRST,g);
    populateFollowTillStable(fafl->FOLLOW,fafl->FIRST,g);
    return fafl;
}
void createParseTable(FirstAndFollow* fafl, ParsingTable* pt) {

    for(int i=1; i <= TOTAL_GRAMMAR_RULES; i++) {
        Rule* r = g->GRAMMAR_RULES[i];
        int lhsNonTerminal = r->SYMBOLS->HEAD_SYMBOL->TYPE.NON_TERMINAL;

        // THIS IS INCORRECT AS THE TERMINALS BEING IN FIRST FROM OTHER RULES ALSO GET DIRECTED TO THIS RULE!
        // for(int j=0; j < TOTAL_GRAMMAR_TERMINALS; j++) {
        //     if(fafl->FIRST[lhsNonTerminal],[j] == 1) {
        //         // Since it is LL(1), no other non-epsilon producing rule will direct to an entry already filled
        //         // Ask Venkat to verify
        //         // Attempt to correct error, epsilon producing rule directs it back to this entry
        //         if(pt->entries[lhsNonTerminal][j] == 0)
        //             pt->entries[lhsNonTerminal][j] = r->RULE_NO;
        //     }
        // }

        Symbol* rhsHead = r->SYMBOLS->HEAD_SYMBOL->next;
        Symbol* trav = rhsHead;
        int epsilonGenerated = 1;

        while(trav != NULL) {
            // Terminal encountered in RHS => It cannot generate epsilon, break!
            if(trav->IS_TERMINAL == 1 && trav->TYPE.TERMINAL != TK_EPS) {
                epsilonGenerated = 0;
                pt->entries[lhsNonTerminal][trav->TYPE.TERMINAL] = r->RULE_NO;
                break;
            }
            else if(trav->IS_TERMINAL == 1 && trav->TYPE.TERMINAL == TK_EPS) {
                // No action
                epsilonGenerated = 1;
                break;
            }
            else {

                // For all the terminals in the first of this Non terminal set the ParsingTable entry
                // Note, no special treatment for epsilon as it will not be recieved from the input source code
                for(int j=0; j < TOTAL_GRAMMAR_TERMINALS; j++) {
                    if(fafl->FIRST[trav->TYPE.NON_TERMINAL][j] == 1)
                        pt->entries[lhsNonTerminal][j] = r->RULE_NO;
                }

                // Check if epsilon is generated by the first of this Non terminal, if not break, else continue
                if(fafl->FIRST[trav->TYPE.NON_TERMINAL][TK_EPS] == 0) {
                    epsilonGenerated = 0;
                    break;
                }
            }

            trav = trav->next;
        }

        // If epsilon is generated by the RHS string then we need to consider follow set of the LHS Non terminal
        if(epsilonGenerated) {
            for(int j=0; j < TOTAL_GRAMMAR_TERMINALS; j++) {
                if(fafl->FOLLOW[lhsNonTerminal][j] == 1)
                    pt->entries[lhsNonTerminal][j] = r->RULE_NO;
            }
        }
    }

}



// Appending at the tail of the list in O(1) using the tail pointer
void addToSymbolList(SymbolList* ls, Symbol* s) {
    Symbol* h = ls->HEAD_SYMBOL;
    // Case when the List is empty
    if(h == NULL) {
        ls->HEAD_SYMBOL = s;
        ls->TAIL_SYMBOL = s;
        ls->RULE_LENGTH = 1;
        return;
    }

    ls->TAIL_SYMBOL->next = s;
    ls->TAIL_SYMBOL = s;
    ls->RULE_LENGTH += 1;
}
// Extracts the grammar from GRAMMAR_FILE, return 1 on success, 0 on error
// Working rationale of the function
//  => Identify the LHS Non_terminal
//  => Keep making the Symbol List
//  => Extract the enum number of the LHS Non terminal.
// Now we are going to start using the functions we have defined earlier

Grammar* extractGrammar(){
    
}
createParseTable(FirstAndFollow F, table T){

}
parseInputSourceCode(char *testcaseFile, table T){

}
printParseTree(parseTree PT, char *outfile){

}