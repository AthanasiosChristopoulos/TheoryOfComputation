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
    //printf("================================================\n" );

    for (int i = 0; i < num_of_included_functions; i++) {
        //printf("%s\n",functionname );
        if (strcmp(included_functionnames[i], functionname) == 0) {
            return 1; 
        }
        //printf("================================================\n" );

    }
    return 0; 
}

// void initialize_predefined_functions() {

//     // Define an array of function names and their argument counts
//     const char *function_names[] = {
//         "readStr", "readInteger", "readScalar",
//         "writeStr", "writeInteger", "writeScalar", "write"
//     };

//     int arg_counts[] = { 0, 0, 0, 1, 1, 1, 2 };
//     int num_predefined_functions = sizeof(function_names) / sizeof(function_names[0]);

//     for (int i = 0; i < num_predefined_functions; i++) {
//         add_function(function_names[i], arg_counts[i]);
//         included_functionnames[i] = strdup(function_names[i]);
//     }

//     num_of_included_functions = num_predefined_functions;
// }

//=========================================================================================================================

int count_args(const char *line) {
    int count = 0;
    const char *p = strchr(line, '(');
    
    if (p) {
        p++;  // Move past '('
        
        // Skip any whitespace
        while (*p == ' ' || *p == '\t') {
            p++;
        }

        // If the next character is ')', it means there are 0 arguments
        if (*p == ')') {
            return 0;
        }

        // Otherwise, count the commas (,) which separate arguments
        while (*p != ')' && *p != '\0') {
            if (*p == ',') {
                count++;
            }
            p++;
        }
        
        count++;  // Count the last argument
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
        start++; // Skip first word
    }
    while (isspace(*start)) {
        start++; // Move to function name
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
            // printf("%s\n", func_name);
            int num_args = count_args(line);
            printf("%d\n", num_args);
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

%}

%debug

%union
{
  char* str;
}

%token <str> IDENTIFIER POSINT STRING DECIMAL REAL
%token <str> KW_IF KW_THEN KW_ELSE KW_ENDIF KW_IN KW_OF 
%token <str> KW_WHILE KW_ENDWHILE KW_FOR KW_ENDFOR KW_BREAK KW_CONTINUE
%token <str> KW_TRUE KW_FALSE KW_VAR KW_INT KW_REAL KW_FUNC KW_BEGIN 
%token <str> KW_DEF KW_ENDDEF KW_COMP KW_ENDCOMP KW_MACRO
%token <str> KW_RETURN KW_MAIN KW_FUNC_RETURN
%token <str> KW_INTEGER KW_SCALAR KW_STR KW_CONST KW_BOOLEAN

%type <str> KW_VARIABLE_TYPE VALUE
%type <str> expr arithmetic_expr_non_empty variable_definition parameter_definition variable_list 
%type <str> function_definition body arguments return_type 
%type <str> func_call func_args 
%type <str> complicated_types attribute_definition function_definition_comp return_type_comp body_comp expr_comp
%type <str> commands stmts else_statement step loop_thing
%type <str> macro input 

%start program

%right <str> EXPONENTIATION_OP 
%left <str> CALC_OP    
%right <str> UNARY_OP             
%left <str> RELATIONAL_OP    
%right <str> LOGICAL_NOT_OP       
%left <str> LOGICAL_AND_OR_OP     
%right <str> ASSIGNMENT_OP        


%%

//==============================================================================================================================================================
//==============================================================================================================================================================

program:
  input { print_evaluation($1); } 

input:  
    %empty                                        {$$ = "";} //@@@@@@@@@
  | input func_call ';'                           { $$ = template("%s%s;\n", $1, $2); } 
  | input variable_definition ';'                 { $$ = template("%s%s\n", $1, $2);}
  | input expr ';'                                { assignment_made = 0;  $$ = template("%s%s;\n", $1, $2);}

//| input expr                                    { assignment_made = 0; $$ = template("%s%s;", $1, $2);}

  | input function_definition ';'                 { assignment_made = 0; $$ = template("%s%s", $1, $2);}
  | input complicated_types ';'                   { assignment_made = 0;  $$ = template("%s%s", $1, $2);} 
  | input commands ';'                            {  $$ = template("%s%s", $1, $2);} 
  | input macro                                   {  $$ = template("%s%s", $1, $2);} 
;

//==============================================================================================================================================================
// macros:

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


//==============================================================================================================================================================
// if-while-for:

commands:
    KW_IF '(' expr ')' ':' {temp1 = template("%s (%s):\n", $1, $3); indentation_level++; is_a_loop = 0;} 
    stmts 
    {
        indented_stmts = indent_statements($7, indentation_level);
        indentation_level--;
    }
    else_statement KW_ENDIF {
        temp3 = template("%s%s", $9, $10); 
        $$ = template("%s%s%s;\n", temp1, indented_stmts, temp3);
    }
    //==============================================================================================================================================================
    | KW_WHILE '(' expr ')' ':' 
    {
        temp7 = template("%s (%s):\n", $1, $3);
        indentation_level++; is_a_loop = 1;
    }
    stmts 
    {
        indented_stmts_3 = indent_statements($7, indentation_level);
        indentation_level--;
    }
    KW_ENDWHILE {
        $$ = template("%s%s%s;\n", temp7, indented_stmts_3, $9);
        is_a_loop = 0;
    }
    //==============================================================================================================================================================
    // ## transpiled
    | KW_FOR IDENTIFIER KW_IN '[' arithmetic_expr_non_empty ':' arithmetic_expr_non_empty step ']' ':' 
    {
        temp5 = template("for (int %s = %s; %s < %s; %s++) {\n", $2, $5, $2,$7, $8);  
        indentation_level++; is_a_loop = 1;
    }
    stmts 
    {
        indented_stmts_2 = indent_statements($12, indentation_level);
        indentation_level--;
    }
    KW_ENDFOR {

        $$ = template("%s%s%s;\n", temp5, indented_stmts_2, $14);
        is_a_loop = 0;
    }

    //==============================================================================================================================================================
    // assignments: ## transpiled

    | IDENTIFIER ASSIGNMENT_OP '[' expr KW_FOR IDENTIFIER ':' POSINT ']' ':' KW_VARIABLE_TYPE  
    {
        char* newType = $11;
        // char* newType = handle_variable_type($11);

        $$ = template("%s* %s =(%s)malloc(%s*sizeof(%s));\nfor (int %s = 0; %s < %s; ++%s) {\n  %s[%s] = %s;\n}\n", 
                        newType, $1,newType,$8, newType, $4, $4, $8, $4, $1, $4, $4);

    }
    //==============================================================================================================================================================
    // ## transpiled

    | IDENTIFIER ASSIGNMENT_OP '[' expr KW_FOR IDENTIFIER ':' KW_VARIABLE_TYPE KW_IN IDENTIFIER KW_OF POSINT ']' ':' KW_VARIABLE_TYPE
    {
        // Get the transformed type for $15 (KW_VARIABLE_TYPE)
        char* newType = $15;

        $$ = template(
            "%s* %s = (%s)malloc(%s * sizeof(%s));\n"
            "for (int %s_i = 0; %s_i < %s; ++%s_i) {\n"
            "  %s[%s_i] = %s[%s_i] / 2;\n"
            "}\n",
            newType, $1, newType, $12, newType, $10, $10, $12, $10, $1, $10, $12, $10
        );
    }
    ;

//==============================================================================================================================================================
// else: 

else_statement:
    %empty                                           { $$ = ""; }
  | KW_ELSE ':'                       {temp4 = template("%s:\n", $1); indentation_level++;}
  stmts 
  {         
      indented_stmts_1 = indent_statements($4, indentation_level);
      indentation_level--;
       $$ = template("%s%s", temp4, indented_stmts_1);
  }

step:
    %empty                      { $$ = ""; }
  | ':' arithmetic_expr_non_empty { $$ = template(":%s", $2);}
;


//==============================================================================================================================================================
//statement:

stmts:
    %empty                                        { $$ = ""; }
  | stmts loop_thing ';'                           
  {     
    if (is_a_loop != 1) {  
      yyerror("This isnt a loop"); 
      YYABORT;
    }
    $$ = template("%s%s;\n", $1, $2);} 
  | stmts func_call ';'                           { $$ = template("%s%s;\n", $1, $2);} 
  | stmts variable_definition ';'                 { $$ = template("%s%s\n", $1, $2); }
  | stmts expr ';'                                
  { 
    assignment_made = 0; 
    $$ = template("%s%s;\n", $1, $2); 
  }
  // | stmts function_definition ';'                 { assignment_made = 0; $$ = template("%s%s;\n", $1, $2);}  ????
  // | stmts complicated_types ';'                   { assignment_made = 0;  $$ = template("%s%s;\n", $1, $2);} ????
  | stmts commands ';'                            { $$ = template("%s%s", $1, $2); } 

loop_thing:
  KW_BREAK|KW_CONTINUE;

//==============================================================================================================================================================
// comp: 

complicated_types:

  KW_COMP IDENTIFIER { func_to_define_comp = $2;}
  ':' body_comp KW_ENDCOMP 
  { 
    comp = 1; add_identifier($2); 
    for (int i = 0; i < function_list_count; i++) {
      add_function_to_component($2, list_of_comp_functions[i]);
    }
    clear_function_list();
    $$ = template("%s %s:\n%s%s;\n", $1, $2, $5, $6); comp = 0;   
  }

body_comp: 
    %empty                                        { $$ = ""; }
  | body_comp attribute_definition ';'           { $$ = template("%s  %s;\n", $1, $2); }
  | body_comp function_definition_comp ';'       {  $$ = template("%s  %s;\n", $1, $2); }

;

attribute_definition:
  '#' IDENTIFIER ':' KW_VARIABLE_TYPE            { add_identifier($2); $$ = template("#%s: %s", $2, $4); }
 |'#' IDENTIFIER ',' attribute_definition        { add_identifier($2); $$ = template("#%s, %s", $2, $4); }


//==============================================================================================================================================================
// function_in_complicated_type:

function_definition_comp:

  KW_DEF IDENTIFIER'(' arguments ')' return_type_comp ':' body_comp KW_ENDDEF 
  {
    if (func_needs_return_type != body_has_return_type) {  
      yyerror("Return statement missing from body"); 
      YYABORT;
    }
    $$ = template("%s %s(%s)%s:   \n%s  %s", $1, $2, $4, $6, $8 , $9 );
    func_needs_return_type = 0;
    body_has_return_type = 0;

    add_function($2, num_of_args_in_function);  
    num_of_args_in_function = 0;
    append_to_function_list($2);
  }
;

return_type_comp:
   %empty                               { $$ = ""; } 
 | KW_FUNC_RETURN KW_VARIABLE_TYPE      { func_needs_return_type = 1; $$ = template("%s %s", $1, $2); }
;

body_comp: 
    %empty                            { $$ = ""; }
  | body_comp expr_comp ';'           { assignment_made = 0; $$ = template("%s    %s;\n", $1, $2); }
  | KW_RETURN expr_comp ';'           { body_has_return_type = 1;$$ = template("    %s #%s;\n", $1 , $2 ); }
;

expr_comp:
    %empty                                     { $$ = ""; }  
  | POSINT                                      { $$ = $1; }
  | func_call                                   { $$ = template("%s", $1);  } 
  | REAL                                        { $$ = $1; }
  |'#' IDENTIFIER                                { $$ = template("#%s", $2); }
  |'#' IDENTIFIER '[' expr ']'                      { $$ = template("#%s[%s]", $2, $4); }
  | IDENTIFIER                                     { $$ = template("%s", $1); }
  | IDENTIFIER '[' expr ']'                          { $$ = template("%s[%s]", $1, $3); }
  | '(' expr_comp ')'                           { $$ = template("(%s)", $2); }
  | expr_comp EXPONENTIATION_OP expr_comp                   { $$ = template("%s %s %s", $1, $2, $3); }
  | expr_comp CALC_OP expr_comp                 { $$ = template("%s %s %s", $1, $2, $3); }
  | expr_comp RELATIONAL_OP expr_comp                 { $$ = template("%s %s %s", $1, $2, $3); }
  | expr_comp LOGICAL_AND_OR_OP expr_comp                { $$ = template("%s %s %s", $1, $2, $3); }
  | LOGICAL_NOT_OP expr_comp                { $$ = template("%s %s", $1, $2); }
  | UNARY_OP expr_comp                { $$ = template("%s %s", $1, $2); }
  | '#' IDENTIFIER ASSIGNMENT_OP expr_comp      
  {
      if (assignment_made == 1) {
          yyerror("Multiple assignments in one expression are not allowed");
          YYABORT; 
      }
      assignment_made = 1; 
      $$ = template("#%s %s %s", $2, $3, $4); 
  }
  | '#' IDENTIFIER '[' expr_comp ']'  ASSIGNMENT_OP expr_comp        
  {
      if (assignment_made == 1) {
          yyerror("Multiple assignments in one expression are not allowed");
          YYABORT; 
      }
      assignment_made = 1; 
      $$ = template("%s[%s] %s %s", $2, $4, $6, $7 ); 
  }
;
//==============================================================================================================================================================
// functions:

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
  | expr                          { num_of_args_in_function_in_func_call++; $$ = template("%s", $1); }
  | IDENTIFIER                    { num_of_args_in_function_in_func_call++; $$ = template("%s", $1); }
  | STRING                    { num_of_args_in_function_in_func_call++; $$ = template("%s", $1); }
  | func_args ',' expr            { num_of_args_in_function_in_func_call++; $$ = template("%s, %s", $1, $3); }
  | func_args ',' IDENTIFIER      { num_of_args_in_function_in_func_call++; $$ = template("%s, %s", $1, $3); }
  | func_args ',' STRING      { num_of_args_in_function_in_func_call++; $$ = template("%s, %s", $1, $3); }
;

//==============================================================================================================================================================
// expressions:

expr:
    // %empty                      { $$ = ""; } // @@@@@@@@@
    func_call                   { $$ = template("%s", $1);  } 
  | POSINT                      { $$ = $1; }
  | REAL                        { $$ = $1; }
  | IDENTIFIER                  { $$ = $1; }
  | '(' expr ')'                { $$ = template("(%s)", $2); }
  | IDENTIFIER '[' expr ']'             { $$ = template("%s[%s]", $1, $3); }
  | expr CALC_OP expr           { $$ = template("%s %s %s", $1, $2, $3); }  // Next
  | expr EXPONENTIATION_OP expr { $$ = template("%s %s %s", $1, $2, $3); }  // Next highest
  | expr RELATIONAL_OP expr     { $$ = template("%s %s %s", $1, $2, $3); }  // Lower than arithmetic
  | expr LOGICAL_AND_OR_OP expr { $$ = template("%s %s %s", $1, $2, $3); }  // Lower than relational
  | LOGICAL_NOT_OP expr         { $$ = template("%s %s", $1, $2); }  // Logical not remains unary
  | UNARY_OP expr               { $$ = template("%s%s", $1, $2); }  
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
    POSINT                      { $$ = $1; }
  | REAL                        { $$ = $1; }
  | IDENTIFIER                  { $$ = $1; }
  | '(' expr ')'                { $$ = template("(%s)", $2); }
  | expr EXPONENTIATION_OP expr                   { $$ = template("%s %s %s", $1, $2, $3); }
  | expr CALC_OP expr           { $$ = template("%s %s %s", $1, $2, $3); }
  | expr RELATIONAL_OP expr      { $$ = template("%s %s %s", $1, $2, $3); }
  | expr LOGICAL_AND_OR_OP expr                { $$ = template("%s %s %s", $1, $2, $3); }
  | LOGICAL_NOT_OP expr                { $$ = template("%s %s", $1, $2); }
  | UNARY_OP expr                { $$ = template("%s%s", $1, $2); }
;

//==============================================================================================================================================================
//==============================================================================================================================================================
// ## transpiled

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
    $$ = template("%s%s}", temp8, indented_stmts_4);
}
;
//============================================================================================================================================

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
    $$ = template("%s%s}", temp8, indented_stmts_4);
    func_needs_return_type = 0;
    body_has_return_type = 0;

}
;
arguments: 
    %empty { $$ = ""; } 
  | parameter_definition { num_of_args_in_function++; $$ = template("%s", $1); }
  | arguments ',' parameter_definition { num_of_args_in_function++; $$ = template("%s, %s", $1, $3); }
;

parameter_definition:  
   IDENTIFIER ':' KW_VARIABLE_TYPE                              { $$ = template("%s: %s", $1, $3); }
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


//==============================================================================================================================================================
//==============================================================================================================================================================


variable_definition:  
   variable_list ':' KW_VARIABLE_TYPE                          { $$ = template("%s: %s;", $1, $3); }
 | IDENTIFIER '[' POSINT ']' ':' KW_VARIABLE_TYPE              { $$ = template("%s[%s]: %s;", $1, $3, $6 ); }
 | IDENTIFIER '[' ']' ':' KW_VARIABLE_TYPE                      { $$ = template("%s[]: %s;", $1, $5 ); }
 | KW_CONST IDENTIFIER ASSIGNMENT_OP VALUE ':' KW_VARIABLE_TYPE         { $$ = template("%s %s = %s: %s;", $1, $2, $4, $6 ); }
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

//==============================================================================================================================================================
//==============================================================================================================================================================

%%

    void print_evaluation(char* result) {

        if (yyerror_count == 0) {
            printf("===================================================== Expression evaluates to =====================================================\n%s\n", result);
            
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