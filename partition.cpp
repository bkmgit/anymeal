#include <sstream>
#include "partition.hpp"


using namespace std;

vector<string> recipes(istream &stream) {
  vector<string> result;
  ostringstream recipe;
  string line;
  bool on = false;
  while (getline(stream, line)) {
    if (!line.empty() && *line.rbegin() == '\r')
      line.erase(line.length() - 1, 1);
    if (line.rfind("MMMMM", 0) == 0 && line.length() > 5)
      on = true;
    if (on)
      recipe << line << "\r\n";
    if (line == "MMMMM")
      on = false;
  };
  if (!recipe.str().empty())
    result.push_back(recipe.str());
  return result;
}
