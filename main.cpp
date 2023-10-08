// encodings for the dejong functions
// dj1 => x \in [-5.12, 5.12]       => 2^10 + 1 values
// dj2 => x \in [-2.048, 2.048]     => 2^12 + 1 values
// dj3 => x \in [-5.12, 5.12]       => 2^10 + 1 values
// dj4 => x \in [-1.28, 1.28]       => 2^8  + 1 values
// dj5 => x \in [-65.536, 65.536]   => 2^17 + 1 values

// so the minimum width for all of these is a 32 bit integer
// pretty whatever ig

// dejongs work
// encodings/decodings work
// now i need to do a GA...

#include <array>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

#include "RandomContext.hpp"
#include "dejong.hpp"
#include "encoding.hpp"

constexpr int NUM_GENERATIONS = 50;
constexpr int GENERATION_SIZE = 100;
constexpr double CROSSOVER_PROB = 0.7;
constexpr double MUTATION_PROB = 0.001;
constexpr int BITSTRING_SIZE = 8;
constexpr int DIMENSIONALITY = 2;
constexpr int CHROMOSOME_SIZE = BITSTRING_SIZE * DIMENSIONALITY;

struct Individual
{
    std::array<uint8_t, CHROMOSOME_SIZE> chromosome;
    double fitness;

    std::vector<uint8_t> GetFirstDouble() const
    {
        std::vector<uint8_t> ret;
        for (int i = 0; i < BITSTRING_SIZE; i++)
        {
            ret.push_back(chromosome[i]);
        }
        return ret;
    }

    std::vector<uint8_t> GetSecondDouble() const
    {
        std::vector<uint8_t> ret;
        for (int i = BITSTRING_SIZE; i < CHROMOSOME_SIZE; i++)
        {
            ret.push_back(chromosome[i]);
        }
        return ret;
    }
};

using Population = std::array<Individual, GENERATION_SIZE>;
using ProbDist = std::array<double, GENERATION_SIZE>;

void InitializeIndividual(RandomContext& rand, Individual& x);
ProbDist MakeCumulativeProbDist(Population& pop);
std::array<Individual, 2> Select(RandomContext& rand, ProbDist& cdf, Population& pop);
std::array<Individual, 2> Crossover(RandomContext& rand, std::array<Individual, 2>& parents);
uint8_t MutateBit(RandomContext& rand, uint8_t bit);

int main()
{
    // struct Individual:
    //     std::vector<uint8_t> chromosome;
    //     // decoded chromosome value (not necessary), aka std::vector<double>
    //     double fitness;
    //     indices(?) of parents (not necessary?)

    // struct Population:
    //     // arrays of populations, pointer swap trick
    //     // metrics like chromosome length and generation number
    //     // fitness metrics
    //     // probabilities for stuff
    //     // indices of the best and worst fitnesses
    //     // metrics of the generation max and population size

    // generate population_0
    // evaluate population_0
    // for (t = 0; t < maxGens; t++):
    //     Select population_t+1 from population_t
    //     NOTE: this is done for every single pair of individuals that we selected
    //         Recombine population_t+1
    //         Evaluate population_t+1
    //     do some bookkeeping

    std::random_device device{};
    RandomContext randContext;
    randContext.seed = device();
    randContext.gen = std::mt19937{randContext.seed};
    randContext.distInts = std::uniform_int_distribution<int>(0, 1);
    randContext.distGauss = std::normal_distribution<double>(0.0, 1.0);

    // let's assume we're working in a two-dimesional input space
    // (since DeJong5 only works on such a domain)
    // so we needs to operate on two doubles

    std::cout << std::fixed << std::setprecision(6);

    std::array<Individual, GENERATION_SIZE> population;
    for (auto& individual : population)
    {
        InitializeIndividual(randContext, individual);
        double d0Decoded = Decode8(individual.GetFirstDouble());
        double d1Decoded = Decode8(individual.GetSecondDouble());
        std::vector<double> x = {d0Decoded, d1Decoded};
        individual.fitness = MakeFitness(DeJong4(x));
    }

    for (int gen = 0; gen < NUM_GENERATIONS; gen++)
    {
        auto cdf = MakeCumulativeProbDist(population);
        Population newGeneration;

        for (int i = 0; i < GENERATION_SIZE; i += 2)
        {
            // std::cout << "\nSelecting...\n";
            auto parents = Select(randContext, cdf, population);
            // for (auto& p : parents)
            // {
            //     std::cout << "     Selected ";
            //     for (auto val : p.chromosome)
            //         std::cout << (int)val;
            //     std::cout << " (Fitness=" << p.fitness << ")\n";
            // }

            // mutation occurs within the Crossover function
            // std::cout << "Reproducing...\n";
            auto children = Crossover(randContext, parents);
            for (auto& c : children)
            {
                double d0Decoded = Decode8(c.GetFirstDouble());
                double d1Decoded = Decode8(c.GetSecondDouble());
                std::vector<double> x = {d0Decoded, d1Decoded};
                c.fitness = MakeFitness(DeJong4(x));

                // std::cout << "    Got child ";
                // for (auto val : c.chromosome)
                //     std::cout << (int)val;
                // std::cout << " (Fitness=" << c.fitness << ")\n";
            }

            newGeneration[i] = children[0];
            newGeneration[i + 1] = children[1];
        }

        double minFitness = std::numeric_limits<double>::max();
        double maxFitness = std::numeric_limits<double>::lowest();
        double sumFitness = 0.0;
        for (auto& indiv : newGeneration)
        {
            // for (auto val : indiv.chromosome)
            //     std::cout << (int)val;
            // std::cout << " (Fitness=" << indiv.fitness << ")\n";

            if (indiv.fitness < minFitness) minFitness = indiv.fitness;
            if (indiv.fitness > maxFitness) maxFitness = indiv.fitness;
            sumFitness += indiv.fitness;
        }

        std::cout << "Min fitness: " << minFitness << "\t";
        std::cout << "Max fitness: " << maxFitness << "\t";
        std::cout << "Avg fitness: " << sumFitness / newGeneration.size() << "\n";

        population = newGeneration;
    }

    // auto newPopulation = Select_old(randContext, population);
    // for (Individual& indiv : newPopulation)
    // {
    //     for (auto val : indiv.GetFirstDouble())
    //         std::cout << (int)val;
    //     std::cout << " | ";
    //     for (auto val : indiv.GetSecondDouble())
    //         std::cout << (int)val;
    //     std::cout << " | Fitness: " << indiv.fitness << "\n";
    // }

    // std::cout << "\n";
    // for (auto val : newPopulation[17].GetFirstDouble())
    //     std::cout << (int)val;
    // for (auto val : newPopulation[17].GetSecondDouble())
    //     std::cout << (int)val;
    // std::cout << "\n";
    // for (auto val : newPopulation[17].chromosome)
    //     std::cout << (int)val;
    // std::cout << "\n";

    return 0;
}

