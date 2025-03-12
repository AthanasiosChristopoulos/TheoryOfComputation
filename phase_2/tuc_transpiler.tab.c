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
#line 1 "tuc_transpiler.y"


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
        if (*p == ')') {return 0;}
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
//==========================================================================================================================

  // #define MAX_IDENTIFIERS 100
  // char* comp_identifier_list[MAX_IDENTIFIERS];
  // int comp = 0;
  // int comp_identifier_count = 0;

  // void add_identifier(const char* id) {
  //     if (comp_identifier_count < MAX_IDENTIFIERS) {
  //         comp_identifier_list[comp_identifier_count] = strdup(id);
  //         comp_identifier_count++;
  //     } else {
  //         fprintf(stderr, "Identifier list is full.\n");
  //     }
  // }

  // int is_identifier_in_list(const char* identifier) {
  //     for (int i = 0; i < comp_identifier_count; i++) {
  //         if (strcmp(comp_identifier_list[i], identifier) == 0) {
  //             return 1;  // Found in the list
  //         }
  //     }
  //     return 0;  // Not found
  // }

int comp = 0;

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

char* func_to_define_comp;

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

  char* transform_type(const char *identifier) {
      for (int i = 0; i < sizeof(typeMappings) / sizeof(typeMappings[0]); i++) {
          if (strcmp(identifier, typeMappings[i].original) == 0) {
              return (char*)typeMappings[i].transformed;
          }
      }
      return NULL; 
  }

  char* handle_variable_type(const char *inputType) {
      return transform_type(inputType);
  }

  int is_a_loop = 0;

//==========================================================================================================================
//==========================================================================================================================
//==========================================================================================================================
//==========================================================================================================================
//==========================================================================================================================
//==========================================================================================================================


