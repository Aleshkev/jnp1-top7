#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>
#include <cstdio>
#include <cinttypes>
#include <iostream>
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

void check_stream(istringstream &stream) {
  if (stream.fail())
    throw invalid_line_of_input("invalid input (like integer overflow)");
}

// Pomiń jedno słowo.
void ignore_stream_until_whitespace(istringstream &stream) {
  string s;
  stream >> s;
}


// Funkcja ze struktury przechowującej liczbę głosów lub punktów
// utworu wybiera daną liczbę utworów o największej liczbie głosów.
vector<song_id_t> top_songs(  const score_counter_t &votes, 
                        const int64_t number_of_songs_in_ranking) {
  vector<song_id_t> ranking;
  ranking.resize(number_of_songs_in_ranking);
  vector<int64_t> ranking_votes;
  ranking_votes.resize(number_of_songs_in_ranking);
  for (auto const &[key, val] : votes) {
    if (val == 0) {
      continue;
    }
    song_id_t cur_key = key;
    int64_t cur_val = val;
    for (int64_t i = 0; i < number_of_songs_in_ranking; i++) {
      if (cur_val > ranking_votes[i] ||
          (cur_val == ranking_votes[i] && 
          (cur_key < ranking[i] || ranking[i] == 0))) {
        swap(cur_key, ranking[i]);
        swap(cur_val, ranking_votes[i]);
      }
    }
  }
  return ranking;
}

// Funkcja która wypisze notowanie tudzież podsumowanie.
void write_out_ranking( const vector<song_id_t> &ranking,
                        const vector<song_id_t> &previous_ranking,
                        const int64_t number_of_songs_in_ranking) {
  for (int64_t i = 0; i < number_of_songs_in_ranking; i++) {
    if (ranking[i] == 0) {
      return;
    }
    cout << ranking[i] << ' ';

    // Niemożliwe żeby piosenka zmieniła pozycję o tyle miejsc
    // ile jest w rankingu. Taka wartość zmiennej 
    // oznacza że jest nowa w rankingu.
    int64_t change_in_position = number_of_songs_in_ranking;
    for (int64_t j = 0; j < number_of_songs_in_ranking; j++) {
      if (previous_ranking[j] == ranking[i]) {
        change_in_position = j - i;
        break;
      }
    }
    if (change_in_position == number_of_songs_in_ranking) {
      cout << '-';
    }
    else {
      cout << change_in_position;
    }
    cout << '\n';
  }
}

// TODO: podzielić na mniejsze funkcje
int main() {
  const int64_t number_of_songs_in_ranking = 7;

  // Wyłącz wypisywanie rzeczy niepotrzebnych w zadaniu. (Ale przydatnych w
  // debugowaniu.)
  clog.setstate(ios_base::failbit);

  // Pozycje utworów w poprzednim notowaniu.
  // Jeśli na danym miejscu znajduje się zero, to oznacza to
  // że w poprzednim notowaniu nie było ono zajęte 
  // z powodu zbyt małej liczby piosenek.
  // W szczególności jeśli na wszystkich miejscach znajdują się zera to
  // żadne notowanie nie zostało jeszcze zamknięte.
  vector<song_id_t> listing;
  listing.resize(number_of_songs_in_ranking);

  // Pozycje utworów w poprzednim podsumowaniu.
  // Została przyjęta taka sama konwencja jak przy reprezentacji
  // pozycji utworów w poprzednim notowaniu.
  vector<song_id_t> summary;
  summary.resize(number_of_songs_in_ranking);

  // Obecna liczba punktów danego utworu w podsumowaniu.
  score_counter_t points;

  // Obecna liczba głosów na dany utwór.
  score_counter_t votes;

  score_counter_t all_time_score;

  // Zmienna która mówi czy po ostatnim wykonaniu polecenia NEW
  // zostało wykonane polecenie TOP.
  bool calculate_top_ranking = false;

  unordered_set<song_id_t> blacklisted_songs;

  song_id_t previous_max_song_id = 0;
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

      // Oddawanie głosu.
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

        continue;
      }

      // Polecenie NEW
      if (regex_match(line, args, new_command)) {
        calculate_top_ranking = false;
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

        // Obsługa zamykanego notowania.
        // Wypisywanie zamykanego notowania.
        // Jeśli wcześniej odbyło się notowanie to:
        if (max_song_id > 0) {
          // Znaleźć utwory, które są w top 7 obecnego notowania
          vector<song_id_t> new_listing = 
          top_songs(votes, number_of_songs_in_ranking);

          // Porównać z utworami które były w top 7 poprzedniego notowania.
          // Wypisać top 7 z zamykanego notowania w raz ze zmianą pozycji.
          write_out_ranking(new_listing, listing, number_of_songs_in_ranking);

          // TODO
          // Dodać do blacklisted_songs utwory, które nie są na pierwszych 7
          // pozycjach
          for (int64_t i = 0; i < number_of_songs_in_ranking; i++) {
            if (listing[i] != 0 && count(new_listing.begin(), new_listing.end(), listing[i]) == 0 ) {
              blacklisted_songs.insert(listing[i]);
            }
          }
          listing = new_listing;
          // Przydzielić punkty za pozycje w rankingu
          for (int64_t i = 0; i < number_of_songs_in_ranking; i++) {
            if (listing[i] != 0) {
              all_time_score[listing[i]] += number_of_songs_in_ranking - i;
            }
          }
        }

        previous_max_song_id = max_song_id;
        max_song_id = new_max_song_id;
        votes.clear();

        continue;
      }
      // Polecenie TOP
      if (regex_match(line, args, top_command)) {
        // Ponieważ punkty dodawane są tylko podczas wywoływania NEW,
        // to można dokonać optymalizacji i wykonywać obliczenia tylko raz 
        // pomiędzy wywołaniami NEW.
        if (!calculate_top_ranking) {
          calculate_top_ranking = true;
          vector<song_id_t> new_summary = 
            top_songs(all_time_score, number_of_songs_in_ranking);

          write_out_ranking(new_summary, summary, number_of_songs_in_ranking);
          summary = new_summary;
        }
        else 
        write_out_ranking(summary, summary, number_of_songs_in_ranking);
        
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