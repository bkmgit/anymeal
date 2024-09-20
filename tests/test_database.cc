/* AnyMeal recipe management software
   Copyright (C) 2021, 2023 Jan Wedekind

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>. */
#include <gtest/gtest.h>
#include "database.hh"


using namespace std;
using namespace testing;

TEST(DatabaseTest, OpenDatabase) {
  Database database;
  database.open(":memory:");
}

int has_row(void *exist, int, char**, char**) {
  *(int *)exist += 1;
  return 0;
}

TEST(DatabaseTest, CreateRecipeTable) {
  Database database;
  database.open(":memory:");
  int exist = 0;
  sqlite3_exec(database.db(), "SELECT name FROM sqlite_master WHERE type='table' AND name='recipes';", &has_row, &exist, NULL);
  EXPECT_EQ(exist, 1);
}

TEST(DatabaseTest, FailedToCreate) {
  Database database;
  EXPECT_THROW(database.open("/tmp/nosuchdir/anymeal.db"), database_exception);
}

TEST(DatabaseTest, AddRecipeTitle) {
  Database database;
  database.open(":memory:");
  Recipe recipe;
  recipe.set_title("apple pie");
  database.insert_recipe(recipe);
  int exist = 0;
  sqlite3_exec(database.db(), "SELECT title FROM recipes;", &has_row, &exist, NULL);
  EXPECT_EQ(exist, 1);
}

TEST(DatabaseTest, BeginAndCommit) {
  Database database;
  database.open(":memory:");
  Recipe recipe;
  recipe.set_title("apple pie");
  database.begin();
  database.insert_recipe(recipe);
  database.commit();
  int exist = 0;
  sqlite3_exec(database.db(), "SELECT title FROM recipes;", &has_row, &exist, NULL);
  EXPECT_EQ(exist, 1);
}

TEST(DatabaseTest, BeginAndRollback) {
  Database database;
  database.open(":memory:");
  Recipe recipe;
  recipe.set_title("apple pie");
  database.begin();
  database.insert_recipe(recipe);
  database.rollback();
  int exist = 0;
  sqlite3_exec(database.db(), "SELECT title FROM recipes;", &has_row, &exist, NULL);
  EXPECT_EQ(exist, 0);
}

TEST(DatabaseTest, AddCategories) {
  Database database;
  database.open(":memory:");
  Recipe recipe;
  recipe.add_category("Dessert");
  recipe.add_category("Muffins");
  recipe.add_category("Muffins");
  database.insert_recipe(recipe);
  int exist = 0;
  sqlite3_exec(database.db(), "SELECT name FROM categories;", &has_row, &exist, NULL);
  EXPECT_EQ(exist, 2);
  int recipe_category = 0;
  sqlite3_exec(database.db(), "SELECT categories.name FROM categories, category WHERE category.recipeid = 1 AND "
               "category.categoryid = categories.id", &has_row, &recipe_category, NULL);
  EXPECT_EQ(recipe_category, 2);
}

TEST(DatabaseTest, NoRecipeFound) {
  Database database;
  database.open(":memory:");
  EXPECT_THROW(database.fetch_recipe(1), database_exception);
}

TEST(DatabaseTest, RecipeHeaderRoundtrip) {
  Database database;
  database.open(":memory:");
  Recipe recipe;
  recipe.set_title("apple pie");
  recipe.set_servings(123);
  recipe.set_servings_unit("servings");
  database.insert_recipe(recipe);
  Recipe result = database.fetch_recipe(1);
  EXPECT_EQ("apple pie", result.title());
  EXPECT_EQ(123, result.servings());
  EXPECT_EQ("servings", result.servings_unit());
}

TEST(DatabaseTest, TestLongTitle) {
  Database database;
  database.open(":memory:");
  Recipe recipe;
  recipe.set_title("aaaaaaaaaaaaaaaaaaaaaaaaaaaa");
  database.insert_recipe(recipe);
  Recipe result = database.fetch_recipe(1);
  EXPECT_EQ("aaaaaaaaaaaaaaaaaaaaaaaaaaaa", result.title());
}

TEST(DatabaseTest, CategoriesRoundtrip) {
  Database database;
  database.open(":memory:");
  Recipe recipe;
  recipe.add_category("cakes");
  recipe.add_category("sweet");
  database.insert_recipe(recipe);
  Recipe result = database.fetch_recipe(1);
  ASSERT_EQ(2, result.categories().size());
  set<string>::iterator category = result.categories().begin();
  EXPECT_EQ("cakes", *category);
  category++;
  EXPECT_EQ("sweet", *category);
}

