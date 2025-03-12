/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "phase_2/tuc_transpiler.y"


    #include <stdio.h>
    #include <string.h>
    #include "cgen.h"
    #include <ctype.h>
    #include <stdbool.h>
    #include <stdlib.h>
    #include <regex.h>


    #define MAX_FUNCTIONS 100
    #define MAX_LINE_LENGTH 1024
    #define MAX_LINES 1000
    #define BUFFER_SIZE 1024  
    #define MAX_FUNCTIONS 100
    #define MAX_NAME_LENGTH 50

    extern int yydebug;
    extern int parse_error;
    extern int yylex(void); // use the lexer, lex.yy.cc has already been generated
    extern int line_num;
    extern void add_macro(const char *id, const char *value);

    void print_evaluation(char* result);
    int match_return_variable_type(char* result);
    bool is_integer(const char* str);

    int used_lambdalib_function = 0;
    char *included_functionnames[MAX_FUNCTIONS];
    int num_of_included_functions = 0;

    char *lines[MAX_LINES];
    int current_line_index = 0;

    int for_has_step = 0;
    char* function_name_temp;
    char* variable_name_temp;
    int first_variable = 1;

//==========================================================================================================================

void store_line(const char *text) {
  
    if (current_line_index < MAX_LINES) {

        char temp[BUFFER_SIZE];
        strncpy(temp, text, BUFFER_SIZE - 1);
        temp[BUFFER_SIZE - 1] = '\0'; 

        char *newline_pos = strchr(temp, '\n');
        if (newline_pos) {
            *newline_pos = '\0';
        }

        lines[current_line_index] = strdup(temp);
        current_line_index++;
    }
}

  void free_lines() {
      for (int i = 0; i < current_line_index; i++) {
          free(lines[i]);
      }
  }

 int main_has_been_defined = 0;

//==========================================================================================================================    

int func_needs_return_type = 0; 
int body_has_return_type = 0;

int num_of_args_in_function = 0; 
int num_of_args_in_function_in_func_call = 0;

typedef struct {
    const char* name;
    int has_arguments;

} DefinedFunction;

DefinedFunction *functions = NULL;
size_t num_functions = 0;

//=========================================================================================================================

void add_function(const char *name, int has_arguments) {

    functions = realloc(functions, (num_functions + 1) * sizeof(DefinedFunction));
    if (!functions) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    DefinedFunction *new_func = &functions[num_functions];
    new_func->name = strdup(name);  
    new_func->has_arguments = has_arguments;
    num_functions++;
}

//=========================================================================================================================

int find_included_function(const char *functionname) {

    for (int i = 0; i < num_of_included_functions; i++) {
        if (strcmp(included_functionnames[i], functionname) == 0) {
            return 1; 
        }

    }
    return 0; 
}

//=========================================================================================================================
//=========================================================================================================================
// Include Functions in lambdalib.h

int count_args(const char *line) {
    int count = 0;
    const char *p = strchr(line, '(');
    
    if (p) {
        p++;  
        while (*p == ' ' || *p == '\t') {p++;}
        if (strstr(p, "...")) {
            return 99; 
        }
        if (*p == ')') { return 0; }
        while (*p != ')' && *p != '\0') {
            if (*p == ',') {
                count++;
            }
            p++;
        } 
        count++;
    }
    return count;
}

//=========================================================================================================================

int is_function_definition(const char *line, char *func_name) {
    regex_t regex;
    regmatch_t matches[2];
    const char *pattern = "^(\\s*(void|int|float|char|double|long|short|unsigned|signed|bool|_Bool|\\w+\\*?\\s+)+[a-zA-Z_][a-zA-Z0-9_]*\\s*\\([^)]*\\)\\s*\\{)|^\\s*#define\\s+[a-zA-Z_][a-zA-Z0-9_]*\\s*\\([^)]*\\)\\s*[^\\{]*$";
    
    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
        return 0;
    }
    
    int result = regexec(&regex, line, 1, matches, 0); // If a match is found, matches[0] contains the start and end positions.
    regfree(&regex);
    
    if (result == 0) { // Extract the Function Name
        int start = matches[0].rm_so;
        int end = matches[0].rm_eo - 1; // Ignore '('
        strncpy(func_name, line + start, end - start);
        func_name[end - start] = '\0';
        return 1;
    }
    return 0;
}


//=========================================================================================================================
//=========================================================================================================================
//=========================================================================================================================

void extractFunctionName(const char *func_name, char *output) {

    const char *start = func_name;
    while (!isspace(*start) && *start != '\0') {
        start++; 
    }
    while (isspace(*start)) {
        start++; 
    }
    
    int i = 0;
    while (*start != '(' && *start != '\0' && i < 99) {
        output[i++] = *start++;
    }
    output[i] = '\0';
}

//=========================================================================================================================

void extractFunctionNames(const char *code, char function_names[MAX_FUNCTIONS][MAX_NAME_LENGTH], int *count) {

    const char *ptr = code;
    *count = 0;

    while (*ptr != '\0') {  
        ptr = strstr(ptr, "double ");
        if (ptr != NULL) {
            ptr += 6;
        } else {
            ptr = strstr(ptr, "void ");
            if (ptr != NULL) {
                ptr += 4; 
            }
        }
        while (*ptr && isspace(*ptr)) {
            ptr++;
        }
        char name[MAX_NAME_LENGTH] = {0};
        int i = 0;

        while (*ptr && (isalnum(*ptr) || *ptr == '_')) { 
            name[i++] = *ptr++;
            if (i >= MAX_NAME_LENGTH - 1) break;  
        }
        name[i] = '\0';
        if (i > 0 && *count < MAX_FUNCTIONS) {
            strcpy(function_names[*count], name);
            (*count)++;
        }
    }
}

//=========================================================================================================================

void include_functions()  {

    FILE *file = fopen("lambdalib.h", "r");
    if (!file) {
        perror("Error opening file");
        return;
    }
    char line[MAX_LINES];

    while (fgets(line, MAX_LINES, file)) {
        char func_name[100];
        char output[100];

        if (is_function_definition(line, func_name)) {
            extractFunctionName(func_name, output);
            strcpy(func_name, output);
            
            int num_args = count_args(line);
            add_function(func_name, num_args);
            included_functionnames[num_of_included_functions] = strdup(func_name);
            num_of_included_functions++;
        }
    }
    fclose(file);
    return;
}

//=========================================================================================================================

const DefinedFunction* find_function(const char* name) {

    for (size_t i = 0; i < num_functions; i++) {
        if (strcmp(functions[i].name, name) == 0) {
            return &functions[i];
        }
    }
    return NULL; 
}

void free_functions() {
    for (size_t i = 0; i < num_functions; i++) {
        free((void*)functions[i].name);
    }
    free(functions);
}

//=========================================================================================================================
//=========================================================================================================================
//=========================================================================================================================
// comp_upper:

int inside_comp= 0;

#define MAX_FUNCTIONS 100

typedef struct FunctionNode {
    char *function_comp;
    char *function_name;
    struct FunctionNode *next;
} FunctionNode;

typedef struct VariableNode {
    char *variable_name;
    struct VariableNode *next;
} VariableNode;

typedef struct CompFunctionMap {

    char *comp_name;
    FunctionNode *functions;  
    VariableNode *variables;
    struct CompFunctionMap *next;

} CompFunctionMap;

#define HASH_TABLE_SIZE 100

CompFunctionMap *compFunctionTable[HASH_TABLE_SIZE] = { NULL };

//=========================================================================================================================

unsigned int hash(const char *str) {
    unsigned int hash = 0;
    while (*str) hash = (hash * 31) + *str++;
    return hash % HASH_TABLE_SIZE;
}

void createNewComp(const char *comp_name) {
    unsigned int index = hash(comp_name);
    CompFunctionMap *entry = compFunctionTable[index];
    entry = (CompFunctionMap *)malloc(sizeof(CompFunctionMap));
    entry->comp_name = strdup(comp_name);
    entry->functions = NULL;  
    entry->variables = NULL; 
    entry->next = compFunctionTable[index];
    compFunctionTable[index] = entry;
}

int is_comp_type(const char *comp_name) {
    unsigned int index = hash(comp_name);
    CompFunctionMap *entry = compFunctionTable[index];
    while (entry) {
        if (strcmp(entry->comp_name, comp_name) == 0) {
            return 1;
        }
        entry = entry->next;
    }
    return 0;
}
//=========================================================================================================================
// variable_comp_upper:

void add_comp_variable(const char *comp_name, const char *variable_name) {
    unsigned int index = hash(comp_name);
    CompFunctionMap *comp = compFunctionTable[index];
    VariableNode *var = comp->variables;

    if(first_variable == 0){
        while (var) {
            if (strcmp(var->variable_name, variable_name) == 0) {
                return; // Variable already exists
            }
            var = var->next;
        }
        first_variable = 0;
    }
    var = (VariableNode *)malloc(sizeof(VariableNode));
    var->variable_name = strdup(variable_name);
    var->next = comp->variables;
    comp->variables = var;
}