#line 516 "tuc_transpiler.tab.c"

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
  YYSYMBOL_body_comp = 87,                 /* body_comp  */
  YYSYMBOL_attribute_definition = 88,      /* attribute_definition  */
  YYSYMBOL_function_definition_comp = 89,  /* function_definition_comp  */
  YYSYMBOL_return_type_comp = 90,          /* return_type_comp  */
  YYSYMBOL_expr_comp = 91,                 /* expr_comp  */
  YYSYMBOL_func_call = 92,                 /* func_call  */
  YYSYMBOL_func_args = 93,                 /* func_args  */
  YYSYMBOL_expr = 94,                      /* expr  */
  YYSYMBOL_arithmetic_expr_non_empty = 95, /* arithmetic_expr_non_empty  */
  YYSYMBOL_function_definition = 96,       /* function_definition  */
  YYSYMBOL_97_9 = 97,                      /* $@9  */
  YYSYMBOL_98_10 = 98,                     /* $@10  */
  YYSYMBOL_99_11 = 99,                     /* $@11  */
  YYSYMBOL_100_12 = 100,                   /* $@12  */
  YYSYMBOL_arguments = 101,                /* arguments  */
  YYSYMBOL_parameter_definition = 102,     /* parameter_definition  */
  YYSYMBOL_return_type = 103,              /* return_type  */
  YYSYMBOL_body = 104,                     /* body  */
  YYSYMBOL_variable_definition = 105,      /* variable_definition  */
  YYSYMBOL_VALUE = 106,                    /* VALUE  */
  YYSYMBOL_KW_VARIABLE_TYPE = 107,         /* KW_VARIABLE_TYPE  */
  YYSYMBOL_NUMBERS = 108,                  /* NUMBERS  */
  YYSYMBOL_variable_list = 109             /* variable_list  */
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
#define YYLAST   447

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  69
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  41
/* YYNRULES -- Number of rules.  */
#define YYNRULES  120
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  272

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
       0,   515,   515,   518,   520,   521,   522,   523,   524,   525,
     532,   539,   552,   554,   552,   564,   569,   563,   580,   585,
     579,   598,   610,   629,   630,   630,   639,   640,   648,   649,
     656,   657,   662,   665,   665,   672,   672,   684,   685,   686,
     687,   688,   692,   693,   701,   718,   719,   724,   725,   726,
     727,   728,   729,   730,   731,   732,   733,   734,   735,   744,
     759,   776,   777,   778,   779,   780,   788,   789,   790,   791,
     792,   793,   794,   795,   796,   797,   798,   799,   808,   820,
     821,   822,   823,   824,   825,   826,   827,   828,   829,   839,
     844,   838,   861,   868,   860,   888,   889,   890,   894,   895,
     896,   901,   902,   906,   907,   915,   916,   917,   918,   922,
     923,   924,   928,   929,   930,   931,   932,   943,   944,   948,
     949
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
  "loop_thing", "complicated_types", "$@8", "body_comp",
  "attribute_definition", "function_definition_comp", "return_type_comp",
  "expr_comp", "func_call", "func_args", "expr",
  "arithmetic_expr_non_empty", "function_definition", "$@9", "$@10",
  "$@11", "$@12", "arguments", "parameter_definition", "return_type",
  "body", "variable_definition", "VALUE", "KW_VARIABLE_TYPE", "NUMBERS",
  "variable_list", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-133)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-120)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -133,    69,   207,  -133,   305,  -133,  -133,    11,    14,    85,
       8,    96,   106,   111,   285,   285,   285,  -133,    64,    65,
    -133,   267,    75,    81,  -133,   -59,    44,   153,   254,   285,
     285,   113,    88,    89,  -133,   160,   110,   -41,   -23,   115,
     347,  -133,  -133,   285,   285,   285,   285,  -133,  -133,  -133,
     183,   163,   285,  -133,  -133,   -31,   383,   122,   124,   192,
     354,   360,   108,   188,   132,   131,  -133,   383,   170,   285,
     285,  -133,   383,   -23,   115,  -133,  -133,  -133,  -133,  -133,
    -133,  -133,  -133,    31,  -133,   280,   133,   183,   161,   139,
     141,   301,    72,   -16,  -133,   142,   181,  -133,  -133,  -133,
     158,   222,  -133,   383,   183,  -133,   285,  -133,  -133,   375,
     -56,    -5,   285,   285,   285,   390,   172,     4,   183,   193,
     188,  -133,   146,    74,   183,   173,  -133,  -133,  -133,  -133,
     -13,    40,   367,   285,   285,   285,   285,   301,   165,   174,
    -133,   183,   177,  -133,  -133,   146,   146,   146,   244,   296,
    -133,   246,  -133,   249,   195,   199,   307,  -133,   295,   259,
     259,    39,   383,   126,    54,    59,   202,   205,   183,  -133,
    -133,   238,  -133,   137,   221,   374,   -30,   146,   146,   146,
     146,  -133,   210,   214,  -133,  -133,  -133,   224,   279,  -133,
    -133,   226,   284,   229,   313,   245,   289,   301,   250,   183,
    -133,  -133,   285,   286,  -133,   146,    13,   403,   137,   221,
    -133,   188,   183,   247,   252,   312,  -133,   253,   310,  -133,
    -133,  -133,  -133,  -133,   258,  -133,  -133,   323,  -133,  -133,
      13,    13,    13,   335,   341,  -133,    18,  -133,   322,  -133,
     183,   315,  -133,  -133,  -133,   314,  -133,   292,  -133,   306,
     117,  -133,   330,  -133,  -133,  -133,   146,   183,   288,   283,
     259,   259,  -133,  -133,   181,   290,   342,   135,   183,  -133,
    -133,  -133
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       3,     0,     2,     1,    68,   117,   118,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     9,     0,     0,
      66,     0,     0,     0,    67,     0,     0,    61,     0,     0,
       0,     0,     0,     0,    35,     0,     0,    68,    76,    75,
       0,     8,     7,     0,     0,     0,     0,     5,     6,     4,
       0,     0,     0,    77,    63,     0,    62,   117,     0,     0,
       0,     0,     0,    95,     0,     0,    10,    11,     0,     0,
       0,    69,    72,    71,    73,    74,   116,   113,   112,   114,
     115,   105,   120,     0,    60,     0,     0,     0,    70,     0,
       0,     0,     0,     0,    96,     0,    37,   109,   111,   110,
       0,     0,    65,    64,     0,   107,     0,    12,    15,    68,
     117,   118,     0,     0,     0,     0,     0,     0,     0,   101,
       0,    89,    47,    47,     0,     0,   106,    78,    28,    28,
      76,    75,     0,     0,     0,     0,     0,     0,     0,     0,
      98,     0,     0,    97,    28,    47,    47,    47,     0,     0,
      48,     0,    36,     0,     0,     0,     0,   108,     0,    13,
      16,    69,    83,    71,    73,    74,    26,     0,     0,   102,
      92,   103,    90,    57,    56,     0,    49,    47,    47,    47,
      47,    41,     0,    49,    38,    39,    40,     0,     0,    33,
      34,     0,    23,     0,     0,     0,     0,     0,     0,     0,
     100,    28,     0,     0,    51,    47,    47,    52,    53,    54,
      55,    95,     0,     0,     0,     0,    32,     0,     0,    29,
      31,    30,    17,    27,     0,    99,    93,     0,    91,    58,
      47,    47,    47,     0,     0,    48,     0,    42,     0,    43,
       0,     0,    24,    14,    18,     0,   104,     0,    50,    45,
       0,    21,     0,    28,    28,    94,    47,     0,     0,     0,
      25,    19,    59,    46,    37,     0,     0,    47,     0,    20,
      44,    22
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -133,  -133,  -133,  -133,   357,  -133,  -133,  -133,  -133,  -133,
    -133,  -133,  -133,  -133,  -126,  -133,  -133,  -133,   102,   147,
    -133,  -133,   -83,  -133,  -133,    -2,  -132,  -133,  -133,  -133,
    -133,  -133,   159,   256,  -133,   189,   380,  -133,   -86,   -87,
    -133
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,     2,    17,   191,   128,   192,   129,   196,   254,
     266,   218,   253,   198,   171,   193,    19,    65,   123,   154,
     155,   258,   156,    20,    55,   194,   116,    22,   144,   203,
     201,   245,    93,    94,   142,   172,   195,   100,    81,    24,
      25
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      21,   105,   159,   160,    69,   166,   -79,    50,   138,    51,
     -79,    32,    38,    39,    40,   205,    37,     5,   126,    45,
       6,    46,    27,    70,    53,    56,    59,    60,    61,    45,
      84,    46,   140,    67,   206,   150,   150,    85,   157,   149,
      33,    72,    73,    74,    75,   119,   101,    37,     5,   -88,
      83,     6,   120,   -88,   230,   169,   231,   -80,   150,   150,
     150,   -80,   173,   174,   175,   223,   139,    53,    59,     3,
      43,    44,   188,    45,    29,    46,   232,    30,     5,   249,
     148,     6,   200,   103,    46,    14,   120,    15,    31,   115,
     150,   150,   150,   150,   207,   208,   209,   210,    46,    34,
     151,   -82,   -87,   152,   127,   -82,   -87,    16,    52,    35,
     130,   131,   132,   225,    36,   145,   -85,   146,   150,   235,
     -85,   -86,   229,   233,    62,   -86,   237,   260,   261,    41,
      42,   162,   163,   164,   165,   115,   117,   147,   118,     5,
      48,   153,     6,   235,   235,   235,    49,   173,   174,   175,
       5,    63,    64,     6,   251,    68,    37,     5,    54,    46,
       6,   151,   270,    37,     5,    66,    82,     6,    45,   150,
      46,   263,    91,   262,    97,    98,   145,    99,   146,   179,
     150,   180,   271,   212,    86,   213,    76,   145,   -84,   146,
      87,    92,   -84,    95,    14,   115,    15,    96,   147,   104,
     227,    14,   153,    15,   234,   107,   106,   108,   121,   147,
       4,     5,   122,   148,     6,     7,    16,    77,    78,    79,
       8,    80,     9,    16,   124,   125,   141,   167,    38,    39,
      40,    43,    44,    10,    45,    11,    46,    12,   137,   158,
     168,     4,     5,   170,    13,     6,     7,   176,    14,   182,
      15,     8,   183,     9,    88,   189,   190,    37,    57,   205,
     184,     6,     4,     5,   185,   180,     6,     7,   197,   202,
      16,   199,     8,   211,     9,    13,   189,   190,   206,    14,
     212,    15,   213,    37,     5,   102,   214,     6,    37,     5,
     215,   216,     6,   217,   219,    14,    13,    15,    76,   187,
      14,    16,    15,   222,   109,   110,    43,    44,   111,    45,
     221,    46,   224,   228,   238,   241,    58,    16,   240,   242,
     243,    14,    16,    15,   244,   250,    14,   252,    15,    77,
      78,    79,    47,    80,   259,   177,   178,   256,   179,   257,
     180,   255,   112,    16,   113,   265,   177,   178,    16,   179,
      26,   180,    43,    44,   264,    45,   268,    46,   269,    18,
     239,   181,    43,    44,   114,    45,   267,    46,    27,    28,
     236,  -119,   186,  -119,   177,   178,   143,   179,   220,   180,
      43,    44,    23,    45,     0,    46,    43,    44,   246,    45,
     226,    46,     0,    43,    44,     0,    45,   247,    46,    43,
      44,     0,    45,   248,    46,     0,    43,    44,    71,    45,
       0,    46,     0,   177,   178,    89,   179,     0,   180,     0,
      69,    90,    43,    44,     0,    45,     0,    46,   161,   133,
     134,     0,   135,     0,   136,   204,     0,   -81,    27,    70,
       0,   -81,   177,   178,     0,   179,     0,   180
};