void InitializeIndividual(RandomContext& rand, Individual& x)
{
    std::uniform_int_distribution<int> dist(0, 1);
    for (int i = 0; i < x.chromosome.size(); i++)
        x.chromosome[i] = dist(rand.gen);
}

ProbDist MakeCumulativeProbDist(Population& pop)
{
    double totalFitness = 0.0;
    for (Individual& indiv : pop)
        totalFitness += indiv.fitness;
    // std::cout << "Total fitness: " << totalFitness << "\n";

    ProbDist fitnessProportions;  // for bookkeeping only
    ProbDist expectedValues;      // for bookkeeping only
    ProbDist cumulativeProbs;     // need
    double accumulator = 0.0;

    for (int i = 0; i < GENERATION_SIZE; i++)
    {
        double fitnessProportion = pop[i].fitness / totalFitness;
        fitnessProportions[i] = fitnessProportion;

        accumulator += fitnessProportion;
        cumulativeProbs[i] = accumulator;

        double expectedValue = fitnessProportion * GENERATION_SIZE;
        expectedValues[i] = expectedValue;
    }

    // for (int i = 0; i < GENERATION_SIZE; i++)
    // {
    //     for (auto val : pop[i].GetFirstDouble())
    //         std::cout << (int)val;
    //     std::cout << " | ";
    //     for (auto val : pop[i].GetSecondDouble())
    //         std::cout << (int)val;
    //     std::cout << " | Fitness: " << pop[i].fitness << " => Proportion: ";
    //     std::cout << fitnessProportions[i] << " => Expected parents: ";
    //     std::cout << expectedValues[i] << " => Rounded: ";
    //     int exptectedInt = static_cast<int>(std::round(expectedValues[i]));
    //     std::cout << exptectedInt << " |\tCumulative prob: ";
    //     std::cout << cumulativeProbs[i] << "\n";
    // }

    return cumulativeProbs;
}

std::array<Individual, 2> Select(RandomContext& rand, ProbDist& cdf, Population& pop)
{
    std::array<Individual, 2> parents;
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    for (int i = 0; i < 2; i++)
    {
        double prob = dist(rand.gen);
        for (int j = 0; j < GENERATION_SIZE; j++)
        {
            if (prob <= cdf[j])
            {
                parents[i] = pop[j];
                break;
            }
        }
    }

    return parents;
}

std::array<Individual, 2> Crossover(RandomContext& rand, std::array<Individual, 2>& parents)
{
    std::array<Individual, 2> children;
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    double prob = dist(rand.gen);
    if (prob <= CROSSOVER_PROB)
    {
        std::uniform_int_distribution<int> distInt(0, CHROMOSOME_SIZE - 1);
        int crossoverIndex = distInt(rand.gen);
        // std::cout << "Crossover occured! (Index=" << crossoverIndex << ")\n";

        for (int i = 0; i < crossoverIndex; i++)
        {
            children[0].chromosome[i] = MutateBit(rand, parents[0].chromosome[i]);
            children[1].chromosome[i] = MutateBit(rand, parents[1].chromosome[i]);
        }
        for (int i = crossoverIndex; i < CHROMOSOME_SIZE; i++)
        {
            children[0].chromosome[i] = MutateBit(rand, parents[1].chromosome[i]);
            children[1].chromosome[i] = MutateBit(rand, parents[0].chromosome[i]);
        }
    }
    else
    {
        for (int i = 0; i < CHROMOSOME_SIZE; i++)
        {
            children[0].chromosome[i] = MutateBit(rand, parents[0].chromosome[i]);
            children[1].chromosome[i] = MutateBit(rand, parents[1].chromosome[i]);
        }
    }

    return children;
}

uint8_t MutateBit(RandomContext& rand, uint8_t bit)
{
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double prob = dist(rand.gen);
    if (prob <= MUTATION_PROB)
    {
        // std::cout << "Mutation occured!\n";
        return !bit;
    }
    return bit;
}
