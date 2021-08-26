#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <cassert>
#include <iomanip>
#include  <random>
#include  <iterator>


namespace random {
        // https://stackoverflow.com/questions/6942273/how-to-get-a-random-element-from-a-c-container

        template<typename Iter, typename RandomGenerator>
        Iter select_randomly(Iter start, Iter end, RandomGenerator &g) {
                std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
                std::advance(start, dis(g));
                return start;
        }

        /// Retourne un itérateur sur un élément random entre start et end.
        /// \tparam Iter
        /// \param start
        /// \param end
        /// \return
        template<typename Iter>
        Iter select_randomly(Iter start, Iter end) {
                static std::random_device rd;
                static std::mt19937 gen(rd());
                return select_randomly(start, end, gen);
        }

}

namespace combin {
        /// Fonction récursive construire les combinaisons
        /// \param all_combinations Vecteur de toutes les combinaisons (en construction)
        /// \param partial_chain Chaîne partiel en cours de construction avant insertion
        /// \param players_left Joueurs qui restent à insérer dans la chaîne partielle
        void recursive_enumerate_all_combinations(std::vector<std::vector<int>> &all_combinations,
                                                  const std::vector<int> &partial_chain,
                                                  const std::vector<int> &players_left) {
                if (players_left.empty()) {
                        all_combinations.push_back(partial_chain);
                        return;
                }

                for (auto player : players_left) {
                        std::vector<int> _players_left = players_left;
                        std::erase(_players_left, player);

                        std::vector<int> _partial_chain = partial_chain;
                        _partial_chain.push_back(player);

                        recursive_enumerate_all_combinations(all_combinations, _partial_chain, _players_left);
                }
        }

        /// Permet d'énumérer toutes les combinaisons de matchs étant donné nb_players.
        /// Cette fonction ne brise pas les symmétries
        /// \param nb_players
        /// \return Avec 4 players, on retourne: 1234,1243,1324,1342,1423,1432...
        std::vector<std::vector<int>> enumerate_all_combinations(const int nb_players) {

                std::vector<std::vector<int>> all_combinations;

                for (int player = 0; player < nb_players; player++) {
                        std::vector<int> players_left(nb_players);
                        std::iota(players_left.begin(), players_left.end(), 0);
                        std::erase(players_left, player);
                        recursive_enumerate_all_combinations(all_combinations, {player}, players_left);
                }

                return all_combinations;
        }

        /// Retourne toutes les combinaison avec PLAYER qui bench.
        /// Cette fonction assume qu'on a 4n+1 joueurs et que c'est le dernier qui bench.
        /// \param all_combinations
        /// \param player
        /// \return Avec 5 player et player=3, retourne des combinaisons sous la forme A,B,C,D,3
        std::vector<std::vector<int>>
        combinations_with_benching_player(const std::vector<std::vector<int>> &all_combinations,
                                          const int player) {
                auto _all_combinations = all_combinations;
                _all_combinations.erase(std::remove_if(_all_combinations.begin(), _all_combinations.end(),
                                                       [&](const auto &combination) {
                                                               return combination.back() != player;
                                                       }),
                                        _all_combinations.end());
                return _all_combinations;
        }
}

/// Calcul l'écart type d'une matrice carré.
/// Prend seulement en compte les éléments du upper triangle.
/// \param matrix
/// \return
double ecart_type(const std::vector<std::vector<int>> &matrix) {
        const int NB_PLAYERS = (int) matrix.size();

        double moy = 0;
        for (int i = 0; i < NB_PLAYERS; i++) {
                for (int j = i + 1; j < NB_PLAYERS; j++) {
                        moy += (double) matrix[i][j];
                }
        }
        double nb_upper_matrix_without_diag = (NB_PLAYERS * NB_PLAYERS) - NB_PLAYERS;
        nb_upper_matrix_without_diag /= 2;

        moy /= nb_upper_matrix_without_diag;

        double ec_t = 0;
        for (int i = 0; i < NB_PLAYERS; i++) {
                for (int j = i + 1; j < NB_PLAYERS; j++) {
                        ec_t += pow((double) matrix[i][j] - moy, 2);
                }
        }
        ec_t /= nb_upper_matrix_without_diag;
        ec_t = sqrt(ec_t);

        return ec_t;
}


void update_matrix_teammate(std::vector<std::vector<int>> &teammates,
                            const std::vector<int> &chain) {
        for (int p = 0; p + 1 < chain.size(); p += 2) {
                int i = std::min(chain[p], chain[p + 1]);
                int j = std::max(chain[p], chain[p + 1]);
                teammates[i][j] += 1;
        }
}

void update_matrix_opp(std::vector<std::vector<int>> &opp,
                       const std::vector<int> &chain) {
        int p = 0;
        for (; p + 3 < chain.size(); p += 4) {
                opp[std::min(chain[p], chain[p + 2])][std::max(chain[p], chain[p + 2])] += 1;
                opp[std::min(chain[p], chain[p + 3])][std::max(chain[p], chain[p + 3])] += 1;
                opp[std::min(chain[p + 1], chain[p + 2])][std::max(chain[p + 1], chain[p + 2])] += 1;
                opp[std::min(chain[p + 1], chain[p + 3])][std::max(chain[p + 1], chain[p + 3])] += 1;
        }
        for (; p + 1 < chain.size(); p += 2) {
                opp[std::min(chain[p], chain[p + 1])][std::max(chain[p], chain[p + 1])] += 1;
        }
}

