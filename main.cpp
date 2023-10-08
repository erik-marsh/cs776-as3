#include <array>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

#include "Rooms.hpp"
#include "encoding.hpp"

constexpr int NUM_GENERATIONS = 50;
constexpr int GENERATION_SIZE = 100;
constexpr double CROSSOVER_PROB = 0.7;
constexpr double MUTATION_PROB = 0.001;

struct Individual
{
    Chromosome chromosome;
    float fitness;
};

using Population = std::array<Individual, GENERATION_SIZE>;
using ProbDist = std::array<double, GENERATION_SIZE>;

void InitializeIndividual(std::mt19937& generator, Individual& x);
ProbDist MakeCumulativeProbDist(Population& pop);
std::array<Individual, 2> Select(std::mt19937& generator, ProbDist& cdf, Population& pop);
std::array<Individual, 2> Crossover(std::mt19937& generator, std::array<Individual, 2>& parents);
uint8_t MutateBit(std::mt19937& generator, uint8_t bit);

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
    auto seed = device();
    std::mt19937 generator{seed};
    // RandomContext randContext;
    // randContext.seed = device();
    // randContext.gen = std::mt19937{randContext.seed};

    // let's assume we're working in a two-dimesional input space
    // (since DeJong5 only works on such a domain)
    // so we needs to operate on two doubles

    std::cout << std::fixed << std::setprecision(6);

    std::array<Individual, GENERATION_SIZE> population;
    for (auto& individual : population)
    {
        InitializeIndividual(generator, individual);
        // double d0Decoded = Decode8(individual.GetFirstDouble());
        // double d1Decoded = Decode8(individual.GetSecondDouble());
        // std::vector<double> x = {d0Decoded, d1Decoded};
        // individual.fitness = MakeFitness(DeJong4(x));
    }

    for (int gen = 0; gen < NUM_GENERATIONS; gen++)
    {
        auto cdf = MakeCumulativeProbDist(population);
        Population newGeneration;

        for (int i = 0; i < GENERATION_SIZE; i += 2)
        {
            // std::cout << "\nSelecting...\n";
            auto parents = Select(generator, cdf, population);
            // for (auto& p : parents)
            // {
            //     std::cout << "     Selected ";
            //     for (auto val : p.chromosome)
            //         std::cout << (int)val;
            //     std::cout << " (Fitness=" << p.fitness << ")\n";
            // }

            // mutation occurs within the Crossover function
            // std::cout << "Reproducing...\n";
            auto children = Crossover(generator, parents);
            for (auto& c : children)
            {
                // double d0Decoded = Decode8(c.GetFirstDouble());
                // double d1Decoded = Decode8(c.GetSecondDouble());
                // std::vector<double> x = {d0Decoded, d1Decoded};
                // c.fitness = MakeFitness(DeJong4(x));

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

void InitializeIndividual(std::mt19937& generator, Individual& x)
{
    std::uniform_int_distribution<int> dist(0, 1);
    for (int i = 0; i < x.chromosome.size(); i++)
        x.chromosome[i] = dist(generator);
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

std::array<Individual, 2> Select(std::mt19937& generator, ProbDist& cdf, Population& pop)
{
    std::array<Individual, 2> parents;
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    for (int i = 0; i < 2; i++)
    {
        double prob = dist(generator);
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

std::array<Individual, 2> Crossover(std::mt19937& generator, std::array<Individual, 2>& parents)
{
    std::array<Individual, 2> children;
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    double prob = dist(generator);
    if (prob <= CROSSOVER_PROB)
    {
        std::uniform_int_distribution<int> distInt(0, CHROMOSOME_BITWIDTH - 1);
        int crossoverIndex = distInt(generator);
        // std::cout << "Crossover occured! (Index=" << crossoverIndex << ")\n";

        for (int i = 0; i < crossoverIndex; i++)
        {
            children[0].chromosome[i] = MutateBit(generator, parents[0].chromosome[i]);
            children[1].chromosome[i] = MutateBit(generator, parents[1].chromosome[i]);
        }
        for (int i = crossoverIndex; i < CHROMOSOME_BITWIDTH; i++)
        {
            children[0].chromosome[i] = MutateBit(generator, parents[1].chromosome[i]);
            children[1].chromosome[i] = MutateBit(generator, parents[0].chromosome[i]);
        }
    }
    else
    {
        for (int i = 0; i < CHROMOSOME_BITWIDTH; i++)
        {
            children[0].chromosome[i] = MutateBit(generator, parents[0].chromosome[i]);
            children[1].chromosome[i] = MutateBit(generator, parents[1].chromosome[i]);
        }
    }

    return children;
}

uint8_t MutateBit(std::mt19937& generator, uint8_t bit)
{
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double prob = dist(generator);
    if (prob <= MUTATION_PROB)
    {
        // std::cout << "Mutation occured!\n";
        return !bit;
    }
    return bit;
}