void add_comp_variables(const char* comp_name, const char *input) {
    char buffer[256];  
    strncpy(buffer, input, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    char *token = strtok(buffer, ",");
    while (token != NULL) {
        while (*token == ' ') token++;
        add_comp_variable(comp_name,token);
        token = strtok(NULL, ",");
    }
}

int is_comp_variable(const char *comp_name, const char *variable_name) {

    unsigned int index = hash(comp_name);
    CompFunctionMap *comp = compFunctionTable[index];

    VariableNode *var = comp->variables;
    
    while (var) {
        // printf("%s\n", var->variable_name);
        if (strcmp(var->variable_name, variable_name) == 0) {
            return 1; // Found
        }
        var = var->next;
    }
    return 0; // Not found
}
//=========================================================================================================================
// function_comp_upper:

void addFunctionToComp(const char *comp_name, const char *function_follow_comp, const char *func_name) {

    unsigned int index = hash(comp_name);

    CompFunctionMap *entry = compFunctionTable[index];
    while (entry) {
        if (strcmp(entry->comp_name, comp_name) == 0) {
            FunctionNode *newNode = (FunctionNode *)malloc(sizeof(FunctionNode));
            newNode->function_comp = strdup(function_follow_comp);
            newNode->function_name = strdup(func_name); 
            newNode->next = entry->functions;
            entry->functions = newNode;
            return;
        }
        entry = entry->next;
    }
}
//=========================================================================================================================

int is_comp_function(const char *comp_name, const char *function_name) {
    unsigned int index = hash(comp_name);
    CompFunctionMap *entry = compFunctionTable[index];

    while (entry) {
        if (strcmp(entry->comp_name, comp_name) == 0) {
            FunctionNode *function_current = entry->functions;
            while (function_current) {
                if (strcmp(function_current->function_name, function_name) == 0) {
                    return 1; // Function found
                }
                function_current = function_current->next;
            }
            return 0; 
        }
        entry = entry->next;
    }

    return 0;
}
//=========================================================================================================================

char *getCompFunctionsAsString(const char *comp_name) {
    unsigned int index = hash(comp_name);
    CompFunctionMap *entry = compFunctionTable[index];

    while (entry) {
        if (strcmp(entry->comp_name, comp_name) == 0) {
            size_t totalSize = 1;
            FunctionNode *fn = entry->functions;
            while (fn) {
                totalSize += strlen(fn->function_comp) + 2; 
                fn = fn->next;
            }
            char *result = (char *)malloc(totalSize);
            if (!result) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            result[0] = '\0'; 

            fn = entry->functions;
            while (fn) {
                strcat(result, fn->function_comp);
                strcat(result, "\n"); 
                fn = fn->next;
            }
            return result; 
        }
        entry = entry->next;
    }

    return strdup("");
}

//=========================================================================================================================

char *getCompFunctionNamesAsString(const char *comp_name) {
    unsigned int index = hash(comp_name);
    CompFunctionMap *entry = compFunctionTable[index];

    while (entry) {
        if (strcmp(entry->comp_name, comp_name) == 0) {
            size_t totalSize = 1; 
            FunctionNode *fn = entry->functions;
            while (fn) {
                totalSize += strlen(fn->function_name) * 2 + 6; 
                fn = fn->next;
            }
            char *result = (char *)malloc(totalSize);
            if (!result) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            result[0] = '\0';

            fn = entry->functions;
            while (fn) {
                strcat(result, ".");
                strcat(result, fn->function_name); 
                strcat(result, " = ");
                strcat(result, fn->function_name);
                strcat(result, ", ");
                fn = fn->next;
            }

            result[strlen(result) - 2] = '\0';

            return result;
        }
        entry = entry->next;
    }
    return strdup("");
}

//==========================================================================================================================
//==========================================================================================================================
// comp_type_variable: 

typedef struct compTypeVariable {
    char *variable_name;
    char *comp_name;
    struct compTypeVariable *next;
} compTypeVariable;

compTypeVariable *head = NULL;

void create_compTypeVariables(const char *comp_name, const char *variable_names) {
    char *var_names_copy = strdup(variable_names); // Create a mutable copy of the string
    if (!var_names_copy) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    char *token = strtok(var_names_copy, ",");
    while (token) {
        while (*token == ' ') token++;
        char *end = token + strlen(token) - 1;
        while (end > token && (*end == ' ' || *end == '\n')) end--;
        *(end + 1) = '\0';
        compTypeVariable *new_entry = (compTypeVariable *)malloc(sizeof(compTypeVariable));
        if (!new_entry) {
            perror("Failed to allocate memory");
            exit(EXIT_FAILURE);
        }
        new_entry->variable_name = strdup(token); 
        new_entry->comp_name = strdup(comp_name); 
        new_entry->next = head;
        head = new_entry;
        token = strtok(NULL, ","); 
    }
    free(var_names_copy); 
}

char *find_comp_type(const char *variable_name) {
    compTypeVariable *current = head;

    while (current) {
        if (strcmp(current->variable_name, variable_name) == 0) {
            return current->comp_name;
        }
        current = current->next;
    }
    return NULL; // Not found
}

//==========================================================================================================================
//==========================================================================================================================
//==========================================================================================================================
//==========================================================================================================================
// Other Stuff:

int indentation_level = 0;

int assignment_made = 0;

char *temp1, *temp2, *temp3, *temp4, *temp5, *temp6, *temp7, *temp8, *temp9, *temp10, *temp11, *temp12;
char *indented_stmts, *indented_stmts_1, *indented_stmts_2, *indented_stmts_3, *indented_stmts_4, *indented_stmts_5;
char* comp_name;

char* indent_statements(const char* statements, int level) {
    int rrrr = 0;
    sstream S;
    ssopen(&S);
    if(level >= 1){
      level = 1;
    } 
    const char* current = statements;
    int is_new_line = 1;  

    while (*current != '\0') {
        if (is_new_line) {
            
            for (int i = 0; i < level; i++) {
                fprintf(S.stream, "  "); 
                rrrr++;
            }
            is_new_line = 0;  
        }
        fputc(*current, S.stream);
        if (*current == '\n') {
            is_new_line = 1;  
        }
        current++;
    }
    char* result = ssvalue(&S);
    ssclose(&S);
    return result;
}

//==========================================================================================================================
//==========================================================================================================================

  typedef struct {
      const char *original;
      const char *transformed;
  } TypeMapping;

  TypeMapping typeMappings[] = {
      {"KW_SCALAR", "double"},
      {"KW_INTEGER", "int"},
      {"KW_STR", "char*"},
      {"KW_BOOLEAN", "int"},
  };

//==========================================================================================================================

  char* transform_type(const char *identifier) {
      for (int i = 0; i < sizeof(typeMappings) / sizeof(typeMappings[0]); i++) {
          if (strcmp(identifier, typeMappings[i].original) == 0) {
              return (char*)typeMappings[i].transformed;
          }
      }
      return NULL; 
  }

//==========================================================================================================================

  char* handle_variable_type(const char *inputType) {
      return transform_type(inputType);
  }

  int is_a_loop = 0;

//==========================================================================================================================
// Strings:

char* replace_identifier_in_expr(const char* expr, const char* oldVar, const char* newVar) {
    char* result = strdup(expr); 
    char* pos = strstr(result, oldVar); 
    
    if (pos) {
        size_t newLen = strlen(newVar);
        size_t oldLen = strlen(oldVar);
        char* newExpr = (char*)malloc(strlen(result) + newLen - oldLen + 1);
        
        strncpy(newExpr, result, pos - result); 
        strcpy(newExpr + (pos - result), newVar); 
        strcat(newExpr, pos + oldLen); 
        
        free(result);
        return newExpr;
    }
    
    return result;
}

//==========================================================================================================================

int contains_substring(const char *s1, const char *s2) {
    if (strstr(s1, s2) != NULL) {
        return 1; 
    }
    return 0;  
}


//==========================================================================================================================
//==========================================================================================================================
//==========================================================================================================================


#line 738 "generated/tuc_transpiler.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "tuc_transpiler.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_IDENTIFIER = 3,                 /* IDENTIFIER  */
  YYSYMBOL_POSINT = 4,                     /* POSINT  */
  YYSYMBOL_STRING = 5,                     /* STRING  */
  YYSYMBOL_DECIMAL = 6,                    /* DECIMAL  */
  YYSYMBOL_REAL = 7,                       /* REAL  */
  YYSYMBOL_KW_IF = 8,                      /* KW_IF  */
  YYSYMBOL_KW_ELSE = 9,                    /* KW_ELSE  */
  YYSYMBOL_KW_ENDIF = 10,                  /* KW_ENDIF  */
  YYSYMBOL_KW_IN = 11,                     /* KW_IN  */
  YYSYMBOL_KW_OF = 12,                     /* KW_OF  */
  YYSYMBOL_KW_WHILE = 13,                  /* KW_WHILE  */
  YYSYMBOL_KW_ENDWHILE = 14,               /* KW_ENDWHILE  */
  YYSYMBOL_KW_FOR = 15,                    /* KW_FOR  */
  YYSYMBOL_KW_ENDFOR = 16,                 /* KW_ENDFOR  */
  YYSYMBOL_KW_BREAK = 17,                  /* KW_BREAK  */
  YYSYMBOL_KW_CONTINUE = 18,               /* KW_CONTINUE  */
  YYSYMBOL_KW_TRUE = 19,                   /* KW_TRUE  */
  YYSYMBOL_KW_FALSE = 20,                  /* KW_FALSE  */
  YYSYMBOL_KW_VAR = 21,                    /* KW_VAR  */
  YYSYMBOL_KW_INT = 22,                    /* KW_INT  */
  YYSYMBOL_KW_REAL = 23,                   /* KW_REAL  */
  YYSYMBOL_KW_FUNC = 24,                   /* KW_FUNC  */
  YYSYMBOL_KW_BEGIN = 25,                  /* KW_BEGIN  */
  YYSYMBOL_KW_DEF = 26,                    /* KW_DEF  */
  YYSYMBOL_KW_ENDDEF = 27,                 /* KW_ENDDEF  */
  YYSYMBOL_KW_COMP = 28,                   /* KW_COMP  */
  YYSYMBOL_KW_ENDCOMP = 29,                /* KW_ENDCOMP  */
  YYSYMBOL_KW_MACRO = 30,                  /* KW_MACRO  */
  YYSYMBOL_KW_RETURN = 31,                 /* KW_RETURN  */
  YYSYMBOL_KW_MAIN = 32,                   /* KW_MAIN  */
  YYSYMBOL_KW_FUNC_RETURN = 33,            /* KW_FUNC_RETURN  */
  YYSYMBOL_KW_INTEGER = 34,                /* KW_INTEGER  */
  YYSYMBOL_KW_SCALAR = 35,                 /* KW_SCALAR  */
  YYSYMBOL_KW_STR = 36,                    /* KW_STR  */
  YYSYMBOL_KW_CONST = 37,                  /* KW_CONST  */
  YYSYMBOL_KW_BOOLEAN = 38,                /* KW_BOOLEAN  */
  YYSYMBOL_EXPONENTIATION_OP = 39,         /* EXPONENTIATION_OP  */
  YYSYMBOL_CALC_OP = 40,                   /* CALC_OP  */
  YYSYMBOL_UNARY_OP = 41,                  /* UNARY_OP  */
  YYSYMBOL_RELATIONAL_OP = 42,             /* RELATIONAL_OP  */
  YYSYMBOL_LOGICAL_NOT_OP = 43,            /* LOGICAL_NOT_OP  */
  YYSYMBOL_LOGICAL_AND_OR_OP = 44,         /* LOGICAL_AND_OR_OP  */
  YYSYMBOL_ASSIGNMENT_OP = 45,             /* ASSIGNMENT_OP  */
  YYSYMBOL_KW_PLUS = 46,                   /* KW_PLUS  */
  YYSYMBOL_KW_MINUS = 47,                  /* KW_MINUS  */
  YYSYMBOL_KW_MUL = 48,                    /* KW_MUL  */
  YYSYMBOL_KW_DIV = 49,                    /* KW_DIV  */
  YYSYMBOL_KW_MOD = 50,                    /* KW_MOD  */
  YYSYMBOL_KW_LESSEQUAL = 51,              /* KW_LESSEQUAL  */
  YYSYMBOL_KW_GREATEREQUAL = 52,           /* KW_GREATEREQUAL  */
  YYSYMBOL_KW_LESSTHAN = 53,               /* KW_LESSTHAN  */
  YYSYMBOL_KW_GREATHAN = 54,               /* KW_GREATHAN  */
  YYSYMBOL_KW_INEQ = 55,                   /* KW_INEQ  */
  YYSYMBOL_KW_AND = 56,                    /* KW_AND  */
  YYSYMBOL_KW_OR = 57,                     /* KW_OR  */
  YYSYMBOL_KW_NOT = 58,                    /* KW_NOT  */
  YYSYMBOL_KW_EQUAL = 59,                  /* KW_EQUAL  */
  YYSYMBOL_KW_EXPONENT = 60,               /* KW_EXPONENT  */
  YYSYMBOL_61_ = 61,                       /* ';'  */
  YYSYMBOL_62_ = 62,                       /* ')'  */
  YYSYMBOL_63_ = 63,                       /* ']'  */
  YYSYMBOL_64_ = 64,                       /* '('  */
  YYSYMBOL_65_ = 65,                       /* '['  */
  YYSYMBOL_66_ = 66,                       /* '#'  */
  YYSYMBOL_67_ = 67,                       /* ':'  */
  YYSYMBOL_68_ = 68,                       /* ','  */
  YYSYMBOL_69_ = 69,                       /* '.'  */
  YYSYMBOL_YYACCEPT = 70,                  /* $accept  */
  YYSYMBOL_program = 71,                   /* program  */
  YYSYMBOL_input = 72,                     /* input  */
  YYSYMBOL_macro = 73,                     /* macro  */
  YYSYMBOL_commands = 74,                  /* commands  */
  YYSYMBOL_75_1 = 75,                      /* $@1  */
  YYSYMBOL_76_2 = 76,                      /* $@2  */
  YYSYMBOL_77_3 = 77,                      /* $@3  */
  YYSYMBOL_78_4 = 78,                      /* $@4  */
  YYSYMBOL_79_5 = 79,                      /* $@5  */
  YYSYMBOL_80_6 = 80,                      /* $@6  */
  YYSYMBOL_else_statement = 81,            /* else_statement  */
  YYSYMBOL_82_7 = 82,                      /* $@7  */
  YYSYMBOL_step = 83,                      /* step  */
  YYSYMBOL_stmts = 84,                     /* stmts  */
  YYSYMBOL_loop_thing = 85,                /* loop_thing  */
  YYSYMBOL_complicated_types = 86,         /* complicated_types  */
  YYSYMBOL_87_8 = 87,                      /* $@8  */
  YYSYMBOL_declaration_comp = 88,          /* declaration_comp  */
  YYSYMBOL_variable_definition_comp = 89,  /* variable_definition_comp  */
  YYSYMBOL_variable_list_comp = 90,        /* variable_list_comp  */
  YYSYMBOL_function_definition_section = 91, /* function_definition_section  */
  YYSYMBOL_function_definition_comp = 92,  /* function_definition_comp  */
  YYSYMBOL_return_type_comp = 93,          /* return_type_comp  */
  YYSYMBOL_func_call = 94,                 /* func_call  */
  YYSYMBOL_func_args = 95,                 /* func_args  */
  YYSYMBOL_expr = 96,                      /* expr  */
  YYSYMBOL_use_of_comp_variable = 97,      /* use_of_comp_variable  */
  YYSYMBOL_use_comp_variable_with_period = 98, /* use_comp_variable_with_period  */
  YYSYMBOL_use_comp_variable = 99,         /* use_comp_variable  */
  YYSYMBOL_arithmetic_expr_non_empty = 100, /* arithmetic_expr_non_empty  */
  YYSYMBOL_function_definition = 101,      /* function_definition  */
  YYSYMBOL_102_9 = 102,                    /* $@9  */
  YYSYMBOL_103_10 = 103,                   /* $@10  */
  YYSYMBOL_104_11 = 104,                   /* $@11  */
  YYSYMBOL_105_12 = 105,                   /* $@12  */
  YYSYMBOL_arguments = 106,                /* arguments  */
  YYSYMBOL_parameter_definition = 107,     /* parameter_definition  */
  YYSYMBOL_return_type = 108,              /* return_type  */
  YYSYMBOL_body = 109,                     /* body  */
  YYSYMBOL_variable_definition = 110,      /* variable_definition  */
  YYSYMBOL_VALUE = 111,                    /* VALUE  */
  YYSYMBOL_KW_VARIABLE_TYPE = 112,         /* KW_VARIABLE_TYPE  */
  YYSYMBOL_NUMBERS = 113,                  /* NUMBERS  */
  YYSYMBOL_variable_list = 114             /* variable_list  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   380

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  70
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  45
/* YYNRULES -- Number of rules.  */
#define YYNRULES  123
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  273

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   315


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,    66,     2,     2,     2,     2,
      64,    62,     2,     2,    68,     2,    69,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    67,    61,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    65,     2,    63,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   736,   736,   739,   741,   742,   743,   744,   745,   746,
     753,   760,   773,   775,   773,   785,   790,   784,   803,   813,
     802,   826,   840,   863,   864,   864,   873,   874,   882,   883,
     890,   891,   896,   899,   899,   908,   908,   922,   923,   927,
     929,   931,   933,   938,   939,   947,   948,   952,   970,   971,
     979,   997,   998,   999,  1000,  1001,  1009,  1010,  1011,  1012,
    1013,  1014,  1015,  1016,  1017,  1018,  1019,  1020,  1021,  1030,
    1039,  1048,  1057,  1068,  1079,  1090,  1104,  1105,  1109,  1114,
    1116,  1123,  1124,  1125,  1126,  1127,  1128,  1129,  1130,  1131,
    1132,  1133,  1142,  1147,  1141,  1166,  1173,  1165,  1193,  1194,
    1195,  1199,  1200,  1201,  1205,  1206,  1210,  1211,  1219,  1226,
    1227,  1228,  1232,  1233,  1234,  1238,  1239,  1240,  1241,  1242,
    1253,  1254,  1258,  1259
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "IDENTIFIER", "POSINT",
  "STRING", "DECIMAL", "REAL", "KW_IF", "KW_ELSE", "KW_ENDIF", "KW_IN",
  "KW_OF", "KW_WHILE", "KW_ENDWHILE", "KW_FOR", "KW_ENDFOR", "KW_BREAK",
  "KW_CONTINUE", "KW_TRUE", "KW_FALSE", "KW_VAR", "KW_INT", "KW_REAL",
  "KW_FUNC", "KW_BEGIN", "KW_DEF", "KW_ENDDEF", "KW_COMP", "KW_ENDCOMP",
  "KW_MACRO", "KW_RETURN", "KW_MAIN", "KW_FUNC_RETURN", "KW_INTEGER",
  "KW_SCALAR", "KW_STR", "KW_CONST", "KW_BOOLEAN", "EXPONENTIATION_OP",
  "CALC_OP", "UNARY_OP", "RELATIONAL_OP", "LOGICAL_NOT_OP",
  "LOGICAL_AND_OR_OP", "ASSIGNMENT_OP", "KW_PLUS", "KW_MINUS", "KW_MUL",
  "KW_DIV", "KW_MOD", "KW_LESSEQUAL", "KW_GREATEREQUAL", "KW_LESSTHAN",
  "KW_GREATHAN", "KW_INEQ", "KW_AND", "KW_OR", "KW_NOT", "KW_EQUAL",
  "KW_EXPONENT", "';'", "')'", "']'", "'('", "'['", "'#'", "':'", "','",
  "'.'", "$accept", "program", "input", "macro", "commands", "$@1", "$@2",
  "$@3", "$@4", "$@5", "$@6", "else_statement", "$@7", "step", "stmts",
  "loop_thing", "complicated_types", "$@8", "declaration_comp",
  "variable_definition_comp", "variable_list_comp",
  "function_definition_section", "function_definition_comp",
  "return_type_comp", "func_call", "func_args", "expr",
  "use_of_comp_variable", "use_comp_variable_with_period",
  "use_comp_variable", "arithmetic_expr_non_empty", "function_definition",
  "$@9", "$@10", "$@11", "$@12", "arguments", "parameter_definition",
  "return_type", "body", "variable_definition", "VALUE",
  "KW_VARIABLE_TYPE", "NUMBERS", "variable_list", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-203)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-123)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -203,    10,    90,  -203,   200,  -203,  -203,   -41,   -31,    35,
      17,    51,    57,    64,   244,   244,   244,   122,  -203,    73,
      83,  -203,   274,  -203,    96,   100,  -203,    85,    15,   133,
     165,   244,   244,   147,    21,   101,  -203,   229,   135,    -3,
      82,   139,   161,    74,  -203,  -203,   244,   244,   244,   244,
    -203,  -203,  -203,    48,   184,   244,  -203,  -203,   -51,   300,
     125,   127,    -8,   242,   257,   124,   189,   136,   137,  -203,
     300,   166,   244,   244,  -203,   244,   244,   207,   300,    82,
     139,  -203,  -203,  -203,  -203,  -203,  -203,  -203,  -203,    62,
    -203,   236,   144,    48,   169,   151,   153,   251,    47,     7,
    -203,   158,  -203,  -203,  -203,  -203,   168,  -203,   217,   173,
    -203,   172,   259,  -203,   300,    48,  -203,   244,  -203,  -203,
     258,   -42,     9,   244,   244,   244,   260,   306,   193,     0,
      48,   233,   189,  -203,   203,    48,   -21,   268,  -203,   211,
    -203,  -203,  -203,  -203,    24,    44,   267,    97,   244,   244,
     244,   244,   251,   213,   216,  -203,    48,   221,  -203,  -203,
      13,   228,   111,    70,  -203,   244,   207,  -203,     5,   209,
     209,    50,   300,    88,   186,    78,   223,   224,    48,  -203,
    -203,   178,  -203,   240,   271,  -203,    48,   232,   309,  -203,
     263,  -203,  -203,   172,   278,   315,  -203,  -203,   270,   311,
     276,   288,   282,   333,   251,   289,    48,  -203,  -203,   244,
     324,     1,   308,  -203,   351,   292,  -203,   290,   355,  -203,
     293,   349,  -203,  -203,  -203,  -203,  -203,   295,  -203,  -203,
     294,  -203,   298,   296,   166,  -203,   189,    48,   352,  -203,
    -203,  -203,   338,  -203,   299,    48,   301,    67,  -203,   363,
    -203,  -203,  -203,    48,  -203,    48,   336,   307,   209,   209,
    -203,  -203,    48,   304,   305,   357,  -203,  -203,    48,  -203,
     347,  -203,  -203
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       3,     0,     2,     1,    59,   120,   121,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     9,     0,
       0,    56,     0,    58,     0,     0,    57,     0,     0,    51,
       0,     0,     0,     0,     0,     0,    35,     0,     0,    59,
      67,    66,     0,    79,     8,     7,     0,     0,     0,     0,
       5,     6,     4,     0,     0,     0,    68,    53,     0,    52,
     120,     0,     0,     0,     0,     0,    98,     0,     0,    10,
      11,     0,     0,     0,    61,     0,     0,    76,    63,    62,
      64,    65,   119,   116,   115,   117,   118,   108,   123,     0,
      50,     0,     0,     0,    60,     0,     0,     0,     0,     0,
      99,     0,    37,   112,   114,   113,     0,    70,     0,     0,
      72,    74,     0,    55,    54,     0,   110,     0,    12,    15,
      59,   120,   121,     0,     0,     0,     0,     0,     0,     0,
       0,   104,     0,    92,    45,     0,    80,     0,    77,     0,
     109,    69,    28,    28,    67,    66,     0,    79,     0,     0,
       0,     0,     0,     0,     0,   101,     0,     0,   100,    28,
       0,     0,     0,     0,   111,     0,    76,    78,     0,    13,
      16,    61,    86,    62,    64,    65,    26,     0,     0,   105,
      95,   106,    93,    43,     0,    38,     0,     0,     0,    36,
       0,    71,    73,    75,     0,     0,    33,    34,     0,    23,
       0,     0,     0,     0,     0,     0,     0,   103,    28,     0,
       0,     0,     0,    39,     0,     0,    46,     0,     0,    32,
       0,     0,    29,    31,    30,    17,    27,     0,   102,    96,
       0,    94,     0,     0,     0,    44,    98,     0,     0,    24,
      14,    18,     0,   107,     0,     0,     0,     0,    21,     0,
      28,    28,    97,     0,    41,     0,    48,     0,    25,    19,
      40,    42,     0,     0,     0,     0,    49,    28,     0,    20,
       0,    22,    47
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -203,  -203,  -203,  -203,   373,  -203,  -203,  -203,  -203,  -203,
    -203,  -203,  -203,  -203,  -141,  -203,  -203,  -203,  -203,  -203,
    -203,  -203,  -203,  -203,   -74,  -203,    -2,   210,  -203,  -203,
    -145,  -203,  -203,  -203,  -203,  -203,   141,   246,  -203,  -202,
     377,   146,   -78,  -203,  -203
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,     2,    18,   198,   142,   199,   143,   203,   251,
     265,   221,   250,   205,   181,   200,    20,    68,   134,   161,
     162,   163,   190,   263,    21,    58,   201,   111,   138,    23,
     128,    24,   159,   210,   208,   242,    99,   100,   157,   182,
     202,   106,    87,    26,    27
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      22,   169,   170,   110,   153,   232,   229,   176,    82,   194,
       3,    90,    40,    41,    42,   116,   183,    91,    39,     5,
      34,   -81,     6,    31,   165,   -81,    56,    59,    62,    63,
      64,    46,    47,    32,    48,    70,    49,   140,    33,    83,
      84,    85,    72,    86,    78,    79,    80,    81,   166,    35,
     184,    82,   155,    89,    36,    94,    14,   164,    15,   226,
      37,    29,    73,   154,   233,   270,    48,    38,    49,   131,
      56,    62,   -82,   107,   108,   132,   -82,   112,   179,    16,
      55,    17,    83,    84,    85,    66,    86,   -91,    49,   114,
     195,   -91,   192,     4,     5,   127,   188,     6,     7,   189,
     207,    46,    47,     8,    48,     9,    49,   -90,   213,   258,
     259,   -90,   129,   -85,   130,   141,    10,   -85,    11,    75,
      12,   144,   145,   146,    48,    43,    49,    13,   228,   256,
      48,    14,    49,    15,    44,   132,    39,     5,    57,    76,
       6,   -89,    75,    77,    45,   -89,   172,   173,   174,   175,
     127,   -87,    53,    54,    16,   -87,    17,    51,    65,   248,
     -84,    52,    76,   191,   -84,    67,    77,   254,    39,    60,
     103,   104,     6,   105,    14,   260,    15,   261,   186,   187,
      71,     4,     5,    49,   266,     6,     7,    88,    92,    97,
     271,     8,    98,     9,    93,   196,   197,    16,   101,    17,
      46,    47,   127,    48,   102,    49,    14,   230,    15,   209,
     109,   115,     4,     5,   117,    13,     6,     7,   118,    14,
     119,    15,     8,    74,     9,   133,   196,   197,    61,    16,
      49,    17,    39,     5,    69,   135,     6,    29,   137,    39,
       5,   113,    16,     6,    17,    28,    13,    39,     5,   -88,
      14,     6,    15,   -88,   120,   121,    46,    47,   122,    48,
     152,    49,   139,   147,    29,    30,   156,  -122,  -122,   160,
      14,   167,    15,    16,   212,    17,   177,    14,   168,    15,
     136,    46,    47,   178,    48,    14,    49,    15,   180,   185,
     204,   206,   123,    16,   124,    17,    46,    47,   214,    48,
      16,    49,    17,    72,    95,   211,    46,    47,    16,    48,
      17,    49,   215,    46,    47,   125,    48,   126,    49,    96,
     220,   -83,    29,    73,   216,   -83,   218,    46,    47,   171,
      48,   219,    49,    46,    47,    50,    48,   222,    49,    46,
      47,   217,    48,   224,    49,   148,   149,   225,   150,   223,
     151,   231,   227,   234,   235,   243,   236,   237,   238,   240,
     239,   244,   241,   245,   249,   252,   253,   257,   255,   262,
     264,   267,   268,   269,   272,    19,   193,   247,   158,    25,
     246
};

