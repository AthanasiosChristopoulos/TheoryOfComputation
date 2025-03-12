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

    while (*ptr != '\0') {  // Continue until the end of the code
        // Find "double " or "void " and handle each separately
        ptr = strstr(ptr, "double ");
        if (ptr != NULL) {
            ptr += 6;  // Skip past "double "
        } else {
            ptr = strstr(ptr, "void ");
            if (ptr != NULL) {
                ptr += 4;  // Skip past "void "
            }
        }

        // Skip whitespace after type
        while (*ptr && isspace(*ptr)) {
            ptr++;
        }

        // Extract function name
        char name[MAX_NAME_LENGTH] = {0};
        int i = 0;

        while (*ptr && (isalnum(*ptr) || *ptr == '_')) { // Read function name
            name[i++] = *ptr++;
            if (i >= MAX_NAME_LENGTH - 1) break;  // Prevent buffer overflow
        }
        name[i] = '\0';

        // If a valid function name, store it
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

//==========================================================================================================================
// comp_upper:

int inside_comp= 0;

#define MAX_FUNCTIONS 100

typedef struct {
    char* comp_name;
    char* function_list[MAX_FUNCTIONS];
    int function_count;
} Component;

Component* components = NULL;
size_t num_components = 0;

int is_identifier_in_list(const char* comp_name) {

    for (size_t i = 0; i < num_components; i++) {
        if (strcmp(components[i].comp_name, comp_name) == 0) {
            return 1;  // Component is defined
        }
    }
    return 0;  
}

//=========================================================================================================================
// Add Comp as Type

void add_identifier(const char* comp_name) {
    if (is_identifier_in_list(comp_name)) {
        fprintf(stderr, "Component %s is already defined.\n", comp_name);
        return;
    }

    components = realloc(components, (num_components + 1) * sizeof(Component));
    if (!components) {
        fprintf(stderr, "Memory allocation failed for components\n");
        exit(1);
    }

    Component* new_comp = &components[num_components];
    new_comp->comp_name = strdup(comp_name);
    new_comp->function_count = 0;
    num_components++;
}

void add_function_to_component(const char* comp_name, const char* func_name) {
    for (size_t i = 0; i < num_components; i++) {
        if (strcmp(components[i].comp_name, comp_name) == 0) {
            if (components[i].function_count < MAX_FUNCTIONS) {
                components[i].function_list[components[i].function_count] = strdup(func_name);
                components[i].function_count++;
            } else {
                fprintf(stderr, "Function list is full for component %s.\n", comp_name);
            }
            return;
        }
    }
    fprintf(stderr, "Component %s not found.\n", comp_name);
}


//=========================================================================================================================

const Component* find_component(const char* comp_name) {
    for (size_t i = 0; i < num_components; i++) {
        if (strcmp(components[i].comp_name, comp_name) == 0) {
            return &components[i];
        }
    }
    return NULL;
}

//=========================================================================================================================

void free_components() {
    for (size_t i = 0; i < num_components; i++) {
        free(components[i].comp_name);
        for (int j = 0; j < components[i].function_count; j++) {
            free(components[i].function_list[j]);
        }
    }
    free(components);
}

//=========================================================================================================================
// comp_follow:

typedef struct FunctionNode {
    char *function_comp;
    char *function_name;
    struct FunctionNode *next;
} FunctionNode;

typedef struct CompFunctionMap {
    char *comp_name;
    FunctionNode *functions;  
    struct CompFunctionMap *next;
} CompFunctionMap;

#define HASH_TABLE_SIZE 100

CompFunctionMap *compFunctionTable[HASH_TABLE_SIZE] = { NULL };

unsigned int hash(const char *str) {
    unsigned int hash = 0;
    while (*str) hash = (hash * 31) + *str++;
    return hash % HASH_TABLE_SIZE;
}

void addFunctionToComp(const char *comp_name, const char *function_follow_comp, const char *func_name) {

    unsigned int index = hash(comp_name);

    CompFunctionMap *entry = compFunctionTable[index];
    while (entry) {
        if (strcmp(entry->comp_name, comp_name) == 0) {
            // Component exists, append function to its list
            FunctionNode *newNode = (FunctionNode *)malloc(sizeof(FunctionNode));
            newNode->function_comp = strdup(function_follow_comp);
            newNode->function_name = strdup(func_name);  // Store the function name
            newNode->next = entry->functions;
            entry->functions = newNode;
            return;
        }
        entry = entry->next;
    }

    // Component not found, create a new entry
    entry = (CompFunctionMap *)malloc(sizeof(CompFunctionMap));
    entry->comp_name = strdup(comp_name);
    entry->functions = (FunctionNode *)malloc(sizeof(FunctionNode));
    entry->functions->function_comp = strdup(function_follow_comp);
    entry->functions->function_name = strdup(func_name);  // Store the function name
    entry->functions->next = NULL;
    entry->next = compFunctionTable[index];
    compFunctionTable[index] = entry;
}

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
                strcat(result, " = .");
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

//=========================================================================================================================

char* list_of_comp_functions[MAX_FUNCTIONS];
int function_list_count = 0;

void append_to_function_list(const char* func_name) {

    if (function_list_count < MAX_FUNCTIONS) {
        list_of_comp_functions[function_list_count] = strdup(func_name);
        if (!list_of_comp_functions[function_list_count]) {
            fprintf(stderr, "Memory allocation failed for function name.\n");
            exit(1);
        }
        function_list_count++;
    } else {
        fprintf(stderr, "Function list is full.\n");
    }
}
void clear_function_list() {
    for (int i = 0; i < function_list_count; i++) {
        free(list_of_comp_functions[i]);
        list_of_comp_functions[i] = NULL;
    }
    function_list_count = 0;
}

//==========================================================================================================================
//==========================================================================================================================

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


#line 697 "generated/tuc_transpiler.tab.c"

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
  YYSYMBOL_61_ = 61,                       /* ')'  */
  YYSYMBOL_62_ = 62,                       /* ']'  */
  YYSYMBOL_63_ = 63,                       /* '('  */
  YYSYMBOL_64_ = 64,                       /* '['  */
  YYSYMBOL_65_ = 65,                       /* ';'  */
  YYSYMBOL_66_ = 66,                       /* ':'  */
  YYSYMBOL_67_ = 67,                       /* '#'  */
  YYSYMBOL_68_ = 68,                       /* ','  */
  YYSYMBOL_YYACCEPT = 69,                  /* $accept  */
  YYSYMBOL_program = 70,                   /* program  */
  YYSYMBOL_input = 71,                     /* input  */
  YYSYMBOL_macro = 72,                     /* macro  */
  YYSYMBOL_commands = 73,                  /* commands  */
  YYSYMBOL_74_1 = 74,                      /* $@1  */
  YYSYMBOL_75_2 = 75,                      /* $@2  */
  YYSYMBOL_76_3 = 76,                      /* $@3  */
  YYSYMBOL_77_4 = 77,                      /* $@4  */
  YYSYMBOL_78_5 = 78,                      /* $@5  */
  YYSYMBOL_79_6 = 79,                      /* $@6  */
  YYSYMBOL_else_statement = 80,            /* else_statement  */
  YYSYMBOL_81_7 = 81,                      /* $@7  */
  YYSYMBOL_step = 82,                      /* step  */
  YYSYMBOL_stmts = 83,                     /* stmts  */
  YYSYMBOL_loop_thing = 84,                /* loop_thing  */
  YYSYMBOL_complicated_types = 85,         /* complicated_types  */
  YYSYMBOL_86_8 = 86,                      /* $@8  */
  YYSYMBOL_declaration_comp = 87,          /* declaration_comp  */
  YYSYMBOL_variable_definition_comp = 88,  /* variable_definition_comp  */
  YYSYMBOL_variable_list_comp = 89,        /* variable_list_comp  */
  YYSYMBOL_body_comp = 90,                 /* body_comp  */
  YYSYMBOL_function_definition_comp = 91,  /* function_definition_comp  */
  YYSYMBOL_return_type_comp = 92,          /* return_type_comp  */
  YYSYMBOL_func_call = 93,                 /* func_call  */
  YYSYMBOL_func_args = 94,                 /* func_args  */
  YYSYMBOL_expr = 95,                      /* expr  */
  YYSYMBOL_arithmetic_expr_non_empty = 96, /* arithmetic_expr_non_empty  */
  YYSYMBOL_function_definition = 97,       /* function_definition  */
  YYSYMBOL_98_9 = 98,                      /* $@9  */
  YYSYMBOL_99_10 = 99,                     /* $@10  */
  YYSYMBOL_100_11 = 100,                   /* $@11  */
  YYSYMBOL_101_12 = 101,                   /* $@12  */
  YYSYMBOL_arguments = 102,                /* arguments  */
  YYSYMBOL_parameter_definition = 103,     /* parameter_definition  */
  YYSYMBOL_return_type = 104,              /* return_type  */
  YYSYMBOL_body = 105,                     /* body  */
  YYSYMBOL_variable_definition = 106,      /* variable_definition  */
  YYSYMBOL_VALUE = 107,                    /* VALUE  */
  YYSYMBOL_KW_VARIABLE_TYPE = 108,         /* KW_VARIABLE_TYPE  */
  YYSYMBOL_NUMBERS = 109,                  /* NUMBERS  */
  YYSYMBOL_variable_list = 110             /* variable_list  */
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
typedef yytype_uint8 yy_state_t;

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
#define YYLAST   405

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  69
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  42
/* YYNRULES -- Number of rules.  */
#define YYNRULES  113
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  251

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
       2,     2,     2,     2,     2,    67,     2,     2,     2,     2,
      63,    61,     2,     2,    68,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    66,    65,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    64,     2,    62,     2,     2,     2,     2,     2,     2,
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
       0,   696,   696,   699,   701,   702,   703,   704,   705,   706,
     713,   720,   733,   735,   733,   745,   750,   744,   763,   773,
     762,   786,   800,   823,   824,   824,   833,   834,   842,   843,
     850,   851,   856,   859,   859,   866,   866,   887,   888,   892,
     896,   897,   904,   905,   906,   907,   912,   932,   933,   941,
     958,   959,   960,   961,   962,   970,   971,   972,   973,   974,
     975,   976,   977,   978,   979,   980,   981,   982,   983,   992,
    1001,  1010,  1022,  1023,  1024,  1025,  1026,  1027,  1028,  1029,
    1030,  1031,  1040,  1045,  1039,  1064,  1071,  1063,  1091,  1092,
    1093,  1097,  1098,  1099,  1103,  1104,  1108,  1109,  1117,  1124,
    1125,  1126,  1130,  1131,  1132,  1136,  1137,  1138,  1139,  1140,
    1151,  1152,  1156,  1157
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
  "KW_EXPONENT", "')'", "']'", "'('", "'['", "';'", "':'", "'#'", "','",
  "$accept", "program", "input", "macro", "commands", "$@1", "$@2", "$@3",
  "$@4", "$@5", "$@6", "else_statement", "$@7", "step", "stmts",
  "loop_thing", "complicated_types", "$@8", "declaration_comp",
  "variable_definition_comp", "variable_list_comp", "body_comp",
  "function_definition_comp", "return_type_comp", "func_call", "func_args",
  "expr", "arithmetic_expr_non_empty", "function_definition", "$@9",
  "$@10", "$@11", "$@12", "arguments", "parameter_definition",
  "return_type", "body", "variable_definition", "VALUE",
  "KW_VARIABLE_TYPE", "NUMBERS", "variable_list", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-140)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-113)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -140,     9,   117,  -140,   228,  -140,  -140,   -42,   -32,    29,
       4,    46,    61,    62,   254,   254,   254,    75,  -140,    26,
      31,  -140,   265,    49,    81,  -140,   -58,    64,   207,   213,
     254,   254,   118,    72,    88,  -140,   226,   111,   -40,   -22,
     115,   321,   -34,  -140,  -140,   254,   254,   254,   254,  -140,
    -140,  -140,    77,   103,   254,  -140,  -140,   -28,   352,   101,
     105,   305,   329,   336,   113,   172,   120,   116,  -140,   352,
      43,   254,   254,  -140,   254,   254,   352,   -22,   115,  -140,
    -140,  -140,  -140,  -140,  -140,  -140,  -140,   244,  -140,   236,
     121,    77,   138,   127,   128,   259,    70,    41,  -140,   135,
    -140,  -140,  -140,  -140,   137,  -140,   312,   205,  -140,   352,
      77,  -140,   254,  -140,  -140,   143,     0,    15,   254,   254,
     254,   359,   147,    -1,    77,   185,   172,  -140,   -25,    77,
     170,   155,  -140,  -140,  -140,  -140,    42,   -27,   344,   254,
     254,   254,   254,   259,   160,   166,  -140,    77,   168,  -140,
    -140,   254,   223,   162,   104,    12,  -140,   254,    54,   182,
     182,    86,   352,   100,    32,    95,   169,   171,    77,  -140,
    -140,   161,  -140,   271,  -140,  -140,    77,   175,   233,  -140,
     173,   285,  -140,   189,   241,  -140,  -140,   179,   238,   188,
     293,   190,   246,   259,   202,    77,  -140,  -140,   254,   245,
    -140,  -140,   262,   208,  -140,  -140,   212,   278,  -140,   216,
     258,  -140,  -140,  -140,  -140,  -140,   219,  -140,  -140,   299,
    -140,  -140,   172,    77,   275,  -140,  -140,  -140,   263,  -140,
      65,  -140,   294,  -140,  -140,  -140,   268,   250,   182,   182,
      77,   240,   242,   298,  -140,   287,    77,  -140,    56,  -140,
    -140
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       3,     0,     2,     1,    59,   110,   111,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     9,     0,
       0,    55,     0,     0,     0,    56,     0,     0,    50,     0,
       0,     0,     0,     0,     0,    35,     0,     0,    59,    67,
      66,     0,    57,     8,     7,     0,     0,     0,     0,     5,
       6,     4,     0,     0,     0,    68,    52,     0,    51,   110,
       0,     0,     0,     0,     0,    88,     0,     0,    10,    11,
       0,     0,     0,    61,     0,     0,    63,    62,    64,    65,
     109,   106,   105,   107,   108,    98,   113,     0,    49,     0,
       0,     0,    60,     0,     0,     0,     0,     0,    89,     0,
      37,   102,   104,   103,     0,    70,     0,     0,    54,    53,
       0,   100,     0,    12,    15,    59,   110,   111,     0,     0,
       0,     0,     0,     0,     0,    94,     0,    82,    42,     0,
      58,     0,    99,    69,    28,    28,    67,    66,     0,     0,
       0,     0,     0,     0,     0,     0,    91,     0,     0,    90,
      28,     0,     0,     0,     0,     0,   101,     0,     0,    13,
      16,    61,    76,    62,    64,    65,    26,     0,     0,    95,
      85,    96,    83,     0,    40,    38,     0,     0,     0,    36,
       0,     0,    71,     0,     0,    33,    34,     0,    23,     0,
       0,     0,     0,     0,     0,     0,    93,    28,     0,     0,
      45,    39,     0,     0,    43,    44,     0,     0,    32,     0,
       0,    29,    31,    30,    17,    27,     0,    92,    86,     0,
      84,    41,    88,     0,     0,    24,    14,    18,     0,    97,
       0,    21,     0,    28,    28,    87,    47,     0,    25,    19,
       0,     0,     0,     0,    48,    42,     0,    20,     0,    22,
      46
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -140,  -140,  -140,  -140,   314,  -140,  -140,  -140,  -140,  -140,
    -140,  -140,  -140,  -140,  -133,  -140,  -140,  -140,  -140,  -140,
    -140,    74,  -140,  -140,  -140,  -140,    -2,  -139,  -140,  -140,
    -140,  -140,  -140,    98,   197,  -140,   131,   332,  -140,   -73,
    -140,  -140
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     1,     2,    18,   187,   134,   188,   135,   192,   234,
     243,   210,   233,   194,   171,   189,    20,    67,   128,   153,
     154,   155,   180,   241,    21,    57,   190,   122,    23,   150,
     199,   197,   228,    97,    98,   148,   172,   191,   104,    85,
      25,    26
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      22,   159,   160,   144,   166,    71,   151,    33,    52,     3,
      53,    74,    39,    40,    41,    38,     5,    48,   111,     6,
      47,    30,    48,    28,    72,    55,    58,    61,    62,    63,
      75,    31,    32,    88,    69,   -80,    34,   132,   178,   -80,
      89,   179,   152,    76,    77,    78,    79,   101,   102,    35,
     103,   146,    87,    14,   215,    15,   156,    80,   183,    38,
       5,   145,   -72,     6,    36,    37,   -72,    38,     5,    55,
      61,     6,   105,   106,   169,    16,    48,   -73,    42,    17,
      80,   -73,   178,   250,    47,   184,    48,   109,    81,    82,
      83,    43,    84,   121,   -78,   196,    44,    14,   -78,    15,
     238,   239,   125,   201,   -81,    14,    86,    15,   -81,   126,
     133,    81,    82,    83,    50,    84,   136,   137,   138,    16,
       4,     5,   217,    17,     6,     7,   236,    16,    54,    64,
       8,    17,     9,   126,   123,    65,   124,   162,   163,   164,
     165,   121,    47,    10,    48,    11,    51,    12,   -75,   173,
     231,    66,   -75,   181,    13,   182,    70,   -79,    14,    48,
      15,   -79,   -77,    90,     4,     5,   -77,   244,     6,     7,
     176,    91,   177,   249,     8,    96,     9,    95,   185,   186,
      16,    99,   100,   112,    17,     4,     5,   110,    71,     6,
       7,   121,   198,   113,   114,     8,   219,     9,    13,   185,
     186,   127,    14,   129,    15,   -74,    28,    72,   131,   -74,
      38,     5,    56,   143,     6,   157,    38,    59,   147,    13,
       6,   158,   167,    14,    16,    15,   174,   175,    17,    38,
       5,    68,   168,     6,   170,   193,   203,   195,   204,    38,
       5,   108,   202,     6,   208,    16,   181,   209,    14,    17,
      15,   206,   207,   211,    14,   213,    15,    38,     5,   107,
     214,     6,   115,   116,   216,   221,   117,    14,   226,    15,
      16,   222,   220,    27,    17,    60,    16,    14,   223,    15,
      17,   224,   225,    45,    46,   227,    47,   232,    48,    16,
     235,    28,    29,    17,  -112,    14,  -112,    15,   237,    16,
     118,   240,   119,    17,    45,    46,   245,    47,   246,    48,
      45,    46,   242,    47,   247,    48,    19,    16,   151,   248,
     230,    17,   120,   149,    45,    46,    17,    47,   218,    48,
      49,     0,    45,    46,    24,    47,   200,    48,    45,    46,
       0,    47,     0,    48,    45,    46,     0,    47,     0,    48,
     205,    45,    46,     0,    47,     0,    48,     0,   212,     0,
      45,    46,     0,    47,   229,    48,     0,    92,    45,    46,
       0,    47,     0,    48,   130,    45,    46,     0,    47,     0,
      48,     0,    73,    45,    46,     0,    47,     0,    48,     0,
      93,    45,    46,     0,    47,     0,    48,    94,   139,   140,
       0,   141,     0,   142,     0,   161
};

