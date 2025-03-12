%{

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
    
%}

%debug

%union
{
  char* str;
}

%token <str> IDENTIFIER POSINT STRING DECIMAL REAL
%token <str> KW_IF KW_ELSE KW_ENDIF KW_IN KW_OF 
%token <str> KW_WHILE KW_ENDWHILE KW_FOR KW_ENDFOR KW_BREAK KW_CONTINUE
%token <str> KW_TRUE KW_FALSE KW_VAR KW_INT KW_REAL KW_FUNC KW_BEGIN 
%token <str> KW_DEF KW_ENDDEF KW_COMP KW_ENDCOMP KW_MACRO
%token <str> KW_RETURN KW_MAIN KW_FUNC_RETURN
%token <str> KW_INTEGER KW_SCALAR KW_STR KW_CONST KW_BOOLEAN

%type <str> input 
%type <str> macro
%type <str> expr arithmetic_expr_non_empty variable_definition parameter_definition variable_list 
%type <str> KW_VARIABLE_TYPE VALUE
%type <str> func_call func_args stmts function_definition

%start program

%left ';'

%right <str> EXPONENTIATION_OP 
%left <str> CALC_OP    
%right <str> UNARY_OP             
%left <str> RELATIONAL_OP    
%right <str> LOGICAL_NOT_OP       
%left <str> LOGICAL_AND_OR_OP     
%right <str> ASSIGNMENT_OP     


%precedence POSINT
%precedence IDENTIFIER
%precedence ']'
%precedence '['

%%

//==============================================================================================================================================================
//==============================================================================================================================================================

program:
  input { print_evaluation($1); } 

input:  
    %empty                                        {$$ = "";} //@@@@@@@@@
  | input expr ';'                                { assignment_made = 0;  $$ = template("%s%s;\n", $1, $2);}     
  | input variable_definition ';'                 { $$ = template("%s%s\n", $1, $2);}
  | input func_call ';'                           { $$ = template("%s%s;\n", $1, $2); } 
  | input macro                                   {  $$ = template("%s%s", $1, $2);} 

;


function_definition:

  KW_DEF KW_MAIN'('')' ':'  
{
    indentation_level++;
    temp8 = template("void %s(){\n", $2);
}
  body 
{      
    indented_stmts_4 = indent_statements($7, indentation_level);
    indentation_level--;
}
  KW_ENDDEF
{     
    if (func_needs_return_type != body_has_return_type) {  
      yyerror("Return statement missing from body"); 
      YYABORT;
    }
    $$ = template("%s%s}\n", temp8, indented_stmts_4);
}
;
//============================================================================================================================================
// Function Definition: ## transpiled

| KW_DEF IDENTIFIER'(' arguments ')' return_type ':'  
{
    add_function($2, num_of_args_in_function);  
    num_of_args_in_function = 0;
    indentation_level++;
    temp8 = template("%s %s(%s){\n", $6, $2, $4 );
}
  body 
{      
    indented_stmts_4 = indent_statements($9, indentation_level);
    indentation_level--;
}
  KW_ENDDEF
{     
    if (func_needs_return_type != body_has_return_type) {  
      yyerror("Return statement missing from body"); 
      YYABORT;
    }
    $$ = template("%s%s}\n", temp8, indented_stmts_4);
    func_needs_return_type = 0;
    body_has_return_type = 0;

}
;

// Arguments of a function:

arguments: 
    %empty { $$ = ""; } 
  | parameter_definition { num_of_args_in_function++; $$ = template("%s", $1); }                        // int n, int k
  | arguments ',' parameter_definition { num_of_args_in_function++; $$ = template("%s, %s", $1, $3); }
;

parameter_definition:  
   IDENTIFIER ':' KW_VARIABLE_TYPE                              { $$ = template("%s %s", $3, $1); }            
 | IDENTIFIER '[' POSINT ']' ':' KW_VARIABLE_TYPE               { $$ = template("%s[%s]: %s", $1, $3, $6 ); }   
 | IDENTIFIER '[' ']' ':' KW_VARIABLE_TYPE                      { $$ = template("%s[]: %s", $1, $5 ); }
; 

// ## transpiled
return_type:
   %empty { $$ = ""; } 
 | KW_FUNC_RETURN KW_VARIABLE_TYPE { func_needs_return_type = 1; $$ = template("%s", $2); }
;

body: 
  stmts { body_has_return_type = 0; $$ = template("%s", $1); }
| stmts KW_RETURN expr ';' { body_has_return_type = 1; $$ = template("%s%s %s;\n", $1, $2 , $3); }
;



macro:
    KW_MACRO IDENTIFIER STRING {
        char *id = $2; 
        char *value = $3;
        
        add_macro(id, value);  

    }
  | KW_MACRO IDENTIFIER expr {
        char *id = $2;  
        char *value = $3; 

        add_macro(id, value);
    }
;

func_call:

    IDENTIFIER '(' func_args ')' 
    {  
        const DefinedFunction* func = find_function($1);

        if(find_included_function($1) == 1) {used_lambdalib_function = 1;}

        if (!func || (num_of_args_in_function_in_func_call != func->has_arguments && func->has_arguments != 999)) {
            yyerror("Wrong function call or function hasn't been defined"); YYABORT;

        } else {
            $$ = template("%s(%s)", $1, $3);
        }
        num_of_args_in_function_in_func_call = 0;
    }