TEST(DatabaseTest, IngredientRoundtrip) {
  Database database;
  database.open(":memory:");
  Recipe recipe;
  Ingredient ingredient;
  ingredient.set_amount_integer(2);
  ingredient.set_amount_numerator(3);
  ingredient.set_amount_denominator(5);
  ingredient.set_amount_float(2.6);
  ingredient.set_unit("g ");
  ingredient.add_text("apples");
  recipe.add_ingredient(ingredient);
  database.insert_recipe(recipe);
  Recipe result_recipe = database.fetch_recipe(1);
  ASSERT_EQ(1, result_recipe.ingredients().size());
  Ingredient result = result_recipe.ingredients()[0];
  EXPECT_EQ(2, result.amount_integer());
  EXPECT_EQ(3, result.amount_numerator());
  EXPECT_EQ(5, result.amount_denominator());
  EXPECT_EQ(2.6, result.amount_float());
  EXPECT_EQ("g ", result.unit());
  EXPECT_EQ("apples", result.text());
}

TEST(DatabaseTest, InstructionsRoundtrip) {
  Database database;
  database.open(":memory:");
  Recipe recipe;
  recipe.add_instruction("Start cooking.");
  recipe.add_instruction("Finish cooking.");
  database.insert_recipe(recipe);
  Recipe result = database.fetch_recipe(1);
  ASSERT_EQ(2, result.instructions().size());
  EXPECT_EQ("Start cooking.", result.instructions()[0]);
  EXPECT_EQ("Finish cooking.", result.instructions()[1]);
}

TEST(DatabaseTest, IngredientSectionRoundtrip) {
  Database database;
  database.open(":memory:");
  Recipe recipe;
  recipe.add_ingredient_section(3, "paste");
  database.insert_recipe(recipe);
  Recipe result = database.fetch_recipe(1);
  ASSERT_EQ(1, result.ingredient_sections().size());
  EXPECT_EQ(3, result.ingredient_sections()[0].first);
  EXPECT_EQ("paste", result.ingredient_sections()[0].second);
}

TEST(DatabaseTest, InstructionSectionRoundtrip) {
  Database database;
  database.open(":memory:");
  Recipe recipe;
  recipe.add_instruction_section(3, "paste");
  database.insert_recipe(recipe);
  Recipe result = database.fetch_recipe(1);
  ASSERT_EQ(1, result.instruction_sections().size());
  EXPECT_EQ(3, result.instruction_sections()[0].first);
  EXPECT_EQ("paste", result.instruction_sections()[0].second);
}

TEST(DatabaseTest, GetRecipeCountAndTitles) {
  Database database;
  database.open(":memory:");
  Recipe recipe1;
  recipe1.set_title("Recipe B");
  database.insert_recipe(recipe1);
  Recipe recipe2;
  recipe2.set_title("Recipe A");
  database.insert_recipe(recipe2);
  ASSERT_EQ(2, database.num_recipes());
  vector<pair<sqlite3_int64, string> > info = database.recipe_info();
  EXPECT_EQ(2, info[0].first);
  EXPECT_EQ("Recipe A", info[0].second);
  EXPECT_EQ(1, info[1].first);
  EXPECT_EQ("Recipe B", info[1].second);
}

TEST(DatabaseTest, SelectByTitle) {
  Database database;
  database.open(":memory:");
  Recipe recipe1;
  recipe1.set_title("Recipe B");
  database.insert_recipe(recipe1);
  Recipe recipe2;
  recipe2.set_title("Recipe A");
  database.insert_recipe(recipe2);
  database.select_all();
  database.select_by_title("B");
  ASSERT_EQ(1, database.num_recipes());
  vector<pair<sqlite3_int64, string> > info = database.recipe_info();
  EXPECT_EQ(1, info[0].first);
  EXPECT_EQ("Recipe B", info[0].second);
}

TEST(DatabaseTest, GetCategoriesOrderedByRecipeCount) {
  Database database;
  database.open(":memory:");
  Recipe recipe1;
  recipe1.add_category("A");
  database.insert_recipe(recipe1);
  Recipe recipe2;
  recipe2.add_category("B");
  database.insert_recipe(recipe2);
  Recipe recipe3;
  recipe3.add_category("B");
  database.insert_recipe(recipe3);
  database.select_all();
  vector<string> result = database.categories();
  ASSERT_EQ(2, result.size());
  EXPECT_EQ("B", result[0]);
  EXPECT_EQ("A", result[1]);
}

TEST(DatabaseTest, GetCategoriesOrderAlphabeticalWhenSameRecipeCount) {
  Database database;
  database.open(":memory:");
  Recipe recipe1;
  recipe1.add_category("A");
  database.insert_recipe(recipe1);
  Recipe recipe2;
  recipe2.add_category("B");
  database.insert_recipe(recipe2);
  database.select_all();
  vector<string> result = database.categories();
  ASSERT_EQ(2, result.size());
  EXPECT_EQ("A", result[0]);
  EXPECT_EQ("B", result[1]);
}