static const yytype_int16 yycheck[] =
{
       2,   134,   135,     4,   143,    45,    31,     3,    66,     0,
      68,    45,    14,    15,    16,     3,     4,    44,    91,     7,
      42,    63,    44,    63,    64,    27,    28,    29,    30,    31,
      64,    63,     3,    61,    36,    62,    32,   110,    26,    66,
      68,    29,    67,    45,    46,    47,    48,     4,     5,     3,
       7,   124,    54,    41,   193,    43,   129,     3,     4,     3,
       4,    62,    62,     7,     3,     3,    66,     3,     4,    71,
      72,     7,    74,    75,   147,    63,    44,    62,     3,    67,
       3,    66,    26,    27,    42,   158,    44,    89,    34,    35,
      36,    65,    38,    95,    62,   168,    65,    41,    66,    43,
     233,   234,    61,   176,    62,    41,     3,    43,    66,    68,
     112,    34,    35,    36,    65,    38,   118,   119,   120,    63,
       3,     4,   195,    67,     7,     8,    61,    63,    64,    11,
      13,    67,    15,    68,    64,    63,    66,   139,   140,   141,
     142,   143,    42,    26,    44,    28,    65,    30,    62,   151,
     223,    63,    66,   155,    37,   157,    45,    62,    41,    44,
      43,    66,    62,    62,     3,     4,    66,   240,     7,     8,
      66,    66,    68,   246,    13,     3,    15,    64,    17,    18,
      63,    61,    66,    45,    67,     3,     4,    66,    45,     7,
       8,   193,    31,    66,    66,    13,   198,    15,    37,    17,
      18,    66,    41,    66,    43,    62,    63,    64,     3,    66,
       3,     4,     5,    66,     7,    45,     3,     4,    33,    37,
       7,    66,    62,    41,    63,    43,     3,    65,    67,     3,
       4,     5,    66,     7,    66,    66,     3,    66,    65,     3,
       4,     5,    67,     7,    65,    63,   248,     9,    41,    67,
      43,    62,    11,    65,    41,    65,    43,     3,     4,    15,
      14,     7,     3,     4,    62,     3,     7,    41,    10,    43,
      63,    63,    27,    45,    67,    62,    63,    41,    66,    43,
      67,     3,    66,    39,    40,    66,    42,    12,    44,    63,
      27,    63,    64,    67,    66,    41,    68,    43,     4,    63,
      41,    33,    43,    67,    39,    40,    66,    42,    66,    44,
      39,    40,    62,    42,    16,    44,     2,    63,    31,   245,
     222,    67,    63,   126,    39,    40,    67,    42,   197,    44,
      65,    -1,    39,    40,     2,    42,    65,    44,    39,    40,
      -1,    42,    -1,    44,    39,    40,    -1,    42,    -1,    44,
      65,    39,    40,    -1,    42,    -1,    44,    -1,    65,    -1,
      39,    40,    -1,    42,    65,    44,    -1,    62,    39,    40,
      -1,    42,    -1,    44,    62,    39,    40,    -1,    42,    -1,
      44,    -1,    61,    39,    40,    -1,    42,    -1,    44,    -1,
      61,    39,    40,    -1,    42,    -1,    44,    61,    39,    40,
      -1,    42,    -1,    44,    -1,    61
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    70,    71,     0,     3,     4,     7,     8,    13,    15,
      26,    28,    30,    37,    41,    43,    63,    67,    72,    73,
      85,    93,    95,    97,   106,   109,   110,    45,    63,    64,
      63,    63,     3,     3,    32,     3,     3,     3,     3,    95,
      95,    95,     3,    65,    65,    39,    40,    42,    44,    65,
      65,    65,    66,    68,    64,    95,     5,    94,    95,     4,
      62,    95,    95,    95,    11,    63,    63,    86,     5,    95,
      45,    45,    64,    61,    45,    64,    95,    95,    95,    95,
       3,    34,    35,    36,    38,   108,     3,    95,    61,    68,
      62,    66,    62,    61,    61,    64,     3,   102,   103,    61,
      66,     4,     5,     7,   107,    95,    95,    15,     5,    95,
      66,   108,    45,    66,    66,     3,     4,     7,    41,    43,
      63,    95,    96,    64,    66,    61,    68,    66,    87,    66,
      62,     3,   108,    95,    74,    76,    95,    95,    95,    39,
      40,    42,    44,    66,     4,    62,   108,    33,   104,   103,
      98,    31,    67,    88,    89,    90,   108,    45,    66,    83,
      83,    61,    95,    95,    95,    95,    96,    62,    66,   108,
      66,    83,   105,    95,     3,    65,    66,    68,    26,    29,
      91,    95,    95,     4,   108,    17,    18,    73,    75,    84,
      95,   106,    77,    66,    82,    66,   108,   100,    31,    99,
      65,   108,    67,     3,    65,    65,    62,    11,    65,     9,
      80,    65,    65,    65,    14,    96,    62,   108,   105,    95,
      27,     3,    63,    66,     3,    66,    10,    66,   101,    65,
     102,   108,    12,    81,    78,    27,    61,     4,    83,    83,
      33,    92,    62,    79,   108,    66,    66,    16,    90,   108,
      27
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    69,    70,    71,    71,    71,    71,    71,    71,    71,
      72,    72,    74,    75,    73,    76,    77,    73,    78,    79,
      73,    73,    73,    80,    81,    80,    82,    82,    83,    83,
      83,    83,    83,    84,    84,    86,    85,    87,    87,    88,
      89,    89,    90,    90,    90,    90,    91,    92,    92,    93,
      94,    94,    94,    94,    94,    95,    95,    95,    95,    95,
      95,    95,    95,    95,    95,    95,    95,    95,    95,    95,
      95,    95,    96,    96,    96,    96,    96,    96,    96,    96,
      96,    96,    98,    99,    97,   100,   101,    97,   102,   102,
     102,   103,   103,   103,   104,   104,   105,   105,   106,   106,
     106,   106,   107,   107,   107,   108,   108,   108,   108,   108,
     109,   109,   110,   110
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     0,     3,     3,     3,     3,     3,     2,
       3,     3,     0,     0,    10,     0,     0,     9,     0,     0,
      14,    11,    15,     0,     0,     4,     0,     2,     0,     3,
       3,     3,     3,     1,     1,     0,     7,     0,     3,     3,
       2,     4,     0,     3,     3,     3,     9,     0,     2,     4,
       0,     1,     1,     3,     3,     1,     1,     2,     5,     1,
       4,     3,     3,     3,     3,     3,     2,     2,     3,     6,
       4,     7,     1,     1,     1,     3,     3,     3,     3,     3,
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
#line 696 "phase_2/tuc_transpiler.y"
        { print_evaluation((yyvsp[0].str)); }
#line 2001 "generated/tuc_transpiler.tab.c"
    break;

  case 3: /* input: %empty  */
#line 699 "phase_2/tuc_transpiler.y"
                                                  {(yyval.str) = "";}
#line 2007 "generated/tuc_transpiler.tab.c"
    break;

  case 4: /* input: input variable_definition ';'  */
#line 701 "phase_2/tuc_transpiler.y"
                                                  { (yyval.str) = template("%s%s\n", (yyvsp[-2].str), (yyvsp[-1].str));}
#line 2013 "generated/tuc_transpiler.tab.c"
    break;

  case 5: /* input: input expr ';'  */
#line 702 "phase_2/tuc_transpiler.y"
                                                  { assignment_made = 0;  (yyval.str) = template("%s%s;\n", (yyvsp[-2].str), (yyvsp[-1].str));}
#line 2019 "generated/tuc_transpiler.tab.c"
    break;

  case 6: /* input: input function_definition ';'  */
#line 703 "phase_2/tuc_transpiler.y"
                                                  { assignment_made = 0; (yyval.str) = template("%s%s", (yyvsp[-2].str), (yyvsp[-1].str));}
#line 2025 "generated/tuc_transpiler.tab.c"
    break;

  case 7: /* input: input complicated_types ';'  */
#line 704 "phase_2/tuc_transpiler.y"
                                                  { assignment_made = 0;  (yyval.str) = template("%s%s", (yyvsp[-2].str), (yyvsp[-1].str));}
#line 2031 "generated/tuc_transpiler.tab.c"
    break;

  case 8: /* input: input commands ';'  */
#line 705 "phase_2/tuc_transpiler.y"
                                                  {  (yyval.str) = template("%s%s", (yyvsp[-2].str), (yyvsp[-1].str));}
#line 2037 "generated/tuc_transpiler.tab.c"
    break;

  case 9: /* input: input macro  */
#line 706 "phase_2/tuc_transpiler.y"
                                                  {  (yyval.str) = template("%s%s", (yyvsp[-1].str), (yyvsp[0].str));}
#line 2043 "generated/tuc_transpiler.tab.c"
    break;

  case 10: /* macro: KW_MACRO IDENTIFIER STRING  */
#line 713 "phase_2/tuc_transpiler.y"
                               {
        char *id = (yyvsp[-1].str); 
        char *value = (yyvsp[0].str);
        
        add_macro(id, value);  

    }
#line 2055 "generated/tuc_transpiler.tab.c"
    break;

  case 11: /* macro: KW_MACRO IDENTIFIER expr  */
#line 720 "phase_2/tuc_transpiler.y"
                             {
        char *id = (yyvsp[-1].str);  
        char *value = (yyvsp[0].str); 

        add_macro(id, value);
    }
#line 2066 "generated/tuc_transpiler.tab.c"
    break;

  case 12: /* $@1: %empty  */
#line 733 "phase_2/tuc_transpiler.y"
                           {temp1 = template("%s (%s):\n", (yyvsp[-4].str), (yyvsp[-2].str)); indentation_level++; is_a_loop = 0;}
#line 2072 "generated/tuc_transpiler.tab.c"
    break;

  case 13: /* $@2: %empty  */
#line 735 "phase_2/tuc_transpiler.y"
    {
        indented_stmts = indent_statements((yyvsp[0].str), indentation_level);
        indentation_level--;
    }
#line 2081 "generated/tuc_transpiler.tab.c"
    break;

  case 14: /* commands: KW_IF '(' expr ')' ':' $@1 stmts $@2 else_statement KW_ENDIF  */
#line 739 "phase_2/tuc_transpiler.y"
                            {
        temp3 = template("%s%s", (yyvsp[-1].str), (yyvsp[0].str)); 
        (yyval.str) = template("%s%s%s;\n", temp1, indented_stmts, temp3);
    }
#line 2090 "generated/tuc_transpiler.tab.c"
    break;

  case 15: /* $@3: %empty  */
#line 745 "phase_2/tuc_transpiler.y"
    {
        temp7 = template("%s (%s):\n", (yyvsp[-4].str), (yyvsp[-2].str));
        indentation_level++; is_a_loop = 1;
    }
#line 2099 "generated/tuc_transpiler.tab.c"
    break;

  case 16: /* $@4: %empty  */
#line 750 "phase_2/tuc_transpiler.y"
    {
        indented_stmts_3 = indent_statements((yyvsp[0].str), indentation_level);
        indentation_level--;
    }
#line 2108 "generated/tuc_transpiler.tab.c"
    break;

  case 17: /* commands: KW_WHILE '(' expr ')' ':' $@3 stmts $@4 KW_ENDWHILE  */
#line 754 "phase_2/tuc_transpiler.y"
                {
        (yyval.str) = template("%s%s%s;\n", temp7, indented_stmts_3, (yyvsp[0].str));
        is_a_loop = 0;
    }
#line 2117 "generated/tuc_transpiler.tab.c"
    break;

  case 18: /* $@5: %empty  */
#line 763 "phase_2/tuc_transpiler.y"
    {
        if(for_has_step == 1){
            temp5 = template("for (int %s = %s; %s < %s; %s++) {\n", (yyvsp[-8].str), (yyvsp[-5].str), (yyvsp[-8].str), (yyvsp[-3].str), (yyvsp[-2].str), (yyvsp[-8].str)); 
        } else {
            temp5 = template("for (int %s = %s; %s < %s; %s++) {\n", (yyvsp[-8].str), (yyvsp[-5].str), (yyvsp[-8].str), (yyvsp[-3].str), (yyvsp[-8].str)); 
        }
         
        indentation_level++; is_a_loop = 1;
    }
#line 2131 "generated/tuc_transpiler.tab.c"
    break;

  case 19: /* $@6: %empty  */
#line 773 "phase_2/tuc_transpiler.y"
    {
        indented_stmts_2 = indent_statements((yyvsp[0].str), indentation_level);
        indentation_level--;
    }
#line 2140 "generated/tuc_transpiler.tab.c"
    break;

  case 20: /* commands: KW_FOR IDENTIFIER KW_IN '[' arithmetic_expr_non_empty ':' arithmetic_expr_non_empty step ']' ':' $@5 stmts $@6 KW_ENDFOR  */
#line 777 "phase_2/tuc_transpiler.y"
              {

        (yyval.str) = template("%s%s}\n", temp5, indented_stmts_2);
        is_a_loop = 0;
    }
#line 2150 "generated/tuc_transpiler.tab.c"
    break;

  case 21: /* commands: IDENTIFIER ASSIGNMENT_OP '[' expr KW_FOR IDENTIFIER ':' POSINT ']' ':' KW_VARIABLE_TYPE  */
#line 787 "phase_2/tuc_transpiler.y"
    {
        char* newType = (yyvsp[0].str);
        if (contains_substring((yyvsp[-7].str),(yyvsp[-5].str)) == 0){
            yyerror("In compact array, the element isnt contained in the expression"); 

        }
        (yyval.str) = template("%s* %s =(%s*)malloc(%s*sizeof(%s));\nfor (int %s = 0; %s < %s; ++%s) {\n  %s[%s] = %s;\n}\n", 
                        newType, (yyvsp[-10].str),newType,(yyvsp[-3].str),     newType,        (yyvsp[-5].str),     (yyvsp[-5].str),  (yyvsp[-3].str),  (yyvsp[-5].str),       (yyvsp[-10].str),(yyvsp[-5].str),   (yyvsp[-7].str));

    }
#line 2165 "generated/tuc_transpiler.tab.c"
    break;

  case 22: /* commands: IDENTIFIER ASSIGNMENT_OP '[' expr KW_FOR IDENTIFIER ':' KW_VARIABLE_TYPE KW_IN IDENTIFIER KW_OF POSINT ']' ':' KW_VARIABLE_TYPE  */
#line 801 "phase_2/tuc_transpiler.y"
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
#line 2186 "generated/tuc_transpiler.tab.c"
    break;

  case 23: /* else_statement: %empty  */
#line 823 "phase_2/tuc_transpiler.y"
                                                     { (yyval.str) = ""; }
#line 2192 "generated/tuc_transpiler.tab.c"
    break;

  case 24: /* $@7: %empty  */
#line 824 "phase_2/tuc_transpiler.y"
                                      {temp4 = template("%s:\n", (yyvsp[-1].str)); indentation_level++;}
#line 2198 "generated/tuc_transpiler.tab.c"
    break;

  case 25: /* else_statement: KW_ELSE ':' $@7 stmts  */
#line 826 "phase_2/tuc_transpiler.y"
  {         
      indented_stmts_1 = indent_statements((yyvsp[0].str), indentation_level);
      indentation_level--;
       (yyval.str) = template("%s%s", temp4, indented_stmts_1);
  }
#line 2208 "generated/tuc_transpiler.tab.c"
    break;

  case 26: /* step: %empty  */
#line 833 "phase_2/tuc_transpiler.y"
                                        { for_has_step = 0; (yyval.str) = ""; }
#line 2214 "generated/tuc_transpiler.tab.c"
    break;

  case 27: /* step: ':' arithmetic_expr_non_empty  */
#line 834 "phase_2/tuc_transpiler.y"
                                        { for_has_step = 1; (yyval.str) = template(":%s", (yyvsp[0].str));}
#line 2220 "generated/tuc_transpiler.tab.c"
    break;

  case 28: /* stmts: %empty  */
#line 842 "phase_2/tuc_transpiler.y"
                                                  { (yyval.str) = ""; }
#line 2226 "generated/tuc_transpiler.tab.c"
    break;

  case 29: /* stmts: stmts loop_thing ';'  */
#line 844 "phase_2/tuc_transpiler.y"
  {     
    if (is_a_loop != 1) {  
      yyerror("This isnt a loop"); 
      YYABORT;
    }
    (yyval.str) = template("%s%s;\n", (yyvsp[-2].str), (yyvsp[-1].str));}
#line 2237 "generated/tuc_transpiler.tab.c"
    break;

  case 30: /* stmts: stmts variable_definition ';'  */
#line 850 "phase_2/tuc_transpiler.y"
                                                  { (yyval.str) = template("%s%s\n", (yyvsp[-2].str), (yyvsp[-1].str)); }
#line 2243 "generated/tuc_transpiler.tab.c"
    break;

  case 31: /* stmts: stmts expr ';'  */
#line 852 "phase_2/tuc_transpiler.y"
  { 
    assignment_made = 0; 
    (yyval.str) = template("%s%s;\n", (yyvsp[-2].str), (yyvsp[-1].str)); 
  }
#line 2252 "generated/tuc_transpiler.tab.c"
    break;

  case 32: /* stmts: stmts commands ';'  */
#line 856 "phase_2/tuc_transpiler.y"
                                                  { (yyval.str) = template("%s%s", (yyvsp[-2].str), (yyvsp[-1].str)); }
#line 2258 "generated/tuc_transpiler.tab.c"
    break;

  case 35: /* $@8: %empty  */
#line 866 "phase_2/tuc_transpiler.y"
                     { inside_comp = 1; comp_name = (yyvsp[0].str);}
#line 2264 "generated/tuc_transpiler.tab.c"
    break;

  case 36: /* complicated_types: KW_COMP IDENTIFIER $@8 ':' declaration_comp body_comp KW_ENDCOMP  */
#line 868 "phase_2/tuc_transpiler.y"
  { 
    add_identifier((yyvsp[-5].str)); 
    for (int i = 0; i < function_list_count; i++) {
      //add_function_to_component($2, list_of_comp_functions[i]);
    }

    clear_function_list();

    char* function_definition_prints = getCompFunctionsAsString(comp_name);   
    char* function_names = getCompFunctionNamesAsString(comp_name);
    (yyval.str) = template("typedef struct %s {\n%s%s} %s;\n%s\n%s ctor_%s = {%s};\n", (yyvsp[-5].str), (yyvsp[-2].str), (yyvsp[-1].str), comp_name, function_definition_prints, comp_name, comp_name, function_names); 

    inside_comp = 0;
  }
#line 2283 "generated/tuc_transpiler.tab.c"
    break;

  case 37: /* declaration_comp: %empty  */
#line 887 "phase_2/tuc_transpiler.y"
                                                            { (yyval.str) = ""; }
#line 2289 "generated/tuc_transpiler.tab.c"
    break;

  case 38: /* declaration_comp: declaration_comp variable_definition_comp ';'  */
#line 888 "phase_2/tuc_transpiler.y"
                                                            { (yyval.str) = template("%s  %s;\n", (yyvsp[-2].str), (yyvsp[-1].str)); }
#line 2295 "generated/tuc_transpiler.tab.c"
    break;

  case 39: /* variable_definition_comp: variable_list_comp ':' KW_VARIABLE_TYPE  */
#line 892 "phase_2/tuc_transpiler.y"
                                                     { (yyval.str) = template("%s %s", (yyvsp[0].str), (yyvsp[-2].str)); }
#line 2301 "generated/tuc_transpiler.tab.c"
    break;

  case 40: /* variable_list_comp: '#' IDENTIFIER  */
#line 896 "phase_2/tuc_transpiler.y"
                                                { add_identifier((yyvsp[0].str)); (yyval.str) = template("%s", (yyvsp[0].str)); }
#line 2307 "generated/tuc_transpiler.tab.c"
    break;

  case 41: /* variable_list_comp: variable_list_comp ',' '#' IDENTIFIER  */
#line 897 "phase_2/tuc_transpiler.y"
                                                { add_identifier((yyvsp[0].str)); (yyval.str) = template("%s, %s", (yyvsp[-3].str), (yyvsp[0].str)); }
#line 2313 "generated/tuc_transpiler.tab.c"
    break;

  case 42: /* body_comp: %empty  */
#line 904 "phase_2/tuc_transpiler.y"
                                                    { (yyval.str) = ""; }
#line 2319 "generated/tuc_transpiler.tab.c"
    break;

  case 43: /* body_comp: body_comp function_definition_comp ';'  */
#line 905 "phase_2/tuc_transpiler.y"
                                                    { (yyval.str) = template("%s  %s;\n", (yyvsp[-2].str), (yyvsp[-1].str)); }
#line 2325 "generated/tuc_transpiler.tab.c"
    break;

  case 44: /* body_comp: body_comp expr ';'  */
#line 906 "phase_2/tuc_transpiler.y"
                                               { assignment_made = 0; (yyval.str) = template("%s    %s;\n", (yyvsp[-2].str), (yyvsp[-1].str)); }
#line 2331 "generated/tuc_transpiler.tab.c"
    break;

  case 45: /* body_comp: KW_RETURN expr ';'  */
#line 907 "phase_2/tuc_transpiler.y"
                                               { body_has_return_type = 1; (yyval.str) = template("    %s %s;\n", (yyvsp[-2].str) , (yyvsp[-1].str) ); }
#line 2337 "generated/tuc_transpiler.tab.c"
    break;

  case 46: /* function_definition_comp: KW_DEF IDENTIFIER '(' arguments ')' return_type_comp ':' body_comp KW_ENDDEF  */
#line 913 "phase_2/tuc_transpiler.y"
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
    append_to_function_list((yyvsp[-7].str));
    char* function_follow_comp = template("%s %s(struct %s *self%s%s) {\n%s\n}", (yyvsp[-3].str), (yyvsp[-7].str), comp_name,strlen((yyvsp[-5].str)) == 0 ? "":", " , (yyvsp[-5].str), (yyvsp[-1].str) );
    addFunctionToComp(comp_name, function_follow_comp, (yyvsp[-7].str));

  }