;

func_args:
    %empty { $$ = ""; } 
  | expr                            { num_of_args_in_function_in_func_call++; $$ = template("%s", $1); }
  | STRING                          { num_of_args_in_function_in_func_call++; $$ = template("%s", $1); }
  | func_args ',' expr              { num_of_args_in_function_in_func_call++; $$ = template("%s, %s", $1, $3); }
  | func_args ',' STRING            { num_of_args_in_function_in_func_call++; $$ = template("%s, %s", $1, $3); }
;

expr:
    // %empty                           { $$ = ""; } // @@@@@@@@@
    POSINT                              { $$ = $1; }
  | REAL                                { $$ = $1; }
  | IDENTIFIER                          { $$ = $1; }
  | '(' expr ')'                        { $$ = template("(%s)", $2); }
  | IDENTIFIER '[' expr ']'             { $$ = template("%s[%s]", $1, $3); }
  | expr CALC_OP expr                   { $$ = template("%s %s %s", $1, $2, $3); }  // Next
  | expr EXPONENTIATION_OP expr         { $$ = template("%s %s %s", $1, $2, $3); }  // Next highest
  | expr RELATIONAL_OP expr             { $$ = template("%s %s %s", $1, $2, $3); }  // Lower than arithmetic
  | expr LOGICAL_AND_OR_OP expr         { $$ = template("%s %s %s", $1, $2, $3); }  // Lower than relational
  | LOGICAL_NOT_OP expr                 { $$ = template("%s %s", $1, $2); }  // Logical not remains unary
  | UNARY_OP expr                       { $$ = template("%s%s", $1, $2); }  
  | IDENTIFIER ASSIGNMENT_OP expr        
  {
      if (assignment_made == 1) {
          yyerror("Multiple assignments in one expression are not allowed");
          YYABORT; 
      }
      assignment_made = 1; 
      $$ = template("%s %s %s", $1, $2, $3); 
  }
  | IDENTIFIER '[' expr ']'  ASSIGNMENT_OP expr     
  {
      if (assignment_made == 1) {
          yyerror("Multiple assignments in one expression are not allowed");
          YYABORT; 
      }
      assignment_made = 1; 
      $$ = template("%s[%s] %s %s", $1, $3, $5, $6 ); 
  }
;

arithmetic_expr_non_empty:
    POSINT                              { $$ = $1; }
  | REAL                                { $$ = $1; }
  | IDENTIFIER                          { $$ = $1; }
  | '(' expr ')'                        { $$ = template("(%s)", $2); }
  | expr EXPONENTIATION_OP expr         { $$ = template("%s %s %s", $1, $2, $3); }
  | expr CALC_OP expr                   { $$ = template("%s %s %s", $1, $2, $3); }
  | expr RELATIONAL_OP expr             { $$ = template("%s %s %s", $1, $2, $3); }
  | expr LOGICAL_AND_OR_OP expr         { $$ = template("%s %s %s", $1, $2, $3); }
  | LOGICAL_NOT_OP expr                 { $$ = template("%s %s", $1, $2); }
  | UNARY_OP expr                       { $$ = template("%s%s", $1, $2); }
;


parameter_definition:  
   IDENTIFIER ':' KW_VARIABLE_TYPE                              { $$ = template("%s %s", $3, $1); }            
 | IDENTIFIER '[' POSINT ']' ':' KW_VARIABLE_TYPE               { $$ = template("%s[%s]: %s", $1, $3, $6 ); }   
 | IDENTIFIER '[' ']' ':' KW_VARIABLE_TYPE                      { $$ = template("%s[]: %s", $1, $5 ); }
; 

//==============================================================================================================================================================
// Variable Definition: 

variable_definition:  
   variable_list ':' KW_VARIABLE_TYPE                                   { $$ = template("%s %s;", $3, $1); }                // a, b: integer; => int a, b;
 | IDENTIFIER '[' POSINT ']' ':' KW_VARIABLE_TYPE                       { $$ = template("%s %s[%s];", $6, $1, $3 ); }       // grades[5]: scalar; => double grades[5];
 | IDENTIFIER '[' ']' ':' KW_VARIABLE_TYPE                              { $$ = template("%s[]: %s;", $1, $5 ); }
 | KW_CONST IDENTIFIER ASSIGNMENT_OP VALUE ':' KW_VARIABLE_TYPE         { $$ = template("%s %s %s = %s;", $1, $6, $2, $4); }
; 

VALUE:
    POSINT 
  | REAL
  | STRING
;

KW_VARIABLE_TYPE:
    KW_SCALAR 
  | KW_INTEGER   {$$ = $1; }
  | KW_STR 
  | KW_BOOLEAN
  | IDENTIFIER {
    if (is_identifier_in_list($1)) {
        $$ = template("%s", $1);
    } else {
        yyerror("Unknown variable type");
        YYABORT;
    }
  }
;

variable_list:
    IDENTIFIER { $$ = template("%s", $1); }
  | variable_list ',' IDENTIFIER { $$ = template("%s, %s", $1, $3); }
;

%%

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