// TODO: to trzeba zmienić na pojedyncze nagłówki
#include <bits/stdc++.h>

#include <charconv>

// TODO: to też raczej trzeba zmienić na wiele "using std::..."
using namespace std;

// Typy danych do zadania.
using song_id_t = int64_t;
using score_counter_t = unordered_map<song_id_t, int64_t>;
using songs_t = unordered_set<song_id_t>;

// Wyrażenia używane do parsowania wejścia.
const regex vote_command("\\s*([0-9]+)(\\s+[0-9]+)*\\s*");
const regex new_command("\\s*NEW\\s+([0-9]+)\\s*");
const regex top_command("\\s*TOP\\s*");

/// Błąd wyrzucany i łapany gdy linia wejścia jest niepoprawna.
struct invalid_line_of_input : public runtime_error {
  invalid_line_of_input(string explanation) : runtime_error(explanation) {}
};

/// Zamienia napis na dany typ. Wyrzuca @c invalid_line_of_input jeżeli się nie
/// da. Funkcja jest używana tylko dla liczb i kiedy w środku napisu nie ma
/// spacji.
template <typename T>
T from_string(const string &s) {
  istringstream stream(s);
  T x;
  stream >> noskipws >> x;
  if (stream.fail() || !stream.eof())
    throw invalid_line_of_input("invalid value (like integer overflow)");
  return x;
}

// Funkcje żeby się dało wypisywać niektóre kolekcje.
// TODO: pewnie usunąć to jak już będzie działało wszystko albo dać do osobnego
// pliku
template <typename T>
ostream &operator<<(ostream &o, const vector<T> &obj) {
  o << "[";
  bool first = true;
  for (auto key : obj) {
    if (!first) o << ", ";
    o << key;
    first = false;
  }
  return o << "]";
}
template <typename T>
ostream &operator<<(ostream &o, const unordered_set<T> &obj) {
  o << "{";
  bool first = true;
  for (auto key : obj) {
    if (!first) o << ", ";
    o << key;
    first = false;
  }
  return o << "}";
}
template <typename T, typename U>
ostream &operator<<(ostream &o, const unordered_map<T, U> &obj) {
  o << "{";
  bool first = true;
  for (auto const &[key, val] : obj) {
    if (!first) o << ", ";
    o << key << ": " << val;
    first = false;
  }
  return o << "}";
}

void check_stream(istringstream &stream) {
  if (stream.fail())
    throw invalid_line_of_input("invalid input (like integer overflow)");
  if (stream.eof()) throw runtime_error("unexpected data");
}

void ignore_stream_until_whitespace(istringstream &stream) {
  string s;
  stream >> s;
}

// TODO: podzielić na mniejsze funkcje
int main() {
  // Wyłącz wypisywanie rzeczy niepotrzebnych w zadaniu. (Ale przydatnych w
  // debugowaniu.) clog.setstate(ios_base::failbit);

  // Obecna liczba głosów na dany utwór.
  score_counter_t votes;

  score_counter_t all_time_score;

  // Utwory, które wypadły z notowań. Nie można na nie głosować.
  unordered_set<song_id_t> blacklisted_songs;

  song_id_t max_song_id = 0;
  const song_id_t max_max_song_id = 99999999;

  int64_t line_i = 0;
  string line;
  while (getline(cin, line)) {
    ++line_i;

    try {
      if (line.empty()) continue;

      istringstream line_stream(line);
      smatch args;

      if (regex_match(line, args, vote_command)) {
        if (max_song_id == 0)
          throw invalid_line_of_input("no ranking has begun yet");

        vector<song_id_t> songs;
        {
          string song_id_as_string;
          while (line_stream >> song_id_as_string) {
            songs.push_back(from_string<song_id_t>(song_id_as_string));
            check_stream(line_stream);
          }
        }
        sort(songs.begin(), songs.end());

        if (songs.front() < 1)
          throw invalid_line_of_input("a song id is not a positive integer");
        if (songs.back() > max_song_id)
          throw invalid_line_of_input(
              "a song id above the limit for the current ranking");

        if (adjacent_find(songs.begin(), songs.end()) != songs.end())
          throw invalid_line_of_input("the list of song ids has duplicates");

        for (auto song : songs)
          if (blacklisted_songs.count(song))
            throw invalid_line_of_input("a song has fallen out of a ranking");

        for (auto song : songs) ++votes[song];

        clog << "votes now: " << votes << endl;

        continue;
      }

      if (regex_match(line, args, new_command)) {
        song_id_t new_max_song_id;

        ignore_stream_until_whitespace(line_stream);
        line_stream >> new_max_song_id;
        check_stream(line_stream);

        if (new_max_song_id < max_song_id)
          throw invalid_line_of_input(
              "limit for the current ranking must not be less than limit for "
              "the previous ranking");
        if (new_max_song_id > max_max_song_id)
          throw invalid_line_of_input("limit too high");

        max_song_id = new_max_song_id;

        clog << "Songs in this ranking:" << endl;
        clog << "  " << votes << endl;

        // Przydzielić punkty za pozycje w rankingu
        // all_time_score[] = ...

        // Dodać do blacklisted_songs utwory, które nie są na pierwszych 7
        // pozycjach

        // ... ?

        // TODO

        votes.clear();

        continue;
      }
      if (regex_match(line, args, top_command)) {
        clog << "Songs rated by their positions in rankings:" << endl;
        clog << "  " << all_time_score << endl;

        // TODO

        continue;
      }
      throw invalid_line_of_input("unknown line format");

    } catch (invalid_line_of_input &error) {
      cerr << "Error in line " << line_i << ": " << line << endl;
      clog << "(" << error.what() << ")" << endl;
    }
  }
  return 0;
}

// TODO: przejrzeć ten styl kodu co tam jest na moodle'u bo ma takie
// nieoczywiste rzeczy niektóre xd