#line 2358 "generated/tuc_transpiler.tab.c"
    break;

  case 47: /* return_type_comp: %empty  */
#line 932 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = ""; }
#line 2364 "generated/tuc_transpiler.tab.c"
    break;

  case 48: /* return_type_comp: KW_FUNC_RETURN KW_VARIABLE_TYPE  */
#line 933 "phase_2/tuc_transpiler.y"
                                        { func_needs_return_type = 1; (yyval.str) = template("%s", (yyvsp[0].str)); }
#line 2370 "generated/tuc_transpiler.tab.c"
    break;

  case 49: /* func_call: IDENTIFIER '(' func_args ')'  */
#line 942 "phase_2/tuc_transpiler.y"
    {  
        const DefinedFunction* func = find_function((yyvsp[-3].str));

        if(find_included_function((yyvsp[-3].str)) == 1) {used_lambdalib_function = 1;}

        if (!func || (func->has_arguments != 99 && num_of_args_in_function_in_func_call != func->has_arguments && func->has_arguments != 999)) {
            yyerror("Wrong function call or function hasn't been defined"); YYABORT;

        } else {
            (yyval.str) = template("%s(%s)", (yyvsp[-3].str), (yyvsp[-1].str));
        }
        num_of_args_in_function_in_func_call = 0;
    }