double cost_of_adding_chain(std::vector<std::vector<int>> teammates,
                            std::vector<std::vector<int>> opp,
                            const std::vector<int> &chain) {
        update_matrix_teammate(teammates, chain);
        update_matrix_opp(opp, chain);
        return ecart_type(teammates) + ecart_type(opp);
}

std::vector<int> find_best_combinations(const std::vector<std::vector<int>> &teammates,
                                        const std::vector<std::vector<int>> &opp,
                                        const std::vector<std::vector<int>> &combinations) {
        double best_cost = DBL_MAX;
        std::vector<std::vector<int>> best_chains;

        for (const auto &chain : combinations) {
                double chain_cost = cost_of_adding_chain(teammates, opp, chain);
                chain_cost = floor((chain_cost * 100) + 0.5) / 100.0;
                if (chain_cost < best_cost) {
                        best_cost = chain_cost;
                        best_chains = {chain};
                } else if (chain_cost == best_cost) {
                        best_cost = chain_cost;
                        best_chains.push_back(chain);
                }
        }
        //std::cout << "choice between " << best_chains.size() << std::endl;

        return *random::select_randomly(best_chains.begin(), best_chains.end());
//        return best_chains[0];
}

void update_matches_with_chain(std::vector<std::vector<int>> &teammates,
                               std::vector<std::vector<int>> &opp,
                               const std::vector<int> &chain,
                               std::vector<std::vector<int>> &matches) {
        update_matrix_teammate(teammates, chain);
        update_matrix_opp(opp, chain);
        matches.push_back(chain);
}

void update_matches_with_best_chain(std::vector<std::vector<int>> &teammates,
                                    std::vector<std::vector<int>> &opp,
                                    const std::vector<std::vector<int>> &combinations,
                                    std::vector<std::vector<int>> &matches) {
        update_matches_with_chain(teammates,
                                  opp,
                                  find_best_combinations(teammates, opp, combinations),
                                  matches);

}

void print_matrix(const std::vector<std::vector<int>> &matrix) {
        const int NB_PLAYERS = (int) matrix.size();
        for (int i = 0; i < NB_PLAYERS; i++) {
                for (int j = 0; j < NB_PLAYERS; j++) {
                        std::cout << std::setw(3) << matrix[i][j];
                }
                std::cout << std::endl;
        }
}

void solution_8_38_3()  {
        const int NB_PLAYERS = 8;
        const int NB_MATCHES = 38 * 3;
        //const int NB_MATCHES = 38*3;

        const std::vector<std::vector<int>> combinations = combin::enumerate_all_combinations(NB_PLAYERS);

        std::vector<std::vector<int>> teammates(NB_PLAYERS, std::vector<int>(NB_PLAYERS, 0));
        std::vector<std::vector<int>> opp(NB_PLAYERS, std::vector<int>(NB_PLAYERS, 0));

        std::vector<std::vector<int>> matches;
        for (int i = 0; i < NB_MATCHES; i++) {
                update_matches_with_best_chain(teammates, opp, combinations, matches);
        }

        std::cout << "matches" << std::endl;

        int i = 0;
        for (const auto &match : matches) {
                std::cout << "match " << std::setw(3) << i++ << ": ";
                for (auto p : match) {
                        std::cout << p << " ";
                }
                std::cout << std::endl;
        }

        std::cout << std::endl;

        std::cout << "teammate: " << std::endl;
        print_matrix(teammates);
        std::cout << std::endl;
        std::cout << "opp: " << std::endl;
        print_matrix(opp);
}

void solution_9_38_3() {
        const int NB_PLAYERS = 9;
        const int NB_SESSION = 3;
        const int NB_MATCH_PER_SESSION = 3;

        const std::vector<std::vector<int>> combinations = combin::enumerate_all_combinations(NB_PLAYERS);

        std::vector<std::vector<int>> teammates(NB_PLAYERS, std::vector<int>(NB_PLAYERS, 0));
        std::vector<std::vector<int>> opp(NB_PLAYERS, std::vector<int>(NB_PLAYERS, 0));

        std::vector<std::vector<int>> matches;
        for (int i = 0; i < NB_SESSION; i++) {
                const int missing_player = i % NB_PLAYERS;
                const auto combinations_with_benching_player = combin::combinations_with_benching_player(combinations, missing_player);
                for (int j = 0; j < NB_MATCH_PER_SESSION; j++) {
                        update_matches_with_best_chain(teammates, opp, combinations_with_benching_player, matches);
                }
        }

        std::cout << "matches" << std::endl;

        int i = 0;
        for (const auto &match : matches) {
                std::cout << "match " << std::setw(3) << i++ << ": ";
                for (auto p : match) {
                        std::cout << p << " ";
                }
                std::cout << std::endl;
        }

        std::cout << std::endl;

        std::cout << "teammate: " << std::endl;
        print_matrix(teammates);
        std::cout << std::endl;
        std::cout << "opp: " << std::endl;
        print_matrix(opp);
}

int main() {

        solution_9_38_3();

        return 0;
}