static const yytype_int16 yycheck[] =
{
       2,   142,   143,    77,     4,     4,   208,   152,     3,     4,
       0,    62,    14,    15,    16,    93,     3,    68,     3,     4,
       3,    63,     7,    64,    45,    67,    28,    29,    30,    31,
      32,    39,    40,    64,    42,    37,    44,   115,     3,    34,
      35,    36,    45,    38,    46,    47,    48,    49,    69,    32,
      37,     3,   130,    55,     3,    63,    41,   135,    43,   204,
       3,    64,    65,    63,    63,   267,    42,     3,    44,    62,
      72,    73,    63,    75,    76,    68,    67,    15,   156,    64,
      65,    66,    34,    35,    36,    64,    38,    63,    44,    91,
     168,    67,   166,     3,     4,    97,    26,     7,     8,    29,
     178,    39,    40,    13,    42,    15,    44,    63,   186,   250,
     251,    67,    65,    63,    67,   117,    26,    67,    28,    45,
      30,   123,   124,   125,    42,     3,    44,    37,   206,    62,
      42,    41,    44,    43,    61,    68,     3,     4,     5,    65,
       7,    63,    45,    69,    61,    67,   148,   149,   150,   151,
     152,    63,    67,    68,    64,    67,    66,    61,    11,   237,
      63,    61,    65,   165,    67,    64,    69,   245,     3,     4,
       4,     5,     7,     7,    41,   253,    43,   255,    67,    68,
      45,     3,     4,    44,   262,     7,     8,     3,    63,    65,
     268,    13,     3,    15,    67,    17,    18,    64,    62,    66,
      39,    40,   204,    42,    67,    44,    41,   209,    43,    31,
       3,    67,     3,     4,    45,    37,     7,     8,    67,    41,
      67,    43,    13,    62,    15,    67,    17,    18,    63,    64,
      44,    66,     3,     4,     5,    67,     7,    64,    66,     3,
       4,     5,    64,     7,    66,    45,    37,     3,     4,    63,
      41,     7,    43,    67,     3,     4,    39,    40,     7,    42,
      67,    44,     3,     3,    64,    65,    33,    67,    68,    66,
      41,     3,    43,    64,     3,    66,    63,    41,    67,    43,
      63,    39,    40,    67,    42,    41,    44,    43,    67,    61,
      67,    67,    41,    64,    43,    66,    39,    40,    66,    42,
      64,    44,    66,    45,    62,    65,    39,    40,    64,    42,
      66,    44,     3,    39,    40,    64,    42,    66,    44,    62,
       9,    63,    64,    65,    61,    67,    11,    39,    40,    62,
      42,    61,    44,    39,    40,    61,    42,    61,    44,    39,
      40,    63,    42,    61,    44,    39,    40,    14,    42,    61,
      44,    27,    63,    45,     3,    61,    64,    67,     3,    10,
      67,    63,    67,    67,    12,    27,    67,     4,    67,    33,
      63,    67,    67,    16,    27,     2,   166,   236,   132,     2,
     234
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    71,    72,     0,     3,     4,     7,     8,    13,    15,
      26,    28,    30,    37,    41,    43,    64,    66,    73,    74,
      86,    94,    96,    99,   101,   110,   113,   114,    45,    64,
      65,    64,    64,     3,     3,    32,     3,     3,     3,     3,
      96,    96,    96,     3,    61,    61,    39,    40,    42,    44,
      61,    61,    61,    67,    68,    65,    96,     5,    95,    96,
       4,    63,    96,    96,    96,    11,    64,    64,    87,     5,
      96,    45,    45,    65,    62,    45,    65,    69,    96,    96,
      96,    96,     3,    34,    35,    36,    38,   112,     3,    96,
      62,    68,    63,    67,    63,    62,    62,    65,     3,   106,
     107,    62,    67,     4,     5,     7,   111,    96,    96,     3,
      94,    97,    15,     5,    96,    67,   112,    45,    67,    67,
       3,     4,     7,    41,    43,    64,    66,    96,   100,    65,
      67,    62,    68,    67,    88,    67,    63,    66,    98,     3,
     112,    96,    75,    77,    96,    96,    96,     3,    39,    40,
      42,    44,    67,     4,    63,   112,    33,   108,   107,   102,
      66,    89,    90,    91,   112,    45,    69,     3,    67,    84,
      84,    62,    96,    96,    96,    96,   100,    63,    67,   112,
      67,    84,   109,     3,    37,    61,    67,    68,    26,    29,
      92,    96,    94,    97,     4,   112,    17,    18,    74,    76,
      85,    96,   110,    78,    67,    83,    67,   112,   104,    31,
     103,    65,     3,   112,    66,     3,    61,    63,    11,    61,
       9,    81,    61,    61,    61,    14,   100,    63,   112,   109,
      96,    27,     4,    63,    45,     3,    64,    67,     3,    67,
      10,    67,   105,    61,    63,    67,   111,   106,   112,    12,
      82,    79,    27,    67,   112,    67,    62,     4,    84,    84,
     112,   112,    33,    93,    63,    80,   112,    67,    67,    16,
     109,   112,    27
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    70,    71,    72,    72,    72,    72,    72,    72,    72,
      73,    73,    75,    76,    74,    77,    78,    74,    79,    80,
      74,    74,    74,    81,    82,    81,    83,    83,    84,    84,
      84,    84,    84,    85,    85,    87,    86,    88,    88,    89,
      89,    89,    89,    90,    90,    91,    91,    92,    93,    93,
      94,    95,    95,    95,    95,    95,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    97,    97,    98,    99,
      99,   100,   100,   100,   100,   100,   100,   100,   100,   100,
     100,   100,   102,   103,   101,   104,   105,   101,   106,   106,
     106,   107,   107,   107,   108,   108,   109,   109,   110,   110,
     110,   110,   111,   111,   111,   112,   112,   112,   112,   112,
     113,   113,   114,   114
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     0,     3,     3,     3,     3,     3,     2,
       3,     3,     0,     0,    10,     0,     0,     9,     0,     0,
      14,    11,    15,     0,     0,     4,     0,     2,     0,     3,
       3,     3,     3,     1,     1,     0,     7,     0,     3,     3,
       7,     6,     7,     2,     4,     0,     3,     9,     0,     2,
       4,     0,     1,     1,     3,     3,     1,     1,     1,     1,
       4,     3,     3,     3,     3,     3,     2,     2,     3,     6,
       4,     7,     4,     7,     4,     7,     0,     2,     2,     2,
       5,     1,     1,     1,     2,     3,     3,     3,     3,     3,
       2,     2,     0,     0,     9,     0,     0,    11,     0,     1,
       3,     3,     6,     5,     0,     2,     1,     4,     3,     6,
       5,     6,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* program: input  */
#line 736 "phase_2/tuc_transpiler.y"
        { print_evaluation((yyvsp[0].str)); }
#line 2053 "generated/tuc_transpiler.tab.c"
    break;

  case 3: /* input: %empty  */
#line 739 "phase_2/tuc_transpiler.y"
                                                  {(yyval.str) = "";}
#line 2059 "generated/tuc_transpiler.tab.c"
    break;

  case 4: /* input: input variable_definition ';'  */
#line 741 "phase_2/tuc_transpiler.y"
                                                  { (yyval.str) = template("%s%s\n", (yyvsp[-2].str), (yyvsp[-1].str));}
#line 2065 "generated/tuc_transpiler.tab.c"
    break;

  case 5: /* input: input expr ';'  */
#line 742 "phase_2/tuc_transpiler.y"
                                                  { assignment_made = 0;  (yyval.str) = template("%s%s;\n", (yyvsp[-2].str), (yyvsp[-1].str));}
#line 2071 "generated/tuc_transpiler.tab.c"
    break;

  case 6: /* input: input function_definition ';'  */
#line 743 "phase_2/tuc_transpiler.y"
                                                  { assignment_made = 0; (yyval.str) = template("%s%s", (yyvsp[-2].str), (yyvsp[-1].str));}
#line 2077 "generated/tuc_transpiler.tab.c"
    break;

  case 7: /* input: input complicated_types ';'  */
#line 744 "phase_2/tuc_transpiler.y"
                                                  { assignment_made = 0;  (yyval.str) = template("%s%s", (yyvsp[-2].str), (yyvsp[-1].str));}
#line 2083 "generated/tuc_transpiler.tab.c"
    break;

  case 8: /* input: input commands ';'  */
#line 745 "phase_2/tuc_transpiler.y"
                                                  {  (yyval.str) = template("%s%s", (yyvsp[-2].str), (yyvsp[-1].str));}
#line 2089 "generated/tuc_transpiler.tab.c"
    break;

  case 9: /* input: input macro  */
#line 746 "phase_2/tuc_transpiler.y"
                                                  {  (yyval.str) = template("%s%s", (yyvsp[-1].str), (yyvsp[0].str));}
#line 2095 "generated/tuc_transpiler.tab.c"
    break;

  case 10: /* macro: KW_MACRO IDENTIFIER STRING  */
#line 753 "phase_2/tuc_transpiler.y"
                               {
        char *id = (yyvsp[-1].str); 
        char *value = (yyvsp[0].str);
        
        add_macro(id, value);  

    }
#line 2107 "generated/tuc_transpiler.tab.c"
    break;

  case 11: /* macro: KW_MACRO IDENTIFIER expr  */
#line 760 "phase_2/tuc_transpiler.y"
                             {
        char *id = (yyvsp[-1].str);  
        char *value = (yyvsp[0].str); 

        add_macro(id, value);
    }
#line 2118 "generated/tuc_transpiler.tab.c"
    break;

  case 12: /* $@1: %empty  */
#line 773 "phase_2/tuc_transpiler.y"
                           {temp1 = template("%s (%s):\n", (yyvsp[-4].str), (yyvsp[-2].str)); indentation_level++; is_a_loop = 0;}
#line 2124 "generated/tuc_transpiler.tab.c"
    break;

  case 13: /* $@2: %empty  */
#line 775 "phase_2/tuc_transpiler.y"
    {
        indented_stmts = indent_statements((yyvsp[0].str), indentation_level);
        indentation_level--;
    }
#line 2133 "generated/tuc_transpiler.tab.c"
    break;

  case 14: /* commands: KW_IF '(' expr ')' ':' $@1 stmts $@2 else_statement KW_ENDIF  */
#line 779 "phase_2/tuc_transpiler.y"
                            {
        temp3 = template("%s%s", (yyvsp[-1].str), (yyvsp[0].str)); 
        (yyval.str) = template("%s%s%s;\n", temp1, indented_stmts, temp3);
    }
#line 2142 "generated/tuc_transpiler.tab.c"
    break;

  case 15: /* $@3: %empty  */
#line 785 "phase_2/tuc_transpiler.y"
    {
        temp7 = template("%s (%s):\n", (yyvsp[-4].str), (yyvsp[-2].str));
        indentation_level++; is_a_loop = 1;
    }
#line 2151 "generated/tuc_transpiler.tab.c"
    break;

  case 16: /* $@4: %empty  */
#line 790 "phase_2/tuc_transpiler.y"
    {
        indented_stmts_3 = indent_statements((yyvsp[0].str), indentation_level);
        indentation_level--;
    }
#line 2160 "generated/tuc_transpiler.tab.c"
    break;

  case 17: /* commands: KW_WHILE '(' expr ')' ':' $@3 stmts $@4 KW_ENDWHILE  */
#line 794 "phase_2/tuc_transpiler.y"
                {
        (yyval.str) = template("%s%s%s;\n", temp7, indented_stmts_3, (yyvsp[0].str));
        is_a_loop = 0;
    }
#line 2169 "generated/tuc_transpiler.tab.c"
    break;

  case 18: /* $@5: %empty  */
#line 803 "phase_2/tuc_transpiler.y"
    {
        if(for_has_step == 1){
            temp5 = template("for (int %s = %s; %s < %s; %s++) {\n", (yyvsp[-8].str), (yyvsp[-5].str), (yyvsp[-8].str), (yyvsp[-3].str), (yyvsp[-2].str), (yyvsp[-8].str)); 
        } else {
            temp5 = template("for (int %s = %s; %s < %s; %s++) {\n", (yyvsp[-8].str), (yyvsp[-5].str), (yyvsp[-8].str), (yyvsp[-3].str), (yyvsp[-8].str)); 
        }
         
        indentation_level++; is_a_loop = 1;
    }
#line 2183 "generated/tuc_transpiler.tab.c"
    break;

  case 19: /* $@6: %empty  */
#line 813 "phase_2/tuc_transpiler.y"
    {
        indented_stmts_2 = indent_statements((yyvsp[0].str), indentation_level);
        indentation_level--;
    }
#line 2192 "generated/tuc_transpiler.tab.c"
    break;

  case 20: /* commands: KW_FOR IDENTIFIER KW_IN '[' arithmetic_expr_non_empty ':' arithmetic_expr_non_empty step ']' ':' $@5 stmts $@6 KW_ENDFOR  */
#line 817 "phase_2/tuc_transpiler.y"
              {

        (yyval.str) = template("%s%s}\n", temp5, indented_stmts_2);
        is_a_loop = 0;
    }
#line 2202 "generated/tuc_transpiler.tab.c"
    break;

  case 21: /* commands: IDENTIFIER ASSIGNMENT_OP '[' expr KW_FOR IDENTIFIER ':' POSINT ']' ':' KW_VARIABLE_TYPE  */
#line 827 "phase_2/tuc_transpiler.y"
    {
        char* newType = (yyvsp[0].str);
        if (contains_substring((yyvsp[-7].str),(yyvsp[-5].str)) == 0){
            yyerror("In compact array, the element isnt contained in the expression"); 

        }
        (yyval.str) = template("%s* %s =(%s*)malloc(%s*sizeof(%s));\nfor (int %s = 0; %s < %s; ++%s) {\n  %s[%s] = %s;\n}\n", 
                        newType, (yyvsp[-10].str),newType,(yyvsp[-3].str),     newType,        (yyvsp[-5].str),     (yyvsp[-5].str),  (yyvsp[-3].str),  (yyvsp[-5].str),       (yyvsp[-10].str),(yyvsp[-5].str),   (yyvsp[-7].str));

    }
#line 2217 "generated/tuc_transpiler.tab.c"
    break;

  case 22: /* commands: IDENTIFIER ASSIGNMENT_OP '[' expr KW_FOR IDENTIFIER ':' KW_VARIABLE_TYPE KW_IN IDENTIFIER KW_OF POSINT ']' ':' KW_VARIABLE_TYPE  */
#line 841 "phase_2/tuc_transpiler.y"
    {
        char* newType = (yyvsp[0].str); 
        char* s2 = template("%s[%s_i]",(yyvsp[-5].str),(yyvsp[-5].str));
        if (contains_substring((yyvsp[-11].str),(yyvsp[-9].str)) == 0){
            yyerror("In compact array, the element isnt contained in the expression"); 

        }

        char* modifiedExpr = replace_identifier_in_expr((yyvsp[-11].str), (yyvsp[-9].str), s2);


        (yyval.str) = template(
            "%s* %s = (%s*)malloc(%s * sizeof(%s));\nfor (%s %s_i = 0; %s_i < %s; ++%s_i) {\n   %s[%s_i] = %s;\n}\n",
            newType,(yyvsp[-14].str), newType, (yyvsp[-3].str), newType,            (yyvsp[-7].str), (yyvsp[-5].str),      (yyvsp[-5].str), (yyvsp[-3].str),    (yyvsp[-5].str),    (yyvsp[-14].str), (yyvsp[-5].str), modifiedExpr
        );
    }
#line 2238 "generated/tuc_transpiler.tab.c"
    break;

  case 23: /* else_statement: %empty  */
#line 863 "phase_2/tuc_transpiler.y"
                                                     { (yyval.str) = ""; }
#line 2244 "generated/tuc_transpiler.tab.c"
    break;

  case 24: /* $@7: %empty  */
#line 864 "phase_2/tuc_transpiler.y"
                                      {temp4 = template("%s:\n", (yyvsp[-1].str)); indentation_level++;}
#line 2250 "generated/tuc_transpiler.tab.c"
    break;

  case 25: /* else_statement: KW_ELSE ':' $@7 stmts  */
#line 866 "phase_2/tuc_transpiler.y"
  {         
      indented_stmts_1 = indent_statements((yyvsp[0].str), indentation_level);
      indentation_level--;
       (yyval.str) = template("%s%s", temp4, indented_stmts_1);
  }
#line 2260 "generated/tuc_transpiler.tab.c"
    break;

  case 26: /* step: %empty  */
#line 873 "phase_2/tuc_transpiler.y"
                                        { for_has_step = 0; (yyval.str) = ""; }
#line 2266 "generated/tuc_transpiler.tab.c"
    break;

  case 27: /* step: ':' arithmetic_expr_non_empty  */
#line 874 "phase_2/tuc_transpiler.y"
                                        { for_has_step = 1; (yyval.str) = template(":%s", (yyvsp[0].str));}
#line 2272 "generated/tuc_transpiler.tab.c"
    break;

  case 28: /* stmts: %empty  */
#line 882 "phase_2/tuc_transpiler.y"
                                                  { (yyval.str) = ""; }
#line 2278 "generated/tuc_transpiler.tab.c"
    break;

  case 29: /* stmts: stmts loop_thing ';'  */
#line 884 "phase_2/tuc_transpiler.y"
  {     
    if (is_a_loop != 1) {  
      yyerror("This isnt a loop"); 
      YYABORT;
    }
    (yyval.str) = template("%s%s;\n", (yyvsp[-2].str), (yyvsp[-1].str));}
#line 2289 "generated/tuc_transpiler.tab.c"
    break;

  case 30: /* stmts: stmts variable_definition ';'  */
#line 890 "phase_2/tuc_transpiler.y"
                                                  { (yyval.str) = template("%s%s\n", (yyvsp[-2].str), (yyvsp[-1].str)); }
#line 2295 "generated/tuc_transpiler.tab.c"
    break;

  case 31: /* stmts: stmts expr ';'  */
#line 892 "phase_2/tuc_transpiler.y"
  { 
    assignment_made = 0; 
    (yyval.str) = template("%s%s;\n", (yyvsp[-2].str), (yyvsp[-1].str)); 
  }
#line 2304 "generated/tuc_transpiler.tab.c"
    break;

  case 32: /* stmts: stmts commands ';'  */
#line 896 "phase_2/tuc_transpiler.y"
                                                  { (yyval.str) = template("%s%s", (yyvsp[-2].str), (yyvsp[-1].str)); }
#line 2310 "generated/tuc_transpiler.tab.c"
    break;

  case 35: /* $@8: %empty  */
#line 908 "phase_2/tuc_transpiler.y"
                     { comp_name = (yyvsp[0].str); inside_comp = 1; createNewComp(comp_name);}
#line 2316 "generated/tuc_transpiler.tab.c"
    break;

  case 36: /* complicated_types: KW_COMP IDENTIFIER $@8 ':' declaration_comp function_definition_section KW_ENDCOMP  */
#line 910 "phase_2/tuc_transpiler.y"
  { 
    char* function_definition_prints = getCompFunctionsAsString(comp_name);   
    char* function_names = getCompFunctionNamesAsString(comp_name);
    (yyval.str) = template("typedef struct %s {\n%s%s} %s;\n%s\n%s ctor_%s = {%s};\n", (yyvsp[-5].str), (yyvsp[-2].str), (yyvsp[-1].str), comp_name, function_definition_prints, comp_name, comp_name, function_names); 

    inside_comp = 0;
  }
#line 2328 "generated/tuc_transpiler.tab.c"
    break;

  case 37: /* declaration_comp: %empty  */
#line 922 "phase_2/tuc_transpiler.y"
                                                            { (yyval.str) = ""; }
#line 2334 "generated/tuc_transpiler.tab.c"
    break;

  case 38: /* declaration_comp: declaration_comp variable_definition_comp ';'  */
#line 923 "phase_2/tuc_transpiler.y"
                                                            { (yyval.str) = template("%s  %s;\n", (yyvsp[-2].str), (yyvsp[-1].str)); }
#line 2340 "generated/tuc_transpiler.tab.c"
    break;

  case 39: /* variable_definition_comp: variable_list_comp ':' KW_VARIABLE_TYPE  */
#line 927 "phase_2/tuc_transpiler.y"
                                                                                { if(is_comp_type((yyvsp[0].str))){ create_compTypeVariables((yyvsp[0].str), (yyvsp[-2].str));} add_comp_variables(comp_name, (yyvsp[-2].str));
                                                                                  (yyval.str) = template("%s %s", (yyvsp[0].str), (yyvsp[-2].str));}
#line 2347 "generated/tuc_transpiler.tab.c"
    break;

  case 40: /* variable_definition_comp: '#' IDENTIFIER '[' POSINT ']' ':' KW_VARIABLE_TYPE  */
#line 929 "phase_2/tuc_transpiler.y"
                                                                                { if(is_comp_type((yyvsp[0].str))){create_compTypeVariables((yyvsp[0].str), (yyvsp[-5].str));} add_comp_variable(comp_name, (yyvsp[-5].str)); 
                                                                                  (yyval.str) = template("%s %s[%s];", (yyvsp[0].str), (yyvsp[-5].str), (yyvsp[-3].str) ); }
#line 2354 "generated/tuc_transpiler.tab.c"
    break;

  case 41: /* variable_definition_comp: '#' IDENTIFIER '[' ']' ':' KW_VARIABLE_TYPE  */
#line 931 "phase_2/tuc_transpiler.y"
                                                                                { if(is_comp_type((yyvsp[0].str))){create_compTypeVariables((yyvsp[0].str), (yyvsp[-4].str));} add_comp_variable(comp_name, (yyvsp[-4].str));
                                                                                  (yyval.str) = template("%s[]: %s;", (yyvsp[-4].str), (yyvsp[0].str) ); }
#line 2361 "generated/tuc_transpiler.tab.c"
    break;

  case 42: /* variable_definition_comp: '#' KW_CONST IDENTIFIER ASSIGNMENT_OP VALUE ':' KW_VARIABLE_TYPE  */
#line 933 "phase_2/tuc_transpiler.y"
                                                                                { if(is_comp_type((yyvsp[0].str))){create_compTypeVariables((yyvsp[0].str), (yyvsp[-4].str));} add_comp_variable(comp_name, (yyvsp[-4].str));
                                                                                  (yyval.str) = template("%s %s %s = %s;", (yyvsp[-5].str), (yyvsp[0].str), (yyvsp[-4].str), (yyvsp[-2].str)); }
#line 2368 "generated/tuc_transpiler.tab.c"
    break;

  case 43: /* variable_list_comp: '#' IDENTIFIER  */
#line 938 "phase_2/tuc_transpiler.y"
                                                    { (yyval.str) = template("%s", (yyvsp[0].str)); }
#line 2374 "generated/tuc_transpiler.tab.c"
    break;

  case 44: /* variable_list_comp: variable_list_comp ',' '#' IDENTIFIER  */
#line 939 "phase_2/tuc_transpiler.y"
                                                    { (yyval.str) = template("%s, %s", (yyvsp[-3].str), (yyvsp[0].str)); }
#line 2380 "generated/tuc_transpiler.tab.c"
    break;

  case 45: /* function_definition_section: %empty  */
#line 947 "phase_2/tuc_transpiler.y"
                                                                { (yyval.str) = "\n"; }
#line 2386 "generated/tuc_transpiler.tab.c"
    break;

  case 46: /* function_definition_section: function_definition_section function_definition_comp ';'  */
#line 948 "phase_2/tuc_transpiler.y"
                                                                {(yyval.str) = template("%s  %s;\n", (yyvsp[-2].str), (yyvsp[-1].str));}
#line 2392 "generated/tuc_transpiler.tab.c"
    break;

  case 47: /* function_definition_comp: KW_DEF IDENTIFIER '(' arguments ')' return_type_comp ':' body KW_ENDDEF  */
#line 953 "phase_2/tuc_transpiler.y"
  {
    if (func_needs_return_type != body_has_return_type) {  
      yyerror("Return statement missing from body"); 
      YYABORT;
    }
    (yyval.str) = template("%s (*%s)(struct %s *self%s%s)", (yyvsp[-3].str), (yyvsp[-7].str), comp_name, strlen((yyvsp[-5].str)) ? ", " : "", (yyvsp[-5].str));
    func_needs_return_type = 0;
    body_has_return_type = 0;

    add_function((yyvsp[-7].str), num_of_args_in_function);  
    num_of_args_in_function = 0;
    char* function_follow_comp = template("%s %s(struct %s *self%s%s) {\n%s\n}", (yyvsp[-3].str), (yyvsp[-7].str), comp_name,strlen((yyvsp[-5].str)) == 0 ? "":", " , (yyvsp[-5].str), (yyvsp[-1].str) );
    addFunctionToComp(comp_name, function_follow_comp, (yyvsp[-7].str));
  }
#line 2411 "generated/tuc_transpiler.tab.c"
    break;

  case 48: /* return_type_comp: %empty  */
#line 970 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = "void"; }
#line 2417 "generated/tuc_transpiler.tab.c"
    break;

  case 49: /* return_type_comp: KW_FUNC_RETURN KW_VARIABLE_TYPE  */
#line 971 "phase_2/tuc_transpiler.y"
                                        { func_needs_return_type = 1; (yyval.str) = template("%s", (yyvsp[0].str)); }
#line 2423 "generated/tuc_transpiler.tab.c"
    break;

  case 50: /* func_call: IDENTIFIER '(' func_args ')'  */
#line 980 "phase_2/tuc_transpiler.y"
    {  
        const DefinedFunction* func = find_function((yyvsp[-3].str));
        function_name_temp = (yyvsp[-3].str);

        if(find_included_function((yyvsp[-3].str)) == 1) {used_lambdalib_function = 1;}

        if (!func || (func->has_arguments != 99 && num_of_args_in_function_in_func_call != func->has_arguments && func->has_arguments != 999)) {
            yyerror("Wrong function call or function hasn't been defined"); YYABORT;

        } else {
            (yyval.str) = template("%s(%s)", (yyvsp[-3].str), (yyvsp[-1].str));
        }
        num_of_args_in_function_in_func_call = 0;
    }
#line 2442 "generated/tuc_transpiler.tab.c"
    break;

  case 51: /* func_args: %empty  */
#line 997 "phase_2/tuc_transpiler.y"
           { (yyval.str) = ""; }
#line 2448 "generated/tuc_transpiler.tab.c"
    break;

  case 52: /* func_args: expr  */
#line 998 "phase_2/tuc_transpiler.y"
                                    { num_of_args_in_function_in_func_call++; (yyval.str) = template("%s", (yyvsp[0].str)); }
#line 2454 "generated/tuc_transpiler.tab.c"
    break;

  case 53: /* func_args: STRING  */
#line 999 "phase_2/tuc_transpiler.y"
                                    { num_of_args_in_function_in_func_call++; (yyval.str) = template("%s", (yyvsp[0].str)); }
#line 2460 "generated/tuc_transpiler.tab.c"
    break;

  case 54: /* func_args: func_args ',' expr  */
#line 1000 "phase_2/tuc_transpiler.y"
                                    { num_of_args_in_function_in_func_call++; (yyval.str) = template("%s, %s", (yyvsp[-2].str), (yyvsp[0].str)); }
#line 2466 "generated/tuc_transpiler.tab.c"
    break;

  case 55: /* func_args: func_args ',' STRING  */
#line 1001 "phase_2/tuc_transpiler.y"
                                    { num_of_args_in_function_in_func_call++; (yyval.str) = template("%s, %s", (yyvsp[-2].str), (yyvsp[0].str)); }
#line 2472 "generated/tuc_transpiler.tab.c"
    break;

  case 56: /* expr: func_call  */
#line 1009 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("%s", (yyvsp[0].str));  }
#line 2478 "generated/tuc_transpiler.tab.c"
    break;

  case 57: /* expr: NUMBERS  */
#line 1010 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = (yyvsp[0].str); }
#line 2484 "generated/tuc_transpiler.tab.c"
    break;

  case 58: /* expr: use_comp_variable  */
#line 1011 "phase_2/tuc_transpiler.y"
                                         { (yyval.str) = (yyvsp[0].str); }
#line 2490 "generated/tuc_transpiler.tab.c"
    break;

  case 59: /* expr: IDENTIFIER  */
#line 1012 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = (yyvsp[0].str); }
#line 2496 "generated/tuc_transpiler.tab.c"
    break;

  case 60: /* expr: IDENTIFIER '[' expr ']'  */
#line 1013 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("%s[%s]", (yyvsp[-3].str), (yyvsp[-1].str)); }
#line 2502 "generated/tuc_transpiler.tab.c"
    break;

  case 61: /* expr: '(' expr ')'  */
#line 1014 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("(%s)", (yyvsp[-1].str)); }
#line 2508 "generated/tuc_transpiler.tab.c"
    break;

  case 62: /* expr: expr CALC_OP expr  */
#line 1015 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2514 "generated/tuc_transpiler.tab.c"
    break;

  case 63: /* expr: expr EXPONENTIATION_OP expr  */
#line 1016 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("pow(%s, %s)", (yyvsp[-2].str), (yyvsp[0].str)); }
#line 2520 "generated/tuc_transpiler.tab.c"
    break;

  case 64: /* expr: expr RELATIONAL_OP expr  */
#line 1017 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2526 "generated/tuc_transpiler.tab.c"
    break;

  case 65: /* expr: expr LOGICAL_AND_OR_OP expr  */
#line 1018 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2532 "generated/tuc_transpiler.tab.c"
    break;

  case 66: /* expr: LOGICAL_NOT_OP expr  */
#line 1019 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("%s %s", (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2538 "generated/tuc_transpiler.tab.c"
    break;

  case 67: /* expr: UNARY_OP expr  */
#line 1020 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("%s%s", (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2544 "generated/tuc_transpiler.tab.c"
    break;

  case 68: /* expr: IDENTIFIER ASSIGNMENT_OP expr  */
#line 1022 "phase_2/tuc_transpiler.y"
  {
      if (assignment_made == 1) {
          yyerror("Multiple assignments in one expression are not allowed");
          YYABORT; 
      }
      assignment_made = 1; 
      (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); 
  }
#line 2557 "generated/tuc_transpiler.tab.c"
    break;

  case 69: /* expr: IDENTIFIER '[' expr ']' ASSIGNMENT_OP expr  */
#line 1031 "phase_2/tuc_transpiler.y"
  {
      if (assignment_made == 1) {
          yyerror("Multiple assignments in one expression are not allowed");
          YYABORT; 
      }
      assignment_made = 1; 
      (yyval.str) = template("%s[%s] %s %s", (yyvsp[-5].str), (yyvsp[-3].str), (yyvsp[-1].str), (yyvsp[0].str) ); 
  }
#line 2570 "generated/tuc_transpiler.tab.c"
    break;

  case 70: /* expr: '#' IDENTIFIER ASSIGNMENT_OP expr  */
#line 1040 "phase_2/tuc_transpiler.y"
  {     
      if (assignment_made == 1 || inside_comp == 0) {
          yyerror("Multiple assignments in one expression are not allowed or not in comp");
          YYABORT; 
      }
      assignment_made = 1; 
      (yyval.str) = template("self->%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); 
  }
#line 2583 "generated/tuc_transpiler.tab.c"
    break;

  case 71: /* expr: '#' IDENTIFIER '[' expr ']' ASSIGNMENT_OP expr  */
#line 1049 "phase_2/tuc_transpiler.y"
  {
      if (assignment_made == 1 || inside_comp == 0) {
          yyerror("Multiple assignments in one expression are not allowed or not in comp");
          YYABORT; 
      }
      assignment_made = 1; 
      (yyval.str) = template("self->%s[%s] %s %s", (yyvsp[-5].str), (yyvsp[-3].str), (yyvsp[-1].str), (yyvsp[0].str) ); 
  }
#line 2596 "generated/tuc_transpiler.tab.c"
    break;

  case 72: /* expr: '#' IDENTIFIER '.' func_call  */
#line 1058 "phase_2/tuc_transpiler.y"
  {     
      char* comp_name = find_comp_type((yyvsp[-2].str));

      if(is_comp_function(comp_name, function_name_temp) == 0){
          yyerror("Function used isnt assigned for this variable type");
          YYABORT; 
      }
      (yyval.str) = template("self->%s.%s", (yyvsp[-2].str), (yyvsp[0].str)); 

  }
#line 2611 "generated/tuc_transpiler.tab.c"
    break;

  case 73: /* expr: '#' IDENTIFIER '[' expr ']' '.' func_call  */
#line 1069 "phase_2/tuc_transpiler.y"
  {     
      char* comp_name = find_comp_type((yyvsp[-5].str));

      if(is_comp_function(comp_name, function_name_temp) == 0){
          yyerror("Function used isnt assigned for this variable type");
          YYABORT; 
      }
      (yyval.str) = template("self->%s[%s].%s", (yyvsp[-5].str), (yyvsp[-3].str), (yyvsp[0].str)); 

  }
#line 2626 "generated/tuc_transpiler.tab.c"
    break;

  case 74: /* expr: '#' IDENTIFIER '.' use_of_comp_variable  */
#line 1080 "phase_2/tuc_transpiler.y"
  {     
      char* comp_name = find_comp_type((yyvsp[-2].str));

      if(is_comp_variable(comp_name, variable_name_temp) == 0){
          yyerror("Variable used isnt assigned for this variable type");
          YYABORT; 
      }
      (yyval.str) = template("self->%s.%s", (yyvsp[-2].str), (yyvsp[0].str)); 

  }
#line 2641 "generated/tuc_transpiler.tab.c"
    break;

  case 75: /* expr: '#' IDENTIFIER '[' expr ']' '.' use_of_comp_variable  */
#line 1091 "phase_2/tuc_transpiler.y"
  {     
      char* comp_name = find_comp_type((yyvsp[-5].str));

      if(is_comp_variable(comp_name, variable_name_temp) == 0){
          yyerror("Variable used isnt assigned for this variable type");
          YYABORT; 
      }
      (yyval.str) = template("self->%s[%s].%s", (yyvsp[-5].str), (yyvsp[-3].str), (yyvsp[0].str)); 

  }
#line 2656 "generated/tuc_transpiler.tab.c"
    break;

  case 76: /* use_of_comp_variable: %empty  */
#line 1104 "phase_2/tuc_transpiler.y"
                                                            { (yyval.str) = ""; }
#line 2662 "generated/tuc_transpiler.tab.c"
    break;

  case 77: /* use_of_comp_variable: use_of_comp_variable use_comp_variable_with_period  */
#line 1105 "phase_2/tuc_transpiler.y"
                                                            { (yyval.str) = template("%s%s", (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2668 "generated/tuc_transpiler.tab.c"
    break;

  case 78: /* use_comp_variable_with_period: '#' IDENTIFIER  */
#line 1109 "phase_2/tuc_transpiler.y"
                                        { variable_name_temp = (yyvsp[0].str); (yyval.str) = template("%s", (yyvsp[0].str));}
#line 2674 "generated/tuc_transpiler.tab.c"
    break;

  case 79: /* use_comp_variable: '#' IDENTIFIER  */
#line 1114 "phase_2/tuc_transpiler.y"
                                        { if (inside_comp == 0 || !(is_comp_variable(comp_name, (yyvsp[0].str)))) {yyerror("Variable, not in comp"); YYABORT;} 
                                          else { (yyval.str) = template("self->%s", (yyvsp[0].str));} }
#line 2681 "generated/tuc_transpiler.tab.c"
    break;

  case 80: /* use_comp_variable: '#' IDENTIFIER '[' expr ']'  */
#line 1116 "phase_2/tuc_transpiler.y"
                                        { if (inside_comp == 0 || !(is_comp_variable(comp_name, (yyvsp[-3].str)))) {yyerror("Variable, not in comp"); YYABORT;} 
                                          else { (yyval.str) = template("self->%s[%s]", (yyvsp[-3].str), (yyvsp[-1].str));} }
#line 2688 "generated/tuc_transpiler.tab.c"
    break;

  case 81: /* arithmetic_expr_non_empty: POSINT  */
#line 1123 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = (yyvsp[0].str); }
#line 2694 "generated/tuc_transpiler.tab.c"
    break;

  case 82: /* arithmetic_expr_non_empty: REAL  */
#line 1124 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = (yyvsp[0].str); }
#line 2700 "generated/tuc_transpiler.tab.c"
    break;

  case 83: /* arithmetic_expr_non_empty: IDENTIFIER  */
#line 1125 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = (yyvsp[0].str); }
#line 2706 "generated/tuc_transpiler.tab.c"
    break;

  case 84: /* arithmetic_expr_non_empty: '#' IDENTIFIER  */
#line 1126 "phase_2/tuc_transpiler.y"
                                        { if(inside_comp == 0){yyerror("Not allowed outside comp");YYABORT;} (yyval.str) = (yyvsp[0].str); }
#line 2712 "generated/tuc_transpiler.tab.c"
    break;

  case 85: /* arithmetic_expr_non_empty: '(' expr ')'  */
#line 1127 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("(%s)", (yyvsp[-1].str)); }
#line 2718 "generated/tuc_transpiler.tab.c"
    break;

  case 86: /* arithmetic_expr_non_empty: expr EXPONENTIATION_OP expr  */
#line 1128 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("pow(%s, %s)", (yyvsp[-2].str), (yyvsp[0].str)); }
#line 2724 "generated/tuc_transpiler.tab.c"
    break;

  case 87: /* arithmetic_expr_non_empty: expr CALC_OP expr  */
#line 1129 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2730 "generated/tuc_transpiler.tab.c"
    break;

  case 88: /* arithmetic_expr_non_empty: expr RELATIONAL_OP expr  */
#line 1130 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2736 "generated/tuc_transpiler.tab.c"
    break;

  case 89: /* arithmetic_expr_non_empty: expr LOGICAL_AND_OR_OP expr  */
#line 1131 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2742 "generated/tuc_transpiler.tab.c"
    break;

  case 90: /* arithmetic_expr_non_empty: LOGICAL_NOT_OP expr  */
#line 1132 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("%s %s", (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2748 "generated/tuc_transpiler.tab.c"
    break;

  case 91: /* arithmetic_expr_non_empty: UNARY_OP expr  */
#line 1133 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("%s%s", (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2754 "generated/tuc_transpiler.tab.c"
    break;

  case 92: /* $@9: %empty  */
#line 1142 "phase_2/tuc_transpiler.y"
{
    indentation_level++;
    temp8 = template("void %s() {\n", (yyvsp[-3].str));
}
#line 2763 "generated/tuc_transpiler.tab.c"
    break;

  case 93: /* $@10: %empty  */
#line 1147 "phase_2/tuc_transpiler.y"
{      
    indented_stmts_4 = indent_statements((yyvsp[0].str), indentation_level);
    indentation_level--;
}
#line 2772 "generated/tuc_transpiler.tab.c"
    break;

  case 94: /* function_definition: KW_DEF KW_MAIN '(' ')' ':' $@9 body $@10 KW_ENDDEF  */
#line 1152 "phase_2/tuc_transpiler.y"
{     
    if (func_needs_return_type != body_has_return_type) {  
      yyerror("Return statement missing from body"); 
      YYABORT;
    }
    (yyval.str) = template("%s%s}\n", temp8, indented_stmts_4);
}
#line 2784 "generated/tuc_transpiler.tab.c"
    break;

  case 95: /* $@11: %empty  */
#line 1166 "phase_2/tuc_transpiler.y"
{
    add_function((yyvsp[-5].str), num_of_args_in_function);  
    num_of_args_in_function = 0;
    indentation_level++;
    temp8 = template("%s %s(%s){\n", (yyvsp[-1].str), (yyvsp[-5].str), (yyvsp[-3].str) );
}
#line 2795 "generated/tuc_transpiler.tab.c"
    break;

  case 96: /* $@12: %empty  */
#line 1173 "phase_2/tuc_transpiler.y"
{      
    indented_stmts_4 = indent_statements((yyvsp[0].str), indentation_level);
    indentation_level--;
}
#line 2804 "generated/tuc_transpiler.tab.c"
    break;

  case 97: /* function_definition: KW_DEF IDENTIFIER '(' arguments ')' return_type ':' $@11 body $@12 KW_ENDDEF  */
#line 1178 "phase_2/tuc_transpiler.y"
{     
    if (func_needs_return_type != body_has_return_type) {  
      yyerror("Return statement missing from body"); 
      YYABORT;
    }
    (yyval.str) = template("%s%s}\n", temp8, indented_stmts_4);
    func_needs_return_type = 0;
    body_has_return_type = 0;

}
#line 2819 "generated/tuc_transpiler.tab.c"
    break;

  case 98: /* arguments: %empty  */
#line 1193 "phase_2/tuc_transpiler.y"
                                            { (yyval.str) = ""; }
#line 2825 "generated/tuc_transpiler.tab.c"
    break;

  case 99: /* arguments: parameter_definition  */
#line 1194 "phase_2/tuc_transpiler.y"
                                            { num_of_args_in_function++; (yyval.str) = template("%s", (yyvsp[0].str)); }
#line 2831 "generated/tuc_transpiler.tab.c"
    break;

  case 100: /* arguments: arguments ',' parameter_definition  */
#line 1195 "phase_2/tuc_transpiler.y"
                                            { num_of_args_in_function++; (yyval.str) = template("%s, %s", (yyvsp[-2].str), (yyvsp[0].str)); }
#line 2837 "generated/tuc_transpiler.tab.c"
    break;

  case 101: /* parameter_definition: IDENTIFIER ':' KW_VARIABLE_TYPE  */
#line 1199 "phase_2/tuc_transpiler.y"
                                                                { (yyval.str) = template("%s %s", (yyvsp[0].str), (yyvsp[-2].str)); }
#line 2843 "generated/tuc_transpiler.tab.c"
    break;

  case 102: /* parameter_definition: IDENTIFIER '[' POSINT ']' ':' KW_VARIABLE_TYPE  */
#line 1200 "phase_2/tuc_transpiler.y"
                                                                { (yyval.str) = template("%s[%s]: %s", (yyvsp[-5].str), (yyvsp[-3].str), (yyvsp[0].str) ); }
#line 2849 "generated/tuc_transpiler.tab.c"
    break;

  case 103: /* parameter_definition: IDENTIFIER '[' ']' ':' KW_VARIABLE_TYPE  */
#line 1201 "phase_2/tuc_transpiler.y"
                                                                { (yyval.str) = template("%s[]: %s", (yyvsp[-4].str), (yyvsp[0].str) ); }
#line 2855 "generated/tuc_transpiler.tab.c"
    break;

  case 104: /* return_type: %empty  */
#line 1205 "phase_2/tuc_transpiler.y"
          { (yyval.str) = ""; }
#line 2861 "generated/tuc_transpiler.tab.c"
    break;

  case 105: /* return_type: KW_FUNC_RETURN KW_VARIABLE_TYPE  */
#line 1206 "phase_2/tuc_transpiler.y"
                                   { func_needs_return_type = 1; (yyval.str) = template("%s", (yyvsp[0].str)); }
#line 2867 "generated/tuc_transpiler.tab.c"
    break;

  case 106: /* body: stmts  */
#line 1210 "phase_2/tuc_transpiler.y"
                            { body_has_return_type = 0; (yyval.str) = template("%s", (yyvsp[0].str)); }
#line 2873 "generated/tuc_transpiler.tab.c"
    break;

  case 107: /* body: stmts KW_RETURN expr ';'  */
#line 1211 "phase_2/tuc_transpiler.y"
                            { body_has_return_type = 1; (yyval.str) = template("%s%s %s;\n", (yyvsp[-3].str), (yyvsp[-2].str) , (yyvsp[-1].str)); }
#line 2879 "generated/tuc_transpiler.tab.c"
    break;

  case 108: /* variable_definition: variable_list ':' KW_VARIABLE_TYPE  */
#line 1220 "phase_2/tuc_transpiler.y"
   {  
        if(is_comp_type((yyvsp[0].str))){
            create_compTypeVariables((yyvsp[0].str), (yyvsp[-2].str));
            (yyval.str) = template("%s %s = ctor_%s;", (yyvsp[0].str), (yyvsp[-2].str), (yyvsp[0].str));        
        } else { (yyval.str) = template("%s %s;", (yyvsp[0].str), (yyvsp[-2].str)); }   
   }
#line 2890 "generated/tuc_transpiler.tab.c"
    break;

  case 109: /* variable_definition: IDENTIFIER '[' POSINT ']' ':' KW_VARIABLE_TYPE  */
#line 1226 "phase_2/tuc_transpiler.y"
                                                                        { (yyval.str) = template("%s %s[%s];", (yyvsp[0].str), (yyvsp[-5].str), (yyvsp[-3].str) ); }
#line 2896 "generated/tuc_transpiler.tab.c"
    break;

  case 110: /* variable_definition: IDENTIFIER '[' ']' ':' KW_VARIABLE_TYPE  */
#line 1227 "phase_2/tuc_transpiler.y"
                                                                        { (yyval.str) = template("%s[]: %s;", (yyvsp[-4].str), (yyvsp[0].str) ); }
#line 2902 "generated/tuc_transpiler.tab.c"
    break;

  case 111: /* variable_definition: KW_CONST IDENTIFIER ASSIGNMENT_OP VALUE ':' KW_VARIABLE_TYPE  */
#line 1228 "phase_2/tuc_transpiler.y"
                                                                        { (yyval.str) = template("%s %s %s = %s;", (yyvsp[-5].str), (yyvsp[0].str), (yyvsp[-4].str), (yyvsp[-2].str)); }
#line 2908 "generated/tuc_transpiler.tab.c"
    break;

  case 116: /* KW_VARIABLE_TYPE: KW_INTEGER  */
#line 1239 "phase_2/tuc_transpiler.y"
                        {(yyval.str) = (yyvsp[0].str); }
#line 2914 "generated/tuc_transpiler.tab.c"
    break;

  case 119: /* KW_VARIABLE_TYPE: IDENTIFIER  */
#line 1242 "phase_2/tuc_transpiler.y"
               {
    if (is_comp_type((yyvsp[0].str))) {
        (yyval.str) = template("%s", (yyvsp[0].str));
    } else {
        yyerror("Unknown variable type");
        YYABORT;
    }
  }
#line 2927 "generated/tuc_transpiler.tab.c"
    break;

  case 122: /* variable_list: IDENTIFIER  */
#line 1258 "phase_2/tuc_transpiler.y"
                                    { (yyval.str) = template("%s", (yyvsp[0].str)); }
#line 2933 "generated/tuc_transpiler.tab.c"
    break;

  case 123: /* variable_list: variable_list ',' IDENTIFIER  */
#line 1259 "phase_2/tuc_transpiler.y"
                                    { (yyval.str) = template("%s, %s", (yyvsp[-2].str), (yyvsp[0].str)); }
#line 2939 "generated/tuc_transpiler.tab.c"
    break;


#line 2943 "generated/tuc_transpiler.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 1265 "phase_2/tuc_transpiler.y"

    void print_evaluation(char* result) {

        if (yyerror_count == 0) {
            printf("===================================================== Expression evaluates to =====================================================\n%s\n", result);
            
            FILE *file = fopen("output.c", "w");
            if (file != NULL) {
                
                fprintf(file, "#include <stdio.h>\n");
                fprintf(file, "#include <stdlib.h>\n");  
                fprintf(file, "#include <math.h>\n");

                if(used_lambdalib_function == 1){
                    fprintf(file, "#include \"lambdalib.h\"\n");
                }
                
                fprintf(file, "%s\n", result);
                fclose(file);
            } else {
                fprintf(stderr, "Error opening output.c for writing.\n");
            }
        }
    }

//==============================================================================================================================================================

bool is_integer(const char* str) {
    if (str == NULL || *str == '\0') return false;
    if (*str == '-' || *str == '+') {
        str++;
    }
    while (*str) {
        if (!isdigit((unsigned char)*str)) {
            return false;
        }
        str++;
    }
    return true;
}

//==============================================================================================================================================================
//==============================================================================================================================================================

int main () {

    char buffer[BUFFER_SIZE];  
    //  yydebug = 1; // Enable debug output

    // initialize_predefined_functions();
    include_functions();
    
    FILE *file = fopen("test1.in", "r");
    if (file == NULL) {
        perror("Could not open file");
        return 1;
    }

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        store_line(buffer);
    }

    if (yyparse() == 0 && parse_error == 0) {
        printf("Your program is syntactically correct!\n");
    } else {
        printf("Rejected\n");
    }

    free_functions();
    free_lines();


    return 0;
}