#line 2388 "generated/tuc_transpiler.tab.c"
    break;

  case 50: /* func_args: %empty  */
#line 958 "phase_2/tuc_transpiler.y"
           { (yyval.str) = ""; }
#line 2394 "generated/tuc_transpiler.tab.c"
    break;

  case 51: /* func_args: expr  */
#line 959 "phase_2/tuc_transpiler.y"
                                    { num_of_args_in_function_in_func_call++; (yyval.str) = template("%s", (yyvsp[0].str)); }
#line 2400 "generated/tuc_transpiler.tab.c"
    break;

  case 52: /* func_args: STRING  */
#line 960 "phase_2/tuc_transpiler.y"
                                    { num_of_args_in_function_in_func_call++; (yyval.str) = template("%s", (yyvsp[0].str)); }
#line 2406 "generated/tuc_transpiler.tab.c"
    break;

  case 53: /* func_args: func_args ',' expr  */
#line 961 "phase_2/tuc_transpiler.y"
                                    { num_of_args_in_function_in_func_call++; (yyval.str) = template("%s, %s", (yyvsp[-2].str), (yyvsp[0].str)); }
#line 2412 "generated/tuc_transpiler.tab.c"
    break;

  case 54: /* func_args: func_args ',' STRING  */
#line 962 "phase_2/tuc_transpiler.y"
                                    { num_of_args_in_function_in_func_call++; (yyval.str) = template("%s, %s", (yyvsp[-2].str), (yyvsp[0].str)); }
