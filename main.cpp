#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <cassert>
#include <iomanip>

// https://stackoverflow.com/questions/6942273/how-to-get-a-random-element-from-a-c-container
#include  <random>
#include  <iterator>

template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g) {
        std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
        std::advance(start, dis(g));
        return start;
}

template<typename Iter>
Iter select_randomly(Iter start, Iter end) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        return select_randomly(start, end, gen);
}


// chain = of possible combination of players as return by enumerate_all_combinations

void recursive_enumerate_all_combinations(std::vector<std::vector<int>>& all_combinations,
                                          const std::vector<int>& partial_chain,
                                          const std::vector<int>& players_left) {
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

std::vector<std::vector<int>> enumerate_all_combinations(int nb_players) {

        std::vector<std::vector<int>> all_combinations;

        for (int player=0; player<nb_players; player++) {
                std::vector<int> players_left(nb_players);
                std::iota(players_left.begin(), players_left.end(), 0);
                std::erase(players_left, player);
                recursive_enumerate_all_combinations(all_combinations, {player}, players_left);
        }

        return all_combinations;
}

double ecart_type(const std::vector<std::vector<int>>& matrix) {
        const int NB_PLAYERS = (int)matrix.size();

        double moy = 0;
        for (int i=0; i<NB_PLAYERS; i++) {
                for (int j=i+1; j<NB_PLAYERS; j++) {
                        moy += (double)matrix[i][j];
                }
        }
        double nb_upper_matrix_without_diag = (NB_PLAYERS * NB_PLAYERS) - NB_PLAYERS;
        nb_upper_matrix_without_diag /= 2;

        moy /= nb_upper_matrix_without_diag;

        double ec_t = 0;
        for (int i=0; i<NB_PLAYERS; i++) {
                for (int j=i+1; j<NB_PLAYERS; j++) {
                        ec_t += pow((double)matrix[i][j] - moy, 2) ;
                }
        }
        ec_t /= nb_upper_matrix_without_diag;
        ec_t = sqrt(ec_t);

        return ec_t;
}

void update_matrix_teammate(std::vector<std::vector<int>>& teammates, const std::vector<int>& chain) {
        for (int p=0; p+1<chain.size(); p+=2) {
                int i = std::min(chain[p], chain[p+1]);
                int j = std::max(chain[p], chain[p+1]);
                teammates[i][j] += 1;
        }
}

void update_matrix_opp(std::vector<std::vector<int>>& opp, const std::vector<int>& chain) {
        int p=0;
        for (; p+3<chain.size(); p+=4) {
                opp[std::min(chain[p], chain[p+2])][std::max(chain[p], chain[p+2])] += 1;
                opp[std::min(chain[p], chain[p+3])][std::max(chain[p], chain[p+3])] += 1;
                opp[std::min(chain[p+1], chain[p+2])][std::max(chain[p+1], chain[p+2])] += 1;
                opp[std::min(chain[p+1], chain[p+3])][std::max(chain[p+1], chain[p+3])] += 1;
        }
        for (; p+1<chain.size(); p+=2) {
                opp[std::min(chain[p], chain[p+1])][std::max(chain[p], chain[p+1])] += 1;
        }
}

double cost_of_adding_chain(std::vector<std::vector<int>> teammates,
                            std::vector<std::vector<int>> opp,
                            const std::vector<int>& chain) {
        update_matrix_teammate(teammates, chain);
        update_matrix_opp(opp, chain);
        return ecart_type(teammates) + ecart_type(opp);
}

std::vector<int> find_best_combinations(const std::vector<std::vector<int>>& teammates,
                                           const std::vector<std::vector<int>>& opp,
                                           const std::vector<std::vector<int>>& combinations) {
        double best_cost = DBL_MAX;
        std::vector<std::vector<int>> best_chains;

        for (const auto& chain : combinations) {
                double chain_cost = cost_of_adding_chain(teammates, opp, chain);
                chain_cost = floor((chain_cost*100)+0.5)/100.0;
                if (chain_cost < best_cost) {
                        best_cost = chain_cost;
                        best_chains = {chain};
                } else if (chain_cost == best_cost) {
                        best_cost = chain_cost;
                        best_chains.push_back(chain);
                }
        }
         //std::cout << "choice between " << best_chains.size() << std::endl;

        return *select_randomly(best_chains.begin(), best_chains.end());
//        return best_chains[0];
}

void update_matches_with_chain(std::vector<std::vector<int>>& teammates,
                               std::vector<std::vector<int>>& opp,
                               const std::vector<int>& chain,
                               std::vector<std::vector<int>>& matches) {
        update_matrix_teammate(teammates, chain);
        update_matrix_opp(opp, chain);
        matches.push_back(chain);
}

void update_matches_with_best_chain(std::vector<std::vector<int>>& teammates,
                                    std::vector<std::vector<int>>& opp,
                                    const std::vector<std::vector<int>>& combinations,
                                    std::vector<std::vector<int>>& matches) {
        update_matches_with_chain(teammates,
                                  opp,
                                  find_best_combinations(teammates, opp, combinations),
                                  matches);

}

void print_matrix(const std::vector<std::vector<int>>& matrix) {
        const int NB_PLAYERS = (int)matrix.size();
        for (int i=0; i<NB_PLAYERS; i++) {
                for (int j=0; j<NB_PLAYERS; j++) {
                        std::cout << std::setw(3) << matrix[i][j];
                }
                std::cout << std::endl;
        }
}

int main() {
        const int NB_PLAYERS = 8;
        const int NB_MATCHES = 38*3;
        //const int NB_MATCHES = 38*3;

        const std::vector<std::vector<int>> combinations = enumerate_all_combinations(NB_PLAYERS);

        std::vector<std::vector<int>> teammates(NB_PLAYERS, std::vector<int>(NB_PLAYERS, 0));
        std::vector<std::vector<int>> opp(NB_PLAYERS, std::vector<int>(NB_PLAYERS, 0));

        std::vector<std::vector<int>> matches;
        for (int i=0; i<NB_MATCHES; i++) {
                update_matches_with_best_chain(teammates, opp, combinations, matches);
        }

        std::cout << "matches" << std::endl;

        int i = 0;
        for (const auto& match : matches) {
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


        return 0;
}