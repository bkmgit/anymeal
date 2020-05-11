%{
#include "mealmaster.hh"

std::istream *yystream;
Ingredient ingredient_;
Recipe recipe;

extern int yylex(void);

#define YY_INPUT(buffer, result, max_size) { \
  yystream->read(buffer, max_size); \
  result = yystream->gcount(); \
}

Recipe parse_mealmaster(std::istream &stream) {
  recipe = Recipe();
  yystream = &stream;
  yylex();
  yyrestart(NULL);
  yy_start = 1;
  return recipe;
}

%}

%option noyywrap
%option never-interactive
%option nostdinit

%x title titletext categories categoriestext servings servingsamount servingsunit ingredient amount1 amount2 amount3 ingredienttext

UNIT "x "|"sm"|"md"|"lg"|"cn"|"pk"|"pn"|"dr"|"ds"|"ct"|"bn"|"sl"|"ea"|"t "|"ts"|"T "|"tb"|"fl"|"c "|"pt"|"qt"|"ga"|"oz"|"lb"|"ml"|"cb"|"cl"|"dl"|"l "|"mg"|"cg"|"dg"|"g "|"kg"|"  "

%%

<INITIAL>(MMMMM|-----)[^\r\n]*[Mm][Ee][Aa][Ll]-[Mm][Aa][Ss][Tt][Ee][Rr][^\r\n]*\r?\n {
  BEGIN(title);
}

<INITIAL>\r?\n

<INITIAL>.

<title>[\ \t]
<title>\r?\n
<title>"Title: " {
  BEGIN(titletext);
}

<titletext>[^\r\n]* {
  recipe.set_title(yytext);
}
<titletext>\r?\n {
  BEGIN(categories);
}

<categories>[\ \t]
<categories>"Categories: " {
  BEGIN(categoriestext);
}

<categoriestext>[^\r\n,]* {
  recipe.add_category(yytext);
}
<categoriestext>,\ *
<categoriestext>\r?\n {
  BEGIN(servings);
}

<servings>[\ \t]
<servings>"Yield: "|"Servings: " {
  BEGIN(servingsamount);
}

<servingsamount>[0-9]+ {
  recipe.set_servings(atoi(yytext));
  BEGIN(servingsunit);
}

<servingsunit>[\ \t]
<servingsunit>[^\ \t\r\n][^\r\n]* {
  recipe.set_servings_unit(yytext);
}
<servingsunit>\r?\n {
  BEGIN(ingredient);
}

<ingredient>\r?\n

<ingredient>\ {0,6}[0-9]+/\  {
  ingredient_ = Ingredient();
  ingredient_.set_amount_type(AMOUNT_INTEGER);
  ingredient_.set_amount_integer(atoi(yytext));
  BEGIN(amount1);
}

<amount1>\  {
  BEGIN(amount2);
}
<amount2>{UNIT} {
  ingredient_.set_unit(yytext);
  BEGIN(amount3);
}

<amount3>\  {
  BEGIN(ingredienttext);
}

<ingredienttext>[^\r\n]* {
  ingredient_.add_text(yytext);
  recipe.add_ingredient(ingredient_);
  BEGIN(INITIAL);
}

<*>(MMMMM|-----)\r?\n {
  BEGIN(INITIAL);
  return 0;
}

%%