#line 2418 "generated/tuc_transpiler.tab.c"
    break;

  case 55: /* expr: func_call  */
#line 970 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("%s", (yyvsp[0].str));  }
#line 2424 "generated/tuc_transpiler.tab.c"
    break;

  case 56: /* expr: NUMBERS  */
#line 971 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = (yyvsp[0].str); }
#line 2430 "generated/tuc_transpiler.tab.c"
    break;

  case 57: /* expr: '#' IDENTIFIER  */
#line 972 "phase_2/tuc_transpiler.y"
                                        { if (inside_comp == 0) {yyerror("Not in comp"); YYABORT;} else { (yyval.str) = template("%s", (yyvsp[0].str));} }
#line 2436 "generated/tuc_transpiler.tab.c"
    break;

  case 58: /* expr: '#' IDENTIFIER '[' expr ']'  */
#line 973 "phase_2/tuc_transpiler.y"
                                        { if (inside_comp == 0) {yyerror("Not in comp"); YYABORT;} else { (yyval.str) = template("%s[%s]", (yyvsp[-3].str), (yyvsp[-1].str));} }
#line 2442 "generated/tuc_transpiler.tab.c"
    break;

  case 59: /* expr: IDENTIFIER  */
#line 974 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = (yyvsp[0].str); }
#line 2448 "generated/tuc_transpiler.tab.c"
    break;

  case 60: /* expr: IDENTIFIER '[' expr ']'  */