static const yytype_int16 yycheck[] =
{
       2,    87,   128,   129,    45,   137,    62,    66,     4,    68,
      66,     3,    14,    15,    16,    45,     3,     4,   104,    42,
       7,    44,    63,    64,    26,    27,    28,    29,    30,    42,
      61,    44,   118,    35,    64,   122,   123,    68,   124,   122,
      32,    43,    44,    45,    46,    61,    15,     3,     4,    62,
      52,     7,    68,    66,    41,   141,    43,    62,   145,   146,
     147,    66,   145,   146,   147,   197,    62,    69,    70,     0,
      39,    40,   158,    42,    63,    44,    63,    63,     4,    61,
      67,     7,   168,    85,    44,    41,    68,    43,     3,    91,
     177,   178,   179,   180,   177,   178,   179,   180,    44,     3,
      26,    62,    62,    29,   106,    66,    66,    63,    64,     3,
     112,   113,   114,   199,     3,    41,    62,    43,   205,   206,
      66,    62,   205,   206,    11,    66,   212,   253,   254,    65,
      65,   133,   134,   135,   136,   137,    64,    63,    66,     4,
      65,    67,     7,   230,   231,   232,    65,   230,   231,   232,
       4,    63,    63,     7,   240,    45,     3,     4,     5,    44,
       7,    26,    27,     3,     4,     5,     3,     7,    42,   256,
      44,   257,    64,   256,     4,     5,    41,     7,    43,    42,
     267,    44,   268,    66,    62,    68,     3,    41,    62,    43,
      66,     3,    66,    61,    41,   197,    43,    66,    63,    66,
     202,    41,    67,    43,   206,    66,    45,    66,    66,    63,
       3,     4,    31,    67,     7,     8,    63,    34,    35,    36,
      13,    38,    15,    63,    66,     3,    33,    62,   230,   231,
     232,    39,    40,    26,    42,    28,    44,    30,    66,    66,
      66,     3,     4,    66,    37,     7,     8,     3,    41,     3,
      43,    13,     3,    15,    62,    17,    18,     3,     4,    45,
      65,     7,     3,     4,    65,    44,     7,     8,    66,    31,
      63,    66,    13,    63,    15,    37,    17,    18,    64,    41,
      66,    43,    68,     3,     4,     5,    62,     7,     3,     4,
      11,    65,     7,     9,    65,    41,    37,    43,     3,     4,
      41,    63,    43,    14,     3,     4,    39,    40,     7,    42,
      65,    44,    62,    27,    67,     3,    62,    63,    66,    66,
      10,    41,    63,    43,    66,     3,    41,    12,    43,    34,
      35,    36,    65,    38,     4,    39,    40,    45,    42,    33,
      44,    27,    41,    63,    43,    62,    39,    40,    63,    42,
      45,    44,    39,    40,    66,    42,    66,    44,    16,     2,
     213,    65,    39,    40,    63,    42,   264,    44,    63,    64,
     211,    66,    65,    68,    39,    40,   120,    42,    65,    44,
      39,    40,     2,    42,    -1,    44,    39,    40,    65,    42,
     201,    44,    -1,    39,    40,    -1,    42,    62,    44,    39,
      40,    -1,    42,    62,    44,    -1,    39,    40,    61,    42,
      -1,    44,    -1,    39,    40,    61,    42,    -1,    44,    -1,
      45,    61,    39,    40,    -1,    42,    -1,    44,    61,    39,
      40,    -1,    42,    -1,    44,    61,    -1,    62,    63,    64,
      -1,    66,    39,    40,    -1,    42,    -1,    44
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    70,    71,     0,     3,     4,     7,     8,    13,    15,
      26,    28,    30,    37,    41,    43,    63,    72,    73,    85,
      92,    94,    96,   105,   108,   109,    45,    63,    64,    63,
      63,     3,     3,    32,     3,     3,     3,     3,    94,    94,
      94,    65,    65,    39,    40,    42,    44,    65,    65,    65,
      66,    68,    64,    94,     5,    93,    94,     4,    62,    94,
      94,    94,    11,    63,    63,    86,     5,    94,    45,    45,
      64,    61,    94,    94,    94,    94,     3,    34,    35,    36,
      38,   107,     3,    94,    61,    68,    62,    66,    62,    61,
      61,    64,     3,   101,   102,    61,    66,     4,     5,     7,
     106,    15,     5,    94,    66,   107,    45,    66,    66,     3,
       4,     7,    41,    43,    63,    94,    95,    64,    66,    61,
      68,    66,    31,    87,    66,     3,   107,    94,    74,    76,
      94,    94,    94,    39,    40,    42,    44,    66,     4,    62,
     107,    33,   103,   102,    97,    41,    43,    63,    67,    91,
     108,    26,    29,    67,    88,    89,    91,   107,    66,    83,
      83,    61,    94,    94,    94,    94,    95,    62,    66,   107,
      66,    83,   104,    91,    91,    91,     3,    39,    40,    42,
      44,    65,     3,     3,    65,    65,    65,     4,   107,    17,
      18,    73,    75,    84,    94,   105,    77,    66,    82,    66,
     107,    99,    31,    98,    61,    45,    64,    91,    91,    91,
      91,    63,    66,    68,    62,    11,    65,     9,    80,    65,
      65,    65,    14,    95,    62,   107,   104,    94,    27,    91,
      41,    43,    63,    91,    94,   108,   101,   107,    67,    88,
      66,     3,    66,    10,    66,   100,    65,    62,    62,    61,
       3,   107,    12,    81,    78,    27,    45,    33,    90,     4,
      83,    83,    91,   107,    66,    62,    79,    87,    66,    16,
      27,   107
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    69,    70,    71,    71,    71,    71,    71,    71,    71,
      72,    72,    74,    75,    73,    76,    77,    73,    78,    79,
      73,    73,    73,    80,    81,    80,    82,    82,    83,    83,
      83,    83,    83,    84,    84,    86,    85,    87,    87,    87,
      87,    87,    88,    88,    89,    90,    90,    91,    91,    91,
      91,    91,    91,    91,    91,    91,    91,    91,    91,    91,
      92,    93,    93,    93,    93,    93,    94,    94,    94,    94,
      94,    94,    94,    94,    94,    94,    94,    94,    94,    95,
      95,    95,    95,    95,    95,    95,    95,    95,    95,    97,
      98,    96,    99,   100,    96,   101,   101,   101,   102,   102,
     102,   103,   103,   104,   104,   105,   105,   105,   105,   106,
     106,   106,   107,   107,   107,   107,   107,   108,   108,   109,
     109
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     0,     3,     3,     3,     3,     3,     2,
       3,     3,     0,     0,    10,     0,     0,     9,     0,     0,
      14,    11,    15,     0,     0,     4,     0,     2,     0,     3,
       3,     3,     3,     1,     1,     0,     6,     0,     3,     3,
       3,     3,     4,     4,     9,     0,     2,     0,     1,     2,
       5,     3,     3,     3,     3,     3,     2,     2,     4,     7,
       4,     0,     1,     1,     3,     3,     1,     1,     1,     3,
       4,     3,     3,     3,     3,     2,     2,     3,     6,     1,
       1,     1,     3,     3,     3,     3,     3,     2,     2,     0,
       0,     9,     0,     0,    11,     0,     1,     3,     3,     6,
       5,     0,     2,     1,     4,     3,     6,     5,     6,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3
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
#line 515 "tuc_transpiler.y"
        { print_evaluation((yyvsp[0].str)); }
#line 1836 "tuc_transpiler.tab.c"
    break;

  case 3: /* input: %empty  */
#line 518 "tuc_transpiler.y"
                                                  {(yyval.str) = "";}
#line 1842 "tuc_transpiler.tab.c"
    break;

  case 4: /* input: input variable_definition ';'  */
#line 520 "tuc_transpiler.y"
                                                  { (yyval.str) = template("%s%s\n", (yyvsp[-2].str), (yyvsp[-1].str));}
#line 1848 "tuc_transpiler.tab.c"
    break;

  case 5: /* input: input expr ';'  */
#line 521 "tuc_transpiler.y"
                                                  { assignment_made = 0;  (yyval.str) = template("%s%s;\n", (yyvsp[-2].str), (yyvsp[-1].str));}
#line 1854 "tuc_transpiler.tab.c"
    break;

  case 6: /* input: input function_definition ';'  */
#line 522 "tuc_transpiler.y"
                                                  { assignment_made = 0; (yyval.str) = template("%s%s", (yyvsp[-2].str), (yyvsp[-1].str));}
#line 1860 "tuc_transpiler.tab.c"
    break;

  case 7: /* input: input complicated_types ';'  */
#line 523 "tuc_transpiler.y"
                                                  { assignment_made = 0;  (yyval.str) = template("%s%s", (yyvsp[-2].str), (yyvsp[-1].str));}
#line 1866 "tuc_transpiler.tab.c"
    break;

  case 8: /* input: input commands ';'  */
#line 524 "tuc_transpiler.y"
                                                  {  (yyval.str) = template("%s%s", (yyvsp[-2].str), (yyvsp[-1].str));}
#line 1872 "tuc_transpiler.tab.c"
    break;

  case 9: /* input: input macro  */
#line 525 "tuc_transpiler.y"
                                                  {  (yyval.str) = template("%s%s", (yyvsp[-1].str), (yyvsp[0].str));}
#line 1878 "tuc_transpiler.tab.c"
    break;

  case 10: /* macro: KW_MACRO IDENTIFIER STRING  */
#line 532 "tuc_transpiler.y"
                               {
        char *id = (yyvsp[-1].str); 
        char *value = (yyvsp[0].str);
        
        add_macro(id, value);  

    }
#line 1890 "tuc_transpiler.tab.c"
    break;

  case 11: /* macro: KW_MACRO IDENTIFIER expr  */
#line 539 "tuc_transpiler.y"
                             {
        char *id = (yyvsp[-1].str);  
        char *value = (yyvsp[0].str); 

        add_macro(id, value);
    }
#line 1901 "tuc_transpiler.tab.c"
    break;

  case 12: /* $@1: %empty  */
#line 552 "tuc_transpiler.y"
                           {temp1 = template("%s (%s):\n", (yyvsp[-4].str), (yyvsp[-2].str)); indentation_level++; is_a_loop = 0;}
#line 1907 "tuc_transpiler.tab.c"
    break;

  case 13: /* $@2: %empty  */
#line 554 "tuc_transpiler.y"
    {
        indented_stmts = indent_statements((yyvsp[0].str), indentation_level);
        indentation_level--;
    }
#line 1916 "tuc_transpiler.tab.c"
    break;

  case 14: /* commands: KW_IF '(' expr ')' ':' $@1 stmts $@2 else_statement KW_ENDIF  */
#line 558 "tuc_transpiler.y"
                            {
        temp3 = template("%s%s", (yyvsp[-1].str), (yyvsp[0].str)); 
        (yyval.str) = template("%s%s%s;\n", temp1, indented_stmts, temp3);
    }
#line 1925 "tuc_transpiler.tab.c"
    break;

  case 15: /* $@3: %empty  */
#line 564 "tuc_transpiler.y"
    {
        temp7 = template("%s (%s):\n", (yyvsp[-4].str), (yyvsp[-2].str));
        indentation_level++; is_a_loop = 1;
    }
#line 1934 "tuc_transpiler.tab.c"
    break;

  case 16: /* $@4: %empty  */
#line 569 "tuc_transpiler.y"
    {
        indented_stmts_3 = indent_statements((yyvsp[0].str), indentation_level);
        indentation_level--;
    }
#line 1943 "tuc_transpiler.tab.c"
    break;

  case 17: /* commands: KW_WHILE '(' expr ')' ':' $@3 stmts $@4 KW_ENDWHILE  */
#line 573 "tuc_transpiler.y"
                {
        (yyval.str) = template("%s%s%s;\n", temp7, indented_stmts_3, (yyvsp[0].str));
        is_a_loop = 0;
    }
#line 1952 "tuc_transpiler.tab.c"
    break;

  case 18: /* $@5: %empty  */
#line 580 "tuc_transpiler.y"
    {
        temp5 = template("for (int %s = %s; %s < %s; %s++) {\n", (yyvsp[-8].str), (yyvsp[-5].str), (yyvsp[-8].str),(yyvsp[-3].str), (yyvsp[-2].str));  
        indentation_level++; is_a_loop = 1;
    }
#line 1961 "tuc_transpiler.tab.c"
    break;

  case 19: /* $@6: %empty  */
#line 585 "tuc_transpiler.y"
    {
        indented_stmts_2 = indent_statements((yyvsp[0].str), indentation_level);
        indentation_level--;
    }
#line 1970 "tuc_transpiler.tab.c"
    break;

  case 20: /* commands: KW_FOR IDENTIFIER KW_IN '[' arithmetic_expr_non_empty ':' arithmetic_expr_non_empty step ']' ':' $@5 stmts $@6 KW_ENDFOR  */
#line 589 "tuc_transpiler.y"
              {

        (yyval.str) = template("%s%s%s;\n", temp5, indented_stmts_2, (yyvsp[0].str));
        is_a_loop = 0;
    }
#line 1980 "tuc_transpiler.tab.c"
    break;

  case 21: /* commands: IDENTIFIER ASSIGNMENT_OP '[' expr KW_FOR IDENTIFIER ':' POSINT ']' ':' KW_VARIABLE_TYPE  */
#line 599 "tuc_transpiler.y"
    {
        char* newType = (yyvsp[0].str);
        // char* newType = handle_variable_type($11);

        (yyval.str) = template("%s* %s =(%s)malloc(%s*sizeof(%s));\nfor (int %s = 0; %s < %s; ++%s) {\n  %s[%s] = %s;\n}\n", 
                        newType, (yyvsp[-10].str),newType,(yyvsp[-3].str), newType, (yyvsp[-7].str), (yyvsp[-7].str), (yyvsp[-3].str), (yyvsp[-7].str), (yyvsp[-10].str), (yyvsp[-7].str), (yyvsp[-7].str));

    }
#line 1993 "tuc_transpiler.tab.c"
    break;

  case 22: /* commands: IDENTIFIER ASSIGNMENT_OP '[' expr KW_FOR IDENTIFIER ':' KW_VARIABLE_TYPE KW_IN IDENTIFIER KW_OF POSINT ']' ':' KW_VARIABLE_TYPE  */
#line 611 "tuc_transpiler.y"
    {
        // Get the transformed type for $15 (KW_VARIABLE_TYPE)
        char* newType = (yyvsp[0].str);

        (yyval.str) = template(
            "%s* %s = (%s)malloc(%s * sizeof(%s));\n"
            "for (int %s_i = 0; %s_i < %s; ++%s_i) {\n"
            "  %s[%s_i] = %s[%s_i] / 2;\n"
            "}\n",
            newType, (yyvsp[-14].str), newType, (yyvsp[-3].str), newType, (yyvsp[-5].str), (yyvsp[-5].str), (yyvsp[-3].str), (yyvsp[-5].str), (yyvsp[-14].str), (yyvsp[-5].str), (yyvsp[-3].str), (yyvsp[-5].str)
        );
    }
#line 2010 "tuc_transpiler.tab.c"
    break;

  case 23: /* else_statement: %empty  */
#line 629 "tuc_transpiler.y"
                                                     { (yyval.str) = ""; }
#line 2016 "tuc_transpiler.tab.c"
    break;

  case 24: /* $@7: %empty  */
#line 630 "tuc_transpiler.y"
                                      {temp4 = template("%s:\n", (yyvsp[-1].str)); indentation_level++;}
#line 2022 "tuc_transpiler.tab.c"
    break;

  case 25: /* else_statement: KW_ELSE ':' $@7 stmts  */
#line 632 "tuc_transpiler.y"
  {         
      indented_stmts_1 = indent_statements((yyvsp[0].str), indentation_level);
      indentation_level--;
       (yyval.str) = template("%s%s", temp4, indented_stmts_1);
  }
#line 2032 "tuc_transpiler.tab.c"
    break;

  case 26: /* step: %empty  */
#line 639 "tuc_transpiler.y"
                                { (yyval.str) = ""; }
#line 2038 "tuc_transpiler.tab.c"
    break;

  case 27: /* step: ':' arithmetic_expr_non_empty  */
#line 640 "tuc_transpiler.y"
                                  { (yyval.str) = template(":%s", (yyvsp[0].str));}
#line 2044 "tuc_transpiler.tab.c"
    break;

  case 28: /* stmts: %empty  */
#line 648 "tuc_transpiler.y"
                                                  { (yyval.str) = ""; }
#line 2050 "tuc_transpiler.tab.c"
    break;

  case 29: /* stmts: stmts loop_thing ';'  */
#line 650 "tuc_transpiler.y"
  {     
    if (is_a_loop != 1) {  
      yyerror("This isnt a loop"); 
      YYABORT;
    }
    (yyval.str) = template("%s%s;\n", (yyvsp[-2].str), (yyvsp[-1].str));}
#line 2061 "tuc_transpiler.tab.c"
    break;

  case 30: /* stmts: stmts variable_definition ';'  */
#line 656 "tuc_transpiler.y"
                                                  { (yyval.str) = template("%s%s\n", (yyvsp[-2].str), (yyvsp[-1].str)); }
#line 2067 "tuc_transpiler.tab.c"
    break;

  case 31: /* stmts: stmts expr ';'  */
#line 658 "tuc_transpiler.y"
  { 
    assignment_made = 0; 
    (yyval.str) = template("%s%s;\n", (yyvsp[-2].str), (yyvsp[-1].str)); 
  }
#line 2076 "tuc_transpiler.tab.c"
    break;

  case 32: /* stmts: stmts commands ';'  */
#line 662 "tuc_transpiler.y"
                                                  { (yyval.str) = template("%s%s", (yyvsp[-2].str), (yyvsp[-1].str)); }
#line 2082 "tuc_transpiler.tab.c"
    break;

  case 35: /* $@8: %empty  */
#line 672 "tuc_transpiler.y"
                     { func_to_define_comp = (yyvsp[0].str);}
#line 2088 "tuc_transpiler.tab.c"
    break;

  case 36: /* complicated_types: KW_COMP IDENTIFIER $@8 ':' body_comp KW_ENDCOMP  */
#line 674 "tuc_transpiler.y"
  { 
    comp = 1; add_identifier((yyvsp[-4].str)); 
    for (int i = 0; i < function_list_count; i++) {
      add_function_to_component((yyvsp[-4].str), list_of_comp_functions[i]);
    }
    clear_function_list();
    (yyval.str) = template("%s %s:\n%s%s;\n", (yyvsp[-5].str), (yyvsp[-4].str), (yyvsp[-1].str), (yyvsp[0].str)); comp = 0;   
  }
#line 2101 "tuc_transpiler.tab.c"
    break;

  case 37: /* body_comp: %empty  */
#line 684 "tuc_transpiler.y"
                                                  { (yyval.str) = ""; }
#line 2107 "tuc_transpiler.tab.c"
    break;

  case 38: /* body_comp: body_comp attribute_definition ';'  */
#line 685 "tuc_transpiler.y"
                                                 { (yyval.str) = template("%s  %s;\n", (yyvsp[-2].str), (yyvsp[-1].str)); }
#line 2113 "tuc_transpiler.tab.c"
    break;

  case 39: /* body_comp: body_comp function_definition_comp ';'  */
#line 686 "tuc_transpiler.y"
                                                 {  (yyval.str) = template("%s  %s;\n", (yyvsp[-2].str), (yyvsp[-1].str)); }
#line 2119 "tuc_transpiler.tab.c"
    break;

  case 40: /* body_comp: body_comp expr_comp ';'  */
#line 687 "tuc_transpiler.y"
                                      { assignment_made = 0; (yyval.str) = template("%s    %s;\n", (yyvsp[-2].str), (yyvsp[-1].str)); }
#line 2125 "tuc_transpiler.tab.c"
    break;

  case 41: /* body_comp: KW_RETURN expr_comp ';'  */
#line 688 "tuc_transpiler.y"
                                      { body_has_return_type = 1;(yyval.str) = template("    %s #%s;\n", (yyvsp[-2].str) , (yyvsp[-1].str) ); }
#line 2131 "tuc_transpiler.tab.c"
    break;

  case 42: /* attribute_definition: '#' IDENTIFIER ':' KW_VARIABLE_TYPE  */
#line 692 "tuc_transpiler.y"
                                                 { add_identifier((yyvsp[-2].str)); (yyval.str) = template("#%s: %s", (yyvsp[-2].str), (yyvsp[0].str)); }
#line 2137 "tuc_transpiler.tab.c"
    break;

  case 43: /* attribute_definition: '#' IDENTIFIER ',' attribute_definition  */
#line 693 "tuc_transpiler.y"
                                                 { add_identifier((yyvsp[-2].str)); (yyval.str) = template("#%s, %s", (yyvsp[-2].str), (yyvsp[0].str)); }
#line 2143 "tuc_transpiler.tab.c"
    break;

  case 44: /* function_definition_comp: KW_DEF IDENTIFIER '(' arguments ')' return_type_comp ':' body_comp KW_ENDDEF  */
#line 702 "tuc_transpiler.y"
  {
    if (func_needs_return_type != body_has_return_type) {  
      yyerror("Return statement missing from body"); 
      YYABORT;
    }
    (yyval.str) = template("%s %s(%s)%s:   \n%s  %s", (yyvsp[-8].str), (yyvsp[-7].str), (yyvsp[-5].str), (yyvsp[-3].str), (yyvsp[-1].str) , (yyvsp[0].str) );
    func_needs_return_type = 0;
    body_has_return_type = 0;

    add_function((yyvsp[-7].str), num_of_args_in_function);  
    num_of_args_in_function = 0;
    append_to_function_list((yyvsp[-7].str));
  }
#line 2161 "tuc_transpiler.tab.c"
    break;

  case 45: /* return_type_comp: %empty  */
#line 718 "tuc_transpiler.y"
                                        { (yyval.str) = ""; }
#line 2167 "tuc_transpiler.tab.c"
    break;

  case 46: /* return_type_comp: KW_FUNC_RETURN KW_VARIABLE_TYPE  */
#line 719 "tuc_transpiler.y"
                                        { func_needs_return_type = 1; (yyval.str) = template("%s %s", (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2173 "tuc_transpiler.tab.c"
    break;

  case 47: /* expr_comp: %empty  */
#line 724 "tuc_transpiler.y"
                                               { (yyval.str) = ""; }
#line 2179 "tuc_transpiler.tab.c"
    break;

  case 48: /* expr_comp: NUMBERS  */
#line 725 "tuc_transpiler.y"
                                                 { (yyval.str) = (yyvsp[0].str); }
#line 2185 "tuc_transpiler.tab.c"
    break;

  case 49: /* expr_comp: '#' IDENTIFIER  */
#line 726 "tuc_transpiler.y"
                                                 { (yyval.str) = template("#%s", (yyvsp[0].str)); }
#line 2191 "tuc_transpiler.tab.c"
    break;

  case 50: /* expr_comp: '#' IDENTIFIER '[' expr ']'  */
#line 727 "tuc_transpiler.y"
                                                    { (yyval.str) = template("#%s[%s]", (yyvsp[-3].str), (yyvsp[-1].str)); }
#line 2197 "tuc_transpiler.tab.c"
    break;

  case 51: /* expr_comp: '(' expr_comp ')'  */
#line 728 "tuc_transpiler.y"
                                                { (yyval.str) = template("(%s)", (yyvsp[-1].str)); }
#line 2203 "tuc_transpiler.tab.c"
    break;

  case 52: /* expr_comp: expr_comp EXPONENTIATION_OP expr_comp  */
#line 729 "tuc_transpiler.y"
                                                            { (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2209 "tuc_transpiler.tab.c"
    break;

  case 53: /* expr_comp: expr_comp CALC_OP expr_comp  */
#line 730 "tuc_transpiler.y"
                                                { (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2215 "tuc_transpiler.tab.c"
    break;

  case 54: /* expr_comp: expr_comp RELATIONAL_OP expr_comp  */
#line 731 "tuc_transpiler.y"
                                                      { (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2221 "tuc_transpiler.tab.c"
    break;

  case 55: /* expr_comp: expr_comp LOGICAL_AND_OR_OP expr_comp  */
#line 732 "tuc_transpiler.y"
                                                         { (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2227 "tuc_transpiler.tab.c"
    break;

  case 56: /* expr_comp: LOGICAL_NOT_OP expr_comp  */
#line 733 "tuc_transpiler.y"
                                            { (yyval.str) = template("%s %s", (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2233 "tuc_transpiler.tab.c"
    break;

  case 57: /* expr_comp: UNARY_OP expr_comp  */
#line 734 "tuc_transpiler.y"
                                      { (yyval.str) = template("%s %s", (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2239 "tuc_transpiler.tab.c"
    break;

  case 58: /* expr_comp: '#' IDENTIFIER ASSIGNMENT_OP expr_comp  */
#line 736 "tuc_transpiler.y"
  {
      if (assignment_made == 1) {
          yyerror("Multiple assignments in one expression are not allowed");
          YYABORT; 
      }
      assignment_made = 1; 
      (yyval.str) = template("#%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); 
  }
#line 2252 "tuc_transpiler.tab.c"
    break;

  case 59: /* expr_comp: '#' IDENTIFIER '[' expr_comp ']' ASSIGNMENT_OP expr_comp  */
#line 745 "tuc_transpiler.y"
  {
      if (assignment_made == 1) {
          yyerror("Multiple assignments in one expression are not allowed");
          YYABORT; 
      }
      assignment_made = 1; 
      (yyval.str) = template("%s[%s] %s %s", (yyvsp[-5].str), (yyvsp[-3].str), (yyvsp[-1].str), (yyvsp[0].str) ); 
  }
#line 2265 "tuc_transpiler.tab.c"
    break;

  case 60: /* func_call: IDENTIFIER '(' func_args ')'  */
#line 760 "tuc_transpiler.y"
    {  
        const DefinedFunction* func = find_function((yyvsp[-3].str));

        if(find_included_function((yyvsp[-3].str)) == 1) {used_lambdalib_function = 1;}

        if (!func || (num_of_args_in_function_in_func_call != func->has_arguments && func->has_arguments != 999)) {
            yyerror("Wrong function call or function hasn't been defined"); YYABORT;

        } else {
            (yyval.str) = template("%s(%s)", (yyvsp[-3].str), (yyvsp[-1].str));
        }
        num_of_args_in_function_in_func_call = 0;
    }
#line 2283 "tuc_transpiler.tab.c"
    break;

  case 61: /* func_args: %empty  */
#line 776 "tuc_transpiler.y"
           { (yyval.str) = ""; }
#line 2289 "tuc_transpiler.tab.c"
    break;

  case 62: /* func_args: expr  */
#line 777 "tuc_transpiler.y"
                                    { num_of_args_in_function_in_func_call++; (yyval.str) = template("%s", (yyvsp[0].str)); }
#line 2295 "tuc_transpiler.tab.c"
    break;

  case 63: /* func_args: STRING  */
#line 778 "tuc_transpiler.y"
                                    { num_of_args_in_function_in_func_call++; (yyval.str) = template("%s", (yyvsp[0].str)); }
#line 2301 "tuc_transpiler.tab.c"
    break;

  case 64: /* func_args: func_args ',' expr  */
#line 779 "tuc_transpiler.y"
                                    { num_of_args_in_function_in_func_call++; (yyval.str) = template("%s, %s", (yyvsp[-2].str), (yyvsp[0].str)); }
#line 2307 "tuc_transpiler.tab.c"
    break;

  case 65: /* func_args: func_args ',' STRING  */
#line 780 "tuc_transpiler.y"
                                    { num_of_args_in_function_in_func_call++; (yyval.str) = template("%s, %s", (yyvsp[-2].str), (yyvsp[0].str)); }
#line 2313 "tuc_transpiler.tab.c"
    break;

  case 66: /* expr: func_call  */
#line 788 "tuc_transpiler.y"
                                        { (yyval.str) = template("%s", (yyvsp[0].str));  }
#line 2319 "tuc_transpiler.tab.c"
    break;

  case 67: /* expr: NUMBERS  */
#line 789 "tuc_transpiler.y"
                                         { (yyval.str) = (yyvsp[0].str); }
#line 2325 "tuc_transpiler.tab.c"
    break;

  case 68: /* expr: IDENTIFIER  */
#line 790 "tuc_transpiler.y"
                                        { (yyval.str) = (yyvsp[0].str); }
#line 2331 "tuc_transpiler.tab.c"
    break;

  case 69: /* expr: '(' expr ')'  */
#line 791 "tuc_transpiler.y"
                                        { (yyval.str) = template("(%s)", (yyvsp[-1].str)); }
#line 2337 "tuc_transpiler.tab.c"
    break;

  case 70: /* expr: IDENTIFIER '[' expr ']'  */
#line 792 "tuc_transpiler.y"
                                        { (yyval.str) = template("%s[%s]", (yyvsp[-3].str), (yyvsp[-1].str)); }
#line 2343 "tuc_transpiler.tab.c"
    break;

  case 71: /* expr: expr CALC_OP expr  */
#line 793 "tuc_transpiler.y"
                                        { (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2349 "tuc_transpiler.tab.c"
    break;

  case 72: /* expr: expr EXPONENTIATION_OP expr  */
#line 794 "tuc_transpiler.y"
                                        { (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2355 "tuc_transpiler.tab.c"
    break;

  case 73: /* expr: expr RELATIONAL_OP expr  */
#line 795 "tuc_transpiler.y"
                                        { (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2361 "tuc_transpiler.tab.c"
    break;

  case 74: /* expr: expr LOGICAL_AND_OR_OP expr  */
#line 796 "tuc_transpiler.y"
                                        { (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2367 "tuc_transpiler.tab.c"
    break;

  case 75: /* expr: LOGICAL_NOT_OP expr  */
#line 797 "tuc_transpiler.y"
                                        { (yyval.str) = template("%s %s", (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2373 "tuc_transpiler.tab.c"
    break;

  case 76: /* expr: UNARY_OP expr  */
#line 798 "tuc_transpiler.y"
                                        { (yyval.str) = template("%s%s", (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2379 "tuc_transpiler.tab.c"
    break;

  case 77: /* expr: IDENTIFIER ASSIGNMENT_OP expr  */
#line 800 "tuc_transpiler.y"
  {
      if (assignment_made == 1) {
          yyerror("Multiple assignments in one expression are not allowed");
          YYABORT; 
      }
      assignment_made = 1; 
      (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); 
  }
#line 2392 "tuc_transpiler.tab.c"
    break;

  case 78: /* expr: IDENTIFIER '[' expr ']' ASSIGNMENT_OP expr  */
#line 809 "tuc_transpiler.y"
  {
      if (assignment_made == 1) {
          yyerror("Multiple assignments in one expression are not allowed");
          YYABORT; 
      }
      assignment_made = 1; 
      (yyval.str) = template("%s[%s] %s %s", (yyvsp[-5].str), (yyvsp[-3].str), (yyvsp[-1].str), (yyvsp[0].str) ); 
  }
#line 2405 "tuc_transpiler.tab.c"
    break;

  case 79: /* arithmetic_expr_non_empty: POSINT  */
#line 820 "tuc_transpiler.y"
                                        { (yyval.str) = (yyvsp[0].str); }
#line 2411 "tuc_transpiler.tab.c"
    break;

  case 80: /* arithmetic_expr_non_empty: REAL  */
#line 821 "tuc_transpiler.y"
                                        { (yyval.str) = (yyvsp[0].str); }
#line 2417 "tuc_transpiler.tab.c"
    break;

  case 81: /* arithmetic_expr_non_empty: IDENTIFIER  */
#line 822 "tuc_transpiler.y"
                                        { (yyval.str) = (yyvsp[0].str); }
#line 2423 "tuc_transpiler.tab.c"
    break;

  case 82: /* arithmetic_expr_non_empty: '(' expr ')'  */
#line 823 "tuc_transpiler.y"
                                        { (yyval.str) = template("(%s)", (yyvsp[-1].str)); }
#line 2429 "tuc_transpiler.tab.c"
    break;

  case 83: /* arithmetic_expr_non_empty: expr EXPONENTIATION_OP expr  */
#line 824 "tuc_transpiler.y"
                                        { (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2435 "tuc_transpiler.tab.c"
    break;

  case 84: /* arithmetic_expr_non_empty: expr CALC_OP expr  */
#line 825 "tuc_transpiler.y"
                                        { (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2441 "tuc_transpiler.tab.c"
    break;

  case 85: /* arithmetic_expr_non_empty: expr RELATIONAL_OP expr  */
#line 826 "tuc_transpiler.y"
                                        { (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2447 "tuc_transpiler.tab.c"
    break;

  case 86: /* arithmetic_expr_non_empty: expr LOGICAL_AND_OR_OP expr  */
#line 827 "tuc_transpiler.y"
                                        { (yyval.str) = template("%s %s %s", (yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2453 "tuc_transpiler.tab.c"
    break;

  case 87: /* arithmetic_expr_non_empty: LOGICAL_NOT_OP expr  */
#line 828 "tuc_transpiler.y"
                                        { (yyval.str) = template("%s %s", (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2459 "tuc_transpiler.tab.c"
    break;

  case 88: /* arithmetic_expr_non_empty: UNARY_OP expr  */
#line 829 "tuc_transpiler.y"
                                        { (yyval.str) = template("%s%s", (yyvsp[-1].str), (yyvsp[0].str)); }
#line 2465 "tuc_transpiler.tab.c"
    break;

  case 89: /* $@9: %empty  */
#line 839 "tuc_transpiler.y"
{
    indentation_level++;
    temp8 = template("void %s(){\n", (yyvsp[-3].str));
}
#line 2474 "tuc_transpiler.tab.c"
    break;

  case 90: /* $@10: %empty  */
#line 844 "tuc_transpiler.y"
{      
    indented_stmts_4 = indent_statements((yyvsp[0].str), indentation_level);
    indentation_level--;
}
#line 2483 "tuc_transpiler.tab.c"
    break;

  case 91: /* function_definition: KW_DEF KW_MAIN '(' ')' ':' $@9 body $@10 KW_ENDDEF  */
#line 849 "tuc_transpiler.y"
{     
    if (func_needs_return_type != body_has_return_type) {  
      yyerror("Return statement missing from body"); 
      YYABORT;
    }
    (yyval.str) = template("%s%s}\n", temp8, indented_stmts_4);
}
#line 2495 "tuc_transpiler.tab.c"
    break;

  case 92: /* $@11: %empty  */
#line 861 "tuc_transpiler.y"
{
    add_function((yyvsp[-5].str), num_of_args_in_function);  
    num_of_args_in_function = 0;
    indentation_level++;
    temp8 = template("%s %s(%s){\n", (yyvsp[-1].str), (yyvsp[-5].str), (yyvsp[-3].str) );
}
#line 2506 "tuc_transpiler.tab.c"
    break;

  case 93: /* $@12: %empty  */
#line 868 "tuc_transpiler.y"
{      
    indented_stmts_4 = indent_statements((yyvsp[0].str), indentation_level);
    indentation_level--;
}
#line 2515 "tuc_transpiler.tab.c"
    break;

  case 94: /* function_definition: KW_DEF IDENTIFIER '(' arguments ')' return_type ':' $@11 body $@12 KW_ENDDEF  */
#line 873 "tuc_transpiler.y"
{     
    if (func_needs_return_type != body_has_return_type) {  
      yyerror("Return statement missing from body"); 
      YYABORT;
    }
    (yyval.str) = template("%s%s}\n", temp8, indented_stmts_4);
    func_needs_return_type = 0;
    body_has_return_type = 0;

}
#line 2530 "tuc_transpiler.tab.c"
    break;

  case 95: /* arguments: %empty  */
#line 888 "tuc_transpiler.y"
           { (yyval.str) = ""; }
#line 2536 "tuc_transpiler.tab.c"
    break;

  case 96: /* arguments: parameter_definition  */
#line 889 "tuc_transpiler.y"
                         { num_of_args_in_function++; (yyval.str) = template("%s", (yyvsp[0].str)); }
#line 2542 "tuc_transpiler.tab.c"
    break;

  case 97: /* arguments: arguments ',' parameter_definition  */
#line 890 "tuc_transpiler.y"
                                       { num_of_args_in_function++; (yyval.str) = template("%s, %s", (yyvsp[-2].str), (yyvsp[0].str)); }
#line 2548 "tuc_transpiler.tab.c"
    break;

  case 98: /* parameter_definition: IDENTIFIER ':' KW_VARIABLE_TYPE  */
#line 894 "tuc_transpiler.y"
                                                                { (yyval.str) = template("%s %s", (yyvsp[0].str), (yyvsp[-2].str)); }
#line 2554 "tuc_transpiler.tab.c"
    break;

  case 99: /* parameter_definition: IDENTIFIER '[' POSINT ']' ':' KW_VARIABLE_TYPE  */
#line 895 "tuc_transpiler.y"
                                                                { (yyval.str) = template("%s[%s]: %s", (yyvsp[-5].str), (yyvsp[-3].str), (yyvsp[0].str) ); }
#line 2560 "tuc_transpiler.tab.c"
    break;

  case 100: /* parameter_definition: IDENTIFIER '[' ']' ':' KW_VARIABLE_TYPE  */
#line 896 "tuc_transpiler.y"
                                                                { (yyval.str) = template("%s[]: %s", (yyvsp[-4].str), (yyvsp[0].str) ); }
#line 2566 "tuc_transpiler.tab.c"
    break;

  case 101: /* return_type: %empty  */
#line 901 "tuc_transpiler.y"
          { (yyval.str) = ""; }
#line 2572 "tuc_transpiler.tab.c"
    break;

  case 102: /* return_type: KW_FUNC_RETURN KW_VARIABLE_TYPE  */
#line 902 "tuc_transpiler.y"
                                   { func_needs_return_type = 1; (yyval.str) = template("%s", (yyvsp[0].str)); }
#line 2578 "tuc_transpiler.tab.c"
    break;

  case 103: /* body: stmts  */
#line 906 "tuc_transpiler.y"
        { body_has_return_type = 0; (yyval.str) = template("%s", (yyvsp[0].str)); }
#line 2584 "tuc_transpiler.tab.c"
    break;

  case 104: /* body: stmts KW_RETURN expr ';'  */
#line 907 "tuc_transpiler.y"
                           { body_has_return_type = 1; (yyval.str) = template("%s%s %s;\n", (yyvsp[-3].str), (yyvsp[-2].str) , (yyvsp[-1].str)); }
#line 2590 "tuc_transpiler.tab.c"
    break;

  case 105: /* variable_definition: variable_list ':' KW_VARIABLE_TYPE  */
#line 915 "tuc_transpiler.y"
                                                                        { (yyval.str) = template("%s %s;", (yyvsp[0].str), (yyvsp[-2].str)); }
#line 2596 "tuc_transpiler.tab.c"
    break;

  case 106: /* variable_definition: IDENTIFIER '[' POSINT ']' ':' KW_VARIABLE_TYPE  */
#line 916 "tuc_transpiler.y"
                                                                        { (yyval.str) = template("%s %s[%s];", (yyvsp[0].str), (yyvsp[-5].str), (yyvsp[-3].str) ); }
#line 2602 "tuc_transpiler.tab.c"
    break;

  case 107: /* variable_definition: IDENTIFIER '[' ']' ':' KW_VARIABLE_TYPE  */
#line 917 "tuc_transpiler.y"
                                                                        { (yyval.str) = template("%s[]: %s;", (yyvsp[-4].str), (yyvsp[0].str) ); }
#line 2608 "tuc_transpiler.tab.c"
    break;

  case 108: /* variable_definition: KW_CONST IDENTIFIER ASSIGNMENT_OP VALUE ':' KW_VARIABLE_TYPE  */
#line 918 "tuc_transpiler.y"
                                                                        { (yyval.str) = template("%s %s %s = %s;", (yyvsp[-5].str), (yyvsp[0].str), (yyvsp[-4].str), (yyvsp[-2].str)); }
#line 2614 "tuc_transpiler.tab.c"
    break;

  case 113: /* KW_VARIABLE_TYPE: KW_INTEGER  */
#line 929 "tuc_transpiler.y"
                 {(yyval.str) = (yyvsp[0].str); }
#line 2620 "tuc_transpiler.tab.c"
    break;

  case 116: /* KW_VARIABLE_TYPE: IDENTIFIER  */
#line 932 "tuc_transpiler.y"
               {
    if (is_identifier_in_list((yyvsp[0].str))) {
        (yyval.str) = template("%s", (yyvsp[0].str));
    } else {
        yyerror("Unknown variable type");
        YYABORT;
    }
  }
#line 2633 "tuc_transpiler.tab.c"
    break;

  case 119: /* variable_list: IDENTIFIER  */
#line 948 "tuc_transpiler.y"
               { (yyval.str) = template("%s", (yyvsp[0].str)); }
#line 2639 "tuc_transpiler.tab.c"
    break;

  case 120: /* variable_list: variable_list ',' IDENTIFIER  */
#line 949 "tuc_transpiler.y"
                                 { (yyval.str) = template("%s, %s", (yyvsp[-2].str), (yyvsp[0].str)); }
#line 2645 "tuc_transpiler.tab.c"
    break;


#line 2649 "tuc_transpiler.tab.c"

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

#line 955 "tuc_transpiler.y"


    void print_evaluation(char* result) {

        if (yyerror_count == 0) {
            //printf("===================================================== Expression evaluates to =====================================================\n%s\n", result);
            
            FILE *file = fopen("output.c", "w");
            if (file != NULL) {
                
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