TEST(DatabaseTest, GetCategoriesAlphabeticalWithRecipeCount) {
  Database database;
  database.open(":memory:");
  Recipe recipe1;
  recipe1.add_category("A");
  database.insert_recipe(recipe1);
  Recipe recipe2;
  recipe2.add_category("B");
  database.insert_recipe(recipe2);
  Recipe recipe3;
  recipe3.add_category("B");
  database.insert_recipe(recipe3);
  vector<pair<string, int>> result = database.categories_and_counts();
  ASSERT_EQ(2, result.size());
  EXPECT_EQ("A", result[0].first);
  EXPECT_EQ("B", result[1].first);
  EXPECT_EQ(1, result[0].second);
  EXPECT_EQ(2, result[1].second);
}

TEST(DatabaseTest, GetCategoriesWithZeroRecipes) {
  Database database;
  database.open(":memory:");
  Recipe recipe;
  recipe.add_category("A");
  vector<sqlite3_int64> recipe_ids;
  recipe_ids.push_back(database.insert_recipe(recipe));
  database.delete_recipes(recipe_ids);
  vector<pair<string, int>> result = database.categories_and_counts();
  ASSERT_EQ(1, result.size());
  EXPECT_EQ("A", result[0].first);
  EXPECT_EQ(0, result[0].second);
}

TEST(DatabaseTest, SelectByCategory) {
  Database database;
  database.open(":memory:");
  Recipe recipe1;
  recipe1.set_title("Recipe A");
  recipe1.add_category("A");
  database.insert_recipe(recipe1);
  Recipe recipe2;
  recipe2.set_title("Recipe B");
  recipe2.add_category("B");
  database.insert_recipe(recipe2);
  database.select_all();
  database.select_by_category("B");
  ASSERT_EQ(1, database.num_recipes());
  vector<pair<sqlite3_int64, string> > info = database.recipe_info();
  EXPECT_EQ("Recipe B", info[0].second);
}

TEST(DatabaseTest, SelectByNoCategory) {
  Database database;
  database.open(":memory:");
  Recipe recipe1;
  recipe1.set_title("Recipe A");
  recipe1.add_category("A");
  database.insert_recipe(recipe1);
  Recipe recipe2;
  recipe2.set_title("Recipe B");
  recipe2.add_category("B");
  database.insert_recipe(recipe2);
  database.select_all();
  database.select_by_no_category("A");
  ASSERT_EQ(1, database.num_recipes());
  vector<pair<sqlite3_int64, string> > info = database.recipe_info();
  EXPECT_EQ("Recipe B", info[0].second);
}

TEST(DatabaseTest, SelectByIngredient) {
  Database database;
  database.open(":memory:");
  Recipe recipe1;
  recipe1.set_title("Recipe A");
  Ingredient ingredient1;
  ingredient1.add_text("apples");
  recipe1.add_ingredient(ingredient1);
  database.insert_recipe(recipe1);
  Recipe recipe2;
  recipe2.set_title("Recipe B");
  Ingredient ingredient2;
  ingredient2.add_text("bananas");
  recipe2.add_ingredient(ingredient2);
  database.insert_recipe(recipe2);
  database.select_all();
  database.select_by_ingredient("bananas");
  ASSERT_EQ(1, database.num_recipes());
  vector<pair<sqlite3_int64, string> > info = database.recipe_info();
  EXPECT_EQ("Recipe B", info[0].second);
}

TEST(DatabaseTest, SelectByNoIngredient) {
  Database database;
  database.open(":memory:");
  Recipe recipe1;
  recipe1.set_title("Recipe A");
  Ingredient ingredient1;
  ingredient1.add_text("apples");
  recipe1.add_ingredient(ingredient1);
  database.insert_recipe(recipe1);
  Recipe recipe2;
  recipe2.set_title("Recipe B");
  Ingredient ingredient2;
  ingredient2.add_text("bananas");
  recipe2.add_ingredient(ingredient2);
  database.insert_recipe(recipe2);
  database.select_all();
  database.select_by_no_ingredient("bananas");
  ASSERT_EQ(1, database.num_recipes());
  vector<pair<sqlite3_int64, string> > info = database.recipe_info();
  EXPECT_EQ("Recipe A", info[0].second);
}