#line 975 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("%s[%s]", (yyvsp[-3].str), (yyvsp[-1].str)); }
#line 2454 "generated/tuc_transpiler.tab.c"
    break;

  case 61: /* expr: '(' expr ')'  */
#line 976 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("(%s)", (yyvsp[-1].str)); }
#line 2460 "generated/tuc_transpiler.tab.c"
    break;

  case 62: /* expr: expr CALC_OP expr  */
#line 977 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2466 "generated/tuc_transpiler.tab.c"
    break;

  case 63: /* expr: expr EXPONENTIATION_OP expr  */
#line 978 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2472 "generated/tuc_transpiler.tab.c"
    break;

  case 64: /* expr: expr RELATIONAL_OP expr  */
#line 979 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2478 "generated/tuc_transpiler.tab.c"
    break;

  case 65: /* expr: expr LOGICAL_AND_OR_OP expr  */
#line 980 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2484 "generated/tuc_transpiler.tab.c"
    break;

  case 66: /* expr: LOGICAL_NOT_OP expr  */
#line 981 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("%s %s", (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2490 "generated/tuc_transpiler.tab.c"
    break;

  case 67: /* expr: UNARY_OP expr  */
#line 982 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("%s%s", (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2496 "generated/tuc_transpiler.tab.c"
    break;

  case 68: /* expr: IDENTIFIER ASSIGNMENT_OP expr  */
