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

void RunGeneticAlgorithm();
void InitializeIndividual(std::mt19937& generator, Individual& x);
float GetFitness(RoomSet& rooms);
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

    // std::cout << std::fixed << std::setprecision(6);

    for (int i = 0; i < 30; i++)
    {
        RunGeneticAlgorithm();
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

void RunGeneticAlgorithm()
{
    std::random_device device{};
    auto seed = device();
    std::mt19937 generator{seed};

    std::cout << "Running GA with seed " << static_cast<unsigned int>(seed) << "...\n";

    Population population;
    for (int i = 0; i < population.size(); i++)
    {
        Individual& individual = population[i];
        InitializeIndividual(generator, individual);
        // std::cout << "\n\n==================== INDIVIDUAL " << i << "====================\n";
        // PrintChromosome(individual.chromosome);
        auto roomset = DecodeChromosome(individual.chromosome);
        // PrintRoomSet(roomset);

        individual.fitness = GetFitness(roomset);
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
                auto roomSet = DecodeChromosome(c.chromosome);
                c.fitness = GetFitness(roomSet);

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
}

void InitializeIndividual(std::mt19937& generator, Individual& x)
{
    // we just initialize the chromosome with random bits
    // EXCEPT for the length and width of the bath and the length of the hall
    // these are constant as per the problem specification
    constexpr int bathLenOffset = 80;
    constexpr int bathWidthOffset = 90;
    constexpr int hallLenOffset = 120;
    static Gene bathLen = EncodeFloat(BATH_LENGTH);   // chromosome offset: 80 + 0
    static Gene bathWidth = EncodeFloat(BATH_WIDTH);  // chromosome offset: 80 + 10
    static Gene hallLen = EncodeFloat(HALL_LENGTH);

    static std::uniform_int_distribution dist(0, 1);

    for (int i = 0; i < bathLenOffset; i++)
        x.chromosome[i] = dist(generator);
    for (int i = 0; i < FLOAT_BITWIDTH; i++)
        x.chromosome[bathLenOffset + i] = bathLen[i];
    for (int i = 0; i < FLOAT_BITWIDTH; i++)
        x.chromosome[bathWidthOffset + i] = bathWidth[i];
    for (int i = bathWidthOffset + FLOAT_BITWIDTH; i < hallLenOffset; i++)
        x.chromosome[i] = dist(generator);
    for (int i = 0; i < FLOAT_BITWIDTH; i++)
        x.chromosome[hallLenOffset + i] = hallLen[i];
    for (int i = hallLenOffset + FLOAT_BITWIDTH; i < CHROMOSOME_BITWIDTH; i++)
        x.chromosome[i] = dist(generator);
}

float GetFitness(RoomSet& rooms)
{
    // if at least one constraint is not met,
    // the entire chromosome is invalid and should have close to zero fitness
    // (we can't use pure zero because of the way the CDF is calculated)
    constexpr float invalidFitness = 0.0f;

    bool doesFit = true;
    for (Room& room : rooms)
    {
        if (!DoesRoomFitConstraints(room))
        {
            // std::cout << "Objective: Invalid\n";
            // std::cout << "Fitness..: " << invalidFitness << "\n";
            return invalidFitness;
        }
    }

    float objective = doesFit ? ObjectiveFunction(rooms) : 0.0f;
    // std::cout << "Objective: " << objective << "\n";
    float fitness = ObjectiveToFitness(objective);
    // std::cout << "Fitness..: " << fitness << "\n";
    return fitness;
}

ProbDist MakeCumulativeProbDist(Population& pop)
{
    double totalFitness = 0.0;
    for (Individual& indiv : pop)
        totalFitness += indiv.fitness;
    // std::cout << "Total fitness: " << totalFitness << "\n";
    ProbDist cumulativeProbs;  // need

    // if the total fitness is 0, the entire probability distribution will be 0s
    // to prevent this, we instead return a uniform CDF.
    if (totalFitness == 0.0f)
    {
        float accumulator = 0.1f;
        for (int i = 0; i < cumulativeProbs.size(); i++)
        {
            cumulativeProbs[i] = accumulator;
            accumulator += 0.1f;
        }
        return cumulativeProbs;
    }

    ProbDist fitnessProportions;  // for bookkeeping only
    ProbDist expectedValues;      // for bookkeeping only
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