TEST(DatabaseTest, DeleteRecipes) {
  Database database;
  database.open(":memory:");
  Recipe recipe1;
  recipe1.set_title("Recipe A");
  recipe1.add_category("A");
  Ingredient ingredient1;
  ingredient1.add_text("apples");
  recipe1.add_ingredient(ingredient1);
  database.insert_recipe(recipe1);
  Recipe recipe2;
  recipe2.set_title("Recipe B");
  recipe2.add_category("B");
  Ingredient ingredient2;
  ingredient2.add_text("bananas");
  recipe2.add_ingredient(ingredient2);
  database.insert_recipe(recipe2);
  database.select_all();
  vector<sqlite3_int64> ids;
  ids.push_back(1);
  database.delete_recipes(ids);
  database.garbage_collect();
  ASSERT_EQ(1, database.num_recipes());
  vector<pair<sqlite3_int64, string> > info = database.recipe_info();
  EXPECT_EQ("Recipe B", info[0].second);
  int exist = 0;
  sqlite3_exec(database.db(), "SELECT name FROM categories;", &has_row, &exist, NULL);
  EXPECT_EQ(exist, 1);
  exist = 0;
  sqlite3_exec(database.db(), "SELECT name FROM ingredients;", &has_row, &exist, NULL);
  EXPECT_EQ(exist, 1);
}

TEST(DatabaseTest, AddRecipeToCategory) {
  Database database;
  database.open(":memory:");
  Recipe recipe;
  recipe.set_title("Recipe A");
  database.insert_recipe(recipe);
  vector<sqlite3_int64> ids;
  ids.push_back(1);
  database.add_recipes_to_category(ids, "A");
  Recipe result = database.fetch_recipe(1);
  ASSERT_EQ(1, result.categories().size());
  EXPECT_EQ("A", *result.categories().begin());
}

TEST(DatabaseTest, RemoveRecipeFromCategory) {
  Database database;
  database.open(":memory:");
  Recipe recipe;
  recipe.set_title("Recipe A");
  recipe.add_category("A");
  database.insert_recipe(recipe);
  vector<sqlite3_int64> ids;
  ids.push_back(1);
  database.remove_recipes_from_category(ids, "A");
  Recipe result = database.fetch_recipe(1);
  ASSERT_EQ(0, result.categories().size());
}

TEST(DatabaseTest, RenameCategory) {
  Database database;
  database.open(":memory:");
  Recipe recipe;
  recipe.set_title("Recipe A");
  recipe.add_category("A");
  database.insert_recipe(recipe);
  database.rename_category("A", "B");
  Recipe result = database.fetch_recipe(1);
  ASSERT_EQ(1, result.categories().size());
  EXPECT_EQ("B", *result.categories().begin());
}

TEST(DatabaseTest, GetCategoryID) {
  Database database;
  database.open(":memory:");
  Recipe recipe;
  recipe.set_title("Recipe A");
  recipe.add_category("A");
  database.insert_recipe(recipe);
  ASSERT_EQ(database.get_category_id("A"), 1);
  ASSERT_EQ(database.get_category_id("C"), 0);
}

TEST(DatabaseTest, MergeIntoCategory) {
  Database database;
  database.open(":memory:");
  Recipe recipe1;
  recipe1.add_category("A");
  database.insert_recipe(recipe1);
  Recipe recipe2;
  recipe2.add_category("B");
  database.insert_recipe(recipe2);
  Recipe recipe3;
  recipe3.add_category("C");
  database.insert_recipe(recipe3);
  database.merge_category("B", "A");
  Recipe result = database.fetch_recipe(1);
  ASSERT_EQ(1, result.categories().size());
  EXPECT_EQ("A", *result.categories().begin());
  result = database.fetch_recipe(2);
  ASSERT_EQ(1, result.categories().size());
  EXPECT_EQ("A", *result.categories().begin());
  result = database.fetch_recipe(3);
  ASSERT_EQ(1, result.categories().size());
  EXPECT_EQ("C", *result.categories().begin());
}

TEST(DatabaseTest, MergeWithRecipesInBothCategories) {
  Database database;
  database.open(":memory:");
  Recipe recipe;
  recipe.add_category("A");
  recipe.add_category("B");
  database.insert_recipe(recipe);
  database.merge_category("B", "A");
  Recipe result = database.fetch_recipe(1);
  ASSERT_EQ(1, result.categories().size());
  EXPECT_EQ("A", *result.categories().begin());
}

TEST(DatabaseTest, DeleteCategory) {
  Database database;
  database.open(":memory:");
  Recipe recipe;
  recipe.add_category("A");
  recipe.add_category("B");
  database.insert_recipe(recipe);
  database.delete_category("A");
  Recipe result = database.fetch_recipe(1);
  ASSERT_EQ(1, result.categories().size());
  EXPECT_EQ("B", *result.categories().begin());
}

TEST(DatabaseTest, CountCategory) {
  Database database;
  database.open(":memory:");
  Recipe recipe1;
  recipe1.add_category("A");
  database.insert_recipe(recipe1);
  Recipe recipe2;
  recipe2.add_category("A");
  database.insert_recipe(recipe2);
  Recipe recipe3;
  recipe3.add_category("B");
  database.insert_recipe(recipe3);
  ASSERT_EQ(2, database.count_recipes("A"));
  ASSERT_EQ(1, database.count_recipes("B"));
}