#line 984 "phase_2/tuc_transpiler.y"
  {
      if (assignment_made == 1) {
          yyerror("Multiple assignments in one expression are not allowed");
          YYABORT; 
      }
      assignment_made = 1; 
      (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); 
  }
#line 2509 "generated/tuc_transpiler.tab.c"
    break;

  case 69: /* expr: IDENTIFIER '[' expr ']' ASSIGNMENT_OP expr  */
#line 993 "phase_2/tuc_transpiler.y"
  {
      if (assignment_made == 1) {
          yyerror("Multiple assignments in one expression are not allowed");
          YYABORT; 
      }
      assignment_made = 1; 
      (yyval.str) = template("%s[%s] %s %s", (yyvsp[-5].str), (yyvsp[-3].str), (yyvsp[-1].str), (yyvsp[0].str) ); 
  }
#line 2522 "generated/tuc_transpiler.tab.c"
    break;

  case 70: /* expr: '#' IDENTIFIER ASSIGNMENT_OP expr  */
#line 1002 "phase_2/tuc_transpiler.y"
  {     
      if (assignment_made == 1 || inside_comp == 0) {
          yyerror("Multiple assignments in one expression are not allowed or not in comp");
          YYABORT; 
      }
      assignment_made = 1; 
      (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); 
  }
#line 2535 "generated/tuc_transpiler.tab.c"
    break;

  case 71: /* expr: '#' IDENTIFIER '[' expr ']' ASSIGNMENT_OP expr  */
#line 1011 "phase_2/tuc_transpiler.y"
  {
      if (assignment_made == 1 || inside_comp == 0) {
          yyerror("Multiple assignments in one expression are not allowed or not in comp");
          YYABORT; 
      }
      assignment_made = 1; 
      (yyval.str) = template("%s[%s] %s %s", (yyvsp[-5].str), (yyvsp[-3].str), (yyvsp[-1].str), (yyvsp[0].str) ); 
  }
#line 2548 "generated/tuc_transpiler.tab.c"
    break;

  case 72: /* arithmetic_expr_non_empty: POSINT  */
#line 1022 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = (yyvsp[0].str); }
#line 2554 "generated/tuc_transpiler.tab.c"
    break;

  case 73: /* arithmetic_expr_non_empty: REAL  */
#line 1023 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = (yyvsp[0].str); }
#line 2560 "generated/tuc_transpiler.tab.c"
    break;

  case 74: /* arithmetic_expr_non_empty: IDENTIFIER  */
#line 1024 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = (yyvsp[0].str); }
#line 2566 "generated/tuc_transpiler.tab.c"
    break;

  case 75: /* arithmetic_expr_non_empty: '(' expr ')'  */
#line 1025 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("(%s)", (yyvsp[-1].str)); }
#line 2572 "generated/tuc_transpiler.tab.c"
    break;

  case 76: /* arithmetic_expr_non_empty: expr EXPONENTIATION_OP expr  */
#line 1026 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2578 "generated/tuc_transpiler.tab.c"
    break;

  case 77: /* arithmetic_expr_non_empty: expr CALC_OP expr  */
#line 1027 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2584 "generated/tuc_transpiler.tab.c"
    break;

  case 78: /* arithmetic_expr_non_empty: expr RELATIONAL_OP expr  */
#line 1028 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2590 "generated/tuc_transpiler.tab.c"
    break;

  case 79: /* arithmetic_expr_non_empty: expr LOGICAL_AND_OR_OP expr  */
#line 1029 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2596 "generated/tuc_transpiler.tab.c"
    break;

  case 80: /* arithmetic_expr_non_empty: LOGICAL_NOT_OP expr  */
#line 1030 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("%s %s", (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2602 "generated/tuc_transpiler.tab.c"
    break;

  case 81: /* arithmetic_expr_non_empty: UNARY_OP expr  */
