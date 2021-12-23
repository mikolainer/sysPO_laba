/*
 *   argv[1] - путь к файлу
 *   допустимые символы в слове перечисляются в строке whitelist (функция isNotAvailable)
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <cassert>
#define WRD_LEN 255

////////////////////////////
//// TABLE
////////////////////////////

char tName[5][14] = {"none", "const", "operator", "keyword", "identificator"};
typedef enum{
    none,
    konst,
    operatr,
    keywd,
    ident,
    start,
    end,
    neterm
}type;

typedef enum{
    ok,
    notAvlConst,
    syntax
}myerrno;

typedef struct literTableVal{
    type type;
    char leksema[WRD_LEN+1];
}litera;

litera table[UINT16_MAX];
int num_of_leksem = 0; // количество записанных лексем
void addWrd(char* wrd, int num, type its){
    table[num].type = its;
    strcpy(table[num].leksema, wrd);
}

////////////////////////////
//// MY STRUCTURES
////////////////////////////
//// using in sintaxer
template <typename T>
struct my_list_node{
    T val;
    my_list_node<T> *next_ptr = nullptr;
    my_list_node<T> *prev_ptr = nullptr;
};

template <typename T>
struct my_list{
    my_list_node<T> *beg = nullptr;
    my_list_node<T> *end = nullptr;
};

template <typename T>
void myList_insert(my_list_node<T> *node, my_list_node<T> *after){
    node->prev_ptr = after;
    node->next_ptr = after->next_ptr;
    after->next_ptr = node;
}

template <typename T>
void myList_delete(T *node){
    if ( node->next_ptr != nullptr )
        node->next_ptr->prev_ptr = node->prev_ptr;

    if ( node->prev_ptr != nullptr )
        node->prev_ptr->next_ptr = node->next_ptr;

    delete node;
}

template <typename T>
void myList_push(my_list_node<T> *node, my_list<T> *list){
    myList_insert(node, list->end);
    list->end = node;
}

template <typename T>
my_list_node<T> myList_pop(my_list<T> *list){
     my_list_node <T> node = *(list->end);
     list->end = list->end->prev_ptr;
     myList_delete(list->end->next_ptr);
     return node;
}


////////////////////////////
//// INPUT ANALYS FUNCS
////////////////////////////
//// using in lexer
int isNotAvailable(char ch){
    const char whiteList[] =
    "_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789();/:=";
    const char find[] = {ch, '\0'};
    if (strstr(whiteList, find) == NULL) return 1;
    return 0;
}
int isNotOperator(char ch){
    const char whiteList[] = "();/:=";
    const char find[] = {ch, '\0'};
    if (strstr(whiteList, find) == NULL) return 1;
    return 0;
}
int isNumber(char ch){
    const char whiteList[] = "1234567890";
    const char find[] = {ch, '\0'};
    if (strstr(whiteList, find) == NULL) return 0;
    return 1;
}

char keywords[10][4] = {"and", "AND", "or", "OR", "xor", "XOR", "not", "NOT"};
char operators[4][3] = {":=", "(", ")", ";"};
char startcomment[3] = "//";
int isKeywd(char* wrd){ // returns num of arrElem
    for (unsigned long long k=0; k<(sizeof(keywords)/sizeof(keywords[0])); k++)
    if (!strcmp(wrd, keywords[k])) return k+1;
    return 0;
}
int isOperator(char* wrd){ // returns num of arrElem
    for (unsigned long long k=0; k<(sizeof(operators)/sizeof(operators[0])); k++)
    if (!strcmp(wrd, operators[k])) return k+1;
    return 0;
}
int haveOperator(char* wrd){ // returns num of arrElem
    for (unsigned long long k=0; k<(sizeof(operators)/sizeof(operators[0])); k++)
    if (strstr(wrd, operators[k]) != NULL) return k+1;
    return 0;
}
int haveKomment(char* wrd){ // returns num of arrElem
    if (strstr(wrd, startcomment) != NULL) return 1;
    return 0;
}

////////////////////////////
//// MAIN FUNC
////////////////////////////
int main(int argc, char *argv[]){
    myerrno error = ok;
    system ("cls"); // очистка консоли
    if (argc == 1) printf("%s\n", "No path to input file!"); // уведомление об ошибке старта

    FILE* inp = fopen(argv[1], "r"); // открытие текста входной программы

//// START OF LEXER
    if (inp == NULL){ // обработка ошибки открытия файла с текстом входной программы
        printf("%s\n%s\n", "Input file was not open!", "press any key to quit");
        getchar();
        return 0;
    }else{ // чтение входной программы
        int strnum = 1;
        char inner;
        char str[WRD_LEN+1]; // считываемая последовательность символов
        int i = 0; // длинна считываемой последовательности символов
        int scan = 1; // флаг полезной части строки

        while (!error){
            inner = fgetc(inp);

            if (inner == '\0' || inner == '\n') {scan = 1; i = 0; strnum++;}

            if (scan){
                    if (isNotAvailable(inner)){
                    if (i) {int detected;
                        str[i] = '\0';
                        if (isKeywd(str)) addWrd(str, num_of_leksem, keywd);
                        else if (isOperator(str)) addWrd(str, num_of_leksem, operatr);
                        else if(detected = haveOperator(str)){
                            str[i+1 - strlen(operators[detected-1])] = '\0';
                            addWrd(str, num_of_leksem, ident); num_of_leksem++;
                            addWrd(operators[detected-1], num_of_leksem, operatr); num_of_leksem++;
                        }
                        else addWrd(str, num_of_leksem, ident);
                        num_of_leksem++;
                    }i = 0;
                    if (inner == EOF){
                        if (i) {break;}
                        else {num_of_leksem--; break;}
                    }
                }else{
                    str[i] = inner;
                    if(isNotOperator(inner)){
                        if (i == 0 && isNumber(inner)){
                            if (inner == '0' || inner == '1'){
                                str[i+1] = '\0';
                                addWrd(str, num_of_leksem, konst); num_of_leksem++;
                                i = 0;
                            }else {
                                error = notAvlConst;
                                printf("ERROR!! the constant in row %d is invalid\n", strnum);
                            }
                        }else i++;
                    }else{int detected;
                        str[i+1] = '\0';
                        if (isOperator(str)){
                        addWrd(str, num_of_leksem, operatr); num_of_leksem++;
                        i = 0;
                        }else if(!strncmp(str, startcomment, 2)) scan = 0;
                        else if((detected = haveOperator(str)) || haveKomment(str)){
                            if (detected){
                                str[i+1 - strlen(operators[detected-1])] = '\0';
                                if (isKeywd(str)) addWrd(str, num_of_leksem, keywd);
                                else if (isOperator(str)) addWrd(str, num_of_leksem, operatr);
                                else addWrd(str, num_of_leksem, ident);
                                num_of_leksem++;
                                addWrd(operators[detected-1], num_of_leksem, operatr); num_of_leksem++;
                            }else {scan = 0;
                                str[i+1 - strlen(strstr(str, startcomment))] = '\0';
                                int det;
                                if (isKeywd(str)) addWrd(str, num_of_leksem, keywd);
                                else if (isOperator(str)) addWrd(str, num_of_leksem, operatr);
                                else if(det = haveOperator(str)){
                                    str[i+1 - strlen(operators[det-1])] = '\0';
                                    addWrd(str, num_of_leksem, ident); num_of_leksem++;
                                    addWrd(operators[det-1], num_of_leksem, operatr); num_of_leksem++;
                                }else addWrd(str, num_of_leksem, ident);
                                num_of_leksem++;
                            }i = 0;
                        }else i++;
                    }
                }
            }
            if (inner == EOF && !scan) {num_of_leksem--; break;}
        }
    }

    if (!error){
        printf("%s\n%25s\n%s\n %15s%25s\n%s\n",
        "--------------------------------------------------",
        "LEKS TABLE",
        "--------------------------------------------------",
        "leksema", "| type ",
        "----------------------------------+---------------");
        for (int i=0; i<=num_of_leksem; i++){
            printf(" %-32s | %s \n", table[i].leksema, tName[table[i].type]);
        }
        printf("Leks analys SUCCESS!\n\n");
    }

    fclose(inp);
//// END OF LEXER

//// START OF SINTAXER

//// init
    my_list<litera> stack;
    stack.beg = stack.end = new my_list_node<litera>;
    stack.end->val.type = start;

    my_list<int> rules;
    rules.beg = rules.end =  new my_list_node<int>;
    rules.end->val = 0; // init by 0 rule as start point

    my_list<litera> input;
    input.beg = input.end = new my_list_node<litera>;

    for (int i=0; i<=num_of_leksem; i++){
        myList_push( new my_list_node<litera>, &input );
        input.end->val.type = table[i].type;
        strcpy( input.end->val.leksema, table[i].leksema );
    }

    myList_push (new my_list_node<litera>, &input );
    input.end->val.type = end;
    input.beg = input.beg->next_ptr;
    myList_delete(input.beg->prev_ptr);
    input.beg->prev_ptr = nullptr;

    //// end of init



    while ( true ){
        int rule = 0;

//////////////////////////
///       TABLE
//////////////////////////

//////////////////////////
///      PERENOS
//////////////////////////
        my_list_node<litera> *left = stack.end;
        while( left->val.type == neterm ){
            left = left->prev_ptr;
        }
        if ( left->val.type == start && input.beg->val.type == end ) {
            printf("syntactic analysis SUCCESS!\n");
            break;
        }

        if (//  !strcmp(stack.end->val.leksema, "")        //   stack.end->val.type == ident || stack.end->val.type == konst

                // [:=]
                ( !strcmp(left->val.leksema, ":=") && input.beg->val.type == keywd )           ||
                ( !strcmp(left->val.leksema, ":=") &&
                                (input.beg->val.type == ident || input.beg->val.type == konst) )    ||
                ( !strcmp(left->val.leksema, ":=") && !strcmp(input.beg->val.leksema, "(") )   ||
                ( !strcmp(left->val.leksema, ":=") && !strcmp(input.beg->val.leksema, ";") )   ||

                // [(]

                ( !strcmp(left->val.leksema, "(") && input.beg->val.type == keywd )            ||
                ( !strcmp(left->val.leksema, "(") &&
                                (input.beg->val.type == ident || input.beg->val.type == konst) )    ||
                ( !strcmp(left->val.leksema, "(") && !strcmp(input.beg->val.leksema, ")") )    ||
                ( !strcmp(left->val.leksema, "(") && !strcmp(input.beg->val.leksema, ")") )    ||

                // [and]
                ( !strcmp(left->val.leksema, "and") &&
                                (input.beg->val.type == ident || input.beg->val.type == konst) )    ||
                ( !strcmp(left->val.leksema, "and") && !strcmp(input.beg->val.leksema, "not") )||
                ( !strcmp(left->val.leksema, "and") && !strcmp(input.beg->val.leksema, "(") )  ||

                // [or]
                ( !strcmp(left->val.leksema, "or") &&
                                (input.beg->val.type == ident || input.beg->val.type == konst) )    ||
                ( !strcmp(left->val.leksema, "or") && !strcmp(input.beg->val.leksema, "not") ) ||
                ( !strcmp(left->val.leksema, "or") && !strcmp(input.beg->val.leksema, "(") )   ||
                ( !strcmp(left->val.leksema, "or") && !strcmp(input.beg->val.leksema, "and") ) ||

                // [xor]
                ( !strcmp(left->val.leksema, "xor") &&
                                (input.beg->val.type == ident || input.beg->val.type == konst) )    ||
                ( !strcmp(left->val.leksema, "xor") && !strcmp(input.beg->val.leksema, "not") )||
                ( !strcmp(left->val.leksema, "xor") && !strcmp(input.beg->val.leksema, "(") )  ||
                ( !strcmp(left->val.leksema, "xor") && !strcmp(input.beg->val.leksema, "and") )||

                // [not]
                ( !strcmp(left->val.leksema, "not") && !strcmp(input.beg->val.leksema, "(") )  ||

                // [a]
                ( ( left->val.type == ident ) && !strcmp(input.beg->val.leksema, ":=") )       ||
                // [start]
                ( ( left->val.type == start ) &&
                                (input.beg->val.type == ident || input.beg->val.type == konst) )
        )
        {// there are perenos action
            myList_push(new my_list_node<litera>, &stack);
            stack.end->val = input.beg->val;
            input.beg = input.beg->next_ptr;
            myList_delete(input.beg->prev_ptr);
            continue;
        }else
//////////////////////////
///      SVERTKA
//////////////////////////
            if (
                // [)]
                ( !strcmp(left->val.leksema, ")") && !strcmp(input.beg->val.leksema, ")") )    ||
                ( !strcmp(left->val.leksema, ")") && !strcmp(input.beg->val.leksema, ";") )    ||
                ( !strcmp(left->val.leksema, ")") && !strcmp(input.beg->val.leksema, "and") )  ||
                ( !strcmp(left->val.leksema, ")") && !strcmp(input.beg->val.leksema, "or") )   ||
                ( !strcmp(left->val.leksema, ")") && !strcmp(input.beg->val.leksema, "xor") )  ||

                // [;]
                ( !strcmp(left->val.leksema, ")") && input.beg->val.type == end )    ||

                // [and]
                ( !strcmp(left->val.leksema, "and") && !strcmp(input.beg->val.leksema, ")") )    ||
                ( !strcmp(left->val.leksema, "and") && !strcmp(input.beg->val.leksema, ";") )    ||
                ( !strcmp(left->val.leksema, "and") && !strcmp(input.beg->val.leksema, "and") )  ||
                ( !strcmp(left->val.leksema, "and") && !strcmp(input.beg->val.leksema, "or") )   ||
                ( !strcmp(left->val.leksema, "and") && !strcmp(input.beg->val.leksema, "xor") )  ||

                // [a]
                ( (left->val.type == ident || stack.end->val.type == ident) &&
                                                            !strcmp(input.beg->val.leksema, ")") )    ||
                ( (left->val.type == ident || stack.end->val.type == ident) &&
                                                            !strcmp(input.beg->val.leksema, ";") )    ||
                ( (left->val.type == ident || stack.end->val.type == ident) &&
                                                            !strcmp(input.beg->val.leksema, "and") )  ||
                ( (left->val.type == ident || stack.end->val.type == ident) &&
                                                            !strcmp(input.beg->val.leksema, "or") )   ||
                ( (left->val.type == ident || stack.end->val.type == ident) &&
                                                            !strcmp(input.beg->val.leksema, "xor") )  ||

                // [or]
                ( !strcmp(left->val.leksema, "or") && !strcmp(input.beg->val.leksema, ")") )    ||
                ( !strcmp(left->val.leksema, "or") && !strcmp(input.beg->val.leksema, ";") )    ||
                ( !strcmp(left->val.leksema, "or") && !strcmp(input.beg->val.leksema, "or") )   ||
                ( !strcmp(left->val.leksema, "or") && !strcmp(input.beg->val.leksema, "xor") )  ||

                // [xor
                ( !strcmp(left->val.leksema, "xor") && !strcmp(input.beg->val.leksema, ")") )   ||
                ( !strcmp(left->val.leksema, "xor") && !strcmp(input.beg->val.leksema, ";") )   ||
                ( !strcmp(left->val.leksema, "xor") && !strcmp(input.beg->val.leksema, "or") )  ||
                ( !strcmp(left->val.leksema, "xor") && !strcmp(input.beg->val.leksema, "xor") ) ||

                ( !strcmp(left->val.leksema, ";") && input.beg->val.type == end )
            )
            {// there are svertka action
            //// S
            if ( !strcmp(stack.end->val.leksema, ";")                       &&
                 stack.end->prev_ptr->val.type == neterm                    &&
                 !strcmp(stack.end->prev_ptr->prev_ptr->val.leksema, ":=")  &&
                 stack.end->prev_ptr->prev_ptr->prev_ptr->val.type == ident
                )
            {
                rule = 1;
                myList_pop(&stack);
                myList_pop(&stack);
                myList_pop(&stack);
                myList_pop(&stack);

                myList_push(new my_list_node<litera>, &stack);
                stack.end->val.type = neterm;
                strcpy( stack.end->val.leksema, "S" );
            }else

            //// ident \ const
            if ( stack.end->val.type == konst || stack.end->val.type == ident )
            {
                    rule = 9;
                    myList_pop(&stack);
                    myList_push(new my_list_node<litera>, &stack);
                    stack.end->val.type = neterm;
                    strcpy( stack.end->val.leksema, "E" );
            }else

            //// keywords
            if ( stack.end->val.type == neterm                  &&
                 !strcmp(stack.end->prev_ptr->val.leksema, "or") &&
                 stack.end->prev_ptr->prev_ptr->val.type == neterm
            ){
                    rule = 2;
                    myList_pop(&stack);
                    myList_pop(&stack);
                    myList_pop(&stack);
                    myList_push(new my_list_node<litera>, &stack);
                    stack.end->val.type = neterm;
                    strcpy( stack.end->val.leksema, "F" );
            }else

            if ( stack.end->val.type == neterm                  &&
                 !strcmp(stack.end->prev_ptr->val.leksema, "xor") &&
                 stack.end->prev_ptr->prev_ptr->val.type == neterm
            ){
                    rule = 3;
                    myList_pop(&stack);
                    myList_pop(&stack);
                    myList_pop(&stack);
                    myList_push(new my_list_node<litera>, &stack);
                    stack.end->val.type = neterm;
                    strcpy( stack.end->val.leksema, "F" );
            }else
            if ( stack.end->val.type == neterm                  &&
                 !strcmp(stack.end->prev_ptr->val.leksema, "and") &&
                 stack.end->prev_ptr->prev_ptr->val.type == neterm
            ){
                    rule = 5;
                    myList_pop(&stack);
                    myList_pop(&stack);
                    myList_pop(&stack);
                    myList_push(new my_list_node<litera>, &stack);
                    stack.end->val.type = neterm;
                    strcpy( stack.end->val.leksema, "T" );
            }else

            //// skobki
            if ( !strcmp(stack.end->val.leksema, ")")       &&
                 stack.end->prev_ptr->val.type == neterm    &&
                 !strcmp(stack.end->prev_ptr->prev_ptr->val.leksema, "(")
                ){
                    rule = 7;
                    myList_pop(&stack);
                    myList_pop(&stack);
                    myList_pop(&stack);
                    if ( !strcmp(stack.end->val.leksema, "not") ){
                        rule = 8;
                        myList_pop(&stack);
                    }
                    myList_push(new my_list_node<litera>, &stack);
                    stack.end->val.type = neterm;
                    strcpy( stack.end->val.leksema, "E" );
            }else

            //// neterm to neterm
            if (stack.end->val.type == neterm)
            {
                if ( !strcmp(stack.end->val.leksema, "T") ){
                        rule = 4;
                        myList_pop(&stack);
                        myList_push(new my_list_node<litera>, &stack);
                        stack.end->val.type = neterm;
                        strcpy( stack.end->val.leksema, "F" );
                }else

                if ( !strcmp(stack.end->val.leksema, "E") ){
                        rule = 6;
                        myList_pop(&stack);
                        myList_push(new my_list_node<litera>, &stack);
                        stack.end->val.type = neterm;
                        strcpy( stack.end->val.leksema, "T" );
                }
            }else

            //// error
            {
                printf("Sintax error!\n");
                error = syntax;
                break;
            }

            myList_push( new my_list_node<int>, &rules );
            rules.end->val = rule;
        }
    }
//////////////////////////
///      END OF TABLE
//////////////////////////
    if (error == ok){
        printf("RULES: ");
        my_list_node<int> *iter = rules.beg->next_ptr;
        for (int i=0; ;++i){
            printf("%d, ", iter->val);
            if (i==10){
                printf("\n       ");
                i=0;
            }
            if ( iter->next_ptr )
                iter = iter->next_ptr;
            else
                break;
        }
    }



//// END OF SINTAXER
    return 0;
}
