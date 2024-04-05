#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <numeric>
#include <map>

using namespace std;

struct Politician {
    string office;
    int age;
    int yearsInOffice;
    int lifeExpectancy;
    int timesReelected = 0; // Yeniden seçilme sayısını takip etmek için yeni alan
};

struct Office {
    string name;
    int minAge;
    int minServiceYears;
    int positions;
};

const int simulationYears = 200;
const int startingPSI = 100;
const int unfilledPositionPenalty = -5;
const int reelectionPenalty = -10;
const double newCandidateMean = 15;
const double newCandidateSD = 5;

vector<Office> offices = {
    {"Quaestor", 30, 0, 20},
    {"Aedile", 36, 2, 10},
    {"Praetor", 39, 2, 8},
    {"Consul", 42, 2, 2}
};

default_random_engine generator(random_device{}());

int generateLifeExpectancy(double mean = 55, double sd = 10, double minAge = 25, double maxAge = 80) {
    normal_distribution<double> distribution(mean, sd);
    double lifeExp;
    do {
        lifeExp = distribution(generator);
    } while (lifeExp < minAge || lifeExp > maxAge);
    return static_cast<int>(lifeExp);
}

int generateAnnualInfluxOfCandidates() {
    normal_distribution<double> distribution(newCandidateMean, newCandidateSD);
    return max(1, static_cast<int>(distribution(generator)));
}

void initializePoliticians(vector<Politician>& politicians) {
    for (auto& office : offices) {
        for (int i = 0; i < office.positions; ++i) {
            Politician newPolitician = {office.name, office.minAge, 0, generateLifeExpectancy()};
            politicians.push_back(newPolitician);
        }
    }
}

void simulateYear(vector<Politician>& politicians, int& psi, int year) {
    // Age progression and mortality
    for (auto& politician : politicians) {
        politician.age++;
        politician.yearsInOffice++;
    }
    politicians.erase(remove_if(politicians.begin(), politicians.end(), [](const Politician& p) {
        return p.age > p.lifeExpectancy;
    }), politicians.end());

    // Handle new candidates
    if (year == 1) { // In the first year, add new candidates directly as Quaestors
        int newCandidates = generateAnnualInfluxOfCandidates();
        for (int i = 0; i < newCandidates; ++i) {
            politicians.push_back({"Quaestor", offices[0].minAge, 0, generateLifeExpectancy()});
        }
    }

    // Re-election process
    for (auto& politician : politicians) {
        if (politician.yearsInOffice > 1) { // Assumes a politician is considered for re-election after 1 year in office
            politician.timesReelected++;
            psi += reelectionPenalty;
        }
    }

    // PSI adjustments
    psi = startingPSI; // Reset PSI for the year
    // Adjust for unfilled positions
    map<string, int> officeCounts;
    for (auto& p : politicians) {
        officeCounts[p.office]++;
    }
    for (auto& office : offices) {
        int filledPositions = officeCounts[office.name];
        if (filledPositions < office.positions) {
            psi += (filledPositions - office.positions) * unfilledPositionPenalty;
        }
    }
}

int main() {
    vector<Politician> politicians;
    int psi = startingPSI;

    initializePoliticians(politicians);

    for (int year = 1; year <= simulationYears; ++year) {
        simulateYear(politicians, psi, year);
    }

    cout << "End-of-Simulation PSI: " << psi << endl;
    // Output for other measurements would go here

    return 0;
}