#line 1031 "phase_2/tuc_transpiler.y"
                                        { (yyval.str) = template("%s%s", (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2608 "generated/tuc_transpiler.tab.c"
    break;

  case 82: /* $@9: %empty  */
#line 1040 "phase_2/tuc_transpiler.y"
{
    indentation_level++;
    temp8 = template("void %s() {\n", (yyvsp[-3].str));
}
#line 2617 "generated/tuc_transpiler.tab.c"
    break;

  case 83: /* $@10: %empty  */
#line 1045 "phase_2/tuc_transpiler.y"
{      
    indented_stmts_4 = indent_statements((yyvsp[0].str), indentation_level);
    indentation_level--;
}
#line 2626 "generated/tuc_transpiler.tab.c"
    break;

  case 84: /* function_definition: KW_DEF KW_MAIN '(' ')' ':' $@9 body $@10 KW_ENDDEF  */
#line 1050 "phase_2/tuc_transpiler.y"
{     
    if (func_needs_return_type != body_has_return_type) {  
      yyerror("Return statement missing from body"); 
      YYABORT;
    }
    (yyval.str) = template("%s%s}\n", temp8, indented_stmts_4);
}
#line 2638 "generated/tuc_transpiler.tab.c"
    break;

  case 85: /* $@11: %empty  */
#line 1064 "phase_2/tuc_transpiler.y"
{
    add_function((yyvsp[-5].str), num_of_args_in_function);  
    num_of_args_in_function = 0;
    indentation_level++;
    temp8 = template("%s %s(%s){\n", (yyvsp[-1].str), (yyvsp[-5].str), (yyvsp[-3].str) );
}
#line 2649 "generated/tuc_transpiler.tab.c"
    break;

  case 86: /* $@12: %empty  */
#line 1071 "phase_2/tuc_transpiler.y"
{      
    indented_stmts_4 = indent_statements((yyvsp[0].str), indentation_level);
    indentation_level--;
}
#line 2658 "generated/tuc_transpiler.tab.c"
    break;

  case 87: /* function_definition: KW_DEF IDENTIFIER '(' arguments ')' return_type ':' $@11 body $@12 KW_ENDDEF  */
#line 1076 "phase_2/tuc_transpiler.y"
{     
    if (func_needs_return_type != body_has_return_type) {  
      yyerror("Return statement missing from body"); 
      YYABORT;
    }
    (yyval.str) = template("%s%s}\n", temp8, indented_stmts_4);
    func_needs_return_type = 0;
    body_has_return_type = 0;

}
#line 2673 "generated/tuc_transpiler.tab.c"
    break;

  case 88: /* arguments: %empty  */
#line 1091 "phase_2/tuc_transpiler.y"
           { (yyval.str) = ""; }
#line 2679 "generated/tuc_transpiler.tab.c"
    break;

  case 89: /* arguments: parameter_definition  */
#line 1092 "phase_2/tuc_transpiler.y"
                                        { num_of_args_in_function++; (yyval.str) = template("%s", (yyvsp[0].str)); }
#line 2685 "generated/tuc_transpiler.tab.c"
    break;

  case 90: /* arguments: arguments ',' parameter_definition  */
#line 1093 "phase_2/tuc_transpiler.y"
                                        { num_of_args_in_function++; (yyval.str) = template("%s, %s", (yyvsp[-2].str), (yyvsp[0].str)); }
#line 2691 "generated/tuc_transpiler.tab.c"
    break;

  case 91: /* parameter_definition: IDENTIFIER ':' KW_VARIABLE_TYPE  */
#line 1097 "phase_2/tuc_transpiler.y"
                                                                { (yyval.str) = template("%s %s", (yyvsp[0].str), (yyvsp[-2].str)); }
#line 2697 "generated/tuc_transpiler.tab.c"
    break;

  case 92: /* parameter_definition: IDENTIFIER '[' POSINT ']' ':' KW_VARIABLE_TYPE  */
#line 1098 "phase_2/tuc_transpiler.y"
                                                                { (yyval.str) = template("%s[%s]: %s", (yyvsp[-5].str), (yyvsp[-3].str), (yyvsp[0].str) ); }
#line 2703 "generated/tuc_transpiler.tab.c"
    break;

  case 93: /* parameter_definition: IDENTIFIER '[' ']' ':' KW_VARIABLE_TYPE  */
#line 1099 "phase_2/tuc_transpiler.y"
                                                                { (yyval.str) = template("%s[]: %s", (yyvsp[-4].str), (yyvsp[0].str) ); }
#line 2709 "generated/tuc_transpiler.tab.c"
    break;

  case 94: /* return_type: %empty  */
#line 1103 "phase_2/tuc_transpiler.y"
          { (yyval.str) = ""; }
#line 2715 "generated/tuc_transpiler.tab.c"
    break;

  case 95: /* return_type: KW_FUNC_RETURN KW_VARIABLE_TYPE  */
#line 1104 "phase_2/tuc_transpiler.y"
                                   { func_needs_return_type = 1; (yyval.str) = template("%s", (yyvsp[0].str)); }
#line 2721 "generated/tuc_transpiler.tab.c"
    break;

  case 96: /* body: stmts  */
#line 1108 "phase_2/tuc_transpiler.y"
        { body_has_return_type = 0; (yyval.str) = template("%s", (yyvsp[0].str)); }
#line 2727 "generated/tuc_transpiler.tab.c"
    break;

  case 97: /* body: stmts KW_RETURN expr ';'  */
#line 1109 "phase_2/tuc_transpiler.y"
                           { body_has_return_type = 1; (yyval.str) = template("%s%s %s;\n", (yyvsp[-3].str), (yyvsp[-2].str) , (yyvsp[-1].str)); }
#line 2733 "generated/tuc_transpiler.tab.c"
    break;

  case 98: /* variable_definition: variable_list ':' KW_VARIABLE_TYPE  */
#line 1118 "phase_2/tuc_transpiler.y"
   {  
        if(is_identifier_in_list((yyvsp[0].str))){
            (yyval.str) = template("%s %s = ctor_%s;", (yyvsp[0].str), (yyvsp[-2].str), (yyvsp[0].str));
        
        } else { (yyval.str) = template("%s %s;", (yyvsp[0].str), (yyvsp[-2].str)); }   
   }
#line 2744 "generated/tuc_transpiler.tab.c"
    break;

  case 99: /* variable_definition: IDENTIFIER '[' POSINT ']' ':' KW_VARIABLE_TYPE  */
#line 1124 "phase_2/tuc_transpiler.y"
                                                                        { (yyval.str) = template("%s %s[%s];", (yyvsp[0].str), (yyvsp[-5].str), (yyvsp[-3].str) ); }
#line 2750 "generated/tuc_transpiler.tab.c"
    break;

  case 100: /* variable_definition: IDENTIFIER '[' ']' ':' KW_VARIABLE_TYPE  */
#line 1125 "phase_2/tuc_transpiler.y"
                                                                        { (yyval.str) = template("%s[]: %s;", (yyvsp[-4].str), (yyvsp[0].str) ); }
#line 2756 "generated/tuc_transpiler.tab.c"
    break;

  case 101: /* variable_definition: KW_CONST IDENTIFIER ASSIGNMENT_OP VALUE ':' KW_VARIABLE_TYPE  */
#line 1126 "phase_2/tuc_transpiler.y"
                                                                        { (yyval.str) = template("%s %s %s = %s;", (yyvsp[-5].str), (yyvsp[0].str), (yyvsp[-4].str), (yyvsp[-2].str)); }
#line 2762 "generated/tuc_transpiler.tab.c"
    break;

  case 106: /* KW_VARIABLE_TYPE: KW_INTEGER  */
#line 1137 "phase_2/tuc_transpiler.y"
                        {(yyval.str) = (yyvsp[0].str); }
#line 2768 "generated/tuc_transpiler.tab.c"
    break;

  case 109: /* KW_VARIABLE_TYPE: IDENTIFIER  */
#line 1140 "phase_2/tuc_transpiler.y"
               {
    if (is_identifier_in_list((yyvsp[0].str))) {
        (yyval.str) = template("%s", (yyvsp[0].str));
    } else {
        yyerror("Unknown variable type");
        YYABORT;
    }
  }
#line 2781 "generated/tuc_transpiler.tab.c"
    break;

  case 112: /* variable_list: IDENTIFIER  */
#line 1156 "phase_2/tuc_transpiler.y"
                                    { (yyval.str) = template("%s", (yyvsp[0].str)); }
#line 2787 "generated/tuc_transpiler.tab.c"
    break;

  case 113: /* variable_list: variable_list ',' IDENTIFIER  */
#line 1157 "phase_2/tuc_transpiler.y"
                                    { (yyval.str) = template("%s, %s", (yyvsp[-2].str), (yyvsp[0].str)); }
#line 2793 "generated/tuc_transpiler.tab.c"
    break;


#line 2797 "generated/tuc_transpiler.tab.c"

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

#line 1163 "phase_2/tuc_transpiler.y"

    void print_evaluation(char* result) {

        if (yyerror_count == 0) {
            printf("===================================================== Expression evaluates to =====================================================\n%s\n", result);
            
            FILE *file = fopen("output.c", "w");
            if (file != NULL) {
                
                fprintf(file, "#include <stdio.h>\n");
                fprintf(file, "#include <stdlib.h>\n");

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
    free_components();
    free_lines();


    return 0;
}
